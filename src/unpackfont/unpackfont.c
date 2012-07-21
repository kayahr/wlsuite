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
    printf("Usage: wl_unpackfont [OPTION]... FONTFILE OUTPUTDIR\n");
    printf("Unpacks font into PNG images.\n");
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
    printf("wl_unpackfont %s\n", VERSION);
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
 * Writes a single font glyph into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param font
 *            The font
 * @param glyph
 *            The glyph number to write
 */

static void writePng(char *filename, wlImages font, int glyph)
{
    gdImagePtr output;
    int x, y, i;
    int palette[17];
    int transparent;
    int color;
    FILE *file;
    wlImage image;
    
    image = font->images[glyph];

    output = gdImageCreate(image->width, image->height);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    transparent = gdImageColorAllocate(output, 0, 0, 0);
    gdImageColorTransparent(output, transparent);
    for (y = 0; y < image->height; y++)       
    {
        for (x = 0; x < image->width; x++)
        {
            color = image->pixels[y * image->width + x];
            gdImageSetPixel(output, x, y, color < 16 ? palette[color] : transparent);
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
 * Writes all the font into the specified output directory.
 * 
 * @param outputDir
 *            The output directory
 * @param font
 *            The font to write
 */

static void writePngs(char *outputDir, wlImages font)
{
    int i;
    char *oldDir;
    char filename[6];
    
    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }
    for (i = 0; i < 172; i++)
    {
        sprintf(filename, "%03i.png", i);
        writePng(filename, font, i); 
    }
    if (chdir(oldDir))
    {
        die("Unable to change to directory %s: %s\n", oldDir,
                strerror(errno));
        return;
    }
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
    wlImages font;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    filename = argv[0];
    outputDir = argv[1];
    
    /* Read the pic file */
    font = wlFontReadFile(filename);
    if (!font)
    {
        die("Unable to read font from %s: %s\n", filename,
                strerror(errno));
    }

    /* Write the PNG files */
    writePngs(outputDir, font);
    
    /* Free resources */
    wlImagesFree(font);
    
    /* Success */
    return 0;
}
