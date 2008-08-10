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

typedef struct
{
    int width;
    int height;
    wlPixel * pixels;
} wlImageStruct;
typedef wlImageStruct * wlImage;

typedef struct
{
    int quantity;
    wlImage * images;
} wlImagesStruct;
typedef wlImagesStruct * wlImages;

typedef struct
{
    int quantity;
    wlImages * tilesets;
} wlTilesetsStruct;
typedef wlTilesetsStruct * wlTilesets;

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

typedef struct
{
    unsigned char x;
    unsigned char y;
    unsigned char quantity;
    unsigned char * pixelXORs;
} wlPicsUpdateStruct;
typedef wlPicsUpdateStruct * wlPicsUpdate;

typedef struct
{
    int quantity;
    wlPicsUpdate * updates;
} wlPicsUpdateSetStruct;
typedef wlPicsUpdateSetStruct * wlPicsUpdateSet;

typedef struct
{
    int quantity;
    wlPicsUpdateSet * sets;
} wlPicsUpdatesStruct;
typedef wlPicsUpdatesStruct * wlPicsUpdates;

typedef struct
{
    unsigned char delay;
    unsigned char update;
} wlPicsInstructionStruct;
typedef wlPicsInstructionStruct * wlPicsInstruction;

typedef struct
{
    int quantity;
    wlPicsInstruction * instructions;
} wlPicsInstructionSetStruct;
typedef wlPicsInstructionSetStruct * wlPicsInstructionSet;

typedef struct
{
    int quantity;
    wlPicsInstructionSet * sets;
} wlPicsInstructionsStruct;
typedef wlPicsInstructionsStruct * wlPicsInstructions;

typedef struct
{
    wlImage baseFrame;
    wlPicsInstructions instructions;
    wlPicsUpdates updates;
} wlPicsAnimationStruct;
typedef wlPicsAnimationStruct * wlPicsAnimation;

typedef struct
{
    int quantity;
    wlPicsAnimation * animations;
} wlPicsAnimationsStruct;
typedef wlPicsAnimationsStruct * wlPicsAnimations;

enum wlMsqType
{
    UNCOMPRESSED,
    COMPRESSED,
    CPA_ANIMATION
};

typedef struct
{
    enum wlMsqType type;
    int disk;
    int size;
} wlMsqHeaderStruct;
typedef wlMsqHeaderStruct * wlMsqHeader;

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
extern unsigned char * wlHuffmanReadBlock(FILE *stream, unsigned char *block,
    int size, wlHuffmanNode *rootNode, unsigned char *dataByte,
    unsigned char *dataMask);
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

/* Image functions */
extern wlImage wlImageCreate(int width, int height);
extern void    wlImageFree(wlImage image);
extern wlImage wlImageClone(wlImage image);
extern void    wlImageVXorEncode(wlImage image);
extern void    wlImageVXorDecode(wlImage image);

/* PIC functions */
extern wlImage wlPicReadFile(char *filename);
extern wlImage wlPicReadStream(FILE *stream);
extern int     wlPicWriteFile(wlImage pixels, char *filename);
extern int     wlPicWriteStream(wlImage pixels, FILE *stream);

/* Images functions */
extern wlImages wlImagesCreate(int quantity, int width, int height);
extern void     wlImagesFree(wlImages images);

/* Sprites functions */
extern wlImages wlSpritesReadFile(char *spritesFilename, char *masksFilename);
extern wlImages wlSpritesReadStream(FILE *spritesStream, FILE *masksStream);
extern int      wlSpritesWriteFile(wlImages sprites, char *spritesFilename,
    char *masksFilename);
extern int      wlSpritesWriteStream(wlImages sprites, FILE *spritesStream,
    FILE *masksStream);

/* Tiles functions */
extern wlTilesets wlTilesetsReadFile(char *filename);
extern void       wlTilesetsFree(wlTilesets tileSets);
extern wlImages   wlTilesReadStream(FILE *stream);

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
extern wlCpaAnimation * wlCpaCreate(int width, int height);
extern void             wlCpaFree(wlCpaAnimation *animation);
extern void             wlCpaApplyFrame(wlImage image, wlCpaFrame *frame);
extern wlCpaAnimation * wlCpaReadFile(char *filename);
extern wlCpaAnimation * wlCpaReadStream(FILE *stream);
extern void             wlCpaAddFrame(wlCpaAnimation *animation, wlImage frame,
    wlImage prevFrame, wlImage lastFrame, int delay);
extern int              wlCpaWriteFile(wlCpaAnimation *animation,
    char *filename);
extern int              wlCpaWriteStream(wlCpaAnimation *animation,
    FILE *stream);

/* MSQ functions */
extern wlMsqHeader wlMsqReadHeader(FILE *stream);

/* PICS animation functions */
extern wlPicsAnimations wlAnimationsReadFile(char *filename);
extern wlPicsAnimation  wlAnimationReadStream(FILE *stream);
extern void wlAnimationFree(wlPicsAnimation animations);
extern void wlAnimationsFree(wlPicsAnimations animations);
extern void wlAnimationApply(wlImage image, wlPicsUpdateSet set);

#endif
