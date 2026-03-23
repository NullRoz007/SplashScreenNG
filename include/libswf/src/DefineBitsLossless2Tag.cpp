#include <zlib.h>
#include "DefineBitsLossless2Tag.h"
#include "SwfParser.h"

DefineBitsLossless2Tag::DefineBitsLossless2Tag(DataStream* ds) : Tag(36, "DefineBitsLossless2Tag") {
    readData(ds);
}

void DefineBitsLossless2Tag::readData(DataStream* ds) {
    characterId = ds->readUI16();
    bitmapFormat = ds->readUI8();
    bitmapWidth = ds->readUI16();
    bitmapHeight = ds->readUI16();

    if(bitmapFormat == 3) {
        bitmapColorTableSize = ds->readUI8() + 1;
    }

    size_t compressedSize = ds->available();
    vector<uint8_t> compressedBitmapData = ds->readBytes(compressedSize);

    size_t expectedSize = expectedSize = (size_t)bitmapWidth * bitmapHeight * 4;
    uLong destSize = (uLong)expectedSize;
    bitmapData.resize(expectedSize);
    uncompress(
        bitmapData.data(), 
        &destSize,
        compressedBitmapData.data(), 
        (uLong)compressedBitmapData.size()
    );
}