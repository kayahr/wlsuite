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
#include "../libwasteland/wasteland.h"
#include <kaytils/str.h>
#include <kaytils/list.h>
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
    printf("Usage: wl_packfont [OPTION]... INPUTDIR FONTFILE\n");
    printf("Packs PNG files into font.\n");
    printf("\nThe first 172 PNG files found in INPUTDIR are used "
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
    printf("wl_packfont %s\n", VERSION);
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
 * Converts image into a wasteland font glyph and stores it in the specified
 * font at the specified index.
 * 
 * @param font
 *            The font
 * @param index
 *            The glyph index
 * @param image
 *            The image
 */

static void storeFontGlyph(wlImages font, int index, gdImagePtr image)
{
    gdImagePtr output;
    int x, y, i;
    int transparency, color;
    
    /* Create a temporary second image for palette conversion */
    output = gdImageCreate(8, 8);
    for (i = 0; i < 16; i++)
    {
        gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    transparency = gdImageColorAllocate(output, 0, 0, 0);
    gdImageColorTransparent(output, transparency);
    for (i = 17; i < 256; i++) gdImageColorAllocate(output, 0, 0, 0);
    gdImageFilledRectangle(output, 0, 0, 8, 8, transparency);
    gdImageCopyResampled(output, image, 0, 0, 0, 0, 8, 8, gdImageSX(image),
            gdImageSY(image));
    
    /* Copy pixels from image to pic */
    for (y = 0; y < 8; y++)       
    {
        for (x = 0; x < 8; x++)
        {
            color = gdImageGetPixel(output, x, y);
            font->images[index]->pixels[y * 8 + x] = color < 16 ? color : color | 0xf0;
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
 * Reads all font glyphs (All PNG files in alphabetical order) from the specified
 * input directory, converts them into wasteland font glyphs and returns the
 * font.
 * 
 * @param inputDir
 *            The input directory
 * @return The wasteland font 
 */

static wlImages readFont(char *inputDir)
{
    char *oldDir;
    DIR *dir;
    struct dirent *entry;
    char **filenames;
    int quantity;
    wlImages font;
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
    listCreate(filenames, &quantity);
    dir = opendir(inputDir);
    while ((entry = readdir(dir)))
    {
        if (strEndsWithIgnoreCase(entry->d_name, ".png"))
        {
            listAdd(filenames, strdup(entry->d_name), &quantity);
        }
    }
    closedir(dir);
    qsort(filenames, quantity, sizeof(char *), sortFilenames);
    
    // Build the font
    font = (wlImages) malloc(172 * sizeof(wlImage));
    for (i = 0; i < 172; i++)
    {
        if (i < quantity)
        {
            file = fopen(filenames[i], "rb");
            if (!file)
            {
                die("Unable to read PNG from %s: %s\n", filenames[i],
                        strerror(errno));
            }
            image = gdImageCreateFromPng(file);
            fclose(file);
            storeFontGlyph(font, i, image);
            gdImageDestroy(image);
        }
    }
    listFreeWithItems(filenames, &quantity);
    
    // Go back to previous directory and then return the font
    if (chdir(oldDir))
    {
        die("Unable to change to directory %s: %s\n", oldDir,
                strerror(errno));
        return NULL;
    }
    free(oldDir);
    return font;
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
    wlImages font;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    inputDir = argv[0];
    filename = argv[1];
    
    /* Read font from PNG files */
    font = readFont(inputDir);
    
    /* Write the font */
    wlFontWriteFile(font, filename);
    
    /* Free memory */
    wlImagesFree(font);
        
    /* Success */
    return 0;
}
