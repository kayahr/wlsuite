/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <assert.h>
#include <stdlib.h>
#include "wasteland.h"


/**
 * Reads a huffman tree node (and all it's sub nodes) from the specified stream.
 * You have to provide pointers to 0-initialized dataByte/dataMask storage
 * bytes for the bit-based IO functions which are used to read the data. You
 * have to release allocated memory with wlHuffmanNodeFree() when you no longer
 * need it. Returns NULL if an error occurs while reading from the stream.
 * 
 * @param file
 *            The file stream
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The read huffman tree node with all its sub nodes
 */            

wlHuffmanNode * wlHuffmanReadNode(FILE *file, unsigned char *dataByte,
        unsigned char *dataMask)
{
    wlHuffmanNode *node, *left, *right;    
    int bit, payload;
    
    // Read payload or sub nodes. 
    if ((bit = wlReadBit(file, dataByte, dataMask)) == -1) return NULL;
    if (bit)
    {
        left = NULL;
        right = NULL;
        if ((payload = wlReadByte(file, dataByte, dataMask)) == -1) return NULL;
    }
    else
    {
        if (!(left = wlHuffmanReadNode(file, dataByte, dataMask))) return NULL;
        if (wlReadBit(file, dataByte, dataMask) == -1) return NULL;
        if (!(right = wlHuffmanReadNode(file, dataByte, dataMask))) return NULL;
        payload = 0;
    }
    
    // Build and return the node
    node = (wlHuffmanNode *) malloc(sizeof(wlHuffmanNode));
    if (node)
    {
        node->left = left;
        node->right = right;
        node->payload = payload;
    }
    return node;
}


/**
 * Releases the allocated memory for the specified huffman tree node.
 * 
 * @param node
 *            The huffman tree node to free
 */

void wlHuffmanFreeNode(wlHuffmanNode *node)
{
    if (node->left != NULL) wlHuffmanFreeNode(node->left);
    if (node->right != NULL) wlHuffmanFreeNode(node->right);        
    free(node);
}


/**
 * Reads a byte from the huffman encoded stream. You have to provide pointers 
 * to the dataByte/dataMask storage bytes for the bit-based IO functions which
 * are used to read the huffman data.
 * 
 * @param file
 *            The file stream
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The byte which was read from the stream or -1 when read failed
 */
 
int wlHuffmanReadByte(FILE *file, wlHuffmanNode *rootNode,
        unsigned char *dataByte, unsigned char *dataMask)
{
    int bit;
    wlHuffmanNode *node;
    
    node = rootNode;
    while (node->left != NULL)
    {
        bit = wlReadBit(file, dataByte, dataMask);
        if (bit < 0) return -1;
        node = bit ? node->right : node->left; 
    }
    return node->payload;
}


/**
 * Reads a 16 bit little-endian value from the specified huffman stream.
 * Returns -1 if read failed.
 * 
 * @param file
 *            The stream to read the word from
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The 16 bit litt-endian value or -1 if an error occured while reading
 */

int wlHuffmanReadWord(FILE *stream, wlHuffmanNode *rootNode,
        unsigned char *dataByte, unsigned char *dataMask)
{
    int low, high;
        
    low = wlHuffmanReadByte(stream, rootNode, dataByte, dataMask);
    if (low == -1) return -1;
    high = wlHuffmanReadByte(stream, rootNode, dataByte, dataMask);
    if (high == -1) return -1;
    return high << 8 | low;
}

