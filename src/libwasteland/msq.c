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
 * Reads a MSQ header from the specified stream. The returned structure must
 * be freed when it is no longer needed. If an error occurs while reading the
 * sream then NULL is returned and you can use errno to find the reason.
 *
 * @param stream
 *            The file stream to read the header from
 * @return The MSQ header structure or NULL if it could not be read.
 */

wlMsqHeader wlMsqReadHeader(FILE *stream)
{
    wlMsqHeader header;
    unsigned char b[4];
    int size;

    // Validate parameters
    assert(stream != NULL);

    // Read the next four bytes and abort if EOF was reached
    if (fread(b, 1, 4, stream) != 4) return NULL;

    // Check for uncompressed MSQ block type
    if (b[0] == 'm' && b[1] == 's' && b[2] == 'q' && (b[3] == '0' || b[3] == '1'))
    {
        header = malloc(sizeof(wlMsqHeaderStruct));
        header->disk = b[3] - '0';
        header->size = 0;
        header->type = UNCOMPRESSED;
        return header;
    }

    // Assume the first four bytes are size information and read the next
    // four bytes
    size = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
    if (fread(b, 1, 4, stream) != 4) return NULL;

    // Check for compressed MSQ block type
    if (b[0] == 'm' && b[1] == 's' && b[2] == 'q' && (b[3] == 0 || b[3] == 1))
    {
        header = malloc(sizeof(wlMsqHeaderStruct));
        header->disk = b[3];
        header->size = size;
        header->type = COMPRESSED;
        return header;
    }

    // Check for CPA animation MSQ block type
    if (b[0] == 0x08 && b[1] == 0x67 && b[2] == 0x01 && b[3] == 0)
    {
        header = malloc(sizeof(wlMsqHeaderStruct));
        header->disk = b[3];
        header->size = size;
        header->type = CPA_ANIMATION;
        return header;
    }

    // Give up, unknown MSQ block type
    wlError("Unknown MSQ block type: %i, %i, %i, %i", b[0], b[1], b[2], b[3]);
    return NULL;
}
