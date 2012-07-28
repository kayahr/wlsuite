/*
 * $Id$
 * Copyright (C) 2007  Klaus Reimer <k@ailis.de>
 * See COPYING file for copying conditions
 */

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include "str.h"
#include "list.h"

/**
 * ANSI C doesn't provide a strcasecmp function so we use our own
 * here.
 *
 * @param s1
 *            First string to compare.
 * @param s2
 *            Second string to compare.
 * @return Comparison result.
 */
static int _strcasecmp(char *s1, char *s2)
{
    int c1, c2;
    
    do
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
        if (c1 != c2) return c1 < c2 ? -1 : 1;
    }
    while (c1 != 0);
    return 0;
}

/**
 * ANSI C doesn't provide a strncasecmp function so we use our own
 * here.
 *
 * @param s1
 *            First string to compare.
 * @param s2
 *            Second string to compare.
 * @param n
 *            Number of characters to compare.
 * @return Comparison result.
 */
static int _strncasecmp(char *s1, char *s2, size_t n)
{
    int c1, c2;
    
    while (n--)
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
        if (c1 != c2) return c1 < c2 ? -1 : 1;
        if (c1 == 0) return 0;
    }
    return 0;
}

/**
 * Allocates a new empty string and returns it.
 *
 * @return The newly allocated empty string.
 */
char *strCreate()
{
    char *ns;

    ns = (char *) malloc(1);
    ns[0] = 0;
    return ns;
}

/**
 * Duplicates the specified string and returns the duplicate.
 *
 * @param string
 *            The string to duplicate.
 * @return The duplicated string.
 */
char *strDup(char *string)
{
    char *newString;
    size_t len;

    assert(string != NULL);
    len = strlen(string) + 1;
    newString = (char *) malloc(len);
    memcpy(newString, string, len);
    return newString;
}

/**
 * Frees the memory allocated by the specified string.
 *
 * @param string
 *            The string to free.
 */
void strFree(char *string)
{
    assert(string != NULL);
    free(string);
}

/**
 * Returns the length of the string.
 *
 * @param string
 *            The string to determine the length from.
 * @return The string length.
 */
size_t strLength(char *string)
{
    assert(string != NULL);
    return strlen(string);
}

/**
 * Compares the two specified strings and returns 1 if the strings are
 * equal. If string2 is NULL or does not match string1 then 0 is returned.
 *
 * @param string1
 *            First string to compare.
 * @param string2
 *            Second string to compare.
 * @return 1 if the strings are equal, 0 if not.
 */
int strEquals(char *string1, char *string2)
{
    assert(string1 != NULL);
    if (!string2) return 0;
    return strcmp(string1, string2) == 0;
}

/**
 * Case-insensitive version of strEquals().
 *
 * @param string1
 *            First string to compare.
 * @param string2
 *            Second string to compare.
 * @return 1 if the strings are equal, 0 if not.
 */
int strEqualsIgnoreCase(char *string1, char *string2)
{
    assert(string1 != NULL);
    if (!string2) return 0;
    return _strcasecmp(string1, string2) == 0;
}

/**
 * @internal
 * Copies the characters from src to dest.
 *
 * @param destPtr
 *            Pointer to the destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 */
void __strCopy(char **destPtr, char *src)
{
    assert(destPtr != NULL);
    assert(*destPtr != NULL);
    assert(src != NULL);
    *destPtr = (char *) realloc(*destPtr, strlen(src) + 1);
    strcpy(*destPtr, src);
}

/**
 * @internal
 * Appends the characters from src to dest.
 *
 * @param destPtr
 *            Pointer to the destination string to append the characters to.
 * @param src
 *            The source string to get the characters from.
 */
void __strAppend(char **destPtr, char *src)
{
    assert(destPtr != NULL);
    assert(*destPtr != NULL);
    assert(src != NULL);
    if (strLength(src) == 0) return;
    *destPtr = (char *) realloc(*destPtr, strlen(*destPtr) + strlen(src) + 1);
    strcat(*destPtr, src);
}

