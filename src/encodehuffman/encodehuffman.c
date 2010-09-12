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


/**
 * Displays the usage text.
 */

static void display_usage(void) 
{
    printf("Usage: wl_encodehuffman [OPTION]... <INPUT >OUTPUT\n");
    printf("Huffman-encodes data from STDIN and writes it to STDOUT.\n");
    printf("\nOptions\n");
    printf("  -h, --help           Display help and exit\n");
    printf("  -V, --version        Display version and exit\n");
    printf("\nReport bugs to %s <%s>\n", AUTHOR, EMAIL);
}


/**
 * Displays the version information.
 */

static void display_version(void) 
{
    printf("wl_encodehuffman %s\n", VERSION);
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
 * Reads all data from stdin and returns it as a byte array. Remember
 * to free this array when no longer needed.
 * 
 * @param stream
 *            The file stream to read the data from.
 * @param size
 *            The array size is stored into this variable.
 * @return The read data as a byte array.
 */
 
static unsigned char *readData(FILE *stream, size_t *size)
{
    unsigned char *result = NULL;
    size_t offset = 0;
    int bufferSize = 256;
    size_t read;
    unsigned char buffer[bufferSize];
    
    *size = 0;
    while ((read = fread(buffer, 1, bufferSize, stream)) > 0)
    {
        /* Create result array if not already done. */
        if (!*size)
        {
            *size = read;
            result = (unsigned char *) malloc(*size);
        }
        
        /* or else enlarge the result array. */
        else
        {
            *size += read;
            result = (unsigned char *) realloc(result, *size);
        }
        
        
        /* Copy buffer into result array. */
        memcpy(&result[offset], buffer, read);
        
        /** Remember current capacity as offset for writing next block. */
        offset = *size;
    }
    
    /* Check for errors. */
    if (ferror(stream))
        die("Unable to read input data: %s\n", strerror(errno));
        
    /* Return the result array. */
    return result;
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
    unsigned char *data;
    wlHuffmanNode *rootNode, **nodeIndex;
    int i;
    size_t size;
    unsigned char dataByte, dataMask;
    
    /* Process options and reset argument pointer */
    check_options(argc, argv);
    argc -= optind;
    argv += optind;
    
    /* Terminate if wrong number of parameters are specified */
    if (argc != 0) die("Wrong number of parameters.\nUse --help to show syntax.\n");

    /* Read data from stdin. */    
    data = readData(stdin, &size);
    
    /* Build huffman tree and write it to stdout */
    rootNode = wlHuffmanBuildTree(data, size, &nodeIndex);
    dataByte = 0;
    dataMask = 0;
    if (!wlHuffmanWriteNode(rootNode, stdout, &dataByte, &dataMask))
        die("Unable to write huffman root node\n");
    for (i = 0; i < size; i++)
        wlHuffmanWriteByte(data[i], stdout, nodeIndex, &dataByte, &dataMask);

    /* Make sure last byte is written */
    if (!wlFillByte(0, stdout, &dataByte, &dataMask)) return 0;
               
    /* Free stuff */ 
    wlHuffmanFreeNode(rootNode);
    free(nodeIndex);
    free(data);

    /* Success */
    return 0;
}
