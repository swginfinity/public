# core3_blocked.py
# GDB Python script for detecting mutex/rwlock/condvar deadlocks and contention
# in SWGEmu Core3 / Engine3 servers.
#
# Understands Engine3's Lockable, Mutex, ReadWriteLock, and Condition classes,
# pthread_mutex_t, pthread_rwlock_t internals, and Core3 thread naming.
#
# Detects:
#   - Classic deadlock cycles (A holds X, waits Y; B holds Y, waits X)
#   - Phantom reader corruption (rlock-under-wlock corrupts glibc __readers)
#   - Active rlock-under-wlock (caught in the act via __cur_writer == blocked LWP)
#   - sendToOwner/sendTo under zone wlock (deserialization stall)
#   - Object::acquire spin (ManagedObject mutex contention)
#   - Contention hotspots sorted by waiter count
#
# Usage (after Ctrl+C on a hung server):
#   (gdb) source ~/core3_blocked.py
#   (gdb) blocked          -- full analysis
#   (gdb) blocked idle     -- also list idle condvar threads
#   (gdb) blocked locks    -- show all lock objects and their holders
#   (gdb) blocked graph    -- show raw wait-for graph edges
#   (gdb) blocked report   -- dump full analysis to /tmp/blocked_report.txt

import gdb
import re
import os
import sys
import time
from collections import defaultdict

# ──────────────────────────────────────────────────────────────────────────────
# Helpers
# ──────────────────────────────────────────────────────────────────────────────

def safe_eval(expr):
    """Evaluate a GDB expression, returning None on failure."""
    try:
        return gdb.parse_and_eval(expr)
    except Exception:
        return None

def safe_int(val):
    """Convert a GDB value to int, returning None on failure."""
    if val is None:
        return None
    try:
        return int(val)
    except Exception:
        return None

def safe_hex(val):
    """Convert a GDB value to hex string, returning None on failure."""
    if val is None:
        return None
    try:
        v = int(val)
        return hex(v) if v != 0 else None
    except Exception:
        return None

def safe_string(val):
    """Get string representation of a GDB value."""
    if val is None:
        return None
    try:
        return str(val)
    except Exception:
        return None

# ──────────────────────────────────────────────────────────────────────────────
# Thread name extraction
# ──────────────────────────────────────────────────────────────────────────────

def get_thread_name(thr):
    """Extract thread name from GDB thread info (e.g., 'TkWk20', 'TW-rori3')."""
    try:
        info = thr.name
        if info:
            return info
    except Exception:
        pass
    # Fallback: parse from thread info string
    try:
        num = thr.num
        result = gdb.execute(f"info thread {num}", to_string=True)
        m = re.search(r'"([^"]+)"', result)
        if m:
            return m.group(1)
    except Exception:
        pass
    return None

def classify_thread_name(name):
    """Classify a Core3 thread by its name prefix."""
    if not name:
        return "unknown"
    if name.startswith("TkWk"):
        return "TaskWorker"
    if name.startswith("TkScheduler"):
        return "TaskScheduler"
    if name.startswith("TW-"):
        return "ZoneWorker"
    if name == "core3":
        return "MainThread"
    if "IO" in name or "Net" in name:
        return "Network"
    if "DB" in name or "database" in name.lower():
        return "Database"
    if "Console" in name:
        return "Console"
    return "other"

# ──────────────────────────────────────────────────────────────────────────────
# Lock type detection keywords
# ──────────────────────────────────────────────────────────────────────────────

MUTEX_LOCK_FUNCS = {
    "pthread_mutex_lock", "__pthread_mutex_lock", "__GI___pthread_mutex_lock",
    "__lll_lock_wait", "__lll_lock_wait_private",
    "__pthread_mutex_cond_lock",
}

RWLOCK_WRLOCK_FUNCS = {
    "pthread_rwlock_wrlock", "__pthread_rwlock_wrlock",
    "___pthread_rwlock_wrlock", "__pthread_rwlock_wrlock_full64",
    "__GI___pthread_rwlock_wrlock",
}

RWLOCK_RDLOCK_FUNCS = {
    "pthread_rwlock_rdlock", "__pthread_rwlock_rdlock",
    "___pthread_rwlock_rdlock", "__pthread_rwlock_rdlock_full",
    "__GI___pthread_rwlock_rdlock",
}

RWLOCK_UNLOCK_FUNCS = {
    "pthread_rwlock_unlock", "__pthread_rwlock_unlock",
    "___pthread_rwlock_unlock", "__pthread_rwlock_wrunlock",
}

CONDVAR_FUNCS = {
    "pthread_cond_wait", "pthread_cond_timedwait",
    "__pthread_cond_wait", "___pthread_cond_wait",
    "__pthread_cond_timedwait", "___pthread_cond_timedwait",
}

FUTEX_FUNCS = {
    "__futex_abstimed_wait_common64", "__futex_abstimed_wait_common",
    "__GI___futex_abstimed_wait_cancelable64",
    "__GI___futex_abstimed_wait64",
    "futex_wait",
}

ENGINE3_LOCK_FUNCS = {
    "sys::thread::ReadWriteLock::wlock",
    "sys::thread::ReadWriteLock::rlock",
    "sys::thread::ReadWriteLock::lock",
    "sys::thread::Mutex::lock",
}

ENGINE3_CROSSLOCK_PATTERN = re.compile(
    r"sys::thread::(ReadWriteLock|Mutex)::(wlock|rlock|lock)\s*\("
)

# Patterns indicating a lock was ACQUIRED (when found above the blocking frame)
LOCK_ACQUIRED_PATTERNS = {
    "Locker::Locker",
    "sys::thread::Locker::Locker",
    "Locker",  # Sometimes abbreviated in backtraces
}

# Patterns for deserialization-related frames (deep DB load under wlock)
DESERIALIZATION_FUNCS = {
    "ObjectManager::deSerializeObject",
    "ObjectManager::loadPersistentObject",
    "ContainerObjectsMap::loadObjects",
    "DistributedObjectBroker::lookUp",
}

# ──────────────────────────────────────────────────────────────────────────────
# __readers bitfield decoder (glibc 2.35+ PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE)
# ──────────────────────────────────────────────────────────────────────────────

