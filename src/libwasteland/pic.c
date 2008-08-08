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
 * Reads image from the specified PIC file and returns them. You have to
 * free the allocated memory for the returned image with the wlImageFree()
 * function when you no longer need it.
 *
 * If the specified file could not be read then NULL is returned and you can
 * retrieve the problem source from errno.
 *
 * @param filename
 *            The filename of the pic file to read
 * @return The image
 */

wlImage wlPicReadFile(char *filename)
{
    FILE *file;
    wlImage image;

    assert(filename != NULL);
    file = fopen(filename, "rb");
    if (!file) return NULL;
    image = wlPicReadStream(file);
    fclose(file);
    return image;
}


/**
 * Reads image from a PIC file stream and returns them. The stream must
 * already be open and pointing to the PIC data. The stream is not closed by
 * this function so you have to do this yourself. You have to
 * free the allocated memory for the returned image with the wlImageFree()
 * function when you no longer need it.
 *
 * If an error occurs while reading data from the stream then NULL is returned
 * and you can retrieve the problem source from errno.
 *
 * @param stream
 *            The stream to read from
 * @return The image
 */

wlImage wlPicReadStream(FILE *stream)
{
    wlImage image;
    int x, y;
    int b;

    image = wlImageCreate(288, 128);
    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x+= 2)
        {
            b = fgetc(stream);
            if (b == EOF) return NULL;
            image->pixels[y * image->width + x] = b >> 4;
            image->pixels[y * image->width + x + 1] = b & 0x0f;
        }
    }

    wlImageVXorDecode(image);
    return image;
}


/**
 * Writes an image to a PIC file. The function returns 1 if write was successfull
 * and 0 if write failed. On failure you can check errno for the reason.
 *
 * @param image
 *            The image to write
 * @param filename
 *            The filename of the file to write the image to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteFile(wlImage image, char *filename)
{
    FILE *file;
    int result;

    assert(image != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlPicWriteStream(image, file);
    fclose(file);
    return result;
}


/**
 * Writes the specified image to a file stream. The stream must already be open
 * and pointing to the location where you want to write the pic to. The stream
 * is not closed by this function so you have to do this yourself. The function
 * returns 1 if write was successfull and 0 if write failed.
 *
 * @param image
 *            The image to write
 * @param stream
 *            The stream to write the image to
 * @return 1 on success, 0 on failure
 */

int wlPicWriteStream(wlImage image, FILE *stream)
{
    int x, y;
    int pixel;
    wlImage encodedImage;

    assert(image != NULL);
    assert(image->width % 2 == 0);
    assert(stream != NULL);

    /* Encode the pixels */
    encodedImage = wlImageClone(image);
    wlImageVXorEncode(encodedImage);

    /* Write encoded pixels to stream */
    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x += 2)
        {
            pixel = (encodedImage->pixels[y * image->width + x] << 4)
                | (encodedImage->pixels[y * image->width + x + 1] & 0x0f);
            if (fputc(pixel, stream) == EOF)
            {
            	wlImageFree(encodedImage);
            	return 0;
            }
        }
    }

    /* Release encoded pixels and report success */
    wlImageFree(encodedImage);
    return 1;
}
