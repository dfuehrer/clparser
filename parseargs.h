#ifndef PARSEARGS_H
#define PARSEARGS_H

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
// - elvish? (no idea what this is, just saw it in a completion help thing on some program)
// - powershell (id have to look into it, but this probably doesnt make sense becuase you probably want to just hook into the powershell get-help framework (if thats how that works)
// - awk (no idea if this is practical or possible or helpful but could be cool to just have an awk script with good command line options)
typedef enum {
    SH,
    BASH,
    ZSH,
    KSH,
    CSH,
    FISH,
    XONSH,
} Shell;

typedef struct {
    bool print;
    bool unknownPositional;
} ParseArgsOptions;

char * parseArgSpec(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType, bool allowDefaults, MapData * * * const positionalArray);

State setState(char * c);


Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], const char * * positionalParams_ptr[], bool unknownPositional);
//Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], const char * * positionalParams_ptr[], const ParseArgsOptions * opts);
Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], const char * * positionalParams_ptr[], bool unknownPositional, bool print);

int printUsage(const map_t * flagMap, const map_t * paramMap, const MapData * * positionalParams, const char * progname);
int printHelp(const map_t * flagMap, const map_t * paramMap, const MapData * positionalParams[], const char * helpMessages);

#endif  // PARSEARGS_H
