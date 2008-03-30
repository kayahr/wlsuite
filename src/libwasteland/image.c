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
 * Creates a new image with the specified size. When you no longer need this
 * image then you must release it with the wlImageFree() function.
 * 
 * @param width
 *            The image width
 * @param height
 *            The image height
 * @return The new image
 */

wlImage wlImageCreate(int width, int height)
{
	wlImage image;
	
	assert(width > 0);
	assert(height > 0);
	image = (wlImage) malloc(sizeof(wlImageStruct));
	image->width = width;
	image->height = height;
	image->pixels = (wlPixel *) malloc(sizeof(wlPixel) * width * height);
	return image;
}


/**
 * Releases all the memory allocated for the specified image.
 * 
 * @param image
 *            The image to free
 */

void wlImageFree(wlImage image)
{
    assert(image != NULL);
    free(image->pixels);
    free(image);
}


/**
 * Clones the specified image. You must free the clone with wlImageFree()
 * when you no longer need it.
 * 
 * @param image
 *            The image to clone
 * @return The cloned image
 */

wlImage wlImageClone(wlImage image)
{
	wlImage clone;
	
	assert(image != NULL);
	clone = wlImageCreate(image->width, image->height);
	memcpy(clone->pixels, image->pixels, sizeof(wlPixel) * image->width * image->height);
	return clone;
}


/**
 * Performs a vertical xor encoding on the specified image.
 * 
 * @param image
 *            The image to encode
 */

void wlImageVXorEncode(wlImage image)
{
    wlVXorEncode(image->pixels, image->width, image->height);
}


/**
 * Performs a vertical xor decoding on the specified image.
 * 
 * @param image
 *            The image to decode
 */

void wlImageVXorDecode(wlImage image)
{
	wlVXorDecode(image->pixels, image->width, image->height);
}
