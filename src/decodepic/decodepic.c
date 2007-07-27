/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <getopt.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include <errno.h>
#include <wasteland.h>
#include "config.h"

int width = 288;
int height = 128;


/**
 * Displays the usage text.
 */

void display_usage(void) 
{
    printf("Usage: decodepic [OPTION]... PICFILE PNGFILE\n");
    printf("Converts a wasteland PIC image file into a PNG image file.\n");
    printf("\nOptions\n");
    printf("  -W, --width=NUMBER   The image width (Default: %i)\n", width);
    printf("  -H, --height=NUMBER  The image height (Default: %i)\n", height);
    printf("  -h, --help           Display help and exit\n");
    printf("  -V, --version        Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

void display_version(void) 
{
    printf("decodepic %s\n", VERSION);
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

void die(char *message, ...)
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

void check_options(int argc, char *argv[])
{
    char opt;
    int index;
    static struct option options[]={
        {"width", 1, NULL, 'W'},
        {"height", 1, NULL, 'H'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'}
    };
    
    opterr = 0;
    while((opt = getopt_long(argc, argv, "W:H:hV", options, &index)) != -1)
    {
        switch(opt) 
        {
            case 'W':
                if (!optarg) die("Missing width argument\n");
                sscanf(optarg, "%i", &width);
                break;
                
            case 'H':
                if (!optarg) die("Missing height argument\n");
                sscanf(optarg, "%i", &height);
                break;
                
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
 * Writes the pic into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param pic
 *            The wasteland pic
 */

void writePng(char *filename, wlPicPtr pic)
{
    gdImagePtr output;
    int x, y, i;
    int palette[16];
    FILE *file;

    output = gdImageCreate(pic->width, pic->height);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    for (y = 0; y < pic->height; y++)       
    {
        for (x = 0; x < pic->width; x++)
        {
            gdImageSetPixel(output, x, y,
                    palette[pic->pixels[y * pic->width + x]]);
        }
    }    
    file = fopen(filename, "wb");
    if (!file)
    {
        die("Unable to write PNG to %s: %s\n", filename, strerror(errno));
    }
    gdImagePng(output, file);
    fclose(file);    
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
    wlPicPtr pic;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    source = argv[0];
    dest = argv[1];
    
    /* Read the pic file */
    pic = wlPicReadFile(source, width, height);
    if (!pic)
    {
        die("Unable to read PIC file\n");
    }

    /* Write the PNG file */
    writePng(dest, pic);
    
    /* Free resources */
    wlPicDestroy(pic);
    
    /* Success */
    return 0;
}
