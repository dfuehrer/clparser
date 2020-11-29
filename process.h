#ifndef PROCESS_H
#define PROCESS_H

typedef struct pllist_t{
    char * str;
    struct pllist_t * headSame;
    struct pllist_t * nextSame;
    struct pllist_t * next;
    //struct pllist_t * prev;
} pllist;
static const pllist defVal;
// TODO figure out how to make this not error in gcc
extern const pllist * const defaultValNULL;

char * linkParams(char * buf, pllist ** head, char argType[]);

int setState(char * c);
pllist ** addParam(pllist ** lp, char * c, int state);

void clearMems(pllist * head);

int parseArgs(int argc, char ** argv, pllist * flagHead, pllist * paramHead);

void printStuffs(char * str, pllist * head);
pllist * mtchStr(char * str, pllist * head);
pllist * mtchChr(char   c,   pllist * head);

#endif
