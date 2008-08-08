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
 * @param width
 *            The animation width
 * @param height
 *            The animation height
 * @return The new CPA animation container
 */

wlCpaAnimation * wlCpaCreate(int width, int height)
{
    wlCpaAnimation *animation;

    animation = (wlCpaAnimation *) malloc(sizeof(wlCpaAnimation));
    animation->baseFrame = wlImageCreate(width, height);
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
            image->pixels[update->y * image->width + update->x + x] =
                update->pixels[x];
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
    int x, y, w, h;
    int b;
    unsigned char dataByte, dataMask;
    wlHuffmanNode *rootNode;
    wlCpaFrame *frame;
    wlCpaUpdate *update;
    int offset, delay;
    wlMsqHeader header;

    assert(stream != NULL);

    // Read and validate the MSQ header of the first MSQ block
    header = wlMsqReadHeader(stream);
    if (!header) return NULL;
    if (header->type != COMPRESSED)
    {
        wlError("Invalid MSQ header in first block. Not a CPA file?");
        return NULL;
    }
    free(header);

    // Initialize huffman stream
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
        return NULL;

    // Create the animation container
    animation = wlCpaCreate(288, 128);

    // Read pixels from huffman stream
    w = animation->baseFrame->width;
    h = animation->baseFrame->height;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x+= 2)
        {
            b = wlHuffmanReadByte(stream, rootNode, &dataByte, &dataMask);
            if (b == -1)
            {
                wlHuffmanFreeNode(rootNode);
                wlCpaFree(animation);
                return NULL;
            }
            animation->baseFrame->pixels[y * w + x] = b >> 4;
            animation->baseFrame->pixels[y * w + x + 1] = b & 0x0f;
        }
    }

    // Release resources
    wlHuffmanFreeNode(rootNode);

    // Decode baseframe (VXOR)
    wlImageVXorDecode(animation->baseFrame);

    // Read and validate the MSQ header of the second MSQ block
    header = wlMsqReadHeader(stream);
    if (!header)
    {
        wlCpaFree(animation);
        return NULL;
    }
    if (header->type != CPA_ANIMATION)
    {
        wlError("Invalid MSQ header in second block. Not a CPA file?");
        wlCpaFree(animation);
        return NULL;
    }
    free(header);

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
 * Writes a CPA animation to a file. The function returns 1 if write was
 * successfull and 0 if write failed. On failure you can check errno for the
 * reason.
 *
 * @param animation
 *            The CPA animation to write
 * @param filename
 *            The filename of the file to write the animation to
 * @return 1 on success, 0 on failure
 */

int wlCpaWriteFile(wlCpaAnimation *animation, char *filename)
{
    FILE *file;
    int result;

    assert(animation != NULL);
    assert(filename != NULL);
    file = fopen(filename, "wb");
    if (!file) return 0;
    result = wlCpaWriteStream(animation, file);
    fclose(file);
    return result;
}


/**
 * Creates the animation data block for the specified animation. The size of
 * the block is stored in the referenced <var>size</var> parameter.
 *
 * @param animation
 *            The CPA animation
 * @param size
 *            The size of the animation block is stored in this variable
 * @return The animation block
 */

static unsigned char * buildAnimationData(wlCpaAnimation *animation, int *size)
{
    int i, ptr, j, k;
    unsigned char *data;
    wlCpaFrame *frame;
    wlCpaUpdate *update;
    int offset;

    // Calculate the size of the animation data
    *size = 6;
    for (i = 0; i < animation->quantity; i++)
        *size += 4 + 6 * animation->frames[i]->quantity;

    // Allocate memory for the animation data
    data = (unsigned char *) malloc(*size);

    // Encode animation data
    ptr = 0;
    data[ptr++] = (*size - 4) & 0xff;
    data[ptr++] = (*size - 4) >> 8;
    for (i = 0; i < animation->quantity; i++)
    {
        frame = animation->frames[i];
        data[ptr++] = frame->delay & 0xff;
        data[ptr++] = frame->delay >> 8;
        for (j = 0; j < frame->quantity; j++)
        {
            update = frame->updates[j];
            offset = (update->y * 320 + update->x) / 8;
            data[ptr++] = offset & 0xff;
            data[ptr++] = offset >> 8;
            for (k = 0; k < 8; k += 2)
            {
                data[ptr++] = update->pixels[k] << 4 | update->pixels[k + 1];
            }
        }
        data[ptr++] = 0xff;
        data[ptr++] = 0xff;
    }
    data[ptr++] = 0xff;
    data[ptr++] = 0xff;
    data[ptr++] = 0;
    data[ptr++] = 0;

    return data;
}


/**
 * Writes a CPA animation to a stream. The function returns 1 if write was
 * successfull and 0 if write failed. On failure you can check errno for the
 * reason.
 *
 * @param animation
 *            The CPA animation to write
 * @param filename
 *            The stream to write the animation to
 * @return 1 on success, 0 on failure
 */

