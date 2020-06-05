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
static const pllist * const defaultValNULL = &defVal;

char * linkParams(char * buf, pllist ** head, char argType[]);
//char * sepBuff(char * buf, char * slBuf, char * wBuf, char type[], char ** link);

int setState(char * c);
pllist ** addParam(pllist ** lp, char * c, int state);

#endif
