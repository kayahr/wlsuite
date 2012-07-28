/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#ifndef LIST_H
#define LIST_H

#define listCreate(list, size) \
    list = malloc(0); *size = 0

#define listFreeItems(list, size) \
    { int __tmp; for (__tmp = 0; __tmp < *size; __tmp++) free(list[__tmp]); }

#define listFree(list) \
    free(list)

#define listFreeWithItems(list, size) \
    listFreeItems(list, size); listFree(list)

#define listAdd(list, data, size) \
    list = realloc(list, sizeof(void *) * ((*size) + 1)); list[(*size)++] = data

#define listRemove(list, index, size) \
    memmove(&(list[index]), &(list[index + 1]), \
        sizeof(void *) * (*size - index - 1)); \
    list = realloc(list, sizeof(void *) * --(*size))

#define listRemoveAndFree(list, index, size) \
    free(list[index]); listRemove(list, index, size)

#define listInsert(list, index, data, size) \
    list = realloc(list, sizeof(void *) * (*size + 1)); \
    memmove(&(list[index + 1]), &(list[index]), sizeof(void *) \
        * ((*size)++ - index)); \
    list[index] = data

#define listClear(list, size) \
    listFree(list); listCreate(list, size)

#define listClearAndFree(list, size) \
    listFreeItems(list, size); listClear(list, size)

#endif
