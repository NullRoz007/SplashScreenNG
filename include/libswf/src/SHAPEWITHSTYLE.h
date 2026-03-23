#ifndef LIBSWF_SHAPEWITHSTYLE_H
#define LIBSWF_SHAPEWITHSTYLE_H


#include <vector>
#include "SHAPERECORD.h"
#include "DataStream.h"
#include "FILLSTYLEARRAY.h"
#include "LINESTYLEARRAY.h"

using namespace std;

class SHAPEWITHSTYLE {
public:
	FILLSTYLEARRAY fillStyles;
	LINESTYLEARRAY lineStyles;

	int numFillBits;
	int numLineBits;
	vector<SHAPERECORD*> shapeRecords;
	SHAPEWITHSTYLE();
	SHAPEWITHSTYLE(DataStream* ds, int shapeNum);
	void readData(DataStream* ds, int shapeNum);
};


#endif //SHAPEWITHSTYLE_H
