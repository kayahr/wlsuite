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
 * Reads a font from the specified file and returns the font glyphs as an array
 * of images. You have to release the allocated memory of this array when you
 * no longer need it. If an error occurs while reading the source file then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * font[glyphNo][y * 8 + x]. A pixel is an integer between 0 and 1 which is
 * simply the EGA color palette index.
 * 
 * @param filename
 *            The filename of the font file to read
 * @return The font as an array of images
 */

wlImages wlFontReadFile(char *filename)
{
    FILE *file;
    wlImages font;
    
    assert(filename != NULL);
    file = fopen(filename, "rb");
    if (!file) return NULL;
    font = wlFontReadStream(file);
    fclose(file);
    return font;
}


/**
 * Reads a font from the specified file stream. The stream must already be
 * open and pointing to the font data. The stream is not closed by this 
 * function so you have to do this yourself.
 * 
 * You have to release the allocated memory of the returned array when you
 * no longer need it. If an error occurs while reading the source streams then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * font[glyphNo][y * 8 + x]. A pixel is an integer between 0 and 15 which is
 * simply the EGA color palette index.
 * 
 * @param stream
 *            The stream to read the font from
 * @return The font as an array of images
 */

wlImages wlFontReadStream(FILE *stream)
{
    wlImages font;
    int glyph, bit, y, pixel, b;
    
    assert(stream != NULL);
    font = (wlImages) malloc(172 * sizeof(wlImage));
    for (glyph = 0; glyph < 172; glyph++)
    {
        font[glyph] = (wlImage) malloc(8 * 8 * sizeof(wlPixel));
        
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < 8; y++)
            {
                b = fgetc(stream);
                if (b == EOF) return NULL;
                for (pixel = 0; pixel < 8; pixel++)
                {
                    font[glyph][y * 8 + pixel] |=
                        ((b >> (7 - pixel)) & 1) << bit;
                }
            }
        }
    }
    return font;
}


/**
 * Writes a font to a file. The function returns 1 if write was successfull
 * and 0 if write failed. In this case you can read the reason from errno.
 *
 * @param font
 *            The font to write (Array with 172 8x8 images)
 * @param filename
 *            The filename of the file to write the font to
 * @return 1 on success, 0 on failure
 */

int wlFontWriteFile(wlImages font, char *filename)
{
    FILE *file;
    int result;
    
    assert(font != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlFontWriteStream(font, file);
    fclose(file);
    return result;
}


/**
 * Writes a font to a stream. The stream must already be open and pointing
 * to the location where you want to write the font to. The stream is not
 * closed by this function so you have to do this yourself. The function
 * returns 1 if write was successfull and 0 if write failed. In
 * this case you can read the reason from errno.
 *
 * @param font
 *            The font to write (Array with 172 8x8 images)
 * @param stream
 *            The stream to write the font to
 * @return 1 on success, 0 on failure
 */

int wlFontWriteStream(wlImages font, FILE *stream)
{
    int glyph, bit, y, pixel, b;
 
    assert(font != NULL);
    assert(stream != NULL);
    for (glyph = 0; glyph < 172; glyph++)
    {
        for (bit = 0; bit < 4; bit++)   
        {
            for (y = 0; y < 8; y++)
            { 
                b = 0;
                for (pixel = 0; pixel < 8; pixel++)
                {
                    b |= ((font[glyph][y * 8 + pixel] >> bit) & 0x01) <<
                        (7 - pixel);
                }
                if (fputc(b, stream) == EOF) return 0;
            }
        }
    }
    return 1;
}
