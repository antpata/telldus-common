#include "cooker_termo.h"
#include "crc.h"
#include "receive.h"
#include "message.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define SHORT0(x) (39>x)
#define LONG0(x) ((56<x) && (77>x))
#define SHORT1(x) (15>x)
#define SYNC0(x) ((x > 130) && (x < 150))

#define INVALID_DATA 2
#define DATA_LENGTH 5
#define DATA_BYTES 512 // should not be here


static unsigned char cookerBit(UCHAR8 b1, UCHAR8 b2) {
    if (SHORT0(b1) && SHORT1(b2)) {
        return 0;
    }
    if (LONG0(b1) && SHORT1(b2)) {
        return 1;
    }

    return INVALID_DATA;
}

//static unsigned char findSync(unsigned short *scanP, unsigned char *scanBit) {
//    short stopP = *scanP < DATA_BYTES-2 ? (*scanP)+1 : 0;
//    UCHAR8 b2;
//    while (stopP != *scanP) {
//        rfCountSimilar(scanP, scanBit);     //1
//        b2 = rfCountSimilar(scanP, scanBit);//0
//        if (SYNC0(b2)) //42 - 44 - 44
//            return 1;
//    }
//    return 0;
//}

char parseCooker(unsigned short scanP, unsigned char scanBit) {
    unsigned char buffer[DATA_LENGTH];
    UCHAR8 b1;
    UCHAR8 b2;
    UCHAR8 lastSync = 0;
    rfRetreatBit(&scanP, &scanBit);  //retreat one bit
//    if (!findSync(&scanP, &scanBit))
//        return 0;

    b2 = rfCountSimilar(&scanP, &scanBit); //skip last 1
    UCHAR8 bitCount = 36;
    for(int i=DATA_LENGTH-1; i>=0; --i) {
        UCHAR8 byte = 0;
        for(int j=0; j<8; ++j){
            b1 = rfCountSimilar(&scanP, &scanBit);
            b2 = rfCountSimilar(&scanP, &scanBit);
            UCHAR8 b = cookerBit(b1, b2);
            if (b == INVALID_DATA) {
                return 0;
            }
            if (b) {
                byte |= (1<<j);
            }
            if (--bitCount==0) {
                lastSync = rfCountSimilar(&scanP, &scanBit);
                if (!SYNC0(lastSync))
                    return 0;

                if (!SHORT1(rfCountSimilar(&scanP, &scanBit)))
                        return 0;

                b1 = rfCountSimilar(&scanP, &scanBit);
                b2 = rfCountSimilar(&scanP, &scanBit);
                UCHAR8 b = cookerBit(b1, b2);
                if (b == INVALID_DATA || b == 1) {
                    return 0;
                }
                b1 = rfCountSimilar(&scanP, &scanBit);
                b2 = rfCountSimilar(&scanP, &scanBit);
                UCHAR8 b = cookerBit(b1, b2);
                if (b == INVALID_DATA || b == 1) {
                    return 0;
                }
                break;
            }
        }

        buffer[i] = byte;
    }

    rfMessageBeginRaw();
        rfMessageAddString("class", "sensor");
        rfMessageAddString("protocol", "cooker");
        rfMessageAddHexString("data", buffer, DATA_LENGTH);
//        rfMessageAddByte("s", lastSync);
//        rfMessageAddByte("b1", b1);
//        rfMessageAddByte("b2", b2);
    rfMessageEnd(1);
    return 1;
}

