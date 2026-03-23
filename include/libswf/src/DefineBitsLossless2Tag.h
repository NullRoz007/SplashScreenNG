#ifndef LIBSWF_DODEFINEBITSLOSSLESS2_H
#define LIBSWF_DODEFINEBITSLOSSLESS2_H


#include "Tag.h"

class DefineBitsLossless2Tag : public Tag {
public:
	static const uint16_t ID = 6;
	uint16_t characterId;
	uint8_t bitmapFormat;
	uint8_t bitmapWidth;
	uint8_t bitmapHeight;
	uint8_t bitmapColorTableSize;
	std::vector<uint8_t> bitmapData;

	DefineBitsLossless2Tag(DataStream* ds);
	void readData(DataStream* ds);
};


#endif LIBSWF_DODEFINEBITSLOSSLESS2_H