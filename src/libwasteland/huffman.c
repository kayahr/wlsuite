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
 * need it.
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
    wlHuffmanNode *node;
    int bit;
    
    node = (wlHuffmanNode *) malloc(sizeof(wlHuffmanNode)); 
    bit = wlReadBit(file, dataByte, dataMask);
    if (bit == -1) return NULL;
    if (bit)
    {
        node->left = NULL;
        node->right = NULL;
        node->payload = wlReadByte(file, dataByte, dataMask);
    }
    else
    {
        node->left = wlHuffmanReadNode(file, dataByte, dataMask);
        wlReadBit(file, dataByte, dataMask);
        node->right = wlHuffmanReadNode(file, dataByte, dataMask);
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


/**
 * Reads huffman encoded data (beginning at the translation table) from the
 * specified pointer and returns the decoded data with the specified size. You
 * have to release the allocated memory of the returned data when you no longer
 * need it.
 * 
 * @param data
 *            The huffman encoded data to decode
 * @param size
 *            The size of the decoded data block
 * @return The decoded data block
 */

unsigned char * wlHuffmanDecode(FILE *file, int size)
{
    wlHuffmanNode *rootNode;
    unsigned char dataByte, dataMask;
    unsigned char *data;
    int i;
    
    assert(file != NULL);
    assert(size >= 0);
    
    // Read the huffman tree nodes
    dataByte = 0;
    dataMask = 0;
    rootNode = wlHuffmanReadNode(file, &dataByte, &dataMask);
    if (!rootNode) return NULL;
    
    // Allocate memory for decoded data
    data = (unsigned char *) malloc(size);
    if (data == NULL) return NULL;
    
    // Read and decode data
    for (i = 0; i < size; i++)
    {
        int b;
        
        b = wlHuffmanReadByte(file, rootNode, &dataByte, &dataMask);
        if (b < 0) return NULL;
        data[i] = (unsigned char) b; 
    }
    
    // Return the decoded data
    return data;
}


/**
 * Encodes the specified data with the huffman algorithm and returns the
 * encoded data. You have to release the allocated memory of the returned data
 * when you no longer need it. The size of the encoded block is stored in
 * the referenced <var>encSize</var> variable.
 * 
 * @param data
 *            The data to encode
 * @param size
 *            The size of the data to encode
 * @param encSize
 *            Pointer to a variable to which the size of the encoded data will
 *            be stored.
 * @return The encoded data
 */

unsigned char * wlHuffmanEncode(unsigned char *data, int size, int *encSize)
{
    assert(data != NULL);
    assert(size >= 0);
    assert(encSize != NULL);
    return NULL;
}