def decode_rwlock_readers(readers_val):
    """Decode glibc's packed __readers bitfield.

    glibc 2.35 (Ubuntu 22.04) layout for PREFER_WRITER_NONRECURSIVE:
      Bit 0 (0x01): WRLOCK  — a writer is present/pending
      Bit 1 (0x02): WRLOCKED — writer has acquired the lock
      Bit 2 (0x04): WRPHASE — in write phase (readers draining)
      Bits 3+: reader count (each reader adds 8 = 1<<3)

    Returns dict with decoded fields and corruption diagnosis.
    """
    if readers_val is None:
        return None

    result = {
        'raw': readers_val,
        'hex': f"0x{readers_val:08x}",
        'wrlock': bool(readers_val & 0x01),
        'wrlocked': bool(readers_val & 0x02),
        'wrphase': bool(readers_val & 0x04),
        'reader_count': readers_val >> 3,
        'phantom_readers': False,
        'corruption_details': None,
    }

    # Detect phantom reader corruption:
    # If WRLOCK+WRLOCKED are set (writer holds the lock) but reader_count > 0,
    # there are phantom readers that will prevent the lock from ever being
    # released cleanly. This is the signature of rlock-under-wlock corruption.
    if result['wrlock'] and result['wrlocked'] and result['reader_count'] > 0:
        result['phantom_readers'] = True
        result['corruption_details'] = (
            f"PHANTOM READER CORRUPTION: __readers={readers_val} (0x{readers_val:02x}) — "
            f"writer holds lock (WRLOCK+WRLOCKED) but reader count = {result['reader_count']}. "
            f"This lock is permanently stuck. Caused by rlock() called while wlock() held "
            f"(POSIX undefined behavior, corrupts glibc internal counter)."
        )

    # Another corruption case: no writer flags but readers stuck at non-zero
    # with no actual reader threads visible — harder to detect without thread info
    # so we just flag the state for manual review
    if not result['wrlock'] and not result['wrlocked'] and result['reader_count'] > 0:
        # This MIGHT be normal (active readers), caller should cross-reference
        # with actual reader threads
        pass

    # Writer pending but not holding, with phantom reader residue
    if result['wrlock'] and not result['wrlocked'] and result['reader_count'] > 0:
        # Writer is pending, readers still draining — could be normal contention
        # or could be stuck phantom readers preventing writer from acquiring
        pass

    return result


def format_rwlock_readers(decoded):
    """Format decoded __readers into a human-readable string."""
    if not decoded:
        return "unknown"

    flags = []
    if decoded['wrlock']:
        flags.append("WRLOCK")
    if decoded['wrlocked']:
        flags.append("WRLOCKED")
    if decoded['wrphase']:
        flags.append("WRPHASE")

    parts = [f"__readers={decoded['raw']} ({decoded['hex']})"]
    if flags:
        parts.append(f"flags=[{'+'.join(flags)}]")
    parts.append(f"reader_count={decoded['reader_count']}")

    if decoded['phantom_readers']:
        parts.append("*** PHANTOM READER CORRUPTION ***")

    return " ".join(parts)

# ──────────────────────────────────────────────────────────────────────────────
# Object identification — try to determine what an Engine3 lock protects
# ──────────────────────────────────────────────────────────────────────────────

def identify_engine3_object(lock_addr_hex):
    """Try to identify the Engine3 object at a lock address.

    Engine3 objects inherit from Lockable (which contains the lock).
    For Zone objects, we can read the zoneName.
    For SceneObject subclasses, we can try to get the class name and objectID.

    Returns a description string or None.
    """
    descriptions = []

    # Try dynamic_cast-style identification via vtable / RTTI
    try:
        # Try as Zone (most common high-contention lock)
        val = safe_eval(f"((server::zone::Zone*){lock_addr_hex})->zoneName")
        if val:
            zone_name = safe_string(val)
            if zone_name and zone_name != "0x0" and "cannot" not in zone_name.lower():
                # Clean up GDB's string representation
                zone_name = zone_name.strip('"').strip("'")
                if zone_name and len(zone_name) < 40:
                    descriptions.append(f"Zone({zone_name})")
    except Exception:
        pass

    # Try as SceneObject — read objectID
    try:
        val = safe_eval(f"((server::zone::objects::scene::SceneObject*){lock_addr_hex})->_objectID")
        oid = safe_int(val)
        if oid and oid > 0:
            descriptions.append(f"objectID={oid:#018x}")
    except Exception:
        pass

    # Try RTTI type name via GDB's whatis/ptype
    try:
        result = gdb.execute(f"p *(void**){lock_addr_hex}", to_string=True)
        # Look for vtable pointer that reveals type
        m = re.search(r'vtable for (\S+)', result)
        if m:
            type_name = m.group(1)
            # Shorten common prefixes
            type_name = type_name.replace("server::zone::objects::", "")
            type_name = type_name.replace("server::zone::", "")
            descriptions.append(f"type={type_name}")
    except Exception:
        pass

    # Try reading the vtable more directly
    if not descriptions:
        try:
            result = gdb.execute(f"info vtbl *(void**){lock_addr_hex}", to_string=True)
            m = re.search(r"'([^']+)'", result)
            if m:
                descriptions.append(f"vtable={m.group(1)}")
        except Exception:
            pass

    return " / ".join(descriptions) if descriptions else None


def identify_zone_from_thread_name(thread_name):
    """Extract zone name from a ZoneWorker thread name like 'TW-corellia3'."""
    if not thread_name or not thread_name.startswith("TW-"):
        return None
    m = re.match(r"TW-([a-z_]+)\d*", thread_name)
    if m:
        return m.group(1)
    return None

# ──────────────────────────────────────────────────────────────────────────────
# Stack frame analysis
# ──────────────────────────────────────────────────────────────────────────────

