#ifndef PRINTARGS_H
#define PRINTARGS_H

#include "parseargs.h"

typedef struct {
    Shell shell;
    bool useArgv;
    bool useNamespace;
} ParsePrintOptions;

Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, ParsePrintOptions * parseOpts);

int printExit(Shell shell);

#endif  // PRINGARGS_H