/**
 * @internal
 * Copies a sub string from src to dest.
 *
 * @param destPtr
 *            Pointer to the destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 * @param index
 *            The starting index of the sub string.
 * @param len
 *            The length of the sub string.
 */
void __strCopySub(char **destPtr, char *src, size_t index, size_t len)
{
    assert(destPtr != NULL);
    assert(*destPtr != NULL);
    assert(src != NULL);
    assert(index >= 0 && index <= strlen(src));
    assert(len >= 0 && len <= strlen(src) - index);
    *destPtr = (char *) realloc(*destPtr, len + 1);
    strncpy(*destPtr, &src[index], len);
    (*destPtr)[len] = 0;
}

/**
 * @internal
 * Appends a sub string from src to dest.
 *
 * @param destPtr
 *            Pointer to the destination string to copy the characters to.
 * @param src
 *            The source string to copy the characters from.
 * @param index
 *            The starting index of the sub string.
 * @param len
 *            The length of the sub string.
 */
void __strAppendSub(char **destPtr, char *src, size_t index, size_t len)
{
    assert(destPtr != NULL);
    assert(*destPtr != NULL);
    assert(src != NULL);
    assert(index >= 0 && index <= strlen(src));
    assert(len >= 0 && len <= strlen(src) - index);
    *destPtr = (char *) realloc(*destPtr, strlen(*destPtr) + len + 1);
    strncat(*destPtr, &src[index], len);
}


/**
 * Returns the index of the first occurrence of the specified search string in
 * <var>string</var>. Returns -1 if the search string is not found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string to search for.
 * @return The position of the first occurence or -1 if no occurence.
 */
ssize_t strFindFirst(char *string, char *search)
{
    char *pos;

    assert(string != NULL);
    assert(search != NULL);
    pos = strstr(string, search);
    if (pos)
        return (size_t) (pos - string);
    else
        return -1;
}

/**
 * Returns the index of the last occurrence of the specified search string in
 * <var>string</var>. Returns -1 if the search string is not found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string to search for.
 * @return The position of the last occurence or -1 if no occurence.
 */
ssize_t strFindLast(char *string, char *search)
{
    char *pos;
    char *lastpos;

    assert(string != NULL);
    assert(search != NULL);
    lastpos = NULL;
    pos = strstr(string, search);
    while (pos)
    {
        lastpos = pos;
        pos = strstr(pos+1, search);
    }
    if (lastpos)
        return (size_t) (lastpos - string);
    else
        return -1;
}

/**
 * Returns the index of the first occurrence of the specified search character
 * in <var>string</var>. Returns -1 if the character is not found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The character to search for.
 * @return The position of the first occurence or -1 if no occurence.
 */
ssize_t strFindFirstChar(char *string, char search)
{
    char *pos;

    assert(string != NULL);
    pos = strchr(string, search);
    if (pos)
        return (size_t) (pos - string);
    else
        return -1;
}

/**
 * Returns the index of the last occurrence of the specified search character
 * in <var>string</var>. Returns -1 if the character is not found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The character to search for.
 * @return The position of the last occurence or -1 if no occurence.
 */
ssize_t strFindLastChar(char *string, char search)
{
    char *pos;
    char *lastpos;

    assert(string != NULL);
    lastpos = NULL;
    pos = strchr(string, search);
    while (pos)
    {
        lastpos = pos;
        pos = strchr(pos + 1, search);
    }
    if (lastpos)
        return (size_t) (lastpos - string);
    else
        return -1;
}

/**
 * Returns the index of the first occurrence of any character from the specified
 * search string in <var>string</var>. Returns -1 if no character was found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string with the search characters.
 * @return The position of the first occurence or -1 if no occurence.
 */
