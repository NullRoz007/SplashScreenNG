#ifndef LIBSWF_DOSHAPE3DEFINETAG_H
#define LIBSWF_DOSHAPE3DEFINETAG_H


#include "Tag.h"
#include "RECT.h"
#include "SHAPEWITHSTYLE.h"

class DefineShape3Tag : public Tag {
public:
	static const uint16_t ID = 32;
	uint16_t shapeId;
	SWFRect shapeBounds;
	SHAPEWITHSTYLE shapes;
	DefineShape3Tag(DataStream* ds);
	void readData(DataStream* ds);
};


#endif //DOSHAPE3DEFINETAG