/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#ifndef WASTELAND_H
#define WASTELAND_H

#include <sys/types.h>
#include <stdio.h>

typedef unsigned char wlPixel;
typedef wlPixel * wlImage;
typedef wlImage * wlImages;

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} wlRGB;

extern wlRGB wlPalette[16];

typedef struct wlHuffmanNode_s
{
    struct wlHuffmanNode_s *parent;
    struct wlHuffmanNode_s *left;
    struct wlHuffmanNode_s *right;
    unsigned char payload;
    int key;
    char keyBits;
    int usage;
} wlHuffmanNode;

typedef struct
{
    unsigned short x;
    unsigned short y;
    wlPixel pixels[8];
} wlCpaUpdate;

typedef struct
{
    int delay;
    int quantity;
    wlCpaUpdate ** updates;
} wlCpaFrame;

typedef struct
{
    wlImage baseFrame;
    int quantity;
    wlCpaFrame ** frames;
} wlCpaAnimation;

                
extern void wlError(char *message, ...);

/* IO functions */
extern int wlReadBit(FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);
extern int wlReadByte(FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);
extern int wlWriteBit(char bit, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);
extern int wlWriteByte(unsigned char byte, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);
extern int wlWriteDWord(unsigned int dword, FILE *file);
extern int wlFillByte(char bit, FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);

/* Vertical XOR functions */
extern void wlVXorDecode(unsigned char *data, int width, int height);
extern void wlVXorEncode(unsigned char *data, int width, int height);

/* Huffman functions */
extern wlHuffmanNode * wlHuffmanReadNode(FILE *file, unsigned char *dataByte,
    unsigned char *dataMask);
extern int             wlHuffmanWriteNode(wlHuffmanNode *node, FILE *stream,
    unsigned char *dataByte, unsigned char *dataMask);
extern void            wlHuffmanFreeNode(wlHuffmanNode *node);
extern int             wlHuffmanReadByte(FILE *file, wlHuffmanNode *rootNode,
    unsigned char *dataByte, unsigned char *dataMask);
extern int             wlHuffmanWriteByte(unsigned char byte, FILE *file,
    wlHuffmanNode **nodeIndex, unsigned char *dataByte,
    unsigned char *dataMask);
extern int             wlHuffmanReadWord(FILE *stream, wlHuffmanNode *rootNode,
    unsigned char *dataByte, unsigned char *dataMask);
extern int             wlHuffmanWriteWord(u_int16_t word, FILE *stream,
    wlHuffmanNode **nodeIndex, unsigned char *dataByte,
    unsigned char *dataMask);
extern wlHuffmanNode * wlHuffmanBuildTree(unsigned char *data, int size,
    wlHuffmanNode ***index);
extern void            wlHuffmanDumpNode(wlHuffmanNode *node, int indent);

/* PIC functions */
extern wlImage wlPicReadFile(char *filename);
extern wlImage wlPicReadStream(FILE *stream);
extern int     wlPicWriteFile(wlImage pixels, char *filename);
extern int     wlPicWriteStream(wlImage pixels, FILE *stream);

/* Sprites functions */
extern wlImages wlSpritesReadFile(char *spritesFilename, char *masksFilename);
extern wlImages wlSpritesReadStream(FILE *spritesStream, FILE *masksStream);
extern int      wlSpritesWriteFile(wlImages sprites, char *spritesFilename,
    char *masksFilename);
extern int      wlSpritesWriteStream(wlImages sprites, FILE *spritesStream,
    FILE *masksStream);

/* Cursors functions */
extern wlImages wlCursorsReadFile(char *filename);
extern wlImages wlCursorsReadStream(FILE *stream);
extern int      wlCursorsWriteFile(wlImages cursors, char *filename);
extern int      wlCursorsWriteStream(wlImages cursors, FILE *stream);

/* Font functions */
extern wlImages wlFontReadFile(char *filename);
extern wlImages wlFontReadStream(FILE *stream);
extern int      wlFontWriteFile(wlImages font, char *filename);
extern int      wlFontWriteStream(wlImages font, FILE *stream);

/* CPA functions */
extern wlCpaAnimation * wlCpaCreate();
extern void             wlCpaFree(wlCpaAnimation *animation);
extern void             wlCpaApplyFrame(wlImage image, wlCpaFrame *frame);
extern wlCpaAnimation * wlCpaReadFile(char *filename);
extern wlCpaAnimation * wlCpaReadStream(FILE *stream);
extern int              wlCpaWriteFile(wlCpaAnimation *animation,
    char *filename);
extern int              wlCpaWriteStream(wlCpaAnimation *animation,
    FILE *stream);

#endif
