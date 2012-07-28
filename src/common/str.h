/*
 * Copyright (C) 2007-2011  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#ifndef STR_H
#define STR_H

/**
 * \file str.h
 * Utility functions to work with null-terminated strings.
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * Appends the characters from src to dest.
 *
 * @param dest
 *            The destination string to append the characters to.
 * @param src
 *            The soruce string to get the characters from.
 */
#define strAppend(dest, src) __strAppend(&dest, src);

/**
 * Appends a sub string from src to dest.
 *
 * @param dest
 *            The destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 * @param index
 *            The start index of the sub string.
 * @param len
 *            The length of the sub string.
 */
#define strAppendSub(dest, src, index, len) __strAppendSub(&dest, src, index, len);

/**
 * Copies the characters from src to dest.
 *
 * @param dest
 *            The destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 */
#define strCopy(dest, src) __strCopy(&dest, src);

/**
 * Copies a sub string from src to dest.
 *
 * @param dest
 *            The destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 * @param index
 *            The start index of the sub string.
 * @param len
 *            The length of the sub string.
 */
#define strCopySub(dest, src, index, len) __strCopySub(&dest, src, index, len);

/**
 * Replaces all occurrences of the string <var>search</var> in
 * <var>string</var> with <var>replace</var>.
 *
 * @param string
 *            The string to modify.
 * @param search
 *            The string to search.
 * @param replace
 *            The string to replace occurences of <var>search</var> with.
 */
#define strReplace(string, search, replace) __strReplace(&string, search, replace)

/**
 * Deletes <var>len</var> characters at position <var>pos</var> from
 * <var>string</var>.
 *
 * @param string
 *            Pointer to the string to modify.
 * @param pos
 *            The start position of the block to delete.
 * @param len
 *            The block length to delete.
 */
#define strDelete(string, pos, len) __strDelete(&string, pos, len);

/**
 * Removes trailing whitespaces (spaces, tabs and newline characters) from
 * the specified string.
 *
 * @param string
 *            String to modify.
 */
#define strTrimRight(string) __strTrimRight(&string);

/**
 * Removes leading whitespaces (spaces, tabs and newline characters) from
 * the specified string.
 *
 * @param string
 *            String to modify.
 */
#define strTrimLeft(string) __strTrimLeft(&string);

/**
 * Removes leading and trailing whitespaces (spaces, tabs and newline
 * characters) from the specified string.
 *
 * @param string
 *            String to modify.
 */
#define strTrim(string) __strTrim(&string);

char *  strCreate();
char *  strDup();
void    strFree(char *string);
size_t  strLength(char *string);
int     strEqualsIgnoreCase(char *string1, char *string2);
int     strEquals(char *string1, char *string2);
void    __strCopy(char **destPtr, char *src);
void    __strCopySub(char **destPtr, char *src, size_t index, size_t len);
void    __strAppend(char **destPtr, char *src);
void    __strAppendSub(char **destPtr, char *src, size_t index, size_t len);
ssize_t strFindFirst(char *string, char *search);
ssize_t strFindLast(char *string, char *search);
ssize_t strFindFirstChar(char *string, char search);
ssize_t strFindLastChar(char *string, char search);
ssize_t strFindFirstOf(char *string, char *search);
ssize_t strFindFirstNotOf(char *string, char *search);
ssize_t strFindLastOf(char *string, char *search);
ssize_t strFindLastNotOf(char *string, char *search);
int     strCount(char *string, char *search);
int     strCountChar(char *string, char search);
void    __strReplace(char **stringPtr, char *search, char *replace);
void    strReplaceChar(char *string, char search, char replace);
void    __strDelete(char **stringPtr, size_t pos, ssize_t len);
void    __strTrimRight(char **stringPtr);
void    __strTrimLeft(char **stringPtr);
void    __strTrim(char **stringPtr);
char ** strSplit(char *delimiter, char *string, int limit, size_t *size);
char *  strJoin(char *glue, char **list, int size);
int     strEndsWith(char *string, char *search);
int     strEndsWithIgnoreCase(char *string, char *search);
int     strStartsWith(char *string, char *search);
int     strStartsWithIgnoreCase(char *string, char *search);

#endif
