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

// TODO maybe instead of naming by what char indicates what the state is
// label by what it actually indicates
typedef enum State_t {
    Error,      // 0 means error
    Space,      // 1 was a space so were on   a  new set of values
    Comma,      // 2 was a comma so were on the same set of values
    Equals,     // 3 was an equals so were looking at a default val
    Semicolon,  // 4 was a semicolon so were done
} State;
// i need to define what errors are
typedef enum Errors_t {
    NotAlnum = 1,
    DidNotFind,
} Errors;

char * linkParams(char * buf, pllist ** head, char argType[]);

State setState(char * c);
pllist ** addParam(pllist ** lp, char * c, State state);

void clearMems(pllist * head);

Errors parseArgs(int argc, char ** argv, pllist * flagHead, pllist * paramHead);

void printStuffs(char * str, pllist * head);
pllist * mtchStr(char * str, pllist * head);
pllist * mtchChr(char   c,   pllist * head);

#endif
