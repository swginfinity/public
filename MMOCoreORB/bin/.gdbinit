############################################
# GDB Configuration for SWGEmu Core3
############################################

############################################
# History (persist across sessions)
############################################
set history save on
set history size 10000
set history filename ~/.gdb_history

############################################
# Output Formatting (C++ Friendly)
############################################
# Indent structs/unions nicely
set print pretty on
# Print array elements one per line
set print array on
# Show indexes when printing arrays
set print array-indexes on
# Demangle C++ names
set print demangle on
# Demangle in disassembly
set print asm-demangle on
# Show all args in backtrace
set print frame-arguments all
# Allow deep stack traces
set backtrace limit 200

############################################
# Disassembly / Assembly
############################################
# Intel syntax (easier to read)
set disassembly-flavor intel

############################################
# Terminal Behavior
############################################
# No line wrapping
set width 0
# No paging
set height 0
# Disable --More--
set pagination off
# No confirmation prompts
set confirm off

############################################
# Thread Debugging (Very Important for Core3)
############################################
# Reduce noise
set print thread-events off
# Normal stepping - "off" is the default, toggle to "on" when single-stepping
# one thread. Can only be set after a target is loaded, so use manually:
#   set scheduler-locking on
#   set scheduler-locking off

############################################
# Catch C++ Exceptions - Only for detailed debugging in dev
############################################
# catch throw
# catch catch

############################################
# Logging (manual enable when needed)
############################################
# Use these manually when investigating crashes:
#   set logging file gdb.log
#   set logging on
#   set logging off

############################################
# Enable libstdc++ STL Pretty Printers
# (Improves std::vector, std::map, etc.)
############################################
python
import sys
# Try common locations
for path in ['/usr/share/gcc/python', '/usr/share/gcc-13/python', '/usr/share/gdb/python']:
    sys.path.insert(0, path)
try:
    from libstdcxx.v6.printers import register_libstdcxx_printers
    register_libstdcxx_printers(None)
except Exception:
    pass
end

# Define shortcuts
define btall
  thread apply all bt
end

define btfull
  thread apply all bt full
end

# Container detection: disable ASLR override when running in Docker.
# Containers lack the personality() syscall, causing GDB's default
# disable-randomization to error out.
python
import os
if os.path.exists('/.dockerenv'):
    gdb.execute('set disable-randomization off')
end

# Download additional debug data (Ubuntu 24.04 dev environment only).
# GDB on 22.04 lacks debuginfod support, so this is wrapped to avoid errors.
python
try:
    gdb.execute("set debuginfod enabled on")
except Exception:
    pass
end

############################################
# Core3 Deadlock Analyzer
# Auto-loads the blocked command on attach
############################################
source core3_blocked.py

############################################
# Deadlock Investigation Shortcuts
############################################

# Show rwlock internal state for an Engine3 ReadWriteLock*
# Usage: lockinfo 0xADDRESS
define lockinfo
  echo --- ReadWriteLock state ---\n
  p ((sys::thread::ReadWriteLock*)$arg0)->rwlock.__data
  echo --- threadLockHolder ---\n
  p ((sys::thread::ReadWriteLock*)$arg0)->threadLockHolder
  echo --- readLockCount ---\n
  p ((sys::thread::ReadWriteLock*)$arg0)->readLockCount
end
document lockinfo
  Dump Engine3 ReadWriteLock internals. Usage: lockinfo 0xADDRESS
end

# Show mutex internal state for an Engine3 Mutex*
# Usage: mutinfo 0xADDRESS
define mutinfo
  echo --- Mutex state ---\n
  p ((sys::thread::Mutex*)$arg0)->mutex.__data
  echo --- threadLockHolder ---\n
  p ((sys::thread::Mutex*)$arg0)->threadLockHolder
end
document mutinfo
  Dump Engine3 Mutex internals. Usage: mutinfo 0xADDRESS
end

# Quick summary: top 5 frames for every thread (less noise than btall)
define bt5
  thread apply all bt 5
end
document bt5
  Show top 5 frames for all threads.
end

# Find threads stuck in lock waits (grep-based quick check)
define lockgrep
  thread apply all bt 10 | grep -E "lll_lock|rwlock_wrlock|rwlock_rdlock|Mutex::lock|ReadWriteLock::(w|r)lock"
end
document lockgrep
  Quick grep for threads stuck in lock operations.
end