def analyze_thread(thr):
    """Analyze a single thread's stack for lock waits.

    Returns a dict with:
      - wait_type: 'mutex', 'rwlock_wr', 'rwlock_rd', 'condvar', 'futex', 'rwlock_unlock', None
      - lock_addr: hex address of the lock object (Engine3 Lockable* or pthread lock)
      - rwlock_addr: hex address of pthread_rwlock_t (if rwlock)
      - mutex_addr: hex address of pthread_mutex_t (if mutex)
      - owner_tid: LWP of the lock holder (if determinable)
      - cross_lock_addr: address of the held lock being cross-locked against
      - engine3_func: the Engine3 lock function name (ReadWriteLock::wlock etc.)
      - blocking_func: the lowest-level blocking function
      - user_frames: list of interesting user-space frame summaries
      - corrupt_stack: whether backtrace was truncated
      - is_idle: whether this looks like a normal idle condvar wait
      - stack_depth: number of frames walked
      - has_deserialization: whether the thread is in a deserialization chain
      - deser_depth: number of nested deserialization frames
      - is_stuck_unlock: whether thread is stuck in rwlock_unlock (unusual)
    """
    result = {
        'wait_type': None,
        'lock_addr': None,
        'rwlock_addr': None,
        'mutex_addr': None,
        'owner_tid': None,
        'cross_lock_addr': None,
        'engine3_func': None,
        'blocking_func': None,
        'user_frames': [],
        'corrupt_stack': False,
        'is_idle': False,
        'stack_depth': 0,
        'blocking_depth': None,
        'has_deserialization': False,
        'deser_depth': 0,
        'is_stuck_unlock': False,
    }

    try:
        thr.switch()
        frame = gdb.selected_frame()
        if not frame:
            return result
    except Exception:
        return result

    # Walk the stack
    saw_futex = False
    saw_condvar = False
    saw_mutex_lock = False
    saw_rwlock_wr = False
    saw_rwlock_rd = False
    saw_rwlock_unlock = False
    saw_engine3_lock = False
    engine3_this = None
    engine3_cross_lock = None
    user_frame_names = []
    deser_count = 0

    current = frame
    depth = 0
    max_depth = 50  # deeper walk to catch Engine3 frames past glibc and deserialization chains

    while current and depth < max_depth:
        depth += 1
        func = current.name() or "??"
        sal = current.find_sal()

        # Count deserialization frames
        for deser_func in DESERIALIZATION_FUNCS:
            if deser_func in func:
                deser_count += 1
                break

        # Identify blocking primitives
        if func in FUTEX_FUNCS or "futex" in func.lower():
            saw_futex = True
            if not result['blocking_func']:
                result['blocking_func'] = func

        if func in MUTEX_LOCK_FUNCS or "__lll_lock_wait" in func:
            saw_mutex_lock = True
            result['blocking_func'] = func
            if result['blocking_depth'] is None:
                result['blocking_depth'] = depth - 1
            # Try to get mutex address
            try:
                mutex_val = safe_eval("mutex")
                if mutex_val is None:
                    mutex_val = safe_eval("$rdi")
                addr = safe_hex(mutex_val)
                if addr:
                    result['mutex_addr'] = addr
            except Exception:
                pass

        if func in RWLOCK_WRLOCK_FUNCS or "rwlock_wrlock" in func:
            saw_rwlock_wr = True
            result['blocking_func'] = func
            if result['blocking_depth'] is None:
                result['blocking_depth'] = depth - 1
            try:
                rwlock_val = safe_eval("rwlock")
                if rwlock_val is None:
                    rwlock_val = safe_eval("$rdi")
                addr = safe_hex(rwlock_val)
                if addr:
                    result['rwlock_addr'] = addr
            except Exception:
                pass

        if func in RWLOCK_RDLOCK_FUNCS or "rwlock_rdlock" in func:
            saw_rwlock_rd = True
            result['blocking_func'] = func
            if result['blocking_depth'] is None:
                result['blocking_depth'] = depth - 1
            try:
                rwlock_val = safe_eval("rwlock")
                if rwlock_val is None:
                    rwlock_val = safe_eval("$rdi")
                addr = safe_hex(rwlock_val)
                if addr:
                    result['rwlock_addr'] = addr
            except Exception:
                pass

        if func in RWLOCK_UNLOCK_FUNCS or "rwlock_wrunlock" in func:
            saw_rwlock_unlock = True
            if result['blocking_depth'] is None:
                result['blocking_depth'] = depth - 1

        if func in CONDVAR_FUNCS or "cond_wait" in func or "cond_timedwait" in func:
            saw_condvar = True

        # Engine3 lock functions - extract this pointer and cross-lock param
        for e3func in ENGINE3_LOCK_FUNCS:
            if func and e3func in func:
                saw_engine3_lock = True
                result['engine3_func'] = func

                # Try to extract 'this' pointer - the Lockable/Lock object
                this_val = safe_eval("this")
                if this_val:
                    addr = safe_hex(this_val)
                    if addr:
                        engine3_this = addr

                # Try to extract cross-lock parameter
                lock_param = safe_eval("lock")
                if lock_param is None:
                    lock_param = safe_eval("lockable")
                if lock_param is None:
                    lock_param = safe_eval("m")
                if lock_param:
                    addr = safe_hex(lock_param)
                    if addr:
                        engine3_cross_lock = addr

                break

        # Collect user-space frame names (skip glibc/pthread internals)
        if func and not func.startswith("__") and not func.startswith("___"):
            if "::" in func or func.startswith("sys::") or func.startswith("engine::") or func.startswith("server::"):
                source = ""
                if sal and sal.symtab:
                    source = f" at {sal.symtab.filename}:{sal.line}"
                user_frame_names.append(f"#{depth-1} {func}{source}")

        try:
            current = current.older()
        except Exception:
            result['corrupt_stack'] = True
            break

    result['stack_depth'] = depth

    # Check for corrupt stack via GDB output
    try:
        bt_out = gdb.execute("bt 50", to_string=True)
        if "corrupt stack" in bt_out or "Cannot access memory" in bt_out:
            result['corrupt_stack'] = True
    except Exception:
        pass

    # Classify wait type
    if saw_rwlock_wr:
        result['wait_type'] = 'rwlock_wr'
    elif saw_rwlock_rd:
        result['wait_type'] = 'rwlock_rd'
    elif saw_mutex_lock:
        result['wait_type'] = 'mutex'
    elif saw_rwlock_unlock and saw_futex:
        result['wait_type'] = 'rwlock_unlock'
        result['is_stuck_unlock'] = True
    elif saw_condvar and saw_futex:
        result['wait_type'] = 'condvar'
    elif saw_futex:
        result['wait_type'] = 'futex'

    # Set lock address (prefer Engine3 this > rwlock addr > mutex addr)
    if engine3_this:
        result['lock_addr'] = engine3_this
    elif result['rwlock_addr']:
        result['lock_addr'] = result['rwlock_addr']
    elif result['mutex_addr']:
        result['lock_addr'] = result['mutex_addr']

    if engine3_cross_lock:
        result['cross_lock_addr'] = engine3_cross_lock

    result['user_frames'] = user_frame_names[:10]  # limit for readability

    # Track deserialization depth
    if deser_count > 0:
        result['has_deserialization'] = True
        result['deser_depth'] = deser_count

    # Detect idle condvar threads (waiting on task queue with no interesting frames)
    if result['wait_type'] == 'condvar' and not saw_engine3_lock:
        result['is_idle'] = True
    elif result['wait_type'] == 'futex' and not saw_engine3_lock and not saw_mutex_lock and not saw_rwlock_wr and not saw_rwlock_rd:
        # Pure futex with no lock context = likely condvar idle
        if len(user_frame_names) <= 1:
            result['is_idle'] = True

    return result

# ──────────────────────────────────────────────────────────────────────────────
# Stack-frame held-lock inference
# ──────────────────────────────────────────────────────────────────────────────

def infer_held_locks(thr, blocking_depth, waiting_lock_addr):
    """Walk frames ABOVE the blocking point to find locks this thread holds.

    When a thread is blocked on a lock, the frames higher up in its call stack
    show what it was doing before it blocked.  Locker::Locker constructor frames
    above the blocking frame indicate locks that were already acquired (RAII).

    Args:
        thr: GDB thread object
        blocking_depth: frame index where the thread is blocked (0-based)
        waiting_lock_addr: hex address of the lock this thread is waiting on

    Returns:
        list of (lock_addr_hex, frame_description) tuples
    """
    held = []
    if blocking_depth is None:
        return held

    try:
        thr.switch()
        frame = gdb.selected_frame()
        if not frame:
            return held
    except Exception:
        return held

    # Walk all frames, only inspect those above blocking_depth
    current = frame
    depth = 0
    max_depth = 50

    while current and depth < max_depth:
        if depth > blocking_depth:
            func = current.name() or "??"

            # Check for Locker constructor frames
            is_locker = False
            for pattern in LOCK_ACQUIRED_PATTERNS:
                if pattern in func and "Locker" in func:
                    is_locker = True
                    break

            if is_locker:
                # Try to extract the lock address from this Locker frame
                lock_addr = None
                try:
                    current.select()
                except Exception:
                    depth += 1
                    try:
                        current = current.older()
                    except Exception:
                        break
                    continue

                # Try various ways to read the lock parameter
                for expr in ["lock", "lockable", "obj", "this->_lockable"]:
                    val = safe_eval(expr)
                    addr = safe_hex(val)
                    if addr:
                        lock_addr = addr
                        break

                # Fallback: try second arg register (first is 'this' for Locker ctor)
                if not lock_addr:
                    val = safe_eval("$rsi")
                    addr = safe_hex(val)
                    if addr:
                        lock_addr = addr

                if lock_addr and lock_addr != waiting_lock_addr:
                    sal = current.find_sal()
                    source = ""
                    if sal and sal.symtab:
                        source = f" at {sal.symtab.filename}:{sal.line}"
                    held.append((lock_addr, f"#{depth} {func}{source}"))

            # Also check for completed Engine3 lock calls above the blocking point
            elif any(e3 in func for e3 in ENGINE3_LOCK_FUNCS):
                try:
                    current.select()
                    this_val = safe_eval("this")
                    addr = safe_hex(this_val)
                    if addr and addr != waiting_lock_addr:
                        sal = current.find_sal()
                        source = ""
                        if sal and sal.symtab:
                            source = f" at {sal.symtab.filename}:{sal.line}"
                        held.append((addr, f"#{depth} {func}{source}"))
                except Exception:
                    pass

        depth += 1
        try:
            current = current.older()
        except Exception:
            break

    return held

