/*
 * TemplateReference.h
 *
 *  Created on: 16/07/2010
 *      Author: victor
 */

#ifndef TEMPLATEREFERENCE_H_
#define TEMPLATEREFERENCE_H_

#include "templates/manager/TemplateManager.h"

#include "engine/util/json_utils.h"

template<class O> class TemplateReference : public Reference<O> {
#ifdef ODB_SERIALIZATION
	int crc = 0;
#endif
	// Infinity: Cache CRC for dbconvert round-trip safety.
	// When TemplateManager isn't initialized (standalone mode),
	// the lookup in parseFromBinaryStream returns null. Without this cache,
	// toBinaryStream writes 0 and the template CRC is permanently lost.
	uint32 cachedCRC = 0;
public:
	TemplateReference() : Reference<O>() {
	}

	TemplateReference(TemplateReference& ref) : Reference<O>(ref) {
#ifdef ODB_SERIALIZATION
		crc = ref.crc;
#endif
		cachedCRC = ref.cachedCRC;
	}

	TemplateReference(const TemplateReference& ref) : Reference<O>(ref) {
#ifdef ODB_SERIALIZATION
		crc = ref.crc;
#endif
		cachedCRC = ref.cachedCRC;
	}

	TemplateReference(const O obj) : Reference<O>(obj) {
	}

	TemplateReference& operator=(const TemplateReference& ref) {
		if (this == &ref)
			return *this;

		Reference<O>::updateObject(ref);

#ifdef ODB_SERIALIZATION
		crc = ref.crc;
#endif
		cachedCRC = ref.cachedCRC;

		return *this;
	}

	bool operator==(const TemplateReference& ref) {
		return Reference<O>::get() == ref.get();
	}

	bool operator==(const O ref) {
		return Reference<O>::get() == ref;
	}

	bool toString(String& str) const {
#ifdef ODB_SERIALIZATION
		str = String::valueOf(crc);

		return true;
#else
		if (Reference<O>::get() != nullptr)
			str = String::valueOf((Reference<O>::get())->getServerObjectCRC());
		else
			str = String::valueOf(0);

		return true;
#endif
	}

	friend void to_json(nlohmann::json& j, const TemplateReference& r) {
		String str;
		r.toString(str);

		j = str;
	}

	bool parseFromString(const String& str, int version = 0) {
		SharedObjectTemplate* obj = TemplateManager::instance()->getTemplate(UnsignedLong::valueOf(str));

		if (obj == nullptr) {
			Reference<O>::updateObject(nullptr);
			return false;
		}

		Reference<O>::updateObject((O) obj);

		return true;
	}

	bool toBinaryStream(ObjectOutputStream* stream) {
#ifdef ODB_SERIALIZATION
		stream->writeInt(crc);

		return true;
#else
		O object = Reference<O>::get();

		if (object != nullptr)
			stream->writeInt(object->getServerObjectCRC());
		else
			// Infinity: Write cached CRC when manager lookup failed (dbconvert standalone mode)
			stream->writeInt(cachedCRC);

		return true;
#endif
	}

	bool parseFromBinaryStream(ObjectInputStream* stream) {
		uint32 oid = stream->readInt();

#ifdef ODB_SERIALIZATION
		crc = oid;

		return true;
#else
		// Infinity: Always cache CRC for dbconvert round-trip safety
		cachedCRC = oid;

		SharedObjectTemplate* obj = TemplateManager::instance()->getTemplate(oid);

		if (obj == nullptr) {
			Reference<O>::updateObject(nullptr);
			return false;
		}

		Reference<O>::updateObject((O) obj);

		return true;
#endif
	}
};


#endif /* TEMPLATEREFERENCE_H_ */
