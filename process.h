#ifndef PROCESS_H
#define PROCESS_H

#include "map.h"


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
    Success = 0,
    NotAlnum = 1,
    DidNotFind,
} Errors;

// TODO rename this function
char * linkParams(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType);

State setState(char * c);


Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * defaultValues[]);
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap);
Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * defaultValues[], bool print);

#endif
