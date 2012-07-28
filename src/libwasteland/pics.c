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
#include "../common/list.h"
#include "wasteland.h"


/**
 * Reads animated pictures from the specified file and returns it. You have to
 * release the allocated memory of the returned data with the wlAnimationsFree()
 * function when you no longer need it. If an error occurs while
 * reading the source files then NULL is returned and you can use errno to
 * find the reason.
 *
 * @param filename
 *            The filename of the PICS file to read
 * @return The animated pictures or NULL if an error occured
 */

wlPicsAnimations wlAnimationsReadFile(char *filename)
{
    FILE *stream;
    wlPicsAnimations animations;
    wlPicsAnimation animation;

    assert(filename != NULL);
    stream = fopen(filename, "rb");
    if (!stream) return NULL;

    // Create the animations structure
    animations = malloc(sizeof(wlPicsAnimationsStruct));
    listCreate(animations->animations, &animations->quantity);

    // Read the animations
    while ((animation = wlAnimationReadStream(stream)))
    {
        listAdd(animations->animations, animation, &animations->quantity);
    }

    fclose(stream);
    return animations;
}


/**
 * Reads base frame from a PICS file stream and returns it. The stream must
 * already be open and pointing to the base MSQ block data. The stream is not
 * closed by this function so you have to do this yourself. You have to
 * free the allocated memory for the returned image with the wlImageFree()
 * function when you no longer need it.
 *
 * If an error occurs while reading data from the stream then NULL is returned
 * and you can retrieve the problem source from errno.
 *
 * @param stream
 *            The stream to read from
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dataMask
 *            Storage for last bit mask
 * @return The image
 */

static wlImage readBaseFrame(FILE *stream, wlHuffmanNode *rootNode,
        unsigned char *dataByte, unsigned char *dataMask)
{
    wlImage image;
    int x, y;
    int b;

    image = wlImageCreate(96, 84);
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
 * Reads the animation instructions from the specified stream. May return
 * NULL if something went wrong.
 *
 * @param stream
 *            The stream to read from
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dataMask
 *            Storage for last bit mask
 * @return The animation instructions or NULL if an error occured.
 */

static wlPicsInstructions readInstructions(FILE *stream,
    wlHuffmanNode *rootNode, unsigned char *dataByte, unsigned char *dataMask)
{
    wlPicsInstructions instructions;
    int size, i;
    unsigned char *data;
    wlPicsInstruction instruction;
    wlPicsInstructionSet set;

    // Read the raw animation data
    size = wlHuffmanReadWord(stream, rootNode, dataByte, dataMask);
    if (size == -1) return NULL;
    data = wlHuffmanReadBlock(stream, NULL, size, rootNode, dataByte, dataMask);
    if (data == NULL) return NULL;

    // Initializes instructions structure
    instructions = malloc(sizeof(wlPicsInstructionsStruct));
    listCreate(instructions->sets, &instructions->quantity);

    // Fill in the instructions
    set = NULL;
    i = 0;
    while (i < size)
    {
        // If 0xff is encountered then the end of an instruction set was found
        // so store the last set if there was one
        if (data[i] == 0xff)
        {
            if (set)
            {
                listAdd(instructions->sets, set, &instructions->quantity);
                set = NULL;
            }
            i++;
            continue;
        }

        // Create instruction set if not already done
        if (!set)
        {
            set = malloc(sizeof(wlPicsInstructionSetStruct));
            listCreate(set->instructions, &set->quantity);
        }

        // Create instruction
        instruction = malloc(sizeof(wlPicsInstructionStruct));
        instruction->delay = data[i];
        instruction->update = data[i + 1];
        listAdd(set->instructions, instruction, &set->quantity);
        i += 2;
    }

    // Free allocated temporary memory
    free(data);

    // Return the animation instructions
    return instructions;
}


/**
 * Reads the animation updates from the specified stream. May return NULL if
 * something went wrong.
 *
 * @param stream
 *            The stream to read from
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dataMask
 *            Storage for last bit mask
 * @return The animation instructions or NULL if an error occured.
 */

static wlPicsUpdates readUpdates(FILE *stream,
    wlHuffmanNode *rootNode, unsigned char *dataByte, unsigned char *dataMask)
{
    wlPicsUpdates updates;
    wlPicsUpdateSet set;
    wlPicsUpdate update;
    int size, i, len, tmp, j;
    unsigned char *data;

    // Read the raw animation data
    size = wlHuffmanReadWord(stream, rootNode, dataByte, dataMask);
    if (size == -1) return NULL;
    data = wlHuffmanReadBlock(stream, NULL, size, rootNode, dataByte, dataMask);
    if (data == NULL) return NULL;

    // Initializes the updates structure
    updates = malloc(sizeof(wlPicsUpdatesStruct));
    listCreate(updates->sets, &updates->quantity);

    // Process the data
    set = NULL;
    i = 0;
    while (i < size)
    {
        // If next two bytes are 0xff then we reached the end of an update
        // block so store the last update block if there was one
        if (data[i] == 0xff && data[i + 1] == 0xff)
        {
            // There is one special update block in allpics2 picture 22 which
            // is empty. So we have to create an empty update block here.
            if (!set)
            {
                set = malloc(sizeof(wlPicsUpdateSetStruct));
                listCreate(set->updates, &set->quantity);
            }
            listAdd(updates->sets, set, &updates->quantity);
            set = NULL;
            i += 2;
            continue;
        }

        // Create update set if not already done
        if (!set)
        {
            set = malloc(sizeof(wlPicsUpdateSetStruct));
            listCreate(set->updates, &set->quantity);
        }

        // Read the length and the position of the update
        len = (data[i + 1] >> 4) + 1;
        tmp = ((data[i + 1] & 15) << 8) + data[i];
        i += 2;

        // Create the update structure
        update = malloc(sizeof(wlPicsUpdateStruct));
        update->quantity = len * 2;
        update->x = (tmp * 2) % 96;
        update->y = (tmp * 2) / 96;
        update->pixelXORs = malloc(sizeof(unsigned char) * len * 2);
        for (j = 0; j < len; j++)
        {
            update->pixelXORs[j * 2] = (data[i] & 0xf0) >> 4;
            update->pixelXORs[j * 2 + 1] = data[i] & 0xf;
            i++;
        }
        listAdd(set->updates, update, &set->quantity);
    }

    // Free allocated temporary memory
    free(data);

    // Return the animation instructions
    return updates;
}


/**
 * Reads a single picture animation from the specified file stream.
 * The stream must already be open and pointing to the picture data. The stream
 * is not closed by this function so you have to do this yourself.
 *
 * You have to release the allocated memory of the returned structure with the
 * wlAnimationFree() function when you no longer need it. If an error occurs
 * while reading the stream then NULL is returned and you can use errno to
 * find the reason.
 *
 * @param stream
 *            The stream to read the picture animation from.
 * @return The picture animation or NULL if an error occured
 */

wlPicsAnimation wlAnimationReadStream(FILE *stream)
{
    wlPicsAnimation animation;
    wlMsqHeader header;
    unsigned char dataByte, dataMask;
    wlHuffmanNode *rootNode;

    // Validate parameters
    assert(stream != NULL);

    // Read and validate first MSQ header
    header = wlMsqReadHeader(stream);
    if (!header) return NULL;
    free(header);

    // Initialize huffman stream for base frame
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
        return NULL;

    // Initialize animation data structure and read base frame
    animation = (wlPicsAnimation) malloc(sizeof(wlPicsAnimationStruct));
    animation->baseFrame = readBaseFrame(stream, rootNode, &dataByte, &dataMask);

    // Free huffman data
    wlHuffmanFreeNode(rootNode);

    // Abort if no base frame was read
    if (!animation->baseFrame)
    {
        wlAnimationFree(animation);
        return NULL;
    }

    // Read and validate second MSQ header
    header = wlMsqReadHeader(stream);
    if (!header) return NULL;
    free(header);

    // Initialize huffman stream for animation data
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stream, &dataByte, &dataMask)))
        return NULL;

    // Read the animation instructions
    animation->instructions = readInstructions(stream, rootNode, &dataByte, &dataMask);

    // Read the animation updates
    animation->updates = readUpdates(stream, rootNode, &dataByte, &dataMask);

    // Free huffman data
    wlHuffmanFreeNode(rootNode);

    // Return the animation
    return animation;
}


