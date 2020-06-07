#ifndef PROCESS_H
#define PROCESS_H

typedef struct pllist_t{
    // TODO decide if i want either, they arent really necessary at all (check if single by !str[1])
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

int setState(char * c);
pllist ** addParam(pllist ** lp, char * c, int state);

void printStuffs(char * str, pllist * head);
pllist * mtchStr(char * str, pllist * head);
pllist * mtchChr(char   c,   pllist * head);

#endif
