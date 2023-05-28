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

// TODO are there any other shells i should try to support?
typedef enum {
    SH,
    BASH,
    ZSH,
    KSH,
    CSH,
    FISH,
    XONSH,
} Shell;

// TODO rename this function
char * parseArgSpec(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType, bool allowDefaults);

State setState(char * c);


Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * * defaultValues_ptr[]);
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, Shell shell, bool useArgv);
Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * * defaultValues_ptr[], bool print, Shell shell, bool useArgv);

void printUsage(map_t * flagMap, map_t * paramMap, const char * progname);
void printHelp (map_t * flagMap, map_t * paramMap, char fmt[], ...);

#endif
