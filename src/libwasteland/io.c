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
 * Writes a single bit to the specified file stream. In fact a whole byte
 * is build in <var>dataByte</var> and is written to the stream when it's full.
 * In <var>dataMask</var> this function remembers which bit was write in
 * previous calls. Make sure these two variables are initialized with 0 when
 * you start writing bits.
 * 
 * @param bit
 *            The bit to write
 * @param file
 *            The file stream to which data is written
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */
  
int wlWriteBit(char bit, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask)
{
    *dataByte <<= 1;
    *dataByte |= bit & 1; 
    *dataMask = *dataMask == 0 ? 1 : (*dataMask << 1);
    if (*dataMask == 0x80)
    {
        if (fputc(*dataByte, file) == EOF) return 0;
        *dataByte = 0;
        *dataMask = 0;
    }
    return 1;
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
 * Writes a byte to the specified file stream. In fact this function calls
 * wlWriteBit() 8 times to write 8 bits to the current bit position in the
 * stream. You have to provide pointers to a data byte and a
 * data mask to keep track of the bits. Returns 1 on success, 0 on failure.
 * 
 * @param byte
 *            The byte to write
 * @param file
 *            The file stream to write the data to
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */

int wlWriteByte(unsigned char byte, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask)
{
    int i, bit;
 
    for (i = 7; i >= 0; i--)
    {
        bit = (byte >> i) & 1;
        if (!wlWriteBit(bit, file, dataByte, dataMask)) return 0;
    }
    return 1;
}


/**
 * In case previous bit writes have not filled a whole byte yet this function
 * fills the remaining bits with the specified bit and therfor forces a write
 * of the byte. If we are already at a byte boundary then this function does
 * nothing. Function returns 1 on success and 0 on failure. 
 * 
 * @param bit
 *            The bit to fill the unfinished byte with
 * @param file
 *            The file stream to which data is written
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */

int wlFillByte(char bit, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask)
{
    while (*dataMask)
    {
        if (!wlWriteBit(bit, file, dataByte, dataMask)) return 0;
    }
    return 1;
}



/**
 * Writes a 32 bit unsigned integer to a stream. Returns 1 on success and 0
 * on failure. 
 * 
 * @param dword
 *            The data to write
 * @param file
 *            The stream to write the data to
 * @return 1 on success, 0 on failure
 */

int wlWriteDWord(unsigned int dword, FILE *file)
{
    if (fputc(dword & 0xff, file) == EOF) return 0;
    if (fputc((dword >> 8) & 0xff, file) == EOF) return 0;
    if (fputc((dword >> 16) & 0xff, file) == EOF) return 0;
    if (fputc((dword >> 24) & 0xff, file) == EOF) return 0;
    return 1;
}
