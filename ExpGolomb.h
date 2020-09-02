#ifndef EXPGOLOMB_H
#define EXPGOLOMB_H


#include "Def.h"


class ExpGolomb
{
public:
    ExpGolomb(uint8* data, int datalen) {
        this->data = data;
        this->datalen = datalen;
        index = 0;
        bitLength = datalen * 8;
    }
    int bitsAvailable() { return bitLength - index; }
    int getIndex () { return index; }
    bool skipBits(int size) {
        if (bitsAvailable() < size) {
            return false;
        }
        index += size;
        return true;
    }
    int readBits(int size, bool moveIndex = true) {
        return getBits(size, index, moveIndex);
    }

    int getBits(int size, int offsetBits, bool moveIndex = true) {
        if (bitsAvailable() < size) {
            //throw new Error('no bytes available');
            return 0;
        }
        int offset = offsetBits % 8;
        // printf("offset %d\n", offset);
        int byte = data[(offsetBits / 8) | 0] & (0xff >> offset);
        int bits = 8 - offset;
        if (bits >= size) {
            if (moveIndex) {
                index += size;
            }
            return byte >> (bits - size);
        } else {
            if (moveIndex) {
                index += bits;
            }
            int nextSize = size - bits;
            return (byte << nextSize) | getBits(nextSize, offsetBits + bits, moveIndex);
        }
    }

    int skipLZ() {
        int leadingZeroCount;
        for (leadingZeroCount = 0; leadingZeroCount < bitLength - index; ++leadingZeroCount) {
            if (getBits(1, index + leadingZeroCount, false) != 0) {
                // console.log(`  skip LZ  : size=${leadingZeroCount}, ${this.index}.`);
                index += leadingZeroCount;
                return leadingZeroCount;
            }
        }
        return leadingZeroCount;
    }

    void skipUEG() {
        skipBits(1 + skipLZ());
    }

    void skipEG() {
        skipBits(1 + skipLZ());
    }

    int readUEG() {
        int prefix = skipLZ();
        return readBits(prefix + 1) - 1;
    }

    int readEG() {
        int value = readUEG();
        if (0x01 & value) {
            // the number is odd if the low order bit is set
            return (1 + value) >> 1; // add 1 to make it even, and divide by 2
        } else {
            return -1 * (value >> 1); // divide by two then make it negative
        }
    }

    bool readBoolean() {
        return readBits(1) == 1;
    }

    uint8 readUByte(int numberOfBytes = 1) {
        return readBits((numberOfBytes * 8));
    }

    uint16 readUShort() {
        return readBits(16);
    }

    uint32 readUInt() {
        return readBits(32);
    }

private:
    uint8* data;
    int datalen;
    int bitLength;
    int index;
};

#endif // EXPGOLOMB_H
