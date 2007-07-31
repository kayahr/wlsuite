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
 * Reads sprites from the specified files and returns it as a an array of
 * pixels. You have to release the allocated memory of this array when you
 * no longer need it. If an error occurs while reading the source files then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * pixels[spriteNo][y * 16 + x]. A pixel is an integer between 0 and 16. 0-15
 * is a color in the EGA color palette, 16 is transparent.
 *
 * @param spritesFilename
 *            The filename of the sprites file to read
 * @param masksFilename
 *            The filename of the sprite masks file to read
 * @return The sprites as an array of pixels
 */

wlSprites wlSpritesReadFile(char *spritesFilename, char *masksFilename)
{
    FILE *spritesFile, *masksFile;
    wlSprites sprites;
    
    assert(spritesFilename != NULL);
    assert(masksFilename != NULL);
    spritesFile = fopen(spritesFilename, "rb");
    if (!spritesFile) return NULL;
    masksFile = fopen(masksFilename, "rb");
    if (!masksFile) return NULL;
    sprites = wlSpritesReadStream(spritesFile, masksFile);
    fclose(spritesFile);
    fclose(masksFile);
    return sprites;
}


/**
 * Reads sprites from the specified file streams. The streams must already be
 * open and pointing to the sprites and sprite masks data. The streams are not
 * closed by this function so you have to do this yourself.
 * 
 * You have to release the allocated memory of the returned array when you
 * no longer need it. If an error occurs while reading the source streams then
 * NULL is returned and you can use errno to find the reason.
 * 
 * A pixel in the returned array can be accessed like this:
 * pixels[spriteNo][y * 16 + x]. A pixel is an integer between 0 and 16. 0-15
 * is a color in the EGA color palette, 16 is transparent.
 *
 * @param spritesStream
 *            The stream to read sprites from
 * @param masksStream
 *            The stream to read sprite masks from
 * @return The sprites as an array of pixels
 */

wlSprites wlSpritesReadStream(FILE *spritesStream, FILE *masksStream)
{
    wlSprites sprites;
    int x, y, bit, pixel, sprite;
    int b;
    
    assert(spritesStream != NULL);
    assert(masksStream != NULL);
    sprites = (wlSprites) malloc(10 * sizeof(wlPixels));
    for (sprite = 0; sprite < 10; sprite++)
    {
        sprites[sprite] = (wlPixels) malloc(16 * 16 * sizeof(wlPixel));
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < 16; y++)
            {
                for (x = 0; x < 16; x+= 8)
                {
                    b = fgetc(spritesStream);
                    if (b == EOF) return NULL;
                    for (pixel = 0; pixel < 8; pixel++)
                    {
                        sprites[sprite][y * 16 + x + pixel] |=
                            ((b >> (7 - pixel)) & 1) << bit;
                    }

                    // Read transparancy information when last bit has been read
                    if (bit == 3)
                    {
                        b = fgetc(masksStream);
                        if (b == EOF) return NULL;
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            sprites[sprite][y * 16 + x + pixel] |=
                                ((b >> (7 - pixel)) & 0x01) << 4;
                        }
                    }
                }
            }            
        }
    }
    return sprites;
}


/**
 * Writes sprites to files. The function returns 1 if write was successfull
 * and 0 if write failed. In this case you can read the reason from errno.
 *
 * @param sprites
 *            The sprites to write
 * @param spritesFilename
 *            The filename of the file to write the sprites to
 * @param masksFilename
 *            The filename of the file to write the sprite masks to
 * @return 1 on success, 0 on failure
 */

int wlSpritesWriteFile(wlSprites sprites, char *spritesFilename,
        char *masksFilename)
{
    FILE *spritesFile, *masksFile;
    int result;
    
    assert(sprites != NULL);
    assert(spritesFilename != NULL);
    assert(masksFilename != NULL);
    spritesFile = fopen(spritesFilename, "wb");
    if (!spritesFile) return 0;
    masksFile = fopen(masksFilename, "wb");
    if (!masksFile)
    {
        fclose(spritesFile);
        return 0;
    }
    result = wlSpritesWriteStream(sprites, spritesFile, masksFile);
    fclose(spritesFile);
    fclose(masksFile);
    return result;
}


/**
 * Writes sprites to streams. The streams must already be open and pointing
 * to the location where you want to write the sprites and the sprite masks to.
 * The streams are not closed by this function so you have to do this yourself.
 * The function returns 1 if write was successfull and 0 if write failed. In
 * this case you can read the reason from errno.
 *
 * @param sprites
 *            The sprites to write
 * @param spritesStream
 *            The stream to write the sprites to
 * @param masksStream
 *            The stream to write the sprite masks to
 * @return 1 on success, 0 on failure
 */

int wlSpritesWriteStream(wlSprites sprites, FILE *spritesStream,
        FILE *masksStream)
{
    int x, y, bit, sprite, b;
    int pixel;

    assert(sprites != NULL);
    assert(spritesStream != NULL);
    assert(masksStream != NULL);
    for (sprite = 0; sprite < 10; sprite++)
    {
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < 16; y++)
            {
                for (x = 0; x < 16; x += 8)
                {
                    b = 0;
                    for (pixel = 0; pixel < 8; pixel++)
                    {
                        b |= ((sprites[sprite][y * 16 +
                            x + pixel] >> bit) & 0x01) << (7 - pixel);
                    }
                    if (fputc(b, spritesStream) == EOF) return 0;
                    
                    // Write transparancy information when last bit has been
                    // written
                    if (bit == 3)
                    {
                        b = 0;
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            b |= (sprites[sprite][y * 16 + x + pixel] >> 4) <<
                                (7 - pixel);
                        }
                        if (fputc(b, masksStream) == EOF) return 0;
                    }
                }
            }
        }
    }
    return 1;
}
