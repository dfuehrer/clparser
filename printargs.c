
#include <string.h>
//#include <stdlib.h>

#include "printargs.h"

typedef struct {
    Shell shell;
    char * arrayName;
    bool useNamespace;
    bool noOutputEmpty;
} PrintValueData;

int printDeclareAssocArr(const char * arrayName, Shell shell){
    const char * fmt = "";
    switch(shell){
        case XONSH:
            fmt = "%s = {}\n";
            break;
        case BASH:
        case ZSH:
        case KSH:
            fmt = "declare -A %s\n";
            break;
        case SH:
        case CSH:
        case FISH:
        default:
            break;
    }
    return printf(fmt, arrayName);
}

int printShellValue(const ArgData * data, Shell shell);

int printShellVar(const char * key, int keylen, const char * arrayName, const ArgData * data, Shell shell, bool useNamespace){
    //printf("printing shell var: %.*s in array %s with data %p (type %d) and shell %i\n", keylen, key, arrayName, data->ptr, data->type, shell);
    int len = 0;
    const char * pre_fmt;
    if(arrayName == NULL){
        arrayName = "";
    }
    // in sh, csh, and fish, there are no associative arrays, so just set variables
    // TODO make no namespace always use plain variables and with namespace use assoc arrays if possible, prefix if not
    //  - this option can be set by shell by default (shells without assoc arrays set namespace false, else true (unless given))
    //  - may need second no-namespace flag or something like the override/maintain argv
    if(useNamespace){
        switch(shell){
            case CSH:
                // TODO figure out how to generalize this..
                pre_fmt = "set %s_%.*s=";
                break;
            case FISH:
                pre_fmt = "set %s_%.*s ";
                break;
            case BASH:
            case ZSH:
            case KSH:
                pre_fmt = "%s[%.*s]=";
                break;
            case XONSH:
                pre_fmt = "%s['%.*s']=";
                break;
            case SH:
            default:
                pre_fmt = "%s_%.*s=";
                break;
        }
    }else{
        switch(shell){
            case CSH:
                pre_fmt = "set %.*s=";
                break;
            case FISH:
                pre_fmt = "set %.*s ";
                break;
            case BASH:
            case ZSH:
            case KSH:
            case XONSH:
            case SH:
            default:
                pre_fmt = "%.*s=";
                break;
        }
    }
    // pre_fmt has format for array name, key len, key, the value prefix
    if(useNamespace){
        len += printf(pre_fmt, arrayName, keylen, key);
    }else{
        len += printf(pre_fmt, keylen, key);
    }
    len += printShellValue(data, shell);
    len += printf("\n");
    return len;
}

int printShellValue(const ArgData * data, Shell shell){
    int len = 0;
    char * pre, * post;
    pre = post = "";
    switch(data->type){
        // TODO make sure the strings are ' escaped
        case STR:
        case STRING_VIEW:
        case CHAR:
            pre = post = "'";
            break;
        case INT:
            // INT case: using command line argv
            switch(shell){
                case FISH:
                    pre  = "$argv[";
                    post = "]";
                    break;
                case XONSH:
                    // apparently in xonsh args are $ARG1 for the first arg ($ARGS[1] only available in python mode)
                    pre  = "$ARG";
                    break;
                // sh and normal shells use "${1}" ({} necessary for args > 1 digit)
                case SH:
                case BASH:
                case ZSH:
                case KSH:
                case CSH:
                default:
                    pre  = "\"${";
                    post = "}\"";
                    break;
            }
            break;
        default:
            break;
    }
    len += printf("%s", pre);
    if(shell == XONSH && data->type == BOOL){
        // handle xonsh bool separately since it has to be capitalized
        len += printf("%s", (*(bool *) data->ptr) ? "True\0" : "False");
    }else{
        len += printArgData(data, stdout);
    }
    len += printf("%s", post);
    return len;
}

// print out values of a key in the map in shell syntax
int printKeyValues(const MapData * node, PrintValueData * inputData){
    int len = 0;
    for(int i = 0; i < node->namesLen; ++i){
        const char * str = node->names[i];
        // TODO should i actually be allocating this every time even though we usually wont need it
        char tmpstr[node->nameLens[i] + 1];
        // look for - in the names of the input
        char * dashLoc = strchr(str, '-');
        if(dashLoc != NULL){
            // if found -, then duplicate str and replace all - with _
            strncpy(tmpstr, str, node->nameLens[i]);
            for(dashLoc += tmpstr - str; dashLoc != NULL; dashLoc = strchr(dashLoc, '-')){
                dashLoc[0] = '_';
            }
            str = tmpstr;
        }
        // print out the key='value' or key="$i" in POSIX sh synax
        len += printShellVar(str, node->nameLens[i], inputData->arrayName, &node->data, inputData->shell, inputData->useNamespace);
    }
    return len;
}

int printInitArgv(Shell shell){
    const char * argv = "";
    switch(shell){
        case SH:
        case BASH:
        case ZSH:
        case KSH:
            argv = "set -- ";
            break;
        case CSH:
            argv = "set argv=(";
            break;
        case FISH:
            argv = "set argv ";
            break;
        case XONSH:
            argv = "$ARGS = [";
            break;
    }
    return printf("%s", argv);
}

