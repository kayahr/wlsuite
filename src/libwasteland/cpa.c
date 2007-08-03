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
 * Creates a new CPA animation container without any frames. The memory for the
 * base frame is already allocated. When you no longer need this container then
 * you must release it with the wlCpaFree() function.
 * 
 * @return The new CPA animation container
 */

wlCpaAnimation * wlCpaCreate()
{
    wlCpaAnimation *animation;
    
    animation = (wlCpaAnimation *) malloc(sizeof(wlCpaAnimation));
    animation->baseFrame = (wlImage) malloc(288 * 128 * sizeof(wlPixel));
    animation->quantity = 0;
    animation->frames = NULL;
    return animation;
}


/**
 * Releases all the memory allocated for the specified animation. This
 * includes all the frames (and the baseframe) and all update sequences in
 * the frames.
 * 
 * @param animation
 *            The animation to free
 */

void wlCpaFree(wlCpaAnimation *animation)
{
    int frame, update;
    
    assert(animation != NULL);
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
    
    assert(image != NULL);
    assert(frame != NULL);
    for (i = 0; i < frame->quantity; i++)
    {
        update = frame->updates[i];
        for (x = 0; x < 8; x++)
        {
            image[update->y * 288 + update->x + x] = update->pixels[x];
        }
    }
}


/**
 * Reads a CPA animation from the specified file and returns it. You have to
 * free the allocated memory for the returned animation data with wlCpaFree()
 * when you no longer need it
 * 
 * If the specified file could not be read then NULL is returned.
 *
 * @param filename
 *            The filename of the CPA animation to read
 * @return The CPA animation
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
 * Reads a CPA animation from the specified stream and returns it. You have to
 * free the allocated memory for the returned animation data with wlCpaFree()
 * when you no longer need it
 * 
 * If the specified stream could not be read then NULL is returned.
 *
 * @param filename
 *            The stream to read the CPA animation from
 * @return The CPA animation
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
    
    assert(stream != NULL);
    
    // Ignore first 8 bytes (Which is the size of the uncompressed base frame
    // block, the MSQ identifier and the disk number
    if (fseek(stream, 8, SEEK_CUR)) return NULL;
    
    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
        return NULL;    
        
    // Create the animation container
    animation = wlCpaCreate();
    
    // Read pixels from huffman stream
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x+= 2)
        {
            b = wlHuffmanReadByte(stream, rootNode, &dataByte, &dataMask);
            if (b == -1)
            {
                wlHuffmanFreeNode(rootNode);
                wlCpaFree(animation);
                return NULL;
            }
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
    if (fseek(stream, 8, SEEK_CUR))
    {
        wlCpaFree(animation);
        return NULL;
    }
    
    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
    {
        wlCpaFree(animation);
        return NULL;
    }
            
    // Skip the animation data size
    if (wlHuffmanReadWord(stream, rootNode, &dataByte, &dataMask) == -1)
    {
        wlHuffmanFreeNode(rootNode);
        wlCpaFree(animation);
        return NULL;
    }
    
    // Read frames until the animation size is reached
    while (1)
    {
        // Read delay value. If it's 0xffff then we reached the end of the
        // animation data
        delay = wlHuffmanReadWord(stream, rootNode, &dataByte, &dataMask);
        if (delay == -1)
        {
            wlHuffmanFreeNode(rootNode);
            wlCpaFree(animation);
            return NULL;
        }
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
            if (offset == -1)
            {
                wlHuffmanFreeNode(rootNode);
                wlCpaFree(animation);
                return NULL;
            }
            if (offset == 0xffff) break;
        
            // Read the update sequence
            update = (wlCpaUpdate *) malloc(sizeof(wlCpaUpdate));
            update->x = offset * 8 % 320;
            update->y = offset * 8 / 320;
            for (x = 0; x < 8; x += 2)
            {
                b = wlHuffmanReadByte(stream, rootNode, &dataByte, &dataMask);
                if (b == -1)
                {
                    wlHuffmanFreeNode(rootNode);
                    wlCpaFree(animation);
                    return NULL;
                }
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
