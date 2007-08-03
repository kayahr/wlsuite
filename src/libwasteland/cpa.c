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
    

wlCpaAnimation * wlCpaCreate()
{
    wlCpaAnimation *animation;
    
    animation = (wlCpaAnimation *) malloc(sizeof(wlCpaAnimation));
    animation->baseFrame = (wlImage) malloc(288 * 128 * sizeof(wlPixel));
    return animation;
}


/**
 * Releases all the memory allocated for the specified animation. THis
 * includes all the frames (and the baseframe) and all update sequences in
 * the frames.
 * 
 * @param animation
 *            The animation to free
 */

void wlCpaFree(wlCpaAnimation *animation)
{
    int frame, update;
    
    for (frame = 0; frame < animation->quantity; frame++)
    {
        for (update = 0; update < animation->frames[frame]->quantity; update++)
        {
            free(animation->frames[frame]->updates[update]);
        }
        free(animation->frames[frame]->updates);
        free(animation->frames[frame]);
    }
    free(animation->frames);
    free(animation->baseFrame);
    free(animation);
}


/**
 * Applies a single CPA animation frame on the specified image.
 * 
 * @param image
 *           The image where you want the frame to apply
 * @param frame
 *           The CPA animation frame to apply
 */

void wlCpaApplyFrame(wlImage image, wlCpaFrame *frame)
{
    int i, x;
    wlCpaUpdate *update;
    
    for (i = 0; i < frame->quantity; i++)
    {
        update = frame->updates[i];
        for (x = 0; x < 8; x++)
        {
            fflush(stdout);
            image[update->y * 288 + update->x + x] = update->pixels[x];
        }
    }
}


/**
 * Reads pixels from the specified PIC file and returns them. You have to
 * free the allocated memory for the returned pixels when you no longer need
 * them.
 * 
 * If the specified file could not be read then NULL is returned and you can
 * retrieve the problem source from errno.
 *
 * @param filename
 *            The filename of the pic file to read
 * @return The pixels
 */

wlCpaAnimation * wlCpaReadFile(char *filename)
{
    FILE *file;
    wlCpaAnimation * animation;
    
    assert(filename != NULL);
    file = fopen(filename, "rb");
    if (!file) return NULL;
    animation = wlCpaReadStream(file);
    fclose(file);
    return animation;
}


/**
 * Reads pixels from a PIC file stream and returns them. The stream must
 * already be open and pointing to the PIC data. The stream is not closed by
 * this function so you have to do this yourself. You have to
 * free the allocated memory for the returned pixels when you no longer need
 * them. 
 * 
 * If an error occurs while reading data from the stream then NULL is returned
 * and you can retrieve the problem source from errno.
 *
 * @param stream
 *            The stream to read from
 * @return The pixels
 */

wlCpaAnimation * wlCpaReadStream(FILE *stream)
{
    wlCpaAnimation *animation;
    int x, y;
    int b;
    unsigned char dataByte, dataMask;
    wlHuffmanNode *rootNode;
    wlCpaFrame *frame;
    wlCpaUpdate *update;
    int offset, delay;
    
    animation = wlCpaCreate();
    animation->quantity = 0;
    animation->frames = NULL;
    
    // Ignore first 8 bytes (Which is the size of the uncompressed base frame
    // block, the MSQ identifier and the disk number
    fseek(stream, 8, SEEK_CUR);
    
    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask);
        
    // Read pixels from huffman stream
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x+= 2)
        {
            b = wlHuffmanReadByte(stream, rootNode, &dataByte, &dataMask);
            animation->baseFrame[y * 288 + x] = b >> 4;
            animation->baseFrame[y * 288 + x + 1] = b & 0x0f;
        }
    }
    
    // Release resources
    wlHuffmanFreeNode(rootNode);
    
    // Decode baseframe (VXOR)
    wlVXorDecode(animation->baseFrame, 288, 128);
    
    // Ignore next 8 bytes (Which is the size of the uncompressed animation
    // data, the next MSQ identifier and the disk number
    fseek(stream, 8, SEEK_CUR);
    
    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask);
            
    // Skip the animation data size
    wlHuffmanReadWord(stream, rootNode, &dataByte, &dataMask);
    
    // Read frames until the animation size is reached
    while (1)
    {
        // Read delay value. If it's 0xffff then we reached the end of the
        // animation data
        delay = wlHuffmanReadWord(stream, rootNode, &dataByte, &dataMask);
        if (delay == 0xffff) break;
        
        // Read animation frame
        frame = (wlCpaFrame *) malloc(sizeof(wlCpaFrame));        
        frame->delay = delay; 
        frame->quantity = 0;
        frame->updates = NULL;
        animation->quantity++;
        animation->frames = (wlCpaFrame **) realloc(animation->frames,
                sizeof(wlCpaFrame *) * animation->quantity);
        animation->frames[animation->quantity - 1] = frame;
        
        // Read animation frame update block until an offset of 0 has been read
        while (1)
        {
            offset = wlHuffmanReadWord(stream, rootNode, &dataByte, &dataMask);
            if (offset == 0xffff) break;
        
            // Read the update sequence
            update = (wlCpaUpdate *) malloc(sizeof(wlCpaUpdate));
            update->x = offset * 8 % 320;
            update->y = offset * 8 / 320;
            for (x = 0; x < 8; x += 2)
            {
                b = wlHuffmanReadByte(stream, rootNode, &dataByte, &dataMask);
                update->pixels[x] = b >> 4;
                update->pixels[x + 1] = b & 0x0f;            
            }
            frame->quantity++;
            frame->updates = (wlCpaUpdate **) realloc(frame->updates,
                    sizeof(wlCpaUpdate *) * frame->quantity);
            frame->updates[frame->quantity - 1] = update;
        }
    }
    
    // Release resources
    wlHuffmanFreeNode(rootNode);
    
    return animation;
}


/**
 * Writes pixels to a PIC file. The function returns 1 if write was successfull
 * and 0 if write failed. On failure you can check errno for the reason.
 *
 * @param pixels
 *            The pixels to write
 * @param filename
 *            The filename of the file to write the pixels to
 * @return 1 on success, 0 on failure
 */
/*
int wlCpaWriteFile(wlImage pixels, char *filename)
{
    FILE *file;
    int result;
    
    assert(pixels != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlPicWriteStream(pixels, file);
    fclose(file);
    return result;
}
*/

/**
 * Writes pixels to a file stream. The stream must already be open and pointing
 * to the location where you want to write the pic to. The stream is not 
 * closed by this function so you have to do this yourself. The function 
 * returns 1 if write was successfull and 0 if write failed.
 *
 * @param pixels
 *            The pixels to write
 * @param stream
 *            The stream to write the pixels to
 * @return 1 on success, 0 on failure
 */
/*
int wlCpaWriteStream(wlImage pixels, FILE *stream)
{
    int x, y;
    int pixel;
    wlPixel encodedPixels[288 * 128];
    
    assert(pixels != NULL);
    assert(stream != NULL);
    
    // Encode the pixels
    memcpy(encodedPixels, pixels, sizeof(wlPixel) * 288 * 128);
    wlVXorEncode(encodedPixels, 288, 128);
    
    // Write encoded pixels to stream 
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x += 2)
        {
            pixel = (encodedPixels[y * 288 + x] << 4)
                | (encodedPixels[y * 288 + x + 1] & 0x0f);
            if (fputc(pixel, stream) == EOF) return 0;
        }
    }
    
    // Release encoded pixels and report success
    return 1;
}
*/
