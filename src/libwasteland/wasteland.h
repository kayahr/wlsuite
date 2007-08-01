/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#ifndef WASTELAND_H
#define WASTELAND_H

#include <stdio.h>

typedef unsigned char wlPixel;
typedef wlPixel * wlImage;
typedef wlImage * wlImages;

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

extern wlImage wlPicReadFile(char *filename);
extern wlImage wlPicReadStream(FILE *stream);
extern int     wlPicWriteFile(wlImage pixels, char *filename);
extern int     wlPicWriteStream(wlImage pixels, FILE *stream);

extern wlImages wlSpritesReadFile(char *spritesFilename, char *masksFilename);
extern wlImages wlSpritesReadStream(FILE *spritesStream, FILE *masksStream);
extern int      wlSpritesWriteFile(wlImages sprites, char *spritesFilename,
    char *masksFilename);
extern int      wlSpritesWriteStream(wlImages sprites, FILE *spritesStream,
    FILE *masksStream);

extern wlImages wlCursorsReadFile(char *filename);
extern wlImages wlCursorsReadStream(FILE *stream);
extern int      wlCursorsWriteFile(wlImages cursors, char *filename);
extern int      wlCursorsWriteStream(wlImages cursors, FILE *stream);

extern wlImages wlFontReadFile(char *filename);
extern wlImages wlFontReadStream(FILE *stream);
extern int      wlFontWriteFile(wlImages font, char *filename);
extern int      wlFontWriteStream(wlImages font, FILE *stream);

#endif
