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
#include <wasteland.h>
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
    printf("Usage: wl_unpackcpa [OPTION]... CPAFILE OUTPUTDIR\n");
    printf("Unpacks CPA animation file into PNG images.\n");
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
    printf("wl_unpackcpa %s\n", VERSION);
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
 * Writes a single image into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param image
 *            The image to write
 */

static void writePng(char *filename, wlImage image)
{    
    gdImagePtr output;
    int x, y, i;
    int palette[17];
    FILE *file;

    output = gdImageCreate(288, 128);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    for (y = 0; y < 128; y++)       
    {
        for (x = 0; x < 288; x++)
        {
            gdImageSetPixel(output, x, y, image->pixels[y * 288 + x]);
        }
    }    
    file = fopen(filename, "wb");
    if (!file)
    {
        die("Unable to write PNG to %s: %s\n", filename, strerror(errno));
    }
    gdImagePng(output, file);
    gdImageDestroy(output);
    fclose(file);    
}


/**
 * Writes animation data into the specified output directory.
 * 
 * @param outputDir
 *            The output directory
 * @param animation
 *            The CPA animation
 */

static void writePngs(char *outputDir, wlCpaAnimation *animation)
{
    int i;
    char *oldDir;
    char filename[6];
    wlImage frame;
    FILE *delays;
    
    // Remember current directory and then switch to output directory
    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }    
    
    // Create a copy of the base frame
    frame = (wlImage) malloc(sizeof(wlPixel) * 288 * 128);
    memcpy(frame, animation->baseFrame, sizeof(wlPixel) * 288 * 128);
    
    // Write the base frame PNG
    writePng("00.png", frame);
    
    delays = fopen("delays.txt", "wt");
    fprintf(delays, "# The delays between the animation frames (0-65534)\n\n");
    
    // Cycle through all animation frames, apply the frame updates to our frame
    // and then write the frame PNG
    for (i = 0; i < animation->quantity; i++)
    {
        wlCpaApplyFrame(frame, animation->frames[i]);
        sprintf(filename, "%02i.png", i + 1);
        writePng(filename, frame);
        fprintf(delays, "%5i\n", animation->frames[i]->delay);
    }
    
    fclose(delays);
        
    // Go back to old directorry
    chdir(oldDir);
    
    // Free resources
    free(frame);
    free(oldDir);
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
    char *filename, *outputDir;
    wlCpaAnimation *animation;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    filename = argv[0];
    outputDir = argv[1];
    
    /* Read the animation */
    animation = wlCpaReadFile(filename);
    if (!animation)
    {
        die("Unable to read CPA animation from %s: %s\n", filename,
                strerror(errno));
    }

    /* Write the PNG files */
    writePngs(outputDir, animation);
    
    /* Free resources */
    wlCpaFree(animation);
    
    /* Success */
    return 0;
}
