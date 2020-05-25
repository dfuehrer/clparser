#ifndef PROCESS_H
#define PROCESS_H

typedef struct pllist_t{
    // int slen;
    // int singleLetter;
    char * str;
    struct pllist_t * headSame;
    struct pllist_t * nextSame;
    struct pllist_t * next;
    //struct pllist_t * prev;
} pllist;
const pllist defVal;
const pllist * const defaultValNULL = &defVal;

char * link(char * buf, pllist * head, char [] type);
//char * sepBuff(char * buf, char * slBuf, char * wBuf, char type[], char ** link);

#endif
