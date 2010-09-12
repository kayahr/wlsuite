/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>
#include <errno.h>
#include <wasteland.h>
#include "config.h"

/** The maximum number of bytes to read. 0 = Infinite */
static size_t maxBytes = 0;


/**
 * Displays the usage text.
 */

static void display_usage(void) 
{
    printf("Usage: wl_decodehuffman [OPTION]... <INPUT >OUTPUT\n");
    printf("Huffman-decodes data from STDIN and writes it to STDOUT.\n");
    printf("\nOptions\n");
    printf("  -m, --max BYTES      The maximum number of bytes to read.\n");
    printf("  -h, --help           Display help and exit\n");
    printf("  -V, --version        Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

static void display_version(void) 
{
    printf("wl_decodehuffman %s\n", VERSION);
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
        {"max", 1, NULL, 'm'},
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'V'}
    };
    
    opterr = 0;
    while((opt = getopt_long(argc, argv, "hVm:", options, &index)) != -1)
    {
        switch(opt) 
        {                
            case 'm':
                maxBytes = atol(optarg);
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
    wlHuffmanNode *rootNode;
    unsigned char dataByte, dataMask;
    int b;
    size_t bytes;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 0) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Open huffman stream */
    dataByte = 0;
    dataMask = 0;
    if (!(rootNode = wlHuffmanReadNode(stdin, &dataByte, &dataMask)))
        die("Unable to read huffman root node.\n");
    
    /* Read bytes and print them to STDOUT */
    bytes = 0;
    while ((b = wlHuffmanReadByte(stdin, rootNode, &dataByte, &dataMask)) != EOF)
    {
        fputc(b, stdout);
        bytes++;
        if (maxBytes && bytes >= maxBytes) break;
    }
        
    /* Close huffman stream */
    wlHuffmanFreeNode(rootNode);
    
    /* Success */
    return 0;
}