ssize_t strFindFirstOf(char *string, char *search)
{
    char *pos;

    assert(string != NULL);
    assert(search != NULL);
    pos = string;
    while (*pos)
    {
        if (strchr(search, *pos)) break;
        pos++;
    }
    return *pos ? (size_t) (pos - string) : -1;
}

/**
 * Returns the index of the first occurrence of any character in
 * <var>string</var> which is not in the specified search string.
 * Returns -1 if no character was found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string with the characters not to search.
 * @return The position of the first occurence or -1 if no occurence.
 */
ssize_t strFindFirstNotOf(char *string, char *search)
{
    char *pos;

    assert(string != NULL);
    assert(search != NULL);
    pos = string;
    while (*pos)
    {
        if (!strchr(search, *pos)) break;
        pos++;
    }
    return *pos ? (size_t) (pos - string) : -1;
}

/**
 * Returns the index of the last occurrence of any character from the specified
 * search string in <var>string</var>. Returns -1 if no character was found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string with the search characters.
 * @return The position of the last occurence or -1 if no occurence.
 */
ssize_t strFindLastOf(char *string, char *search)
{
    char *pos;

    assert(string != NULL);
    assert(search != NULL);
    pos = string + strlen(string) - 1;
    while (pos >= string)
    {
        if (strchr(search, *pos)) break;
        pos--;
    }
    return pos >= string ? (size_t) (pos - string) : -1;
}

/**
 * Returns the index of the last occurrence of any character in
 * <var>string</var> which is not in the specified search string.
 * Returns -1 if no character was found.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string with the characters not to search.
 * @return The position of the last occurence or -1 if no occurence.
 */
ssize_t strFindLastNotOf(char *string, char *search)
{
    char *pos;

    assert(string != NULL);
    assert(search != NULL);
    pos = string + strlen(string) - 1;
    while (pos >= string)
    {
        if (!strchr(search, *pos)) break;
        pos--;
    }
    return pos >= string ? (size_t) (pos - string) : -1;
}

/**
 * Returns how often the string search is found in <var>string</var>.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The string to count in <var>string</var>.
 * @return The number of occurences.
 */
int strCount(char *string, char *search)
{
    int c;
    char *pos;
    c=0;

    assert(string != NULL);
    assert(search != NULL);
    pos = strstr(string, search);
    while (pos)
    {
        c++;
        pos = strstr(pos + 1, search);
    }
    return c;
}

/**
 * Returns how often the search character is found in <var>string</var>.
 *
 * @param string
 *            The string to search in.
 * @param search
 *            The character to count in <var>string</var>.
 * @return The number of occurences.
 */
int strCountChar(char *string, char search)
{
    int c;
    char *pos;
    c=0;

    assert(string != NULL);
    pos = strchr(string, search);
    while (pos)
    {
        c++;
        pos = strchr(pos + 1, search);
    }
    return c;
}

/**
 * @internal
 * Replaces all occurrences of the string <var>search</var> in
 * <var>string</var> with <var>replace</var>.
 *
 * @param stringPtr
 *            Pointer to the string to modify.
 * @param search
 *            The string to search.
 * @param replace
 *            The string to replace occurences of <var>search</var> with.
 */
void __strReplace(char **stringPtr, char *search, char *replace)
{
    int c;
    char *ns;
    char *pos;
    char *newpos;
    size_t l;

    assert(stringPtr != NULL);
    assert(*stringPtr != NULL);
    assert(search != NULL);
    assert(replace != NULL);
    c = strCount(*stringPtr, search);
    ns = (char *) malloc(strlen(*stringPtr)
        - c * strlen(search)
        + c * strlen(replace) + 1);
    l = strlen(search);
    *ns = 0;
    pos = *stringPtr;
    newpos = strstr(pos, search);
    while (newpos)
    {
        strncat(ns, pos, newpos - pos);
        strcat(ns, replace);
        pos += newpos - pos + l;
        newpos = strstr(pos, search);
    }
    strcat(ns, pos);
    free(*stringPtr);
    *stringPtr = ns;
}

