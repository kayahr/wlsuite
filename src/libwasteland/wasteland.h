/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#ifndef WASTELAND_H
#define WASTELAND_H

#include <stdio.h>

typedef unsigned char wlPixel;
typedef wlPixel * wlPixels;

typedef wlPixels * wlSprites;

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} wlRGB;

extern wlRGB wlPalette[16];
                
extern void wlError(char *message, ...);

extern void wlVXorDecode(unsigned char *data, int width, int height);
extern void wlVXorEncode(unsigned char *data, int width, int height);

extern wlPixels wlPicReadFile(char *filename);
extern wlPixels wlPicReadStream(FILE *stream);
extern int      wlPicWriteFile(wlPixels pixels, char *filename);
extern int      wlPicWriteStream(wlPixels pixels, FILE *stream);

extern wlSprites wlSpritesReadFile(char *spritesFilename, char *masksFilename);
extern wlSprites wlSpritesReadStream(FILE *spritesStream, FILE *masksStream);
extern int       wlSpritesWriteFile(wlSprites sprites, char *spritesFilename,
        char *masksFilename);
extern int       wlSpritesWriteStream(wlSprites sprites, FILE *spritesStream,
        FILE *masksStream);

#endif
