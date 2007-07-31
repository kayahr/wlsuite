/*
 * $Id$
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
#include <wasteland.h>
#include "config.h"

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif

static int quantity = 10;
static int width = 16;
static int height = 16;


/**
 * Displays the usage text.
 */

static void display_usage(void) 
{
    printf("Usage: unpacksprites [OPTION]... SPRITESFILE MASKSFILE OUTPUTDIR\n");
    printf("Unpacks sprites into PNG images.\n");
    printf("\nOptions\n");
    printf("  -q, --quantity=NUMBER   The number of sprites (Default: %i)\n", quantity);
    printf("  -W, --width=NUMBER      The image width (Default: %i)\n", width);
    printf("  -H, --height=NUMBER     The image height (Default: %i)\n", height);
    printf("  -h, --help              Display help and exit\n");
    printf("  -V, --version           Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

static void display_version(void) 
{
    printf("unpacksprites %s\n", VERSION);
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
        {"quantity", 1, NULL, 'q'},
        {"width", 1, NULL, 'W'},
        {"height", 1, NULL, 'H'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'}
    };
    
    opterr = 0;
    while((opt = getopt_long(argc, argv, "q:W:H:hV", options, &index)) != -1)
    {
        switch(opt) 
        {
            case 'q':
                if (!optarg) die("Missing quantity argument\n");
                sscanf(optarg, "%i", &quantity);
                break;
                
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
 * Writes a single sprite into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param sprites
 *            The sprites
 * @param spriteNo
 *            The sprite number to write
 */

static void writePng(char *filename, wlSpritesPtr sprites, int spriteNo)
{
    gdImagePtr output;
    int x, y, i;
    int palette[17];
    int transparent;
    int color;
    FILE *file;

    output = gdImageCreate(sprites->spriteWidth, sprites->spriteHeight);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    transparent = gdImageColorAllocate(output, 0, 0, 0);
    gdImageColorTransparent(output, transparent);
    for (y = 0; y < sprites->spriteHeight; y++)       
    {
        for (x = 0; x < sprites->spriteWidth; x++)
        {
            color = sprites->pixels[spriteNo][y * sprites->spriteWidth + x];
            gdImageSetPixel(output, x, y, color < 16 ? palette[color] : transparent);
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
 * Writes all the sprites into the specified output directory.
 * 
 * @param outputDir
 *            The output directory
 * @param sprites
 *            The sprites to write
 */

static void writePngs(char *outputDir, wlSpritesPtr sprites)
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
    for (i = 0; i < sprites->quantity; i++)
    {
        sprintf(filename, "%i.png", i);
        writePng(filename, sprites, i); 
    }
    chdir(oldDir);
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
    char *spritesFilename, *masksFilename, *outputDir;
    wlSpritesPtr sprites;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 3) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    spritesFilename = argv[0];
    masksFilename = argv[1];
    outputDir = argv[2];
    
    /* Read the pic file */
    sprites = wlSpritesReadFile(spritesFilename, masksFilename, quantity,
            width, height);
    if (!sprites)
    {
        die("Unable to read sprites\n");
    }

    /* Write the PNG files */
    writePngs(outputDir, sprites);
    
    /* Free resources */
    wlSpritesDestroy(sprites);
    
    /* Success */
    return 0;
}
