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
    printf("Usage: wl_unpackpics [OPTION]... PICSFILE OUTPUTDIR\n");
    printf("Unpacks animated pictures into PNG images.\n");
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
    printf("wl_unpacksprites %s\n", VERSION);
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
    int palette[17];
    int color;

    output = gdImageCreate(image->width, image->height);
    for (i = 0; i < 16; i++)
    {
        palette[i] = gdImageColorAllocate(output, wlPalette[i].red,
                wlPalette[i].green, wlPalette[i].blue);
    }
    palette[16] = gdImageColorAllocate(output, 0, 0, 0);
    gdImageColorTransparent(output, palette[16]);
    for (y = 0; y < image->height; y++)
    {
        for (x = 0; x < image->width; x++)
        {
            color = image->pixels[y * image->width + x];
            gdImageSetPixel(output, x, y, palette[color]);
        }
    }
    return output;
}


/**
 * Writes a single picture animation to the specified output directory.
 *
 * @param outputDir
 *            The output directory
 * @param animation
 *            The animatin to write
 */

static void writeAnimation(char *outputDir, wlPicsAnimation animation)
{
    gdImagePtr baseImage, transpImage, prevImage;
    gdImagePtr frameImage;
    wlImage frame, transp;
    FILE *file, *htmlFile;
    char *oldDir;
    char filename[8];
    char format[8];
    int i, j, x,y;
    wlPicsInstructionSet set;
    wlPicsInstruction instruction;

    // Remember current directory and then go to output directory
    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }

    // Initialize HTML file
    htmlFile = fopen("index.html", "wt");
    if (!htmlFile)
        die("Unable to write index.html: %s\n", strerror(errno));
    fprintf(htmlFile, "<html>\n");
    fprintf(htmlFile, "  <body>\n");
    fprintf(htmlFile, "    <div style=\"position:relative;width:96px;height:84px\">\n");

    // Determine the filename format for all written files of this animation
    sprintf(format, "%%0%ii.%%s", (int) log10(animation->instructions->quantity) + 1);

    // Write the base frame
    sprintf(filename, format, 0, "png");
    fprintf(htmlFile, "      <img src=\"%s\" style=\"position:absolute;width:100%%;height:100%%\" />\n", filename);
    baseImage = createImage(animation->baseFrame);
    file = fopen(filename, "wb");
    if (!file)
        die("Unable to write base PNG to %s: %s\n", filename, strerror(errno));
    gdImagePng(baseImage, file);
    fclose(file);

    // Create a transparent image which builds the base for the animated GIFs
    transp = wlImageCreate(96, 84);
    for (x = 0; x < 96; x++)
    {
        for (y = 0; y < 84; y++)
        {
            transp->pixels[x + y * 96] = 16;
        }
    }
    transpImage = createImage(transp);
    wlImageFree(transp);

    // Write the animated layers
    for (i = 0; i < animation->instructions->quantity; i++)
    {
        frame = wlImageClone(animation->baseFrame);
        set = animation->instructions->sets[i];

        // Initialize the animated GIF for this animation layer
        sprintf(filename, format, i + 1, "gif");
        fprintf(htmlFile, "      <img src=\"%s\" style=\"position:absolute;width:100%%;height:100%%\" />\n", filename);
        file = fopen(filename, "wb");
        if (!file) die("Unable to write animation layer to %s: %s\n",
                filename, strerror(errno));
        gdImageGifAnimBegin(transpImage, file, 1, 0);
        gdImageGifAnimAdd(transpImage, file, 0, 0, 0, set->instructions[0]->delay * 6, gdDisposalNone, NULL);
        prevImage = NULL;
        for (j = 0; j < set->quantity - 1; j++)
        {
            instruction = set->instructions[j];

            // There is one empty update frame in allpics2 picture 22. We
            // simply ignore it
            if (animation->updates->sets[instruction->update]->quantity == 0)
                continue;

            wlAnimationApply(frame, animation->updates->sets[instruction->update]);
            frameImage = createImage(frame);
            gdImageGifAnimAdd(frameImage, file, 0, 0, 0,
                    set->instructions[j + 1]->delay * 6,
                    gdDisposalNone, prevImage ? prevImage : baseImage);
            if (prevImage) gdImageDestroy(prevImage);
            prevImage = frameImage;
        }
        gdImageDestroy(prevImage);
        gdImageGifAnimEnd(file);
        fclose(file);

        // Free the image
        wlImageFree(frame);
    }

    // Finish HTML file
    fprintf(htmlFile, "    </div>\n");
    fprintf(htmlFile, "  </body>\n");
    fprintf(htmlFile, "</html>\n");
    fclose(htmlFile);

    // Release the base image and the transparent images
    gdImageDestroy(baseImage);
    gdImageDestroy(transpImage);

    // Switch back to old current directory
    if (chdir(oldDir))
    {
        die("Unable to change to directory %s: %s\n", oldDir,
                strerror(errno));
        return;
    }
    free(oldDir);
}


/**
 * Writes all the picture animations into the specified output directory.
 *
 * @param outputDir
 *            The output directory
 * @param animations
 *            The animations to write
 */

static void writeAnimations(char *outputDir, wlPicsAnimations animations)
{
    int i;
    char *oldDir;
    char filename[5];
    char format[5];

    oldDir = getcwd(NULL, 0);
    if (chdir(outputDir))
    {
        die("Unable to change to output directory %s: %s\n", outputDir,
                strerror(errno));
        return;
    }
    
    sprintf(format, "%%0%ii", (int) log10(animations->quantity) + 1);
    for (i = 0; i < animations->quantity; i++)
    {
        sprintf(filename, format, i);
        mkdir(filename, 0755);
        writeAnimation(filename, animations->animations[i]);
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
    wlPicsAnimations animations;

    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;

    /* Terminate if wrong number of parameters are specified */
    if (argc != 2) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Process parameters */
    filename = argv[0];
    outputDir = argv[1];

    /* Read the picture animations file */
    animations = wlAnimationsReadFile(filename);
    if (!animations)
    {
        die("Unable to read picture animations from %s: %s\n", filename,
                strerror(errno));
    }

    /* Write the PNG files */
    writeAnimations(outputDir, animations);

    /* Free resources */
    wlAnimationsFree(animations);

    /* Success */
    return 0;
}
