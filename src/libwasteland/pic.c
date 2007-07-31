/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wasteland.h"
    

/**
 * Reads pixels from the specified PIC file and returns them. You have to
 * free the allocated memory for the returned pixels when you no longer need
 * them.
 * 
 * If the specified file could not be read then NULL is returned and you can
 * retrieve the problem source from errno.
 *
 * @param filename
 *            The filename of the pic file to read
 * @return The pixels
 */

wlPixels wlPicReadFile(char *filename)
{
    FILE *file;
    wlPixels pixels;
    
    assert(filename != NULL);
    file = fopen(filename, "rb");
    if (!file) return NULL;
    pixels = wlPicReadStream(file);
    fclose(file);
    return pixels;
}


/**
 * Reads pixels from a PIC file stream and returns them. The stream must
 * already be open and pointing to the PIC data. The stream is not closed by
 * this function so you have to do this yourself. You have to
 * free the allocated memory for the returned pixels when you no longer need
 * them. 
 * 
 * If an error occurs while reading data from the stream then NULL is returned
 * and you can retrieve the problem source from errno.
 *
 * @param stream
 *            The stream to read from
 * @return The pixels
 */

wlPixels wlPicReadStream(FILE *stream)
{
    wlPixels pixels;
    int x, y;
    int b;
    
    pixels = (wlPixels) malloc(sizeof(wlPixel) * 288 * 128);
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x+= 2)
        {
            b = fgetc(stream);
            if (b == EOF) return NULL;
            pixels[y * 288 + x] = b >> 4;
            pixels[y * 288 + x + 1] = b & 0x0f;
        }
    }
    wlVXorDecode(pixels, 288, 128);
    return pixels;
}


/**
 * Writes pixels to a PIC file. The function returns 1 if write was successfull
 * and 0 if write failed. On failure you can check errno for the reason.
 *
 * @param pixels
 *            The pixels to write
 * @param filename
 *            The filename of the file to write the pixels to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteFile(wlPixels pixels, char *filename)
{
    FILE *file;
    int result;
    
    assert(pixels != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlPicWriteStream(pixels, file);
    fclose(file);
    return result;
}


/**
 * Writes pixels to a file stream. The stream must already be open and pointing
 * to the location where you want to write the pic to. The stream is not 
 * closed by this function so you have to do this yourself. The function 
 * returns 1 if write was successfull and 0 if write failed.
 *
 * @param pixels
 *            The pixels to write
 * @param stream
 *            The stream to write the pixels to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteStream(wlPixels pixels, FILE *stream)
{
    int x, y;
    int pixel;
    wlPixel encodedPixels[288 * 128];
    
    assert(pixels != NULL);
    assert(stream != NULL);
    
    /* Encode the pixels */
    memcpy(encodedPixels, pixels, sizeof(wlPixel) * 288 * 128);
    wlVXorEncode(encodedPixels, 288, 128);
    
    /* Write encoded pixels to stream */
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x += 2)
        {
            pixel = (encodedPixels[y * 288 + x] << 4)
                | (encodedPixels[y * 288 + x + 1] & 0x0f);
            if (fputc(pixel, stream) == EOF) return 0;
        }
    }
    
    /* Release encoded pixels and report success */
    return 1;
}
