/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

/**
 * @name libwasteland
 *
 * This library is a collection of functions to deal with the data files
 * of the famous old role playing game Wasteland.
 *
 * @author Klaus Reimer <k@ailis.de>
 * @version 0.1
 */

#include <stdio.h>
#include <stdarg.h>
#include "wasteland.h"

wlRGB wlPalette[16] = {
        { red: 0x00, green: 0x00, blue: 0x00 },
        { red: 0x00, green: 0x00, blue: 0xaa },
        { red: 0x00, green: 0xaa, blue: 0x00 },
        { red: 0x00, green: 0xaa, blue: 0xaa },
        { red: 0xaa, green: 0x00, blue: 0x00 },
        { red: 0xaa, green: 0x00, blue: 0xaa },
        { red: 0xaa, green: 0x55, blue: 0x00 },
        { red: 0xaa, green: 0xaa, blue: 0xaa },
        { red: 0x55, green: 0x55, blue: 0x50 },
        { red: 0x55, green: 0x55, blue: 0xff },
        { red: 0x55, green: 0xff, blue: 0x55 },
        { red: 0x55, green: 0xff, blue: 0xff },
        { red: 0xff, green: 0x55, blue: 0x55 },
        { red: 0xff, green: 0x55, blue: 0xff },
        { red: 0xff, green: 0xff, blue: 0x55 },
        { red: 0xff, green: 0xff, blue: 0xff }
};


/**
 * Outputs a libwasteland error message.
 *
 * @param message
 *            The error message
 */

void wlError(char *message, ...)
{
    va_list args;

    va_start(args, message);
    fprintf(stderr, "libwasteland error: ");
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
    va_end(args);
}