# ──────────────────────────────────────────────────────────────────────────────
# Lock owner detection
# ──────────────────────────────────────────────────────────────────────────────

def get_mutex_owner(mutex_addr_hex):
    """Read __data.__owner from a pthread_mutex_t to get the owning LWP.

    glibc x86_64 layout (2.35 / Ubuntu 22.04 and 2.39 / Ubuntu 24.04):
      __lock:   offset 0  (int)
      __count:  offset 4  (unsigned int)
      __owner:  offset 8  (int)       <-- LWP of lock holder
      __nusers: offset 12 (unsigned int)
      __kind:   offset 16 (int)
    """
    try:
        expr = f"((pthread_mutex_t*){mutex_addr_hex})->__data.__owner"
        val = safe_eval(expr)
        owner = safe_int(val)
        if owner and owner > 0:
            return owner
    except Exception:
        pass
    # Fallback: raw offset read (offset 8 on glibc x86_64)
    try:
        expr = f"*(int*)((char*){mutex_addr_hex} + 8)"
        val = safe_eval(expr)
        owner = safe_int(val)
        if owner and owner > 0:
            return owner
    except Exception:
        pass
    return None

def get_rwlock_cur_writer(rwlock_addr_hex):
    """Read __data.__cur_writer from pthread_rwlock_t to get write-lock holder.

    glibc x86_64 layout (2.35 / Ubuntu 22.04 and 2.39 / Ubuntu 24.04):
      __readers:        offset 0  (unsigned int)
      __writers:        offset 4  (unsigned int)
      __wrphase_futex:  offset 8  (unsigned int)
      __writers_futex:  offset 12 (unsigned int)
      __pad3:           offset 16 (unsigned int)
      __pad4:           offset 20 (unsigned int)
      __cur_writer:     offset 24 (int)   <-- LWP of write-lock holder
      __shared:         offset 28 (int)
    """
    try:
        expr = f"((pthread_rwlock_t*){rwlock_addr_hex})->__data.__cur_writer"
        val = safe_eval(expr)
        writer = safe_int(val)
        if writer and writer > 0:
            return writer
    except Exception:
        pass
    # Fallback: raw offset read (offset 24 on glibc x86_64)
    try:
        expr = f"*(int*)((char*){rwlock_addr_hex} + 24)"
        val = safe_eval(expr)
        writer = safe_int(val)
        if writer and writer > 0:
            return writer
    except Exception:
        pass
    return None

def get_rwlock_readers(rwlock_addr_hex):
    """Read __data.__readers from pthread_rwlock_t (offset 0)."""
    try:
        expr = f"((pthread_rwlock_t*){rwlock_addr_hex})->__data.__readers"
        val = safe_eval(expr)
        return safe_int(val)
    except Exception:
        pass
    try:
        expr = f"*(unsigned int*)((char*){rwlock_addr_hex})"
        val = safe_eval(expr)
        return safe_int(val)
    except Exception:
        pass
    return None

def get_rwlock_writers_futex(rwlock_addr_hex):
    """Read __data.__writers_futex from pthread_rwlock_t (offset 12).
    Non-zero when writers are queued/waiting."""
    try:
        expr = f"((pthread_rwlock_t*){rwlock_addr_hex})->__data.__writers_futex"
        val = safe_eval(expr)
        return safe_int(val)
    except Exception:
        pass
    try:
        expr = f"*(unsigned int*)((char*){rwlock_addr_hex} + 12)"
        val = safe_eval(expr)
        return safe_int(val)
    except Exception:
        pass
    return None

def get_engine3_lock_holder(lockable_addr_hex):
    """Read threadLockHolder from an Engine3 Lockable object.

    Returns the Thread* address if available (can be mapped to a thread).
    """
    try:
        # threadLockHolder is an AtomicReference<Thread*>
        expr = f"((sys::thread::Lockable*){lockable_addr_hex})->threadLockHolder.value.load()"
        val = safe_eval(expr)
        return safe_hex(val)
    except Exception:
        pass
    return None

def get_engine3_rwlock_pthread_addr(lock_addr_hex):
    """Get the pthread_rwlock_t address inside an Engine3 ReadWriteLock.

    Engine3 ReadWriteLock layout:
      - vtable ptr (8 bytes)
      - Lockable members: threadLockHolder AtomicReference<Thread*> (8 bytes)
      - readLockCount (unsigned int, 4 bytes)
      - padding (4 bytes for alignment)
      - pthread_rwlock_t rwlock
    Total offset to rwlock = 8 + 8 + 4 + 4 = 24 (0x18)

    But we try the symbolic path first.
    """
    try:
        expr = f"&((sys::thread::ReadWriteLock*){lock_addr_hex})->rwlock"
        val = safe_eval(expr)
        addr = safe_hex(val)
        if addr:
            return addr
    except Exception:
        pass
    return None

# ──────────────────────────────────────────────────────────────────────────────
# Owner thread backtrace — show what the lock holder is doing
# ──────────────────────────────────────────────────────────────────────────────

def get_thread_top_frames(thr, max_frames=8):
    """Get the top N user-space frames from a thread's backtrace.

    Returns list of frame summary strings, filtering out glibc internals
    to show what the thread is actually doing.
    """
    frames = []
    try:
        thr.switch()
        current = gdb.selected_frame()
        depth = 0
        while current and depth < 50:
            func = current.name() or "??"
            sal = current.find_sal()

            # Skip glibc/pthread internals
            if not func.startswith("__") and not func.startswith("___"):
                if "::" in func or func == "main":
                    source = ""
                    if sal and sal.symtab:
                        source = f" at {sal.symtab.filename}:{sal.line}"
                    frames.append(f"#{depth} {func}{source}")
                    if len(frames) >= max_frames:
                        break

            depth += 1
            try:
                current = current.older()
            except Exception:
                break
    except Exception:
        pass
    return frames

# ──────────────────────────────────────────────────────────────────────────────
# Cycle detection (N-way deadlock via DFS)
# ──────────────────────────────────────────────────────────────────────────────

def find_cycles(wait_for_graph):
    """Find all cycles in a wait-for graph.

    wait_for_graph: dict mapping tid → set of tids it's waiting for.
    Returns a list of cycles, each cycle is a list of tids.
    """
    visited = set()
    on_stack = set()
    stack = []
    cycles = []

    def dfs(node):
        visited.add(node)
        on_stack.add(node)
        stack.append(node)

        for neighbor in wait_for_graph.get(node, set()):
            if neighbor not in visited:
                dfs(neighbor)
            elif neighbor in on_stack:
                # Found a cycle - extract it
                idx = stack.index(neighbor)
                cycle = stack[idx:]
                # Normalize: start from smallest tid
                min_idx = cycle.index(min(cycle))
                cycle = cycle[min_idx:] + cycle[:min_idx]
                if cycle not in cycles:
                    cycles.append(cycle)

        stack.pop()
        on_stack.remove(node)

    for node in wait_for_graph:
        if node not in visited:
            dfs(node)

    return cycles

