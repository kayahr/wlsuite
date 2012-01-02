/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
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
    node->left = left;
    node->right = right;
    node->payload = payload;
    return node;
}

/**
 * Writes the specified huffman node to a stream. You have to provide pointers
 * to 0-initialized dataByte/dataMask storage bytes for the bit-based IO
 * functions which are used to write the data. Returns 1 on success or 0 on
 * failure.
 *
 * @param node
 *            The node to write
 * @param stream
 *            The file stream
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */

int wlHuffmanWriteNode(wlHuffmanNode *node, FILE *stream,
    unsigned char *dataByte, unsigned char *dataMask)
{
    if (node->left && node->right)
    {
        if (!wlWriteBit(0, stream, dataByte, dataMask)) return 0;
        if (!wlHuffmanWriteNode(node->left, stream, dataByte, dataMask))
            return 0;
        if (!wlWriteBit(0, stream, dataByte, dataMask)) return 0;
        if (!wlHuffmanWriteNode(node->right, stream, dataByte, dataMask))
            return 0;
    }
    else
    {
        if (!wlWriteBit(1, stream, dataByte, dataMask)) return 0;
        if (!wlWriteByte(node->payload, stream, dataByte, dataMask)) return 0;
    }
    return 1;
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
 * Writes a byte to the huffman encoded stream. You have to provide pointers
 * to the dataByte/dataMask storage bytes for the bit-based IO functions which
 * are used to read the huffman data and you also have to provide a huffman
 * node index (which is created by wlHuffmanBuildTree()) which is used to
 * lookup huffman nodes by payload.
 *
 * @param file
 *            The file stream
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */

int wlHuffmanWriteByte(unsigned char byte, FILE *file,
    wlHuffmanNode **nodeIndex, unsigned char *dataByte,
    unsigned char *dataMask)
{
    wlHuffmanNode *node;
    int i, bit;

    node = nodeIndex[byte];
    for (i = node->keyBits - 1; i >= 0; i--)
    {
        bit = (node->key >> i) & 1;
        if (!wlWriteBit(bit, file, dataByte, dataMask)) return 0;
    }
    return 1;
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
 * Reads the specified number of bytes and returns them. If <var>block</var>
 * is NULL then the necessary memory is allocated automatically (And you must
 * free it yourself afterwards). Otherwise the data is stored in
 * <var>block</var> and the pointer is returned. Returns NULL if reading
 * the data fails.
 *
 * @param file
 *            The stream to read the word from
 * @param block
 *            The byte array in which the read bytes are stored. Can be NULL
 *            if this function should allocate the memory automatically.
 * @param size
 *            The number of bytes to read
 * @param rootNode
 *            The root node of the huffman tree
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return The 16 bit litt-endian value or -1 if an error occured while reading
 */

unsigned char * wlHuffmanReadBlock(FILE *stream, unsigned char *block, int size,
    wlHuffmanNode *rootNode, unsigned char *dataByte, unsigned char *dataMask)
{
    int i, byte;

    if (!block) block = (unsigned char *) malloc(sizeof(unsigned char) * size);
    for (i = 0; i < size; i++)
    {
        byte = wlHuffmanReadByte(stream, rootNode, dataByte, dataMask);
        if (byte == -1) return NULL;
        block[i] = byte;
    }
    return block;
}


/**
 * Writes a 16 bit little-endian value to the specified huffman stream.
 * Returns 1 on success and 0 on failure.
 *
 * @param word
 *            The 16 bit little-endian value to write
 * @param file
 *            The stream to write the word to
 * @param nodeIndex
 *            The huffman node index as provided by wlHuffmanBuildTree()
 * @param dataByte
 *            Storage for last read byte
 * @param dateMask
 *            Storage for last bit mask
 * @return 1 on success, 0 on failure
 */

int wlHuffmanWriteWord(u_int16_t word, FILE *stream,
    wlHuffmanNode **nodeIndex, unsigned char *dataByte,
    unsigned char *dataMask)
{
    int low, high;

    low = word & 0xff;
    high = word >> 8;

    if (!wlHuffmanWriteByte(low, stream, nodeIndex, dataByte, dataMask))
        return 0;
    if (!wlHuffmanWriteByte(high, stream, nodeIndex, dataByte, dataMask))
        return 0;
    return 1;
}


/**
 * Used for sorting the huffman nodes by usage
 *
 * @param a
 *            First node to compare
 * @param b
 *            Second node to compare
 * @return Comparison result
 */

static int compareNode(const void *a, const void *b)
{
    wlHuffmanNode *node1, *node2;

    node1 = *((wlHuffmanNode **) a);
    node2 = *((wlHuffmanNode **) b);
    if (!node1 && !node2) return 0;
    if (!node1) return 1;
    if (!node2) return -1;
    if (node1->usage == node2->usage) return 0;
    if (node1->usage > node2->usage) return -1;
    return 1;
}


/**
 * Builds the keys for the specified node and all it's subnodes.
 *
 * @param node
 *            The huffman tree node
 * @param key
 *            The key to store in the node
 * @param keyBits
 *            The number of bits in the key
 */

static void buildKeys(wlHuffmanNode *node, int key, int keyBits)
{
    // Special case (only one node is present)
    if (!keyBits && !node->left && !node->right)
    {
        node->key = 0;
        node->keyBits = 1;
        return;
    }

    // Store key
    node->key = key;
    node->keyBits = keyBits;

    // Dive into sub nodes
    if (node->left) buildKeys(node->left, key << 1, keyBits + 1);
    if (node->right) buildKeys(node->right, (key << 1) | 1, keyBits + 1);
}


/**
 * Dumps the specified huffman node to stdout. This is just for debugging
 * purposes.
 *
 * @param node
 *            The node to dump
 * @param indent
 *            Current Indentation level (For recursion). Set to 0 on initial
 *            call.
 */

void wlHuffmanDumpNode(wlHuffmanNode *node, int indent)
{
    char *spaces;
    int i;

    spaces = (char *) malloc(indent + 1);
    for (i = 0; i < indent; i++) spaces[i] = ' ';
    spaces[i] = 0;
    if (node->left && node->right)
    {
        printf("%sLeft:\n", spaces);
        wlHuffmanDumpNode(node->left, indent + 1);
        printf("%sRight:\n", spaces);
        wlHuffmanDumpNode(node->right, indent + 1);
    }
    else
    {
        printf("%s", spaces);
        if (node->keyBits <= 0 || node->keyBits > 16)
        {
            printf("Invalid keybits on payload node!!!!");
            exit(1);
        }
        for (i = node->keyBits - 1; i >= 0; i--)
        {
            printf("%i", (node->key >> i) & 1);
        }
        printf(" = ");
        printf("%i (Usage: %i)\n", node->payload, node->usage);
    }
    free(spaces);
}

/**
 * Builds a huffman tree for the specified data, A pointer to a list of
 * huffman nodes must be passed as last parameter. The function stores an
 * index array there which can be used to lookup a huffman node with the
 * payload as the key. The returned root node of the huffman tree must be
 * freed with wlHuffmanFreeNode() and the stored node index must be freed with
 * a standard free() when no longer needed.
 *
 * @param data
 *            Pointer to the data
 * @param size
 *            The data size
 * @param nodeIndex
 *            Pointer to a list of huffman nodes where the node index will
 *            be stored
 * @return The root node of the huffman tree
 */

wlHuffmanNode * wlHuffmanBuildTree(unsigned char *data, int size,
    wlHuffmanNode ***nodeIndex)
{
    wlHuffmanNode **nodes, *node, *left, *right;
    int i;
    unsigned char payload;

    // Initialize the list of huffman nodes
    nodes = (wlHuffmanNode **) malloc(sizeof(wlHuffmanNode *) * 256);
    memset(nodes, 0, sizeof(wlHuffmanNode *) * 256);

    // Cycle through data and create huffman nodes for every used data byte
    // and count the usage
    node = NULL;
    for (i = 0; i < size; i++)
    {
        payload = data[i];
        node = nodes[payload];
        if (!node)
        {
            node = (wlHuffmanNode *) malloc(sizeof(wlHuffmanNode));
            node->parent = NULL;
            node->left = NULL;
            node->right = NULL;
            node->payload = payload;
            node->usage = 1;
            node->key = 0;
            node->keyBits = 0;
            nodes[payload] = node;
        }
        else
        {
            node->usage++;
        }
    }

    // Save current state of the list as node index
    *nodeIndex = (wlHuffmanNode **) malloc(sizeof(wlHuffmanNode *) * 256);
    memcpy(*nodeIndex, nodes, sizeof(wlHuffmanNode *) * 256);

    // Now sort the list by usage
    qsort(nodes, 256, sizeof(wlHuffmanNode *), compareNode);

    // Convert the list into a tree
    for (i = 254; i >= 0; i--)
    {
        // Read last two nodes
        right = nodes[i];
        left = nodes[i + 1];
        if (!left || !right) continue;

        // Create a parent node for the two nodes
        node = (wlHuffmanNode *) malloc(sizeof(wlHuffmanNode));
        node->payload = 0;
        node->left = left;
        node->right = right;
        node->parent = NULL,
        left->parent = node;
        right->parent = node;
        node->usage = left->usage + right->usage;
        node->key = 0;
        node->keyBits = 0;

        // Remove the two nodes and put the new parent node at the end of the
        // list
        nodes[i + 1] = NULL;
        nodes[i] = node;
        qsort(nodes, 256, sizeof(wlHuffmanNode *), compareNode);
    }

    // Build the node keys
    buildKeys(node, 0, 0);

    // Free the list and return the root node of the tree
    free(nodes);
    return node;
}
