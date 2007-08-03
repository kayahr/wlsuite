/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <stdio.h>
#include "wasteland.h"


/**
 * Reads a single bit from the specified file stream. In fact a whole byte
 * is read and stored in <var>dataByte</var>. In <var>dataMask</var> this
 * function remembers which bit was read in previous calls to this method. Make
 * sure these two variables are initialized with 0 when you start reading bits.
 * 
 * @param file
 *            The file stream from which data is read
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The bit (0 or 1) which has been read or -1 if read failed
 */
  
int wlReadBit(FILE *file, unsigned char *dataByte, unsigned char *dataMask)
{
    int tmp;
    
    if (*dataMask == 0)
    {
        if (fread(dataByte, 1, 1, file) != 1) return -1;
        *dataMask = 0x80;
    }
    tmp = *dataByte & *dataMask;
    *dataMask = *dataMask >> 1;
    return tmp ? 1 : 0;
}


/**
 * Reads a byte from the specified file stream. In fact this function calls
 * wlReadBit() 8 times to read 8 bits from the current bit position in the
 * stream. You have to provide pointers to a data byte and a
 * data mask to keep track of the bits.
 * 
 * @param file
 *            The file stream from which data is read
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The byte which has been read or -1 if read failed
 */

int wlReadByte(FILE *file, unsigned char *dataByte, unsigned char *dataMask)
{
    int byte, i, bit;
 
    byte = 0;
    for (i = 0 ; i < 8 ; i++)
    {
        bit = wlReadBit(file, dataByte, dataMask);
        if (bit < 0) return bit;
        byte = (byte << 1) | bit;
    }
    return byte;
}


/**
 * Reads a 16 bit little-endian value from the specified stream. Returns -1
 * if read failed.
 * 
 * @param file
 *            The stream to read the word from
 * @return The 16 bit litt-endian value or -1 if an error occured while reading
 */

int wlReadWord(FILE *stream)
{
    int low, high;
        
    low = fgetc(stream);
    if (low == EOF) return -1;
    high = fgetc(stream);
    if (high == EOF) return -1;
    return high << 8 | low;
}


/**
 * Reads a 32 bit little-endian value from the specified stream. Returns -1
 * if read failed.
 * 
 * @param file
 *            The stream to read the word from
 * @return The 32 bit litt-endian value or -1 if an error occured while reading
 */

long wlReadDWord(FILE *stream)
{
    long b1, b2, b3, b4;
        
    b1 = fgetc(stream);
    if (b1 == EOF) return -1;
    b2 = fgetc(stream);
    if (b2 == EOF) return -1;
    b3 = fgetc(stream);
    if (b3 == EOF) return -1;
    b4 = fgetc(stream);
    if (b4 == EOF) return -1;
    return b1 << 24 | b2 << 16 || b3 << 8 | b4;
}
