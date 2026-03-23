#include "SHAPEWITHSTYLE.h"
SHAPEWITHSTYLE::SHAPEWITHSTYLE(){}
SHAPEWITHSTYLE::SHAPEWITHSTYLE(DataStream *ds, int shapeNum) {
	readData(ds, shapeNum);
}

void SHAPEWITHSTYLE::readData(DataStream *ds, int shapeNum) {
	fillStyles = FILLSTYLEARRAY(ds, shapeNum);
	lineStyles = LINESTYLEARRAY(ds, shapeNum);
	numFillBits = (int) ds->readUB(4);
	numLineBits = (int) ds->readUB(4);
	shapeRecords = ds->readSHAPERECORDS(numFillBits, numLineBits, shapeNum);
}
