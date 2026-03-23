#include "RECT.h"

SWFRect::SWFRect() {
}

SWFRect::SWFRect(int64_t Xmin, int64_t Xmax, int64_t Ymin, int64_t Ymax) {
	this->Xmin = Xmin;
	this->Xmax = Xmax;
	this->Ymin = Ymin;
	this->Ymax = Ymax;
}

SWFRect::SWFRect(SWFRect *r) {
	Xmin = r->Xmin;
	Xmax = r->Xmax;
	Ymin = r->Ymin;
	Ymax = r->Ymax;
}

SWFRect::SWFRect(DataStream * ds) {
	readData(ds);
}

void SWFRect::readData(DataStream *ds) {
	nBits = ds->readUB(5);
	Xmin = ds->readSB((unsigned) nBits);
	Xmax = ds->readSB((unsigned) nBits);
	Ymin = ds->readSB((unsigned) nBits);
	Ymax = ds->readSB((unsigned) nBits);
	ds->alignByte();
}
