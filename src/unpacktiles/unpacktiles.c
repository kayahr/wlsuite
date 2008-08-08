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
#include <math.h>
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
    printf("Usage: wl_unpacktiles [OPTION]... HTDS-FILE OUTPUTDIR\n");
    printf("Unpacks the tiles into PNG images.\n");
    printf("\nOptions\n");
    printf("  -h, --help      Display help and exit\n");
    printf("  -V, --version   Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

static void display_version(void)
{
    printf("wl_unpacktiles %s\n", VERSION);
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
 * Writes a single tile into the specified file in PNG format.
 *
 * @param filename
 *            The output filename
 * @param tiles
 *            The tiles
 * @param tileNo
 *            The tile number to write
 */

static void writePng(char *filename, wlImages tiles, int tileNo)
{
    gdImagePtr output;
    int x, y, i;
    int palette[17];
    int transparent;
    int color;
    FILE *file;
    wlImage image;

    image = tiles->images[tileNo];
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
 * Writes all the tiles into the specified output directory.
 *
 * @param outputDir
 *            The output directory
 * @param tiles
 *            The tiles to write
 */

static void writePngs(char *outputDir, wlImages tiles)
{
    int i;
    char *oldDir;
    char filename[8];
    char format[9];

    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }
    sprintf(format, "%%0%ii.png", (int) log10(tiles->quantity) + 1);
    for (i = 0; i < tiles->quantity; i++)
    {
        sprintf(filename, format, i);
        writePng(filename, tiles, i);
    }
    chdir(oldDir);
    free(oldDir);
}




/**
 * Writes all the tilesets into the specified output directory.
 *
 * @param outputDir
 *            The output directory
 * @param tilesets
 *            The tilesets to write
 */

static void writeTilesets(char *outputDir, wlTilesets tilesets)
{
    int i;
    char *oldDir;
    char filename[5];
    char format[4];

    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }
    sprintf(format, "%%0%ii", (int) log10(tilesets->quantity) + 1);
    for (i = 0; i < tilesets->quantity; i++)
    {
        sprintf(filename, format, i);
        mkdir(filename, 0755);
        writePngs(filename, tilesets->tilesets[i]);
    }
    chdir(oldDir);
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
    wlTilesets tilesets;

    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;

    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    filename = argv[0];
    outputDir = argv[1];

    /* Read the tilesets */
    tilesets = wlTilesetsReadFile(filename);
    if (!tilesets)
    {
        die("Unable to read tilesets from %s\n", filename, strerror(errno));
    }

    /* Write the PNG files */
    writeTilesets(outputDir, tilesets);

    /* Free resources */
    wlTilesetsFree(tilesets);

    /* Success */
    return 0;
}
