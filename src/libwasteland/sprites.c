/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wasteland.h"
    

/**
 * Creates a new sprites container for the specified number of sprites and
 * the specified sprite dimensions. Returns a pointer
 * to a newly allocated wlSprites structure. You have to free this
 * structure with wlSpritesDestroy() when you no longer need it.
 *
 * @param quantity
 *            The number of sprites
 * @param width
 *            The sprite width
 * @param height
 *            The sprite height
 */

wlSpritesPtr wlSpritesCreate(int quantity, int width, int height)
{
    wlSpritesPtr sprites;
    int i;
    
    sprites = malloc(sizeof(wlSprites));
    sprites->quantity = quantity;
    sprites->spriteWidth = width;
    sprites->spriteHeight = height;
    sprites->pixels = malloc(quantity * sizeof(unsigned char *));
    for (i = 0; i < quantity; i++)
    {
        sprites->pixels[i] = malloc(width * height * sizeof(unsigned char));
    }
    return sprites;
}


/**
 * Frees the memory allocated by the specified sprites container.
 *
 * @param sprites
 *            The sprites to free
 */
 
void wlSpritesDestroy(wlSpritesPtr sprites)
{
    int i;
    
    for (i = 0; i < sprites->quantity; i++)
    {
        free(sprites->pixels[i]);
    }
    free(sprites->pixels);
    free(sprites);
}


/**
 * Clones the specified sprites container and returns the cloned sprites. The
 * clone is a newly allocated wlSprites structure and the pointer to this
 * structure is returned. You have to free this structure with
 * wlSpritesDestroy() when you no longer need it.
 *
 * @param sprites
 *            The sprites to clone
 * @return The cloned sprites
 */

wlSpritesPtr wlSpritesClone(wlSpritesPtr sprites)
{
    wlSpritesPtr clone;
    
    clone = wlSpritesCreate(sprites->quantity, sprites->spriteWidth,
            sprites->spriteHeight);
    memcpy(clone->pixels, sprites->pixels,
            sprites->quantity * sprites->spriteWidth * sprites->spriteHeight);
    return clone;
}                
    

/**
 * Reads sprites from the specified files and returns it as a wlSprites
 * struct. You have to specify quantity, width and height of the pic because
 * this information can't be read safely from the files.
 *
 * The function returns a pointer to a newly created wlSprites structure.
 * If you no longer need this structure you must free it with the function
 * wlSpritesDestroy().
 *
 * @param spritesFilename
 *            The filename of the sprites file to read
 * @param masksFilename
 *            The filename of the sprite masks file to read
 * @param quantity
 *            The number of sprites to read
 * @param width
 *            The width of the sprites
 * @param height
 *            The height of the sprites
 * @return The pic
 */

wlSpritesPtr wlSpritesReadFile(char *spritesFilename, char *masksFilename, 
        int quantity, int width, int height)
{
    FILE *spritesFile, *masksFile;
    wlSpritesPtr sprites;
    
    spritesFile = fopen(spritesFilename, "rb");
    if (!spritesFile)
    {
        wlError("Unable to open '%s' for reading: %s\n", spritesFilename,
            strerror(errno));
        return NULL;
    }
    masksFile = fopen(masksFilename, "rb");
    if (!masksFile)
    {
        fclose(spritesFile);
        wlError("Unable to open '%s' for reading: %s\n", masksFilename,
            strerror(errno));
        return NULL;
    }
    sprites = wlSpritesReadStream(spritesFile, masksFile, quantity,
            width, height);
    fclose(spritesFile);
    fclose(masksFile);
    return sprites;
}


/**
 * Reads sprites from the specified streams. The streams must already be open
 * and pointing to the sprites and sprite masks data. The streams are not
 * closed by this function so you have to do this yourself.
 *
 * The function returns a pointer to a newly created wlSprites structure.
 * If you no longer need this structure you must free it with the function
 * wlSpritesDestroy().
 *
 * @param spritesStream
 *            The stream to read sprites from
 * @param masksStream
 *            The stream to read sprite masks from
 * @param quantity
 *            The number of sprites to read
 * @param width
 *            The width of the sprites
 * @param height
 *            The height of the sprites
 * @return The sprites
 */

