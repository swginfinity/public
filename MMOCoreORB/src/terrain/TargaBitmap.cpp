/*
 * TargaBitmap.cpp
 *
 *  Created on: 06/09/2013
 *      Author: victor
 */

#include "engine/engine.h"
#include "TargaBitmap.h"

class TargaBlackPixel : public TargaPixel {
public:
	unsigned char val;

	TargaBlackPixel() {
		val = 0;
	}

	TargaBlackPixel(const TargaBlackPixel& c) {
		val = c.val;
	}

	void read(ObjectInputStream* data, int bytes) {
		val = data->readByte();
	}

	void clear() {
		val = 0;
	}
};

class TargaColorPixel : public TargaPixel {
public:
	unsigned char r, g, b, a;

	void clear() {
		r = 0, g = 0, b = 0, a = 0;
	}

	void read(ObjectInputStream* data, int bytes) {
		switch (bytes) {
		case 4:
			r = data->readByte();
			g = data->readByte();
			b = data->readByte();
			a = data->readByte();
			break;
		case 3:
			r = data->readByte();
			g = data->readByte();
			b = data->readByte();
			a = 0;
			break;
		case 2: {
			byte one = data->readByte();
			byte two = data->readByte();

			r = (one & 0x7c) << 1;
			g = ((one & 0x03) << 6) | ((two & 0xe0) << 2);
			b = (two & 0x1f) << 3;
			a = (one & 0x80);
			break;
		}

		}
	}
};


TargaBitmap::TargaBitmap() {
	pixelData = nullptr;
}

TargaBitmap::~TargaBitmap() {
	if (pixelData != nullptr) {
		// Fixed: iterate linearly over all pixels instead of buggy nested loop
		for (int i = 0; i < header.width * header.height; ++i) {
			delete pixelData[i];
		}

		delete [] pixelData;
		pixelData = nullptr;
	}
}

void TargaBitmap::readObject(ObjectInputStream* stream) {
	header.readData(stream);

	if (header.datatypecode != 3 && header.datatypecode != 2) {
		throw Exception("unsopported targa format " + String::valueOf(header.datatypecode));
	}

	if (header.colourmaptype != 0 && header.colourmaptype != 1) {
		throw Exception("unsopported targa coulourmaptype " + String::valueOf(header.colourmaptype));
	}

	pixelData = new TargaPixel*[header.width * header.height];

	for (int i = 0; i < header.width * header.height; ++i) {
		TargaPixel* obj = nullptr;
		switch (header.datatypecode) {
		case 3:
			obj = new TargaBlackPixel();
			break;
		case 10:
		case 2:
			obj = new TargaColorPixel();
			break;

		}

		pixelData[i] = obj;

		obj->clear();
	}

	int skip = header.idlength + (header.colourmaptype * header.colourmaplength);

	stream->shiftOffset(skip);

	int read = header.bitsperpixel / 8;

	// Original buggy code had: i * header.width + j which overflows when width != height
	// Fixed to use correct indexing: row * width + col
	for (int row = header.height - 1; row >= 0; --row) {
		for (int col = 0; col < header.width; ++col) {
			int index = row * header.width + col;
			if (index < 0 || index >= header.width * header.height) {
				return;
			}
			pixelData[index]->read(stream, read);
		}
	}
}

unsigned char TargaBitmap::getData(int offset) const {
	if (offset < 0 || offset >= header.width * header.height)
		throw ArrayIndexOutOfBoundsException(offset);

	const TargaBlackPixel* tbp = dynamic_cast<const TargaBlackPixel*>(pixelData[offset]);

	if (tbp == nullptr)
		throw Exception("pixelData[" + String::valueOf(offset) + "] is not a TargaBlackPixel");

	return tbp->val;
}
