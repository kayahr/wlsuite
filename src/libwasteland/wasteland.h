/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for licensing information
 */

#ifndef WASTELAND_H
#define WASTELAND_H

#include <stdio.h>               

typedef struct
{
    int width;
    int height;
    unsigned char *pixels;
} wlPic;

typedef wlPic* wlPicPtr;

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

extern wlPicPtr wlPicCreate(int width, int height); 
extern void     wlPicDestroy(wlPicPtr pic);
extern wlPicPtr wlPicClone(wlPicPtr pic);
extern wlPicPtr wlPicReadFile(char *filename, int width, int height);
extern wlPicPtr wlPicReadStream(FILE *stream, int width, int height);
extern int      wlPicWriteFile(wlPicPtr pic, char *filename);
extern int      wlPicWriteStream(wlPicPtr pic, FILE *stream);

#endif
