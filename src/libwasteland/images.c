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
 * Creates a new image list with the specified number of images and the
 * specified image size. When you no longer need this
 * image list then you must release it with the wlImagesFree() function.
 * 
 * @param quantity
 *            The number of images
 * @param width
 *            The image width
 * @param height
 *            The image height
 * @return The new image list
 */

wlImages wlImagesCreate(int quantity, int width, int height)
{
	wlImages images;
	int i;
	
	assert(quantity > 0);
	assert(width > 0);
	assert(height > 0);
	images = (wlImages) malloc(sizeof(wlImagesStruct));
	images->quantity = quantity;
    images->images = (wlImage *) malloc(sizeof(wlImage) * quantity);
	for (i = 0; i < quantity; i++)
	{
	    images->images[i] = wlImageCreate(width, height);
	}
	return images;
}


/**
 * Releases all the memory allocated for the specified image list.
 * 
 * @param images
 *            The image list to free
 */

void wlImagesFree(wlImages images)
{
    int i;
    
    assert(images != NULL);
    for (i = 0; i < images->quantity; i++)
    {
        wlImageFree(images->images[i]);
    }
    free(images);
}
