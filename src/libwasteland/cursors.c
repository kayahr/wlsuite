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
 * Reads cursors from the specified file and returns it as a an array of
 * images. You have to release the allocated memory of this array when you
 * no longer need it. If an error occurs while reading the source file then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * cursors[cursorNo][y * 16 + x]. A pixel is an integer between 0 and 255. The
 * lower 4 bits is the EGA color palette index. The higher 4 bits are the
 * transparency bits for the lower 4 bit. So if bit 4 is set then this
 * means that bit 0 (The blue component) is transparent. This feature is not
 * used in the original wasteland cursors. These original cursors only have
 * fully solid pixels or fully transparent pixels (Which means that all high
 * bits are set if the pixel is transparent or they are cleared if the pixel
 * is solid). If you don't need sub-pixel-transparency either then you can
 * interpret the value like this: If the pixel value is lower than 16 then the
 * pixel value is simply the EGA color index. If the value is higher or equal 
 * 16 then the pixel is transparent.
 *
 * @param filename
 *            The filename of the cursor file to read
 * @return The cursors as an array of images
 */

wlImages wlCursorsReadFile(char *filename)
{
    FILE *file;
    wlImages cursors;
    
    assert(filename != NULL);
    file = fopen(filename, "rb");
    if (!file) return NULL;
    cursors = wlCursorsReadStream(file);
    fclose(file);
    return cursors;
}


/**
 * Reads sprites from the specified file stream. The stream must already be
 * open and pointing to the cursors data. The stream is not closed by this 
 * function so you have to do this yourself.
 * 
 * You have to release the allocated memory of the returned array when you
 * no longer need it. If an error occurs while reading the source streams then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * cursors[cursorNo][y * 16 + x]. A pixel is an integer between 0 and 255. The
 * lower 4 bits is the EGA color palette index. The higher 4 bits are the
 * transparency bits for the lower 4 bit. So if bit 4 is set then this
 * means that bit 0 (The blue component) is transparent. This feature is not
 * used in the original wasteland cursors. These original cursors only have
 * fully solid pixels or fully transparent pixels (Which means that all high
 * bits are set if the pixel is transparent or they are cleared if the pixel
 * is solid). If you don't need sub-pixel-transparency either then you can
 * interpret the value like this: If the pixel value is lower than 16 then the
 * pixel value is simply the EGA color index. If the value is higher or equal 
 * 16 then the pixel is transparent.
 * 
 * @param stream
 *            The stream to read cursors from
 * @return The cursors as an array of images
 */

wlImages wlCursorsReadStream(FILE *stream)
{
    wlImages cursors;
    int cursor, bit, x, y, type, pixel, b;
    
    assert(stream != NULL);
    cursors = (wlImages) malloc(8 * sizeof(wlImage));
    for (cursor = 0; cursor < 8; cursor++)
    {
        cursors[cursor] = (wlImage) malloc(16 * 16 * sizeof(wlPixel));
        
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < 16; y++)
            {
                for (type = 0; type < 2; type++)
                {
                    for (x = 8; x >= 0; x -= 8)
                    {
                        b = fgetc(stream);
                        if (b == EOF) return NULL;
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            if (type)
                            {
                                cursors[cursor][y * 16 + x + pixel] |=
                                    ((b >> (7 - pixel)) & 1) << bit;
                            }
                            else
                            {
                                cursors[cursor][y * 16 + x + pixel] |=
                                    !((b >> (7 - pixel)) & 1) << (4 + bit);
                            }
                        }
                    }
                }
            }
        }
    }
    return cursors;
}


/**
 * Writes cursors to a file. The function returns 1 if write was successfull
 * and 0 if write failed. In this case you can read the reason from errno.
 *
 * @param cursors
 *            The cursors to write (Array with 8 16x16 images)
 * @param filename
 *            The filename of the file to write the cursors to
 * @return 1 on success, 0 on failure
 */

int wlCursorsWriteFile(wlImages cursors, char *filename)
{
    FILE *file;
    int result;
    
    assert(cursors != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlCursorsWriteStream(cursors, file);
    fclose(file);
    return result;
}


/**
 * Writes cursors to a stream. The stream must already be open and pointing
 * to the location where you want to write the cursors to. The stream is not
 * closed by this function so you have to do this yourself. The function
 * returns 1 if write was successfull and 0 if write failed. In
 * this case you can read the reason from errno.
 *
 * @param cursors
 *            The cursors to write (Array with 8 16x16 images)
 * @param stream
 *            The stream to write the cursors to
 * @return 1 on success, 0 on failure
 */

int wlCursorsWriteStream(wlImages cursors, FILE *stream)
{
    int cursor, bit, y, type, x, pixel, b;
 
    assert(cursors != NULL);
    assert(stream != NULL);
    for (cursor = 0; cursor < 8; cursor++)
    {
        for (bit = 0; bit < 4; bit++)   
        {
            for (y = 0; y < 16; y++)
            {
                for (type = 0; type < 2; type++)
                {
                    for (x = 8; x >= 0; x -= 8)
                    {
                        b = 0;
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            if (type)
                            {
                                b |= ((cursors[cursor][y * 16 + x + pixel]
                                    >> bit) & 0x01) << (7 - pixel);
                            }
                            else
                            {
                                b |= !((cursors[cursor][y * 16 + x + pixel]
                                    >> (4 + bit)) & 0x01) << (7 - pixel);
                            }
                        }
                        if (fputc(b, stream) == EOF) return 0;
                    }
                }
            }
        }
    }
    return 1;
}