int wlCpaWriteStream(wlCpaAnimation *animation, FILE *stream)
{
    int x, y, i, size;
    wlPixel encodedPixels[288 * 128];
    unsigned char *data;
    wlHuffmanNode *rootNode;
    wlHuffmanNode **nodeIndex;
    unsigned char dataByte, dataMask;

    assert(animation != NULL);
    assert(stream != NULL);

    // Write the uncompressed picture size
    if (!wlWriteDWord(288 * 128 / 2, stream)) return 0;

    // Write the MSQ header
    if (fprintf(stream, "msq") != 3) return 0;
    if (fputc(0, stream) == EOF) return 0;

    // Encode the pixels of the base frame
    memcpy(encodedPixels, animation->baseFrame, sizeof(wlPixel) * 288 * 128);
    wlVXorEncode(encodedPixels, 288, 128);

    // Write encoded pixels to data block
    data  = (unsigned char *) malloc(288 * 128 / 2);
    for (y = 0; y < 128; y++)
    {
        for (x = 0; x < 288; x += 2)
        {
            data[y * 144 + x / 2] = (encodedPixels[y * 288 + x] << 4)
                | (encodedPixels[y * 288 + x + 1] & 0x0f);
        }
    }

    // Build the huffman tree and write it to the stream
    rootNode = wlHuffmanBuildTree(data, 288 * 128 / 2, &nodeIndex);
    dataByte = 0;
    dataMask = 0;
    if (!wlHuffmanWriteNode(rootNode, stream, &dataByte, &dataMask)) return 0;

    // Write encoded pixel data
    for (i = 0; i < 288 * 128 / 2; i++)
    {
        if (!wlHuffmanWriteByte(data[i], stream, nodeIndex, &dataByte,
                &dataMask)) return 0;
    }

    // Release the huffman tree and the node index and the base frame data
    wlHuffmanFreeNode(rootNode);
    free(nodeIndex);
    free(data);

    // Make sure last byte is written
    if (!wlFillByte(0, stream, &dataByte, &dataMask)) return 0;

    // Encode the animation data
    data = buildAnimationData(animation, &size);

    // Write the uncompressed picture size
    if (!wlWriteDWord(size, stream)) return 0;

    // Write the animation data header
    if (fputc(0x08, stream) == EOF) return 0;
    if (fputc(0x67, stream) == EOF) return 0;
    if (fputc(0x01, stream) == EOF) return 0;
    if (fputc(0x00, stream) == EOF) return 0;

    // Build huffman tree for animation data and write it to the stream
    rootNode = wlHuffmanBuildTree(data, size, &nodeIndex);
    dataByte = 0;
    dataMask = 0;
    if (!wlHuffmanWriteNode(rootNode, stream, &dataByte, &dataMask)) return 0;

    // Write encoded animation data
    for (i = 0; i < size; i++)
    {
        if (!wlHuffmanWriteByte(data[i], stream, nodeIndex, &dataByte,
                &dataMask)) return 0;
    }

    // Release the huffman tree and the node index and the animation data
    wlHuffmanFreeNode(rootNode);
    free(nodeIndex);
    free(data);

    // Make sure last byte is written
    if (!wlFillByte(0, stream, &dataByte, &dataMask)) return 0;

    // Report success
    return 1;
}


/**
 * Calculates the difference between the two specified frames and writes this
 * difference as an animation frame into the CPA animation. When encoding the
 * 12th frame you must also specify the LAST frame. That's because Wasteland
 * starts looping at this frame so we must also calculate the difference
 * between the last and the 12th frame.
 *
 * @param animation
 *            The CPA animation
 * @param frame
 *            The current frame
 * @param prevFrame
 *            The previous frame
 * @param lastFrame
 *            The last frame of the animation. Must be provided when the 12th
 *            frame is added. On all other calls this parameter must be NULL.
 * @param delay
 *            The delay before the new frame
 */

void wlCpaAddFrame(wlCpaAnimation *animation, wlImage curFrame,
    wlImage prevFrame, wlImage lastFrame, int delay)
{
    int x, y, changed, i, w, h;
    wlCpaFrame *frame;
    wlCpaUpdate *update;

    // Create the frame
    frame = (wlCpaFrame *) malloc(sizeof(wlCpaFrame));
    frame->delay = delay;
    frame->quantity = 0;
    frame->updates = NULL;
    animation->quantity++;
    animation->frames = realloc(animation->frames, sizeof(wlCpaFrame *) *
            animation->quantity);
    animation->frames[animation->quantity - 1] = frame;

    // Find out what is new in this frame
    w = animation->baseFrame->width;
    h = animation->baseFrame->height;
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x += 8)
        {
            changed = 0;
            for (i = 0; i < 8; i++)
            {
                if (prevFrame->pixels[y * w + x + i] !=
                    curFrame->pixels[y * w + x + i]) changed = 1;
                if (lastFrame && lastFrame->pixels[y * w + x + i]
                        != curFrame->pixels[y * w + x + i])
                    changed = 1;
            }
            if (changed)
            {
                update = (wlCpaUpdate *) malloc(sizeof(wlCpaUpdate));
                update->x = x;
                update->y = y;
                memcpy(update->pixels, &curFrame[y * 288 + x], 8);
                frame->quantity++;
                frame->updates = realloc(frame->updates, sizeof(wlCpaUpdate *)
                        * frame->quantity);
                frame->updates[frame->quantity - 1] = update;
            }
        }
    }
}