int printInitArray(const char * arrayName, Shell shell){
    const char * argsFmt;
    switch(shell){
        case SH:
            argsFmt = "%s=";
            break;
        case BASH:
        case ZSH:
        case KSH:
            argsFmt = "declare -a %s=(";
            break;
        case CSH:
            argsFmt = "set %s=(";
            break;
        case FISH:
            argsFmt = "set %s ";
            break;
        case XONSH:
            argsFmt = "%s = [";
            break;
    }
    return printf(argsFmt, arrayName);
}

int printInitArgs(const char * arrayName, Shell shell, bool useArgv){
    if(useArgv){
        return printInitArgv(shell);
    }
    return printInitArray(arrayName, shell);
}

int printEndArgv(Shell shell){
    const char * str = "";
    switch(shell){
        case SH:
        case BASH:
        case ZSH:
        case KSH:
        case FISH:
            break;
        case CSH:
            str = ")";
            break;
        case XONSH:
            str = "]";
            break;
    }
    return printf("%s\n", str);
}

int printEndArray(Shell shell){
    const char * str = "";
    switch(shell){
        case SH:
        case FISH:
            break;
        case BASH:
        case ZSH:
        case KSH:
        case CSH:
            str = ")";
            break;
        case XONSH:
            str = "]";
            break;
    }
    return printf("%s\n", str);
}

int printEndArgs(Shell shell, bool useArgv){
    if(useArgv){
        return printEndArgv(shell);
    }
    return printEndArray(shell);
}

int printArraySep(Shell shell, bool useArgv){
    const char * sep = " ";
    switch(shell){
        case SH:
        case BASH:
        case ZSH:
        case KSH:
        case CSH:
        case FISH:
            break;
        case XONSH:
            sep = ", ";
            break;
    }
    if(!useArgv && shell == SH){
        // for sh, need all args to be 1 string, so hack that together by having spaces be strings, no spaces in between (normally itd make more sense to just not have quotes around everything but thats harder to get around)
        sep = "\" \"";
    }
    return printf("%s", sep);
}

int printKeyValuesWrapper(map_t * map, MapData * node, void * input){
    PrintValueData * printInput = input;
    (void) map;
    //return printKeyValues(node, printInput->arrayName, printInput->shell, printInput->useNamespace);
    // TODO maybe have an options for whether or not to output empty variables
    //  - in sh it is unnecessary and would mask the values of flags of the same names
    //      - sometimes nice to have flag to say "do this thing" with optional parameter of the same name that says how
    //  - of course, just using different names (or using the namespace option) will solve this issue, but its slightly less elegant
    //      - this does have to be an option because many shells will error if the variable isnt declared
    if(printInput->noOutputEmpty && node->data.ptr == NULL){
        return 0;
    }
    return printKeyValues(node, printInput);
}

int freeNodeInts(map_t * map, MapData * node, void * input){
    (void) map;
    (void) input;
    if(node->data.ptr != NULL && node->data.type == INT){
        free((void *)node->data.ptr);
        node->data.ptr = NULL;
    }
    return 0;
}

// define function to parse args for sh (print)
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], ParsePrintOptions * parseOpts){
    const char ** positionalParams = NULL;
    Errors err = parseArgsBase(argc, argv, flagMap, paramMap, positionalNodes, &positionalParams, parseOpts->unknownPositional, true);

    Shell shell  = parseOpts->shell;
    bool useArgv = parseOpts->useArgv;
    // print out all values
    PrintValueData printInput = {
        .shell          = shell,
        .arrayName      = "flags",
        .useNamespace   = parseOpts->useNamespace,
        .noOutputEmpty  = parseOpts->noOutputEmpty,
    };
    printDeclareAssocArr(printInput.arrayName, shell);
    iterMapSingle(flagMap,  printKeyValuesWrapper, &printInput);
    printInput.arrayName = "params";
    printDeclareAssocArr(printInput.arrayName, shell);
    iterMapSingle(paramMap, printKeyValuesWrapper, &printInput);
    printInput.arrayName = "positionals";
    printDeclareAssocArr(printInput.arrayName, shell);
    //for(MapData ** posParam_ptr = positionalNodes; *posParam_ptr != NULL; ++posParam_ptr){
    //    printKeyValuesWrapper(NULL, *posParam_ptr, &printInput);
    //}
    // free the ints now
    iterMapSingle(paramMap, freeNodeInts, NULL);
    if(positionalNodes != NULL){
        for(MapData ** posParam_ptr = positionalNodes; *posParam_ptr != NULL; ++posParam_ptr){
            // TODO technically this could do the same thing as the args below instead of needing allocated args (this is going to be in order so it should work easily)
            printKeyValuesWrapper(NULL, *posParam_ptr, &printInput);
            freeNodeInts(NULL, *posParam_ptr, NULL);
        }
    }

    printInitArgs("args", shell, useArgv);
    int i = 1;
    ArgData argvData = {
        .ptr  = &i,
        .type = INT,
    };
    for(const char ** thisDef = positionalParams; *thisDef != NULL; ++thisDef){
        if(i != 1){
            printArraySep(shell, useArgv);
        }
        // find index of this string for printing out var to use
        for( ; i < argc && *thisDef != argv[i-1]; ++i);
        printShellValue(&argvData, shell);
    }
    printEndArgs(shell, useArgv);

    free(positionalParams);
    return err;
}

int printExit(Shell shell){
    if(shell == XONSH){
        return printf("exit()\n");
    }
    return printf("exit\n");
}
