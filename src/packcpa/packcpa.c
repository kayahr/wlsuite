/*
 * $Id:packsprites.c 231 2007-07-28 22:25:22Z k $
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include <errno.h>
#include <dirent.h>
#include <wasteland.h>
#include <strutils/str.h>
#include <strutils/strlist.h>
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
    printf("Usage: wl_packcpa [OPTION]... INPUTDIR CPAFILE\n");
    printf("Packs PNG files into CPA animation.\n");
    printf("\nThe first 8 PNG files found in INPUTDIR are used "
            "(Alphabetically sorted).\nSize and colors doesn't matter because "
            "the images are automatically converted.\n");
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
    printf("wl_packcpa %s\n", VERSION);
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
        }
    }
}


/**
 * Reads GD image from the specified PNG and converts it into a wasteland
 * image.
 * 
 * @param filename
 *            The PNG filename
 * return The wasteland image
 */

static wlImage readImage(char *filename)
{
    gdImagePtr image;
    gdImagePtr output;
    wlImage result;
    int x, y, i;
    FILE *file;
        
    // Read base frame
    file = fopen(filename, "rb");
    if (!file)
    {
        die("Unable to read PNG from %s: %s\n", filename, strerror(errno));
    }
    image = gdImageCreateFromPng(file);
    fclose(file);

    // Create a temporary second image for palette conversion
    output = gdImageCreate(288, 128);
    for (i = 0; i < 16; i++)
    {
        gdImageColorAllocate(output, wlPalette[i].red, wlPalette[i].green,
                wlPalette[i].blue);
    }
    for (i = 16; i < 256; i++) gdImageColorAllocate(output, 0, 0, 0);
    gdImageCopyResampled(output, image, 0, 0, 0, 0, 288, 128, gdImageSX(image),
            gdImageSY(image));
    
    // Copy pixels from image to pic
    result = (wlImage) malloc(sizeof(wlPixel) * 288 * 128);
    for (y = 0; y < 128; y++)       
    {
        for (x = 0; x < 288; x++)
        {
            result[y * 288 + x] = gdImageGetPixel(output, x, y);
        }
    }
    
    // Free resources and return result image
    gdImageDestroy(image);
    gdImageDestroy(output);
    return result;
}


/**
 * Used by qsort for sorting filenames.
 */
 
static int sortFilenames(const void *p1, const void *p2)
{
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}


/**
 * Reads all animation frames (All PNG files in alphabetical order) from the
 * specified input directory, converts them into wasteland CPA animation and
 * returns the animation container.
 * 
 * @param inputDir
 *            The input directory
 * @return The wasteland CPA animation container 
 */

static wlCpaAnimation *readAnimation(char *inputDir)
{
    char *oldDir;
    DIR *dir;
    struct dirent *entry;
    char **filenames;
    int quantity;
    wlCpaAnimation *animation;

    // Change to input directory but remember current directory
    oldDir = getcwd(NULL, 0);
    if (chdir(inputDir))
    {
        die("Unable to change to input directory %s: %s\n", inputDir,
                strerror(errno));
        return NULL;
    }
    
    // Build list of PNG files found in directory
    filenames = strListCreate();
    dir = opendir(inputDir);
    while ((entry = readdir(dir)))
    {
        if (strEndsWithIgnoreCase(entry->d_name, ".png"))
        {
            strListAdd(&filenames, strdup(entry->d_name));
        }
    }
    closedir(dir);
    quantity = strListSize(filenames);
    qsort(filenames, quantity, sizeof(char *), sortFilenames);
    
    // Build the animation container
    animation = wlCpaCreate();
    animation->baseFrame = readImage(filenames[0]);

    strListFreeWithItems(filenames);
    
    // Go back to previous directory and then return the animation
    chdir(oldDir);
    free(oldDir);
    return animation;
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
    char *filename, *inputDir;
    wlCpaAnimation *animation;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    inputDir = argv[0];
    filename = argv[1];
  
    /* Read animation from PNG files */
    animation = readAnimation(inputDir);
    
    /* Write the cursors */
    if (!wlCpaWriteFile(animation, filename))
    {
        die("Write to animation file %s failed: %s\n", filename, strerror(errno));
    }
    
    /* Free memory */
    wlCpaFree(animation);
        
    /* Success */
    return 0;
}