# ──────────────────────────────────────────────────────────────────────────────
# Main command
# ──────────────────────────────────────────────────────────────────────────────

class Core3Blocked(gdb.Command):
    """Analyze blocked threads in Core3: mutex, rwlock, condvar waits, and deadlock detection.

    Usage:
      blocked          -- Full analysis (suppress idle condvar threads)
      blocked idle     -- Also list idle condvar threads
      blocked locks    -- Show lock objects and their holders
      blocked graph    -- Show raw wait-for graph edges
      blocked report   -- Write full analysis to /tmp/blocked_report.txt
    """

    def __init__(self):
        super(Core3Blocked, self).__init__("blocked", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        args = arg.strip().split() if arg else []
        show_idle = "idle" in args
        show_locks = "locks" in args
        show_graph = "graph" in args
        write_report = "report" in args

        # If report mode, capture all output
        output_lines = []
        def out(line=""):
            output_lines.append(line)
            print(line)

        out("")
        out("=" * 90)
        out("  Core3 Deadlock & Contention Analyzer")
        out("  Engine3 Lockable / Mutex / ReadWriteLock aware")
        out("=" * 90)

        # ── Phase 1: Collect thread data ──────────────────────────────────
        threads = {}         # tid (LWP) → analysis dict
        thread_names = {}    # tid → thread name string
        thread_roles = {}    # tid → classified role
        thread_nums = {}     # tid → GDB thread number
        thread_objs = {}     # tid → GDB thread object
        lwp_to_tid = {}      # LWP → tid (they're the same for us)

        for inf in gdb.inferiors():
            for thr in inf.threads():
                if not thr.is_valid():
                    continue
                tid = thr.ptid[1]  # LWP
                if tid == 0:
                    continue

                name = get_thread_name(thr)
                role = classify_thread_name(name)

                thread_names[tid] = name or f"Thread-{thr.num}"
                thread_roles[tid] = role
                thread_nums[tid] = thr.num
                thread_objs[tid] = thr

                analysis = analyze_thread(thr)
                threads[tid] = analysis

        total_threads = len(threads)
        out(f"\nScanned {total_threads} threads.\n")

        # ── Phase 2: Categorize threads ───────────────────────────────────
        idle_threads = {}
        blocked_threads = {}
        suspicious_threads = {}
        running_threads = {}

        for tid, info in threads.items():
            if info['is_idle']:
                idle_threads[tid] = info
            elif info['wait_type'] in ('mutex', 'rwlock_wr', 'rwlock_rd'):
                blocked_threads[tid] = info
            elif info['wait_type'] == 'rwlock_unlock':
                # Stuck in unlock is very suspicious
                suspicious_threads[tid] = info
            elif info['wait_type'] == 'futex' and info['corrupt_stack']:
                suspicious_threads[tid] = info
            elif info['wait_type'] is None:
                running_threads[tid] = info
            elif info['corrupt_stack']:
                suspicious_threads[tid] = info
            else:
                # condvar or unclassified futex that isn't idle
                idle_threads[tid] = info

        # ── Phase 3: Resolve lock owners ──────────────────────────────────
        # Build: lock_addr → set of waiting tids
        lock_waiters = defaultdict(set)
        lock_types = {}  # lock_addr → type string

        for tid, info in blocked_threads.items():
            addr = info.get('lock_addr')
            if addr:
                lock_waiters[addr].add(tid)
                lock_types[addr] = info['wait_type']

        # For each lock, try to find the owner
        lock_owners = {}  # lock_addr → owner LWP

        for addr, wtype in lock_types.items():
            owner = None

            # Try pthread-level owner
            if wtype == 'mutex':
                for tid, info in blocked_threads.items():
                    if info.get('lock_addr') == addr and info.get('mutex_addr'):
                        owner = get_mutex_owner(info['mutex_addr'])
                        if owner:
                            break

            elif wtype in ('rwlock_wr', 'rwlock_rd'):
                # Find the rwlock_addr from any thread waiting on this lock
                for tid, info in blocked_threads.items():
                    if info.get('lock_addr') == addr and info.get('rwlock_addr'):
                        owner = get_rwlock_cur_writer(info['rwlock_addr'])
                        break

                # If no rwlock_addr, try casting the Engine3 lock object
                if not owner:
                    try:
                        expr = f"((sys::thread::ReadWriteLock*){addr})->rwlock.__data.__cur_writer"
                        val = safe_eval(expr)
                        owner = safe_int(val)
                        if owner and owner <= 0:
                            owner = None
                    except Exception:
                        pass

            if owner:
                lock_owners[addr] = owner

        # ── Phase 3a: Read rwlock internal state for all contended rwlocks ─
        rwlock_state = {}  # lock_addr → decoded __readers dict

        for addr, wtype in lock_types.items():
            if 'rwlock' not in wtype:
                continue

            # Get pthread rwlock address (try Engine3 cast first)
            pthread_addr = get_engine3_rwlock_pthread_addr(addr)
            if not pthread_addr:
                # Try from any blocked thread's rwlock_addr
                for tid, info in blocked_threads.items():
                    if info.get('lock_addr') == addr and info.get('rwlock_addr'):
                        pthread_addr = info['rwlock_addr']
                        break

            if pthread_addr:
                readers = get_rwlock_readers(pthread_addr)
                if readers is not None:
                    decoded = decode_rwlock_readers(readers)
                    if decoded:
                        decoded['pthread_addr'] = pthread_addr
                        decoded['cur_writer'] = get_rwlock_cur_writer(pthread_addr)
                        decoded['writers_futex'] = get_rwlock_writers_futex(pthread_addr)
                        rwlock_state[addr] = decoded

        # ── Phase 3b: Infer held locks from stack frames ─────────────────
        for tid, info in blocked_threads.items():
            blocking_depth = info.get('blocking_depth')
            waiting_lock = info.get('lock_addr')
            thr_obj = thread_objs.get(tid)
            if thr_obj and blocking_depth is not None:
                held = infer_held_locks(thr_obj, blocking_depth, waiting_lock)
                info['held_locks'] = held
            else:
                info['held_locks'] = []

        # Build reverse map: lock_addr -> set of tids that hold it (inferred)
        inferred_holders = defaultdict(set)
        for tid, info in blocked_threads.items():
            for held_addr, frame_desc in info.get('held_locks', []):
                inferred_holders[held_addr].add(tid)

        # ── Phase 3c: Try to identify lock objects ────────────────────────
        lock_identities = {}  # lock_addr → description string
        for addr in lock_waiters:
            identity = identify_engine3_object(addr)
            if identity:
                lock_identities[addr] = identity

        # ── Phase 4: Build wait-for graph ─────────────────────────────────
        wait_for = defaultdict(set)  # tid → set of tids it waits for

        for tid, info in blocked_threads.items():
            addr = info.get('lock_addr')
            if addr and addr in lock_owners:
                owner = lock_owners[addr]
                if owner != tid:  # don't add self-edges
                    wait_for[tid].add(owner)

        # Add edges from stack-frame inference
        for tid, info in blocked_threads.items():
            wait_addr = info.get('lock_addr')
            if wait_addr and wait_addr in inferred_holders:
                for holder_tid in inferred_holders[wait_addr]:
                    if holder_tid != tid:
                        wait_for[tid].add(holder_tid)

        # ── Phase 5: Detect cycles ────────────────────────────────────────
        cycles = find_cycles(wait_for)

        # ══════════════════════════════════════════════════════════════════
        # OUTPUT
        # ══════════════════════════════════════════════════════════════════

        # ── Summary ───────────────────────────────────────────────────────
        out("─── Thread Summary ─────────────────────────────────────────────────")
        out(f"  Total threads:      {total_threads}")
        out(f"  Idle (condvar):     {len(idle_threads)}")
        out(f"  Blocked (lock):     {len(blocked_threads)}")
        out(f"  Suspicious:         {len(suspicious_threads)}")
        out(f"  Running/other:      {len(running_threads)}")
        out()

        # ── Phantom reader corruption alerts ─────────────────────────────
        corruption_found = False
        for addr, state in rwlock_state.items():
            if state.get('phantom_readers'):
                if not corruption_found:
                    out("─── *** RWLOCK CORRUPTION DETECTED *** ────────────────────────────")
                    corruption_found = True
                identity = lock_identities.get(addr, "")
                identity_str = f" [{identity}]" if identity else ""
                out(f"\n  Lock {addr}{identity_str}:")
                out(f"    {state['corruption_details']}")
                if state.get('cur_writer'):
                    writer = state['cur_writer']
                    wname = thread_names.get(writer, '??')
                    out(f"    Current writer: LWP {writer} ({wname})")
                    # Show what the writer is doing
                    if writer in thread_objs:
                        writer_frames = get_thread_top_frames(thread_objs[writer])
                        if writer_frames:
                            out(f"    Writer stack (top frames):")
                            for f in writer_frames[:5]:
                                out(f"      {f}")
                        # Check for deserialization
                        writer_info = threads.get(writer, {})
                        if writer_info.get('has_deserialization'):
                            out(f"    ⚠ Writer is in deserialization chain "
                                f"({writer_info['deser_depth']} nested DB loads) "
                                f"— zone wlock held during lazy object loading!")
                out()

        if not corruption_found:
            # Still show rwlock state summary if anything interesting
            interesting_rwlocks = {a: s for a, s in rwlock_state.items()
                                   if s.get('reader_count', 0) > 0 or s.get('cur_writer')}
            if interesting_rwlocks:
                out("─── RWLock State ───────────────────────────────────────────────────")
                for addr, state in interesting_rwlocks.items():
                    identity = lock_identities.get(addr, "")
                    identity_str = f" [{identity}]" if identity else ""
                    out(f"  Lock {addr}{identity_str}: {format_rwlock_readers(state)}")
                    if state.get('cur_writer'):
                        writer = state['cur_writer']
                        wname = thread_names.get(writer, '??')
                        wstatus = " [ALSO BLOCKED]" if writer in blocked_threads else ""
                        out(f"    Writer: LWP {writer} ({wname}){wstatus}")
                out()

        # ── Deadlock cycles ───────────────────────────────────────────────
        if cycles:
            out("─── *** DEADLOCK CYCLES DETECTED *** ──────────────────────────────")
            for i, cycle in enumerate(cycles, 1):
                # Determine if cycle was found via metadata or stack inference
                uses_inference = False
                for t in cycle:
                    wait_addr = blocked_threads.get(t, {}).get('lock_addr')
                    if wait_addr and wait_addr in inferred_holders:
                        for target in wait_for.get(t, set()):
                            if target in inferred_holders[wait_addr]:
                                if wait_addr not in lock_owners or lock_owners.get(wait_addr) != target:
                                    uses_inference = True
                method = " (via stack-frame inference)" if uses_inference else ""

                tids_str = " → ".join(
                    f"{t} ({thread_names.get(t, '??')})" for t in cycle
                )
                tids_str += f" → {cycle[0]} ({thread_names.get(cycle[0], '??')})"
                out(f"\n  Cycle {i}{method}: {tids_str}")

                for t in cycle:
                    info = blocked_threads.get(t) or threads.get(t, {})
                    lock = info.get('lock_addr', '??')
                    wtype = info.get('wait_type', '??')
                    e3func = info.get('engine3_func', '')
                    cross = info.get('cross_lock_addr', '')
                    lock_id = lock_identities.get(lock, '')
                    lock_id_str = f" [{lock_id}]" if lock_id else ""
                    out(f"    Thread {t:>6} ({thread_names.get(t, '??'):>20}): "
                          f"waiting {wtype} on {lock}{lock_id_str}"
                          f"{' via ' + e3func if e3func else ''}"
                          f"{' [cross-lock held: ' + cross + ']' if cross else ''}")
                    # Show what locks this cycle member holds
                    held = info.get('held_locks', [])
                    if held:
                        for h_addr, h_desc in held:
                            out(f"             holds: {h_addr} (from {h_desc})")
            out()
        else:
            if blocked_threads:
                out("─── No classic deadlock cycles detected ───────────────────────────")
                if corruption_found:
                    out("  (This is expected for phantom reader corruption — it is NOT a")
                    out("   cycle deadlock. The rwlock is permanently corrupted.)")
                out()

        # ── Blocked threads (grouped by lock) ─────────────────────────────
        if blocked_threads:
            out("─── Blocked Threads (grouped by contended lock) ─────────────────")
            for addr in sorted(lock_waiters.keys()):
                waiters = lock_waiters[addr]
                wtype = lock_types.get(addr, '??')
                owner = lock_owners.get(addr)
                owner_name = thread_names.get(owner, '??') if owner else None
                count = len(waiters)

                severity = ""
                if count >= 10:
                    severity = " *** SEVERE CONTENTION ***"
                elif count >= 5:
                    severity = " ** HIGH CONTENTION **"
                elif count >= 2:
                    severity = " * contended *"

                identity = lock_identities.get(addr, "")
                identity_str = f" [{identity}]" if identity else ""

                out(f"\n  Lock {addr} ({wtype}){identity_str} - {count} waiter(s){severity}")

                # Show rwlock internal state inline
                if addr in rwlock_state:
                    state = rwlock_state[addr]
                    out(f"    State: {format_rwlock_readers(state)}")

                if owner:
                    # Check if owner is also blocked
                    owner_blocked = owner in blocked_threads
                    owner_status = " [ALSO BLOCKED]" if owner_blocked else " [running]"
                    out(f"    Owner: LWP {owner} ({owner_name}){owner_status}")
                    # Show owner's top frames if not blocked (what's it doing?)
                    if not owner_blocked and owner in thread_objs:
                        owner_frames = get_thread_top_frames(thread_objs[owner])
                        if owner_frames:
                            out(f"    Owner doing:")
                            for f in owner_frames[:4]:
                                out(f"      {f}")
                        owner_info = threads.get(owner, {})
                        if owner_info.get('has_deserialization'):
                            out(f"    ⚠ Owner in deserialization chain "
                                f"({owner_info['deser_depth']} nested DB loads)")
                else:
                    out(f"    Owner: unknown (symbols may be optimized out)")

                # Categorize waiters by type
                rd_waiters = [t for t in waiters if blocked_threads[t].get('wait_type') == 'rwlock_rd']
                wr_waiters = [t for t in waiters if blocked_threads[t].get('wait_type') == 'rwlock_wr']
                mx_waiters = [t for t in waiters if blocked_threads[t].get('wait_type') == 'mutex']

                if rd_waiters and wr_waiters:
                    out(f"    Breakdown: {len(rd_waiters)} readers + {len(wr_waiters)} writers waiting")

                for tid in sorted(waiters):
                    info = blocked_threads[tid]
                    name = thread_names.get(tid, '??')
                    role = thread_roles.get(tid, '??')
                    e3func = info.get('engine3_func', '')
                    cross = info.get('cross_lock_addr')
                    corrupt = " [CORRUPT STACK]" if info.get('corrupt_stack') else ""

                    cross_str = ""
                    if cross:
                        cross_id = lock_identities.get(cross, "")
                        cross_id_str = f" ({cross_id})" if cross_id else ""
                        cross_str = f" [cross-locking against {cross}{cross_id_str}]"

                    out(f"    → LWP {tid:>6} ({name:>20} / {role:>14})"
                          f"{corrupt}{cross_str}")
                    if e3func:
                        out(f"                  in {e3func}")
                    # Show inferred held locks
                    held = info.get('held_locks', [])
                    if held:
                        out(f"                  Holds locks (inferred from stack):")
                        for h_addr, h_desc in held:
                            h_id = lock_identities.get(h_addr, "")
                            h_id_str = f" ({h_id})" if h_id else ""
                            out(f"                    {h_addr}{h_id_str} (from {h_desc})")
                    # Show deserialization chain warning
                    if info.get('has_deserialization'):
                        out(f"                  ⚠ In deserialization chain ({info['deser_depth']} nested)")
                    # Show top user frames
                    for uf in info.get('user_frames', [])[:4]:
                        out(f"                  {uf}")

            out()

        # ── Suspicious threads ────────────────────────────────────────────
        if suspicious_threads:
            out("─── Suspicious Threads (corrupt stacks / unusual state) ─────────")
            for tid in sorted(suspicious_threads.keys()):
                info = suspicious_threads[tid]
                name = thread_names.get(tid, '??')
                wtype = info.get('wait_type', 'unknown')
                lock = info.get('lock_addr', 'N/A')
                extra = ""
                if info.get('is_stuck_unlock'):
                    extra = " *** STUCK IN RWLOCK UNLOCK (very unusual!) ***"
                out(f"  LWP {tid:>6} ({name:>20}): {wtype} at {lock}"
                      f"{' [CORRUPT STACK]' if info.get('corrupt_stack') else ''}{extra}")
                for uf in info.get('user_frames', [])[:3]:
                    out(f"    {uf}")
            out()

        # ── Lock details (always shown for rwlocks now) ───────────────────
        if show_locks or (blocked_threads and rwlock_state):
            out("─── Lock Object Details ────────────────────────────────────────")
            for addr in sorted(lock_waiters.keys()):
                wtype = lock_types.get(addr, '??')
                identity = lock_identities.get(addr, "")
                identity_str = f" [{identity}]" if identity else ""
                out(f"\n  Lock {addr} ({wtype}){identity_str}:")

                # For rwlocks, show internal state
                if 'rwlock' in wtype:
                    if addr in rwlock_state:
                        state = rwlock_state[addr]
                        out(f"    {format_rwlock_readers(state)}")
                        if state.get('writers_futex'):
                            out(f"    __writers_futex: {state['writers_futex']} (writers queued)")
                        if state.get('cur_writer'):
                            writer = state['cur_writer']
                            wname = thread_names.get(writer, '??')
                            out(f"    __cur_writer: LWP {writer} ({wname})")
                    else:
                        # Try reading directly
                        readers = None
                        writer = None
                        writers_futex = None
                        try:
                            expr = f"((sys::thread::ReadWriteLock*){addr})->rwlock.__data.__readers"
                            val = safe_eval(expr)
                            readers = safe_int(val)
                        except Exception:
                            pass
                        try:
                            expr = f"((sys::thread::ReadWriteLock*){addr})->rwlock.__data.__cur_writer"
                            val = safe_eval(expr)
                            writer = safe_int(val)
                        except Exception:
                            pass
                        try:
                            expr = f"((sys::thread::ReadWriteLock*){addr})->rwlock.__data.__writers_futex"
                            val = safe_eval(expr)
                            writers_futex = safe_int(val)
                        except Exception:
                            pass

                        if readers is not None:
                            decoded = decode_rwlock_readers(readers)
                            if decoded:
                                out(f"    {format_rwlock_readers(decoded)}")
                            else:
                                out(f"    __readers: {readers:#010x} ({readers})")
                        if writers_futex is not None:
                            out(f"    __writers_futex: {writers_futex} {'(writers queued)' if writers_futex else ''}")
                        if writer is not None:
                            wname = thread_names.get(writer, '??')
                            out(f"    __cur_writer: LWP {writer} ({wname})")

                    # threadLockHolder
                    holder = get_engine3_lock_holder(addr)
                    if holder:
                        out(f"    threadLockHolder: {holder}")

                elif wtype == 'mutex':
                    owner = None
                    for tid, info in blocked_threads.items():
                        if info.get('lock_addr') == addr and info.get('mutex_addr'):
                            owner = get_mutex_owner(info['mutex_addr'])
                            break
                    if owner:
                        out(f"    __owner: {owner} (LWP)")
            out()

        # ── Wait-for graph (optional) ─────────────────────────────────────
        if show_graph:
            out("─── Wait-For Graph Edges ───────────────────────────────────────")
            if wait_for:
                for tid, targets in sorted(wait_for.items()):
                    for target in sorted(targets):
                        # Determine edge source
                        source = "metadata"
                        wait_addr = blocked_threads.get(tid, {}).get('lock_addr')
                        if wait_addr and wait_addr in inferred_holders and target in inferred_holders[wait_addr]:
                            if wait_addr not in lock_owners or lock_owners.get(wait_addr) != target:
                                source = "stack-inference"
                        out(f"  {tid} ({thread_names.get(tid, '??')}) "
                              f"→ waits for → "
                              f"{target} ({thread_names.get(target, '??')}) [{source}]")
            else:
                out("  (no edges - could not determine lock owners)")

            # Show inferred lock holdings
            if inferred_holders:
                out()
                out("─── Inferred Lock Holdings (from stack frames) ──────────────────")
                for addr, holders in sorted(inferred_holders.items()):
                    holder_strs = [f"{t} ({thread_names.get(t, '??')})" for t in sorted(holders)]
                    identity = lock_identities.get(addr, "")
                    identity_str = f" [{identity}]" if identity else ""
                    out(f"  Lock {addr}{identity_str} held by: {', '.join(holder_strs)}")
            out()

        # ── Idle condvar threads (optional) ───────────────────────────────
        if show_idle and idle_threads:
            # Group idle threads by role
            idle_by_role = defaultdict(list)
            for tid in sorted(idle_threads.keys()):
                role = thread_roles.get(tid, 'unknown')
                idle_by_role[role].append(tid)

            out("─── Idle Threads (condvar / futex wait) ────────────────────────")
            for role in sorted(idle_by_role.keys()):
                tids = idle_by_role[role]
                names = [thread_names.get(t, '??') for t in tids[:5]]
                more = f" ... +{len(tids)-5} more" if len(tids) > 5 else ""
                out(f"  {role:>16}: {len(tids):>3} threads "
                      f"({', '.join(names)}{more})")
            out()
        elif idle_threads:
            # Just show summary
            idle_by_role = defaultdict(int)
            for tid in idle_threads:
                idle_by_role[thread_roles.get(tid, 'unknown')] += 1
            parts = [f"{v} {k}" for k, v in sorted(idle_by_role.items(),
                                                      key=lambda x: -x[1])]
            out(f"  Idle threads (use 'blocked idle' for details): {', '.join(parts)}")
            out()

        # ── Diagnosis summary ─────────────────────────────────────────────
        if blocked_threads:
            out("─── Diagnosis ──────────────────────────────────────────────────")

            if corruption_found:
                out("  TYPE: Phantom Reader Corruption (rwlock permanently stuck)")
                out("  The rwlock __readers counter has been corrupted by a thread calling")
                out("  rlock() while holding wlock() on the same lock (POSIX undefined behavior).")
                out("  This is NOT a deadlock cycle — the lock is permanently broken.")
                out("  FIX: Eliminate isLockedByCurrentThread() guard pattern in Zone functions.")
                out()

            if cycles:
                out("  TYPE: Classic Deadlock Cycle")
                out("  Two or more threads hold locks in opposite order, creating a circular wait.")
                out("  FIX: Ensure consistent lock ordering (Zone before Object).")
                out()

            if not corruption_found and not cycles:
                # Check for deserialization-under-wlock
                deser_holders = []
                for addr, owner_tid in lock_owners.items():
                    owner_info = threads.get(owner_tid, {})
                    if owner_info.get('has_deserialization'):
                        deser_holders.append((addr, owner_tid))

                if deser_holders:
                    out("  TYPE: Likely Deserialization Stall Under Write Lock")
                    out("  A thread holds a zone wlock while performing cascading DB deserialization.")
                    out("  All other zone operations are blocked waiting for the wlock to release.")
                    out("  This may resolve on its own (slow), or the deserialization may be stuck")
                    out("  on a reference cycle or contended object.")
                    out()
                else:
                    out("  TYPE: Contention (no cycle or corruption detected)")
                    out("  Threads are blocked on locks but no deadlock cycle was found.")
                    out("  This may indicate:")
                    out("    - A lock holder that is very slow (long operation under lock)")
                    out("    - A lock holder in another thread category (check 'blocked idle')")
                    out("    - Lock owner detection failed (try 'blocked locks' for manual inspection)")
                    out()

        # ── Diagnostic suggestions ────────────────────────────────────────
        out("─── Diagnostic Commands ────────────────────────────────────────")

        if blocked_threads:
            # Find the most interesting thread to inspect
            shown_inspect = False
            for tid in sorted(blocked_threads.keys()):
                info = blocked_threads[tid]
                if info.get('engine3_func') and info.get('lock_addr'):
                    gdb_num = thread_nums.get(tid, '?')
                    addr = info['lock_addr']
                    out(f"  Inspect blocked thread (LWP {tid}, GDB #{gdb_num}):")
                    out(f"    thread {gdb_num}")
                    out(f"    bt full")
                    if 'rwlock' in (info.get('wait_type') or ''):
                        out(f"  Inspect rwlock state:")
                        out(f"    p ((sys::thread::ReadWriteLock*){addr})->rwlock.__data")
                        out(f"    p ((sys::thread::ReadWriteLock*){addr})->threadLockHolder")
                        out(f"    p ((sys::thread::ReadWriteLock*){addr})->readLockCount")
                    elif info.get('wait_type') == 'mutex':
                        out(f"  Inspect mutex state:")
                        out(f"    p ((sys::thread::Mutex*){addr})->mutex.__data")
                        out(f"    p ((sys::thread::Mutex*){addr})->threadLockHolder")
                    shown_inspect = True
                    break

            if lock_owners:
                for addr, owner_tid in lock_owners.items():
                    if owner_tid in thread_nums:
                        gdb_num = thread_nums[owner_tid]
                        out(f"  Inspect lock owner (LWP {owner_tid}, GDB #{gdb_num}):")
                        out(f"    thread {gdb_num}")
                        out(f"    bt full")
                        break

            # Show all unique contended lock addresses
            if lock_waiters:
                out(f"  Quick lock state dump (copy/paste):")
                seen = set()
                for addr, wtype in lock_types.items():
                    if addr in seen:
                        continue
                    seen.add(addr)
                    if 'rwlock' in wtype:
                        out(f"    p ((sys::thread::ReadWriteLock*){addr})->rwlock.__data")
                    else:
                        out(f"    p *(pthread_mutex_t*){addr}")

        if not blocked_threads and idle_threads:
            out("  No blocked threads - server may be idle or stuck in condvar wait.")
            out("  Check if task dispatchers are running:")
            out("    thread apply all bt 5 | grep -i -E 'TaskManager|TaskQueue|dispatch'")

        out(f"\n  Subcommands:")
        out(f"    blocked              -- full analysis (default)")
        out(f"    blocked idle         -- also list idle condvar threads")
        out(f"    blocked locks        -- show lock internal state (readers, writer, etc.)")
        out(f"    blocked graph        -- show raw wait-for graph edges")
        out(f"    blocked report       -- write full analysis to /tmp/blocked_report.txt")
        out(f"\n  General GDB:")
        out(f"    thread apply all bt 5")
        out(f"    info threads")
        out("=" * 90)

        # ── Write report file ─────────────────────────────────────────────
        if write_report:
            report_path = "/tmp/blocked_report.txt"
            try:
                with open(report_path, "w") as f:
                    f.write(f"Core3 Blocked Thread Report — {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
                    f.write(f"{'=' * 90}\n\n")
                    for line in output_lines:
                        f.write(line + "\n")

                    # Also dump full backtraces of blocked + suspicious threads
                    f.write(f"\n{'=' * 90}\n")
                    f.write(f"FULL BACKTRACES OF BLOCKED/SUSPICIOUS THREADS\n")
                    f.write(f"{'=' * 90}\n\n")

                    all_interesting = set(blocked_threads.keys()) | set(suspicious_threads.keys())
                    # Also include lock owners
                    for owner in lock_owners.values():
                        if owner in thread_objs:
                            all_interesting.add(owner)

                    for tid in sorted(all_interesting):
                        if tid not in thread_objs:
                            continue
                        name = thread_names.get(tid, '??')
                        gdb_num = thread_nums.get(tid, '?')
                        f.write(f"\n--- Thread {gdb_num} (LWP {tid}, {name}) ---\n")
                        try:
                            thread_objs[tid].switch()
                            bt = gdb.execute("bt full", to_string=True)
                            f.write(bt)
                        except Exception as e:
                            f.write(f"  (error getting backtrace: {e})\n")

                out(f"\n  Report written to: {report_path}")
            except Exception as e:
                out(f"\n  Error writing report: {e}")


# Register the command
Core3Blocked()
print("Core3 blocked thread analyzer loaded. Type 'blocked' to run.")
