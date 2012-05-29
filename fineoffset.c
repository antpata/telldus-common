#include "fineoffset.h"
#include "receive.h"
#include "message.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SHORT(x) (x<11)
#define LONG(x) (x>13)

#define INVALID_DATA 2
#define DATA_LENGTH 5

unsigned char fineOffsetBit(unsigned short *scanP, unsigned char *scanBit) {
	UCHAR8 b1 = rfCountSimilar(scanP, scanBit);
	UCHAR8 b2 = rfCountSimilar(scanP, scanBit);

	if (LONG(b1) && LONG(b2)) { 
		return 0;
	}
	if (SHORT(b1) && LONG(b2)) {
		return 1;
	}

	return INVALID_DATA;
}

void parseFineOffset(unsigned short scanP, unsigned char scanBit) {
	unsigned char buffer[DATA_LENGTH];

	rfRetreatBit(&scanP, &scanBit); //skip last bit

	for(int i=DATA_LENGTH-1; i>=0; --i) {
		UCHAR8 byte = 0;
		for(int j=0; j<8; ++j){
			UCHAR8 b = fineOffsetBit(&scanP, &scanBit);
			if (b == INVALID_DATA) {
				return;
			}
			if (b) {
				byte |= (1<<j);
			}
		}
		buffer[i] = byte;
	}
	rfMessageBeginRaw();
		rfMessageAddString("class", "sensor");
		rfMessageAddString("protocol", "fineoffset");
		rfMessageAddHexString("data", buffer, DATA_LENGTH);
	rfMessageEnd(1);
}
