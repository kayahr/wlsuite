/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for licensing information
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wasteland.h"


/**
 * Decodes a data block with the vertical xor scheme. The data is modified
 * in-place so if you don't want your data to be modified then you have to
 * make a copy of it before calling this function.
 *
 * @param data
 *            The data to decode
 * @param width
 *            The width of the data block (for example the picture width)
 * @param height
 *            The height of the data block (for example the picture height)
 */

void wlVXorDecode(unsigned char *data, int width, int height)
{
    int x, y;
    unsigned char xor;
    
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            xor = y == 0 ? 0 : data[(y - 1) * width + x];
            data[y * width + x] ^= xor;
        }
    }
}


/**
 * Encodes a data block with the vertical xor scheme. The data is modified
 * in-place so if you don't want your data to be modified then you have to
 * make a copy of it before calling this function.
 *
 * @param data
 *            The data to encode
 * @param width
 *            The width of the data block (for example the picture width)
 * @param height
 *            The height of the data block (for example the picture height)
 */

void wlVXorEncode(unsigned char *data, int width, int height)
{
    unsigned char *xors, byte, xor;
    int x, y;
    
    xors = malloc(width * sizeof(unsigned char));
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            byte = data[y * width + x];
            xor = y == 0 ? 0 : xors[x];
            data[y * width + x] = byte ^ xor;
            xors[x] = byte;
        }
    }
    free(xors);
}
