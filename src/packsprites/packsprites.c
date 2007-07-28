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
#include "config.h"

#ifdef WIN32
#define SEPARATOR '\\'
#else
#define SEPARATOR '/'
#endif


/**
 * Displays the usage text.
 */

void display_usage(void) 
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

void display_version(void) 
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
 * Writes a single sprite into the specified file in PNG format.
 * 
 * @param filename
 *            The output filename
 * @param sprites
 *            The sprites
 * @param spriteNo
 *            The sprite number to write
 */

void writePng(char *filename, wlSpritesPtr sprites, int spriteNo)
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

void writePngs(char *outputDir, wlSpritesPtr sprites)
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

wlSpritesPtr readSprites(char *inputDir)
{
    char *oldDir;
    DIR *dir;
    struct dirent *entry;

    oldDir = getcwd(NULL, 0);
    if (chdir(inputDir))
    {
        die("Unable to change to input directory %s: %s\n", inputDir,
                strerror(errno));
        return NULL;
    }
    
    dir = opendir(inputDir);
    while ((entry = readdir(dir)))
    {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    
    chdir(oldDir);
    return NULL;
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
