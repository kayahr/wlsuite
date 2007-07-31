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

typedef struct
{
    int quantity;
    int spriteWidth;
    int spriteHeight;
    unsigned char **pixels;
} wlSprites;

typedef wlSprites* wlSpritesPtr;

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

extern wlSpritesPtr wlSpritesCreate(int quantity, int width, int height);
extern void         wlSpritesDestroy(wlSpritesPtr sprites);
extern wlSpritesPtr wlSpritesClone(wlSpritesPtr sprites);
extern wlSpritesPtr wlSpritesReadFile(char *spritesFilename, char *masksFilename, 
        int quantity, int width, int height);
extern wlSpritesPtr wlSpritesReadStream(FILE *spritesStream, FILE *masksStream,
        int quantity, int width, int height);
extern int wlSpritesWriteFile(wlSpritesPtr sprites, char *spritesFilename,
        char *masksFilename);
extern int          wlSpritesWriteStream(wlSpritesPtr sprites,
        FILE *spritesStream, FILE *masksStream);

#endif
