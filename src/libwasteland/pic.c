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
 * Creates a new pic with the specified dimensions. Returns a pointer
 * to a newly allocated wlPic structure. You have to free this
 * structure with wlPicDestroy() when you no longer need it.
 *
 * @param width
 *            The picture width
 * @param height
 *            The picture height
 */

wlPicPtr wlPicCreate(int width, int height)
{
    wlPicPtr pic;
    
    pic = malloc(sizeof(wlPic));
    pic->width = width;
    pic->height = height;
    pic->pixels = malloc(width * height * sizeof(unsigned char));
    return pic;
}


/**
 * Frees the memory allocated by the specicfied pic.
 *
 * @param pic
 *            The pic to free
 */
 
void wlPicDestroy(wlPicPtr pic)
{
    free(pic->pixels);
    free(pic);
}


/**
 * Clones the specified pic and returns the cloned pic. The clone is a
 * newly allocated wlPic structure and the pointer to this structure is
 * returned. You have to free this structure with wlPicDestroy() when you
 * no longer need it.
 *
 * @param pic
 *            The pic to clone
 * @return The clone
 */

wlPicPtr wlPicClone(wlPicPtr pic)
{
    wlPicPtr clone;
    
    clone = wlPicCreate(pic->width, pic->height);
    memcpy(clone->pixels, pic->pixels, pic->width * pic->height);
    return clone;
}                
    

/**
 * Reads a pic from the specified file and returns it as a wlPic
 * struct. You have to specify width and height of the pic because this
 * information can't be read safely from the pic file.
 *
 * The function returns a pointer to a newly created wlPic structure.
 * If you no longer need this structure you must free it with the function
 * wlPicDestroy().
 *
 * @param filename
 *            The filename of the picpic file to read
 * @param width
 *            The width of the pic
 * @param height
 *            The height of the pic
 * @return The pic
 */

wlPicPtr wlPicReadFile(char *filename, int width, int height)
{
    FILE *file;
    wlPicPtr pic;
    
    file = fopen(filename, "rb");
    if (!file)
    {
        wlError("Unable to open '%s' for reading: %s\n", filename,
            strerror(errno));
        return NULL;
    }
    pic = wlPicReadStream(file, width, height);
    fclose(file);
    return pic;
}


/**
 * Reads a PIC from a stream. The stream must already be open and pointing
 * to the PIC data. The stream is not closed by this function so you have to
 * do this yourself.
 *
 * The function returns a pointer to a newly created wlPic structure.
 * If you no longer need this structure you must free it with the function
 * wlPicDestroy().
 *
 * @param stream
 *            The stream to read from
 * @param width
 *            The width of the pic
 * @param height
 *            The height of the pic
 * @return The pic
 */

wlPicPtr wlPicReadStream(FILE *stream, int width, int height)
{
    wlPicPtr pic;
    int x, y;
    int b;
    
    pic = wlPicCreate(width, height);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x+= 2)
        {
            if (feof(stream))
            {
                wlError("Unexpected end of PIC stream at offset %i\n",
                    y * height + x);
                return NULL;
            }
            b = fgetc(stream);
            pic->pixels[y * width + x] = b >> 4;
            pic->pixels[y * width + x + 1] = b & 0x0f;
        }
    }
    wlVXorDecode(pic->pixels, width, height);
    return pic;
}


/**
 * Writes a pic to a file. The function returns 1 if write was successfull
 * and 0 if write failed.
 *
 * @param pic
 *            The pic to write
 * @param filename
 *            The filename of the file to write the pic to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteFile(wlPicPtr pic, char *filename)
{
    FILE *file;
    int result;
    
    file = fopen(filename, "wb");
    if (!file)
    {
        wlError("Unable to open '%s' for writing: %s\n", filename,
            strerror(errno));
        return 0;
    }
    result = wlPicWriteStream(pic, file);
    fclose(file);
    return result;
}


/**
 * Writes a pic to a stream.  The stream must already be open and pointing
 * to the location where you want to write the pic to. The stream is not 
 * closed by this function so you have to do this yourself. The function 
 * returns 1 if write was successfull and 0 if write failed.
 *
 * @param pic
 *            The pic to write
 * @param stream
 *            The stream to write the pic to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteStream(wlPicPtr pic, FILE *stream)
{
    int x, y;
    int pixel;
    wlPicPtr tmp;
    
    tmp = wlPicClone(pic);
    wlVXorEncode(tmp->pixels, tmp->width, tmp->height);
    for (y = 0; y < pic->height; y++)
    {
        for (x = 0; x < pic->width; x += 2)
        {
            pixel = (tmp->pixels[y * tmp->width + x] << 4)
                | (tmp->pixels[y * tmp->width + x + 1] & 0x0f);
            fputc(pixel, stream);
        }
    }
    wlPicDestroy(tmp);
    return 1;
}