/**
 * Replaces all occurrences of the character <var>search</var> in
 * <var>string</var> with <var>replace</var>.
 *
 * @param string
 *            The string to modify.
 * @param search
 *            The character to search.
 * @param replace
 *            The character to replace occurences of <var>search</var> with.
 */
void strReplaceChar(char *string, char search, char replace)
{
    size_t i;

    assert(string != NULL);
    for (i = 0; i < strlen(string); i++)
        if (string[i] == search) string[i] = replace;
}

/**
 * @internal
 * Deletes <var>len</var> characters at position <var>pos</var> from
 * <var>string</var>.
 *
 * @param stringPtr
 *            Pointer to the string to modify.
 * @param pos
 *            The start position of the block to delete.
 * @param len
 *            The block length to delete.
 */
void __strDelete(char **stringPtr, size_t pos, ssize_t len)
{
    char *ns;

    assert(stringPtr != NULL);
    assert(*stringPtr != NULL);
    assert(pos >= 0 && pos < strlen(*stringPtr));
    assert(len >= 0);
    if (len == 0 ) return;
    ns = strCreate();
    __strCopySub(&ns, *stringPtr, 0, pos);
    if ((len >= 0) && ((pos + len) <= strlen(*stringPtr)))
        __strAppend(&ns, &(*stringPtr)[pos + len]);
    strFree(*stringPtr);
    *stringPtr = ns;
}

/**
 * @internal
 * Removes trailing whitespaces (spaces, tabs and newline characters) from
 * the specified string.
 *
 * @param stringPtr
 *            Pointer to the string to modify.
 */
void __strTrimRight(char **stringPtr)
{
    char *ns;
    size_t pos;

    assert(stringPtr != NULL);
    assert(*stringPtr != NULL);
    ns = strCreate();
    pos = strFindLastNotOf(*stringPtr, " \t\r\n");
    if (pos >= 0) __strCopySub(&ns, *stringPtr, 0, pos + 1);
    strFree(*stringPtr);
    *stringPtr = ns;
}

/**
 * @internal
 * Removes leading whitespaces (spaces, tabs and newline characters) from
 * the specified string.
 *
 * @param stringPtr
 *            Pointer to the string to modify.
 */
void __strTrimLeft(char **stringPtr)
{
    char *ns;
    size_t pos;

    assert(stringPtr != NULL);
    assert(*stringPtr != NULL);
    ns = strCreate();
    pos = strFindFirstNotOf(*stringPtr, " \t\r\n");
    if (pos >= 0) __strCopy(&ns, &(*stringPtr)[pos]);
    strFree(*stringPtr);
    *stringPtr = ns;
}

/**
 * @internal
 * Removes leading and trailing whitespaces (spaces, tabs and newline
 * characters) from the specified string.
 *
 * @param stringPtr
 *            Pointer to the string to modify.
 */
void __strTrim(char **stringPtr)
{
    char *ns;
    size_t pos1;
    size_t pos2;

    assert(stringPtr != NULL);
    assert(*stringPtr != NULL);
    ns = strCreate();
    pos1 = strFindFirstNotOf(*stringPtr, " \t\r\n");
    if (pos1 >= 0)
    {
        pos2 = strFindLastNotOf(&(*stringPtr)[pos1], " \t\r\n");
        if (pos2 >= 0) __strCopySub(&ns, &(*stringPtr)[pos1], 0, pos2 + 1);
    }
    strFree(*stringPtr);
    *stringPtr = ns;
}

/**
 * Splits a string into parts and returns them in a string list. You have to
 * free the returned list with strListFreeWithItems() when you no longer need
 * it.
 *
 * @param delimiter
 *            The delimiter string.
 * @param string
 *            The string to split.
 * @param limit
 *            The maximum number of splits to perform. 0 always returns an
 *            empty list. A negative number means unlimited splits.
 * @param size
 *            The list size pointer. After splitting this variable contains
 *            the size of the list.
 * @return The splitted strings as a string list.
 */
