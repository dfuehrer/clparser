
#include <string.h>
//#include <stdlib.h>

#include "printargs.h"

typedef struct {
    Shell shell;
    char * arrayName;
    bool useNamespace;
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
    switch(shell){
        case CSH:
            // TODO figure out how to generalize this..
            if(useNamespace){
                pre_fmt = "set %s_%.*s=";
            }else{
                arrayName = "";
                pre_fmt = "set %s%.*s=";
            }
            break;
        case FISH:
            if(useNamespace){
                pre_fmt = "set %s_%.*s ";
            }else{
                arrayName = "";
                pre_fmt = "set %s%.*s ";
            }
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
            if(useNamespace){
                pre_fmt = "%s_%.*s=";
            }else{
                arrayName = "";
                pre_fmt = "%s%.*s=";
            }
            break;
    }
    // pre_fmt has format for array name, key len, key, the value prefix
    len += printf(pre_fmt, arrayName, keylen, key);
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
                // TODO check that all of these match this syntax
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
// TODO add array name and shell args
int printKeyValues(const MapData * node, const char * arrayName, Shell shell, bool useNameSpace){
    int len = 0;
    for(int i = 0; i < node->namesLen; ++i){
        const char * str = node->names[i];
        char * tmpstr = NULL;
        // look for - in the names of the input
        char * dashLoc = strchr(str, '-');
        if(dashLoc != NULL){
            // if found -, then duplicate str and replace all - with _
            tmpstr = strndup(str, node->nameLens[i]);
            for(dashLoc += tmpstr - str; dashLoc != NULL; dashLoc = strchr(dashLoc, '-')){
                dashLoc[0] = '_';
            }
            str = tmpstr;
        }
        // print out the key='value' or key="$i" in POSIX sh synax
        // TODO make a way to select which syntax to print out in
        //  - start with bash becuase i think there would be some great features to take advantage of (hopefully associative arrays (dicts/maps) exist)
        //  - hopefully zsh would have associative arrays or something
        //  - csh should be easy, same as POSIX sh but `set var val`, no fancy features
        //  - probably dont care at all about fish or conch but they exist maybe
        //      - i guess ksh exists too or something
        // TODO get the shell from an arg
        //  - itd be cool to get this from the calling process rather than a command line input (or default that could be overridden)
        len += printShellVar(str, node->nameLens[i], arrayName, &node->data, shell, useNameSpace);
        // TODO maybe handle this str so we dont free it multiple times in a loop
        if(tmpstr != NULL){
            // if created tmp str to replace - then free it
            free(tmpstr);
        }
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
    return printKeyValues(node, printInput->arrayName, printInput->shell, printInput->useNamespace);
    //if(node->data.ptr != NULL){
    //    return printKeyValues(node, printInput->arrayName, printInput->shell);
    ////}else if(node->data.type == INT){
    ////    free(node->data.ptr);
    ////    node->data.ptr = NULL;
    //}
    //return 0;
}

int freeNodeInts(map_t * map, MapData * node, void * input){
    if(node->data.ptr != NULL && node->data.type == INT){
        free((void *)node->data.ptr);
        node->data.ptr = NULL;
    }
    return 0;
}

// define function to parse args for sh (print)
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, ParsePrintOptions * parseOpts){
    const char ** positionalParams = NULL;
    Errors err = parseArgsBase(argc, argv, flagMap, paramMap, &positionalParams, true);

    Shell shell  = parseOpts->shell;
    bool useArgv = parseOpts->useArgv;
    // print out all values
    PrintValueData printInput = {
        .shell          = shell,
        .arrayName      = "flags",
        .useNamespace   = parseOpts->useNamespace,
    };
    printDeclareAssocArr(printInput.arrayName, shell);
    iterMapSingle(flagMap,  printKeyValuesWrapper, &printInput);
    printInput.arrayName = "params";
    printDeclareAssocArr(printInput.arrayName, shell);
    iterMapSingle(paramMap, printKeyValuesWrapper, &printInput);
    // free the ints now
    iterMapSingle(paramMap, freeNodeInts, NULL);

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
