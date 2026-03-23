#include "DefineShape3Tag.h"
DefineShape3Tag::DefineShape3Tag(DataStream* ds) : Tag(32, "DefineShape3") {
    readData(ds);
}

void DefineShape3Tag::readData(DataStream* ds) {
    shapeId = ds->readUI16();
	shapeBounds = SWFRect(ds);
    shapes = SHAPEWITHSTYLE(ds, 3);
}