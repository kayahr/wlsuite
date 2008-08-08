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
    int b1, b2, b3, b4;
    int size;

    // Validate parameters
    assert(stream != NULL);

    // Allocate header
    header = (wlMsqHeader) malloc(sizeof(wlMsqHeaderStruct));

    // Read the next four bytes and abort if one of them is EOF
    b1 = fgetc(stream);
    if (b1 == EOF) return NULL;
    b2 = fgetc(stream);
    if (b2 == EOF) return NULL;
    b3 = fgetc(stream);
    if (b3 == EOF) return NULL;
    b4 = fgetc(stream);
    if (b4 == EOF) return NULL;

    // Check for uncompressed MSQ block type
    if (b1 == 'm' && b2 == 's' && b3 == 'q' && (b4 == '0' || b4 == '1'))
    {
        header->disk = b4 - '0';
        header->size = 0;
        header->type = UNCOMPRESSED;
        return header;
    }

    // Assume the first four bytes are size information and read the next
    // four bytes
    size = b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
    b1 = fgetc(stream);
    if (b1 == EOF) return NULL;
    b2 = fgetc(stream);
    if (b2 == EOF) return NULL;
    b3 = fgetc(stream);
    if (b3 == EOF) return NULL;
    b4 = fgetc(stream);
    if (b4 == EOF) return NULL;

    // Check for compressed MSQ block type
    if (b1 == 'm' && b2 == 's' && b3 == 'q' && (b4 == 0 || b4 == 1))
    {
        header->disk = b4;
        header->size = size;
        header->type = COMPRESSED;
        return header;
    }

    // Check for CPA animation MSQ block type
    if (b1 == 0x08 && b2 == 0x67 && b3 == 0x01 && b4 == 0)
    {
        header->disk = b4;
        header->size = size;
        header->type = CPA_ANIMATION;
        return header;
    }

    // Give up, unknown MSQ block type
    wlError("Unknown MSQ block type: %i, %i, %i, %i", b1, b2, b3, b4);
    return NULL;
}
