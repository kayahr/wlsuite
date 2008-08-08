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
 * Reads all tilesets from the specified file and returns it. You have to
 * release the allocated memory of the returned data when you no longer need it
 * by using the wlTilesetsFree() function. If an error occurs while reading the
 * source file then NULL is returned and you can use errno to find the reason.
 *
 * A pixel in the returned array can be accessed like this:
 * tilesets->tilesets[tilesetNo]->images[tileNo]->pixels[y * 16 + x]
 * A pixel is an integer between 0 and 16. 0-15 is a color in the EGA color
 * palette, 16 is transparent.
 *
 * @param filename
 *            The filename of the tiles file to read
 * @return The tilesets or NULL if they could not be read
 */

wlTilesets wlTilesetsReadFile(char *filename)
{
    FILE *file;
    wlImages tiles;
    wlTilesets tilesets;

    // Validate parameters
    assert(filename != NULL);

    // Open the file for reading and abort if this fails
    file = fopen(filename, "rb");
    if (!file) return NULL;

    // Create the tilesets structure
    tilesets = (wlTilesets) malloc(sizeof(wlTilesetsStruct));
    tilesets->quantity = 0;
    tilesets->tilesets = NULL;

    // Read the tilesets
    while ((tiles = wlTilesReadStream(file)))
    {
        if (tilesets->quantity)
        {
            tilesets->tilesets = (wlImages *) realloc((tilesets->tilesets),
                    sizeof(wlImages *) * (tilesets->quantity + 1));
            tilesets->tilesets[tilesets->quantity] = tiles;
        }
        else
        {
            tilesets->tilesets = (wlImages * ) malloc(sizeof(wlImages *));
            tilesets->tilesets[0] = tiles;
        }
        tilesets->quantity++;
    }

    // Close the file stream
    fclose(file);

    // Return the tilesets
    return tilesets;
}

/**
 * Releases all the memory allocated for the specified tilesets.
 *
 * @param tilesets
 *            The tilesets to free
 */

void wlTilesetsFree(wlTilesets tilesets)
{
    int i;

    assert(tilesets != NULL);
    for (i = 0; i < tilesets->quantity; i++)
    {
        wlImagesFree(tilesets->tilesets[i]);
    }
    free(tilesets);
}


/**
 * Reads image from a huffman encoded file stream and returns it. The stream
 * must already be open and pointing to the encoded PIC data. The stream is not
 * closed by this function so you have to do this yourself. You have to
 * free the allocated memory for the returned image with the wlImageFree()
 * function when you no longer need it.
 *
 * If an error occurs while reading data from the stream then NULL is returned
 * and you can retrieve the problem source from errno.
 *
 * @param stream
 *            The stream to read from
 * @param image
 *            The image to put the pixels in
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The image
 */

static wlImage readTile(FILE *stream, wlImage image, wlHuffmanNode *rootNode,
        unsigned char *dataByte, unsigned char *dataMask)
{
    int x, y;
    int b;

    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x+= 2)
        {
            b = wlHuffmanReadByte(stream, rootNode, dataByte, dataMask);
            if (b == EOF) return NULL;
            image->pixels[y * image->width + x] = b >> 4;
            image->pixels[y * image->width + x + 1] = b & 0x0f;
        }
    }

    wlImageVXorDecode(image);
    return image;
}


/**
 * Reads a tileset from the specified file stream. The stream must already be
 * open and pointing to correct tileset. The stream is not closed by this
 * function so you have to do this yourself.
 *
 * You have to release the allocated memory of the returned array when you
 * no longer need it by using the wlImagesFree function. If an error occurs
 * while reading the source stream then NULL is returned and you can use
 * errno to find the reason.
 *
 * A pixel in the returned array can be accessed like this:
 * tileset->images[tileNo]->pixels[y * 16 + x]. A pixel is an integer
 * between 0 and 16. 0-15 is a color in the EGA color palette, 16 is
 * transparent.
 *
 * @param stream
 *            The stream to read the tiles from
 * @return The tiles as an array of pixels
 */

wlImages wlTilesReadStream(FILE *stream)
{
    wlImages tiles;
    wlImage tile;
    wlMsqHeader header;
    int quantity, i;
    unsigned char dataByte, dataMask;
    wlHuffmanNode *rootNode;

    // Validate parameters
    assert(stream != NULL);

    // Read and validate the MSQ header
    header = wlMsqReadHeader(stream);
    if (!header) return NULL;
    if (header->type != COMPRESSED)
    {
        wlError("Expected MSQ block of tileset to be compressed");
        return NULL;
    }

    // Calculate the number of tiles
    quantity = header->size * 2 / 16 / 16;

    // Free header structure
    free(header);

    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
        return NULL;

    // Create the images structure which is going to hold the tiles
    tiles = wlImagesCreate(quantity, 16, 16);

    // Read the tiles
    for (i = 0; i < quantity; i++)
    {
        tile = tiles->images[i];
        readTile(stream, tile, rootNode, &dataByte, &dataMask);
    }

    // Free huffman data
    wlHuffmanFreeNode(rootNode);

    // Return the tiles
    return tiles;
}