wlSpritesPtr wlSpritesReadStream(FILE *spritesStream, FILE *masksStream,
        int quantity, int width, int height)
{
    wlSpritesPtr sprites;
    int x, y, bit, pixel, sprite;
    int b;
    
    sprites = wlSpritesCreate(quantity, width, height);
    for (sprite = 0; sprite < quantity; sprite++)
    {
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < height; y++)
            {
                for (x = 0; x < width; x+= 8)
                {
                    if (feof(spritesStream))
                    {
                        wlError("Unexpected end of stream while reading sprite data\n");
                        return NULL;
                    }
                    b = fgetc(spritesStream);
                    for (pixel = 0; pixel < 8; pixel++)
                    {
                        sprites->pixels[sprite][y * width + x + pixel] |=
                            ((b >> (7 - pixel)) & 1) << bit;
                    }

                    // Read transparancy information when last bit has been read
                    if (bit == 3)
                    {
                        if (feof(masksStream))
                        {
                            wlError("Unexpected end of stream while reading sprite mask data\n");
                            return NULL;
                        }
                        b = fgetc(masksStream);                        
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            sprites->pixels[sprite][y * width + x + pixel] |=
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
 * and 0 if write failed.
 *
 * @param sprites
 *            The sprites to write
 * @param spritesFilename
 *            The filename of the file to write the sprites to
 * @param masksFilename
 *            The filename of the file to write the sprite masks to
 * @return 1 on success, 0 on failure
 */

int wlSpritesWriteFile(wlSpritesPtr sprites, char *spritesFilename,
        char *masksFilename)
{
    FILE *spritesFile, *masksFile;
    int result;
    
    spritesFile = fopen(spritesFilename, "wb");
    if (!spritesFile)
    {
        wlError("Unable to open '%s' for writing: %s\n", spritesFilename,
            strerror(errno));
        return 0;
    }
    masksFile = fopen(masksFilename, "wb");
    if (!masksFile)
    {
        fclose(spritesFile);
        wlError("Unable to open '%s' for writing: %s\n", masksFilename,
            strerror(errno));
        return 0;
    }
    result = wlSpritesWriteStream(sprites, spritesFile, masksFile);
    fclose(spritesFile);
    fclose(masksFile);
    return result;
}


/**
 * Writes sprites to streams.  The streams must already be open and pointing
 * to the location where you want to write the sprites and the sprites masks to.
 * The streams are not closed by this function so you have to do this yourself.
 * The function returns 1 if write was successfull and 0 if write failed.
 *
 * @param sprites
 *            The sprites to write
 * @param spritesStream
 *            The stream to write the sprites to
 * @param masksStream
 *            The stream to write the sprite masks to
 * @return 1 on success, 0 on failure
 */

int wlSpritesWriteStream(wlSpritesPtr sprites, FILE *spritesStream,
        FILE *masksStream)
{
    int x, y, bit, sprite, b;
    int pixel;
    
    for (sprite = 0; sprite < sprites->quantity; sprite++)
    {
        for (bit = 0; bit < 4; bit++)
        {
            for (y = 0; y < sprites->spriteHeight; y++)
            {
                for (x = 0; x < sprites->spriteWidth; x += 8)
                {
                    b = 0;
                    for (pixel = 0; pixel < 8; pixel++)
                    {
                        b |= ((sprites->pixels[sprite][y * sprites->spriteWidth +
                            x + pixel] >> bit) & 0x01) << (7 - pixel);
                    }
                    fputc(b, spritesStream);
                    
                    // Write transparancy information when last bit has been
                    // written
                    if (bit == 3)
                    {
                        b = 0;
                        for (pixel = 0; pixel < 8; pixel++)
                        {
                            b |= (sprites->pixels[sprite][y *
                                sprites->spriteWidth + x + pixel] >> 4) <<
                                (7 - pixel);
                        }
                        fputc(b, masksStream);
                    }
                }
            }
        }
    }
    return 1;
}
