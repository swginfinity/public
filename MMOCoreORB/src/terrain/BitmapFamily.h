/*
 * BitmapFamily.h
 *
 *  Created on: 06/09/2013
 *      Author: victor
 */

#ifndef BITMAPFAMILY_H_
#define BITMAPFAMILY_H_

#include "TemplateVariable.h"
#include "TargaBitmap.h"
#include "templates/manager/DataArchiveStore.h"

class BitMapFamily : public TemplateVariable<'MFAM'> {
	int var1;
	String name;
	String file;

	TargaBitmap map;
public:
	BitMapFamily() : var1(0) {

	}

	void parseFromIffStream(engine::util::IffStream* iffStream) {
		iffStream->openChunk('DATA');

		var1 = iffStream->getInt();
		iffStream->getString(name);
		iffStream->getString(file);

		iffStream->closeChunk('DATA');

		Logger::console.info(true) << "[DIAG] BitmapFamily loading file: " << file << " (name: " << name << ")";

		int size = 0 ;
		byte* data = DataArchiveStore::instance()->getData(file, size);

		if (data != nullptr) {
			Logger::console.info(true) << "[DIAG] BitmapFamily file loaded, size: " << size;
			ObjectInputStream stream((char*)data, size);

			try {
				map.readObject(&stream);
				Logger::console.info(true) << "[DIAG] BitmapFamily TGA parsed successfully";
			} catch (Exception& e) {
				Logger::console.error() << "[DIAG] Could not parse targa file for BitMapFamily: " << file;
				Logger::console.error() << "[DIAG] Exception: " << e.getMessage();
			}

			delete [] data;
		} else {
			Logger::console.warning() << "[DIAG] BitmapFamily file not found in TRE: " << file;
		}
	}

	inline TargaBitmap* getMap() {
		return &map;
	}

	inline const String& getName() const {
		return name;
	}

	inline int getVar1() const {
		return var1;
	}

	inline const String& getFile() const {
		return file;
	}
};

#endif /* BITMAPFAMILY_H_ */
