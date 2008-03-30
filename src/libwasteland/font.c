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
 * Reads a font from the specified file and returns the font glyphs as a list
 * of images. You have to release the allocated memory of this array with the
 * wlImagesFree() function when you no longer need it. If an error occurs while
 * reading the source file then NULL is returned and you can use errno to find
 * the reason.
 * 
 * A pixel in the returned list can be accessed like this:
 * font->images[glyphNo]->pixels[y * 8 + x]. A pixel is an integer between 0
 * and 1 which is simply the EGA color palette index.
 * 
 * @param filename
 *            The filename of the font file to read
 * @return The font as a list of images
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
 * You have to release the allocated memory of the returned list with the
 * wlImagesFree() function when you no longer need it. If an error occurs
 * while reading the source streams then NULL is returned and you can use
 * errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * font->images[glyphNo]->pixels[y * 8 + x]. A pixel is an integer between 0
 * and 15 which is simply the EGA color palette index.
 * 
 * @param stream
 *            The stream to read the font from
 * @return The font as a list of images
 */

wlImages wlFontReadStream(FILE *stream)
{
    wlImages font;
    wlImage image;
    int glyph, bit, y, pixel, b;
    
    assert(stream != NULL);
    font = wlImagesCreate(172, 8, 8);
    for (glyph = 0; glyph < 172; glyph++)
    {
        image = font->images[glyph];
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < image->height; y++)
            {
                b = fgetc(stream);
                if (b == EOF) return NULL;
                for (pixel = 0; pixel < image->width; pixel++)
                {
                    image->pixels[y * image->width + pixel] |=
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
 *            The font to write
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
 *            The font to write
 * @param stream
 *            The stream to write the font to
 * @return 1 on success, 0 on failure
 */

int wlFontWriteStream(wlImages font, FILE *stream)
{
    int glyph, bit, y, pixel, b;
    wlImage image;
 
    assert(font != NULL);
    assert(stream != NULL);
    for (glyph = 0; glyph < font->quantity; glyph++)
    {
        image = font->images[glyph];
        for (bit = 0; bit < 4; bit++)   
        {
            for (y = 0; y < image->height; y++)
            { 
                b = 0;
                for (pixel = 0; pixel < image->width; pixel++)
                {
                    b |= ((image->pixels[y * image->width + pixel] >> bit) & 0x01) <<
                        (7 - pixel);
                }
                if (fputc(b, stream) == EOF) return 0;
            }
        }
    }
    return 1;
}
