/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include <errno.h>
#include "../libwasteland/wasteland.h"
#include "config.h"

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif


/**
 * Displays the usage text.
 */

static void display_usage(void) 
{
    printf("Usage: wl_decodecpa [OPTION]... CPAFILE GIFFILE\n");
    printf("Decodes CPA animation file into an animated GIF image.\n");
    printf("\nOptions\n");
    printf("  -h, --help              Display help and exit\n");
    printf("  -V, --version           Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

static void display_version(void) 
{
    printf("wl_decodecpa %s\n", VERSION);
    printf("\n%s\n", COPYRIGHT);
    printf("This is free software; see the source for copying conditions. ");
    printf("There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS ");
    printf("FOR A PARTICULAR PURPOSE.\n\nWritten by %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Terminate the program with code 1 and the specified error message.
 *
 * @param message
 *            The error message
 */

static void die(char *message, ...)
{
    va_list args;
    
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    exit(1);
}


/**
 * Check options.
 *
 * @param argc
 *            The number of arguments
 * @param argv
 *            The argument array
 */

static void check_options(int argc, char *argv[])
{
    char opt;
    int index;
    static struct option options[]={
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'}
    };
    
    opterr = 0;
    while((opt = getopt_long(argc, argv, "hV", options, &index)) != -1)
    {
        switch(opt) 
        {
            case 'V':
                display_version();
                exit(1);
                break;
                
            case 'h':
                display_usage();
                exit(1);
                break;
                
            default:
                die("Unknown option: %s\nUse --help to show valid options.\n",
                        argv[optind - 1]);
                break;
        }
    }
}


/**
 * Creates a GD image from the specified wasteland image 
 * 
 * @param image
 *            The wasteland image
 * @return The GD image
 */

static gdImagePtr createImage(wlImage image)
{    
    gdImagePtr output;
    int x, y, i;

    output = gdImageCreate(288, 128);
    for (i = 0; i < 16; i++)
    {
        gdImageColorAllocate(output, wlPalette[i].red, wlPalette[i].green,
                wlPalette[i].blue);
    }
    gdImageColorTransparent(output, gdImageColorAllocate(output, 0, 0, 0));
    for (y = 0; y < 128; y++)       
    {
        for (x = 0; x < 288; x++)
        {
            gdImageSetPixel(output, x, y, image->pixels[y * 288 + x]);
        }
    }    
    return output;
}


/**
 * Writes animation data into the specified output directory.
 * 
 * @param outputDir
 *            The output directory
 * @param animation
 *            The CPA animation
 */

static void writeGif(char *filename, wlCpaAnimation *animation)
{
    int i;
    wlImage frame;
    gdImagePtr image, prevImage; 
    FILE *file;
    
    file = fopen(filename, "wb");
    
    // Create a copy of the base frame
    frame = (wlImage) malloc(sizeof(wlPixel) * 288 * 128);
    memcpy(frame, animation->baseFrame, sizeof(wlPixel) * 288 * 128);
    
    image = createImage(frame);
    prevImage = NULL;
    gdImageGifAnimBegin(image, file, 1, -1);
    gdImageGifAnimAdd(image, file, 0, 0, 0, animation->frames[0]->delay * 8,
            gdDisposalNone, NULL);
    
    // Cycle through all animation frames, apply the frame updates to our frame
    // and then write the frame PNG
    for (i = 0; i < animation->quantity; i++)
    {
        wlCpaApplyFrame(frame, animation->frames[i]);
        if (prevImage) gdImageDestroy(prevImage);
        prevImage = image;
        image = createImage(frame); 
        gdImageGifAnimAdd(image, file, 0, 0, 0,
                (i + 1 == animation->quantity) ? 0 :
                    animation->frames[i + 1]->delay * 8,
                gdDisposalNone, prevImage);
    }
    gdImageGifAnimEnd(file);
    fclose(file);
    
    // Free resources
    gdImageDestroy(prevImage);
    gdImageDestroy(image);
    free(frame);
}


/**
 * Main method
 *
 * @param argc
 *            The number of arguments
 * @param argv
 *            The argument array
 * @return Exit value
 */

int main(int argc, char *argv[])
{  
    char *source, *dest;
    wlCpaAnimation *animation;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    source = argv[0];
    dest = argv[1];
    
    /* Read the animation */
    animation = wlCpaReadFile(source);
    if (!animation)
    {
        die("Unable to read CPA animation from %s: %s\n", source,
                strerror(errno));
    }

    /* Create and write the GIF file */
    writeGif(dest, animation);
    
    /* Free resources */
    wlCpaFree(animation);
    
    /* Success */
    return 0;
}