/**
 * Releases all the memory allocated for the specified animations.
 *
 * @param animations
 *            The animations to free
 */

void wlAnimationsFree(wlPicsAnimations animations)
{
    int i;

    assert(animations != NULL);
    for (i = 0; i < animations->quantity; i++)
    {
        wlAnimationFree(animations->animations[i]);
    }
    free(animations->animations);
    free(animations);
}


/**
 * Releases all the memory allocated for the specified animation.
 *
 * @param animation
 *            The animation to free
 */

void wlAnimationFree(wlPicsAnimation animation)
{
    int i, j;
    wlPicsInstructionSet instructionSet;
    wlPicsUpdateSet updateSet;

    wlImageFree(animation->baseFrame);
    for (i = 0; i < animation->instructions->quantity; i++)
    {
        instructionSet = animation->instructions->sets[i];
        listFreeWithItems(instructionSet->instructions, &instructionSet->quantity);
        free(instructionSet);
    }
    free(animation->instructions->sets);
    free(animation->instructions);

    for (i = 0; i < animation->updates->quantity; i++)
    {
        updateSet = animation->updates->sets[i];
        for (j = 0; j < updateSet->quantity; j++)
            free(updateSet->updates[j]->pixelXORs);
        listFreeWithItems(updateSet->updates, &updateSet->quantity);
        free(updateSet);
    }
    free(animation->updates->sets);
    free(animation->updates);
    free(animation);
}


/**
 * Applies an animation update set onto the specified image;
 *
 * @param image
 *            The image to apply the animation update set to
 * @param set
 *            The animation update set to apply
 */

void wlAnimationApply(wlImage image, wlPicsUpdateSet set)
{
    int i, j;
    wlPicsUpdate update;

    assert(image != NULL);
    assert(set != NULL);
    for (i = 0; i < set->quantity; i++)
    {
        update = set->updates[i];
        for (j = 0; j < update->quantity; j++)
        {
            image->pixels[(update->x + j) + update->y * image->width] ^=
                update->pixelXORs[j];
        }
    }
}
