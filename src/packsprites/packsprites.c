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
    printf("Usage: packsprites [OPTION]... INPUTDIR SPRITESFILE MASKSFILE\n");
    printf("Packs PNG files into sprites.\n");
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
    printf("packsprites %s\n", VERSION);
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
 * Converts image into a wasteland sprite and stores it in the specified
 * sprites container at the specified index.
 * 
 * @param sprites
 *            The sprites container
 * @param index
 *            The sprite index
 * @param image
 *            The image
 */

static void storeSprite(wlSpritesPtr sprites, int index, gdImagePtr image)
{
    gdImagePtr output;
    int x, y, i, width, height;
    int palette[16], transparency;
    
    width = sprites->spriteWidth;
    height = sprites->spriteHeight;

    /* Create a temporary second image for palette conversion */
    output = gdImageCreate(width, height);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    transparency = gdImageColorAllocate(output, 45, 100, 160);
    gdImageColorTransparent(output, transparency);
     for (i = 17; i < 256; i++) gdImageColorAllocate(output, 0, 0, 0);
    gdImageFilledRectangle(output, 0, 0, width, height, transparency);
    gdImageCopy(output, image, 0, 0, 0, 0, width, height);
    
    FILE* tmp = fopen("/tmp/test.png", "wb");
    gdImagePng(output, tmp);
    fclose(tmp);
    
    /* Copy pixels from image to pic */
    for (y = 0; y < height; y++)       
    {
        for (x = 0; x < width; x++)
        {
            sprites->pixels[index][y * width + x] =
                gdImageGetPixel(output, x, y);
        }
    }
    
    /* Free resources */
    gdImageDestroy(output);
}


/**
 * Used by qsort for sorting filenames.
 */
 
static int sortFilenames(const void *p1, const void *p2)
{
    return strcmp(* (char * const *) p1, * (char * const *) p2);
}


/**
 * Reads all sprites (All PNG files in alphabetical order) from the specified
 * input directory, converts them into wasteland sprites and returns the
 * sprites container.
 * 
 * @param inputDir
 *            The input directory
 * @return The wasteland sprites container 
 */

static wlSpritesPtr readSprites(char *inputDir)
{
    char *oldDir;
    DIR *dir;
    struct dirent *entry;
    char **filenames;
    int quantity, width = 0, height = 0;
    wlSpritesPtr sprites = NULL;
    int i;
    gdImagePtr image;
    FILE *file;

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

    // Build the sprite container
    for (i = 0; i < quantity; i++)
    {
        file = fopen(filenames[i], "rb");
        if (!file)
        {
            die("Unable to read PNG to %s: %s\n", filenames[i], strerror(errno));
        }
        image = gdImageCreateFromPng(file);
        fclose(file);
        if (i == 0)
        {
            width = gdImageSX(image);
            height = gdImageSY(image);
            sprites = wlSpritesCreate(quantity, width, height);            
        }
        else
        {
            if (gdImageSX(image) != width || gdImageSY(image) != height)
            {
                die("Sprites with different sizes are not supported.\n");
            }
        }
        storeSprite(sprites, i, image);
        gdImageDestroy(image);        
    }
    strListFreeWithItems(filenames);
    
    // Go back to previous directory and then return the sprites
    chdir(oldDir);
    free(oldDir);
    return sprites;
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
    char *spritesFilename, *masksFilename, *inputDir;
    wlSpritesPtr sprites;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 3) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    inputDir = argv[0];
    spritesFilename = argv[1];
    masksFilename = argv[2];
    
    /* Read sprites from PNG files */
    sprites = readSprites(inputDir);
    
    /* Write the sprites */
    wlSpritesWriteFile(sprites, spritesFilename, masksFilename);
    
    /* Free memory */
    wlSpritesDestroy(sprites);
        
    /* Success */
    return 0;
}