char ** strSplit(char *delimiter, char *string, int limit, size_t *size)
{
    char *cur, *item;
    char **list;
    ssize_t pos;

    assert(delimiter != NULL);
    assert(strlen(delimiter) > 0);
    assert(string != NULL);
    listCreate(list, size);
    if (limit == 0) return list;
    cur = string;
    while (limit > 1 || limit < 0)
    {
        pos = strFindFirst(cur, delimiter);
        if (pos == -1) break;
        item = strCreate();
        __strCopySub(&item, cur, 0, pos);
        listAdd(list, item, size);
        cur = &cur[pos + strlen(delimiter)];
        if (limit > 0) limit--;
    }
    item = strCreate();
    __strCopy(&item, cur);
    listAdd(list, item, size);
    return list;
}

/**
 * Joins all the strings in the specified list into a single string. The strings
 * are glued together with the specified <var>glue</var> string. The returned
 * string is allocated on the heap so you have to free it when you no longer
 * need it.
 *
 * @param glue
 *            The glue string.
 * @param list
 *            The list containing the strings to join.
 * @param size
 *            The size of the string list.
 * @return The joined string.
 */
char *strJoin(char *glue, char **list, int size)
{
    char *string;
    int i;

    string = strCreate();
    for (i = 0; i < size; i++)
    {
        if (i) __strAppend(&string, glue);
        __strAppend(&string, list[i]);
    }
    return string;
}

/**
 * Checks if the specified string <var>string</var> ends with the specified
 * search string <var>search</var>. Returns 1 if true and 0 if false.
 *
 * @param string
 *            The string to check.
 * @param search
 *            The string to look for.
 * @return 1 if string ends with specified search string, 0 if not.
 */
int strEndsWith(char *string, char *search)
{
    int stringLen, searchLen;

    assert(string != NULL);
    assert(search != NULL);
    stringLen = strlen(string);
    searchLen = strlen(search);
    assert(searchLen > 0);
    if (searchLen > stringLen) return 0;
    return strcmp(&string[stringLen - searchLen], search) == 0;
}

/**
 * Case-insensitive version of strEndsWith().
 *
 * @param string
 *            The string to check.
 * @param search
 *            The string to look for.
 * @return 1 if string ends with specified search string, 0 if not.
 */
int strEndsWithIgnoreCase(char *string, char *search)
{
    int stringLen, searchLen;

    assert(string != NULL);
    assert(search != NULL);
    stringLen = strlen(string);
    searchLen = strlen(search);
    assert(searchLen > 0);
    if (searchLen > stringLen) return 0;
    return _strcasecmp(&string[stringLen - searchLen], search) == 0;
}

/**
 * Checks if the specified string <var>string</var> starts with the specified
 * search string <var>search</var>. Returns 1 if true and 0 if false.
 *
 * @param string
 *            The string to check.
 * @param search
 *            The string to look for.
 * @return 1 if string starts with specified search string, 0 if not.
 */
int strStartsWith(char *string, char *search)
{
    int stringLen, searchLen;

    assert(string != NULL);
    assert(search != NULL);
    stringLen = strlen(string);
    searchLen = strlen(search);
    assert(searchLen > 0);
    if (searchLen > stringLen) return 0;
    return strncmp(string, search, searchLen) == 0;
}

/**
 * Case-insensitive version of strStartsWith().
 *
 * @param string
 *            The string to check.
 * @param search
 *            The string to look for.
 * @return 1 if string starts with specified search string, 0 if not.
 */
int strStartsWithIgnoreCase(char *string, char *search)
{
    int stringLen, searchLen;

    assert(string != NULL);
    assert(search != NULL);
    stringLen = strlen(string);
    searchLen = strlen(search);
    assert(searchLen > 0);
    if (searchLen > stringLen) return 0;
    return _strncasecmp(string, search, searchLen) == 0;
}
