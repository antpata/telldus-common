#include "mandolyn.h"
#include "receive.h"
#include "message.h"
#include <stdio.h>

#define ZERO(x) (x>=30 && x<=40)
#define SHORT(x) (x>=10 && x<=20)
#define ONE(x,y) (SHORT(x) && SHORT(y))

#define INVALID_DATA 2

unsigned char mandolynBit(unsigned short *scanP, unsigned char *scanBit) {
	UCHAR8 b1 = rfCountSimilar(scanP, scanBit);
	if (ZERO(b1)) {
		return 0;
	}
	UCHAR8 b2 = rfCountSimilar(scanP, scanBit);
	if (ONE(b1,b2)) {
		return 1;
	}
	return INVALID_DATA;
}

void parseMandolyn(unsigned short scanP, unsigned char scanBit) {
	unsigned short P, B;
	unsigned char preamble = 0;
	unsigned long data = 0;
	unsigned long mask = 1;

	rfRetreatBit(&scanP, &scanBit); //skip last bit
	rfCountSimilar(&scanP, &scanBit); //skip last pulse

	P = scanP; B = scanBit;

	for(int i=0;i<32;++i){
		UCHAR8 b = mandolynBit(&scanP, &scanBit);
		if (b == INVALID_DATA) {
			return;
		}
		if (b) {
			data |= mask;
		}
		mask <<= 1;
	}

	for(int i=0;i<4;++i){
		UCHAR8 b = mandolynBit(&scanP, &scanBit);
		if (b == INVALID_DATA) {
			return;
		}
		if (b) {
			preamble |= (1<<i);
		}
	}
	if (preamble != 0xC) {
		return;
	}

	rfMessageBeginRaw();
		rfMessageAddString("class", "sensor");
		rfMessageAddString("protocol", "mandolyn");
		rfMessageAddString("model", "temperaturehumidity");
		rfMessageAddLong("data", data);
	rfMessageEnd(1);
}
