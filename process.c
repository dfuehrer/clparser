#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <error.h>
#include <stdarg.h>
#include "process.h"

#define ARG_SPACE   (50)


typedef struct nllist_s {
    StringView sv;
    MapNode * origNode;
    struct nllist_s * next;
} nllist_t;

// TODO add C++ library

// so the idea of this is that it will go through buff and add pointers to a linked list to dostuffs
// so im gonna need to make linked list functions to do all the linked list things like swapping pointers and stuff
// also i might want to make it sort the linked list as it isbeing created and stuff but im not sure
// cause sorting the list means i have to keep looping through it and that probably wont save me much inwhat i need to do later
// i may also want to specify if itis a single letter
// the other thing i could do is make a separate list for the single letters and words
// i think i want separate lists for the parameters and words and have this do them separately
// also this should probably replace spaces and commas with \0 so that it looks like a full string then it can be easily compared
//
// - maybe iterate through all arg vals to find how many tben create fmt str
//  - now that i think about it i dont have a way of putting together the variadic arvuments so i need another interface
// then parse the args by setting stuff in the maps and getting stuff from the maps
// TODO also make a handy function to easily add values from args to the list? (for C interface that doesnt need to parse a string)
char * parseArgSpec(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType, bool allowDefaults){
    // create all the links and variables
    // also figure out dynamically creating all these linked list node things
    char * typeptr = strstr(buf, argType);
    if(typeptr == NULL){
        return buf;         // didnt find this argType (not necessarily a fault but if both are missing then it doesnt work), return buf to indicate just didnt find this
    }
    typeptr += strlen(argType);
    char * end = strchr(typeptr, ';');
    if(end == NULL){
        fprintf(stderr, "spec must end in ';', but got '%s'\n", buf);
        return NULL;        // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
    }
    char * c = typeptr;
    for( ; *c == ' '; c++);
    char * ce = c + 1;
    // i guess for this i want different values like
    // 1 was a space so were on   a  new set of values
    // 2 was a comma so were on the same set of values
    // 3 was an equals so were looking at a default val
    // 4 was a semicolon so were done
    // 0 means error
    State state = Space, nextState = Space;
    // loop throguh everything and make all the linked list stuffs
    //printf("state = %d, nextState = %d\n", state, nextState);
    sllist_t * head = NULL;
    nllist_t * negHead = NULL;
    for( ; nextState != Semicolon; c = ++ce){
        void * defaultVal = defaultValue;
        DataType type = defaultType;
        sllist_t * node = head;
        bool haveDefault = false;
        // loop through a param/flag list
        int sum = 0;
        for( ; ; c = ++ce){
            state = nextState;  // set state for next iteration
            // TODO check for whitespace etc or decide its not allowed and then error gracefully
            for( ; isalnum(*ce) || (*ce == '-') || (*ce == '_'); ++ce);
            // set next state based on the upcoming symbol
            nextState = setState(ce);
            // if the next state is error then exit unless were on the last line
            if(nextState == Error && (state != Semicolon)){
                fprintf(stderr, "spec got bad value '%c' before end with ';'", *ce);
                return NULL;
            }
            // add a new node onto the linked list
            // TODO check for issues in the format (like multiple =)
            //printf("param: '%.*s', state = %d, nextState = %d\n", (int)(ce - c), c, state, nextState);
            if(state == Space || state == Comma){
                ++sum;
                sllist_t * n = node;
                // if we are at the end of the list, then just add a new at the beginning
                // we keep the list through all the params/flags and just override the data
                // NOTE this only works if the addMapMembers_fromList function uses the length of the list directly
                if(node == NULL){
                    //printf("allocating new node\n");
                    node = (sllist_t *) calloc(1, sizeof (sllist_t));
                    node->next = head;
                    head = node;
                    n = node;
                    node = NULL;
                }else{
                    node = node->next;
                }
                n->sv.str = c;
                n->sv.len = ce - c;
            }else if(state == Equals){
                if(!allowDefaults){
                    if(*c != '-'){
                        // error if not allowing defaults and this isnt a negation
                        fprintf(stderr, "not allowing defaults, but got '=%s', use '=-%s' for negation\n", c, c);
                        return NULL;
                    }
                    // increment c to get past -
                    ++c;
                    // add this negation to a list to iter over later
                    nllist_t * negNode = (nllist_t *) calloc(1, sizeof (nllist_t));
                    negNode->next = negHead;
                    negHead = negNode;
                    negNode->sv.str = c;
                    negNode->sv.len = ce - c;
                }else{
                    // TODO have some checking so there arent Equals in flags
                    StringView * sv = (StringView *) calloc(1, sizeof (StringView));
                    defaultVal = sv;
                    sv->str = c;
                    sv->len = (int) (ce - c);
                    type = STRING_VIEW;
                    haveDefault = true;
                }
            }
            if(nextState == Space || nextState == Semicolon){
                break;
            }
        }
        MapNode * mapNode = addMapMembers_fromList(map, defaultVal, type, head, sum, haveDefault);
        if(!allowDefaults){
            // if no default then check if there are negations
            for(nllist_t * negNode = negHead; negNode != NULL && negNode->origNode == NULL; negNode = negNode->next){
                negNode->origNode = mapNode;
            }
        }
    }
    // cleanup the list
    for(sllist_t * node = head ; node != NULL; ){
        sllist_t * n = node->next;
        free(node);
        node = n;
    }
    // find the negations (and cleanup list)
    for(nllist_t * node = negHead ; node != NULL; ){
        nllist_t * n = node->next;
        // find the negation map node
        MapNode * negMapNode = getMapNode(map, node->sv.str, node->sv.len);
        if(negMapNode == NULL){
            // if didnt find the negation then error
            fprintf(stderr, "could not find '%.*s' defined for negation for %.*s\n", node->sv.len, node->sv.str, node->origNode->nameLens[0], node->origNode->names[0]);
            return NULL;
        }
        //node->origNode->data.negation = negMapNode;
        // add negation node negation to this node's list so the negation node will negate this node
        mnllist_t * mnlNode = (mnllist_t *) malloc(sizeof (mnllist_t));
        mnlNode->node = node->origNode;
        mnlNode->next = negMapNode->data.negations;
        negMapNode->data.negations = mnlNode;

        free(node);
        node = n;
    }

    return end + 1;
}

// TODO if after an equals dont change the state till you see a space
//  and then within that if there are quotes then wait till after the closing quote to quit from the space
//  if you wanted a ; you would also need to quote it
//  but this would allow you to put symbols and stuff in the default values so you dont have to set them later in bash cause who wants to do that
State setState(char * c){
    State state;
    // TODO what if the states were actually set to their ascii char values, would make setting much easier
    switch(*c){
        case ' ':
            state = Space;
            break;
        case ',':
            state = Comma;
            break;
        case '=':
            state = Equals;
            break;
        case ';':
            state = Semicolon;
            break;
        default:
            // TODO figure out if i want to have it replace the character if its not one of these cause if so then put the *c = '\0' after the switch-case cause its all the same
            // it probably doesnt matter really but the : on the second section would be replaced and then that would be a tad weird
            state = Error;
            break;
    }
    return state;
}



// print out values of a key in the map in POSIX sh syntax
//void printKeyValues(const map_t * map, const char * key, int len){
//    MapNode * node = getMapNode(map, key, len);
int printKeyValues(const MapNode * node){
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
        char * pre, * post;
        switch(node->data.type){
            // TODO maybe make sure the strings are ' escaped
            case STR:
            case STRING_VIEW:
            case BOOL:
            case CHAR:
                pre = post = "'";
                break;
            case INT:
                pre = "\"${";
                post = "}\"";
                break;
        }
        printf("%.*s=%s", node->nameLens[i], str, pre);
        printArgData(&node->data, stdout);
        printf("%s\n", post);
        // TODO maybe handle this str so we dont free it multiple times in a loop
        if(tmpstr != NULL){
            // if created tmp str to replace - then free it
            free(tmpstr);
        }
    }
    return len;
}


int printKeyValuesWrapper(map_t * map, MapNode * node){
    if(node->data.ptr != NULL){
        return printKeyValues(node);
    //}else if(node->data.type == INT){
    //    free(node->data.ptr);
    //    node->data.ptr = NULL;
    }
    return 0;
}
int freeNodeInts(map_t * map, MapNode * node){
    if(node->data.ptr != NULL && node->data.type == INT){
        free((void *)node->data.ptr);
        node->data.ptr = NULL;
    }
    return 0;
}

Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * * defaultValues_ptr[], bool print){

    // defaultValues_ptr is a 
    // TODO maybe try to accomodate if defaultValues_ptr is NULL
    *defaultValues_ptr = (const char **) calloc((argc), sizeof (const char *));
    memset(*defaultValues_ptr, (long) NULL, argc);
    const char ** thisDef = *defaultValues_ptr;

    bool checkFlags;

    for(int i = 1; i < argc; ++i){
        checkFlags = true;
        // if arg starts with - then its a single letter / word param
        if(argv[i][0] == '-'){
            // if second char is not - then its a single letter params
            if(argv[i][1] != '-'){
                // this is a single letter flag so loop through all the next letters
                //printf("sing let:\t");
                //puts(argv[i]);
                for(const char * f = argv[i] + 1; *f; f++){
                    //printf("let:\t%c\n", *f);
                    if(!isalnum(*f)){
                        fprintf(stderr, "char '%c' in %s not allowed\n", *f, argv[i]);
                        return NotAlnum;   // error if f not alphanumeric
                    // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                    // if this is last char and the next param doesnt have a - then this is probably a parameter
                    }else if(((f[1]) == '\000') && ((i+1 < argc) && (argv[i+1][0] != '-'))){
                        MapNode * node = getMapNode(paramMap, f, 1);
                        if(node != NULL){
                            ++i;
                            if(print){
                                // set node value to this argv ind to use for $i var
                                int * varnum = (int *) calloc(1, sizeof (int));
                                *varnum = i;
                                node->data.ptr  = varnum;
                                node->data.type = INT;
                            }else{
                                if(node->data.type == INT){
                                    int * num = (int *) calloc(1, sizeof (int));
                                    *num = strtol(argv[i], NULL, 0);
                                    node->data.ptr  = num;
                                }else{
                                    node->data.ptr  = argv[i];
                                    node->data.type = STR;
                                }
                            }
                            //printf("wtharg:\t");
                            //puts(argv[i]);
                            checkFlags = false;
                        }else{
                            //puts("didnt find");
                            //return DidNotFind;       // didnt find it
                        }
                    }
                    if(checkFlags){                  // i think the only other option is its a flag
                        MapNode * node = getMapNode(flagMap, f, 1);
                        if(node == NULL){
                            //puts("didnt find");
                            printf("did not find '%c' as either a flag or param\n", *f);
                            return DidNotFind;       // didnt find it
                        }
                        node->data.ptr  = &flagTrue;
                        node->data.type = BOOL;
                        for(mnllist_t * negNode = node->data.negations; negNode != NULL; negNode = negNode->next){
                            negNode->node->data.ptr  = &flagFalse;
                            negNode->node->data.type = BOOL;
                        }
                    }
                }
            }else if(argv[i][2] == '\000'){
                // if this arg is just '--', then all the rest of the args are just default vals, dont parse
                for(++i; i < argc; ++i, ++thisDef){
                    *thisDef = argv[i];
                }
                break;
            }else{
                // this is a word thign
                const char * word = argv[i] + 2;
                const char * val  = NULL;
                int wordlen = -1;
                //printf("word:\t");
                //puts(word);
                bool isAlnum = true;
                for(const char * c = word; *c && isAlnum; c++){
                    if(*c == '='){
                        wordlen = c - word;
                        val = c + 1;
                        break;
                    }
                    isAlnum = (isalnum(*c) || (*c == '-') || (*c == '_'));
                }
                if(wordlen == -1){
                    // if no = in word, then set len to strlen
                    wordlen = strlen(word);
                    if((i+1 < argc) && (argv[i+1][0] != '-')){
                        // if next word doesn't start with - then set that as the val to try
                        val = argv[i+1];
                    }
                }
                if(!isAlnum){
                    //puts("well its gotta be alnum");
                    fprintf(stderr, "arg %s not allowed, should be alnum or -/_", argv[i]);
                    return NotAlnum;   // error if f not alphanumeric
                // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                //}else if((i+1 < argc) && (argv[i+1][0] != '-')){   // then this is probably a parameter
                }else if(val != NULL){   // then this is probably a parameter
                    MapNode * node = getMapNode(paramMap, word, wordlen);
                    if(node != NULL){
                        if(val == argv[i+1]){
                            ++i;
                        }
                        if(print){
                            if(val == argv[i]){
                                // set node value to this argv ind to use for $i var
                                int * varnum = (int *) calloc(1, sizeof (int));
                                *varnum = i;
                                node->data.ptr  = varnum;
                                node->data.type = INT;
                            }else{
                                node->data.ptr  = val;
                                node->data.type = STR;
                            }
                        }else{
                            if(node->data.type == INT){
                                int * num = (int *) calloc(1, sizeof (int));
                                *num = strtol(val, NULL, 0);
                                node->data.ptr = num;
                            }else{
                                node->data.ptr  = val;
                                node->data.type = STR;
                            }
                        }
                        //printf("wtharg:\t");
                        //puts(argv[i]);
                        checkFlags = false;
                    }else{
                        //puts("didnt find");
                        //return DidNotFind;       // didnt find it
                    }
                }
                if(checkFlags){                                          // i think the only other option is its a flag
                    MapNode * node = getMapNode(flagMap, word, wordlen);
                    if(node == NULL){
                        //puts("didnt find");
                        printf("did not find '%s' as either a flag or param\n", word);
                        return DidNotFind;       // didnt find it
                    }
                    node->data.ptr  = &flagTrue;
                    // TODO this shouldnt be necessary
                    node->data.type = BOOL;
                    for(mnllist_t * negNode = node->data.negations; negNode != NULL; negNode = negNode->next){
                        negNode->node->data.ptr  = &flagFalse;
                        negNode->node->data.type = BOOL;
                    }
                }
            }
        }else{
            // this is a default
            //printf("default:\t");
            *thisDef = argv[i];
            //puts(*thisDef);
            ++thisDef;
        }
    }

    // now print out the values without parameters (defaults)
    if(print){
        // print out all values
        iterMapSingle(flagMap,  printKeyValuesWrapper);
        iterMapSingle(paramMap, printKeyValuesWrapper);
        // free the ints now
        iterMapSingle(paramMap, freeNodeInts);

        // TODO have an option to not lose the optional args
        // TODO have this use something that defines the shell syntax
        printf("set --");
        //printf("defaults='%s", *defaultValues_ptr[0]);
        int i = 1;
        for(thisDef = *defaultValues_ptr; *thisDef != NULL; ++thisDef){
            //printf(" '%s'", *thisDef);
            // find index of this string for printing out var to use
            for( ; i < argc && *thisDef != argv[i]; ++i);
            printf(" \"${%d}\"", i);
            //printf(" %s", *thisDef);
        }
        printf("\n");
        //printf("'\n");
    }

    return Success;
}


// define function to parse args for C lib (no printing)
Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * * defaultValues_ptr[]){
    return parseArgsBase(argc, argv, flagMap, paramMap, defaultValues_ptr, false);
}
// define function to parse args for sh (print)
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap){
    const char ** defaultValues_ptr = NULL;
    // TODO should we just use bools for the print stuff?
    Errors err = parseArgsBase(argc, argv, flagMap, paramMap, &defaultValues_ptr, true);
    free(defaultValues_ptr);
    return err;
}


int printFlags(MapNode * node, FILE * file){
    int len = fprintf(file, " [ ");
    for(int i = 0; i < node->namesLen; ++i){
        if(node->nameLens[i] == 1){
            len += fprintf(file, "-%c ", node->names[i][0]);
        }else{
            len += fprintf(file, "--%.*s ", node->nameLens[i], node->names[i]);
        }
    }
    len += fprintf(file, "]");
    return len;
}
int printFlagsWrapper(map_t * map, MapNode * node){
    return printFlags(node, stderr);
}

int printParams(MapNode * node, FILE * file, bool optional){
    int i;
    int lastWordInd = 0;
    int len  = fprintf(file, " ");
    if(optional){
        len += fprintf(file, "[ ");
    }
    for(i = 0; i < node->namesLen; ++i){
        if(node->nameLens[i] == 1){
            len += fprintf(file, "-%c ", node->names[i][0]);
        }else{
            len += fprintf(file, "--%.*s ", node->nameLens[i], node->names[i]);
            lastWordInd = i;
        }
    }
    len += fprintf(file, "%.*s", node->nameLens[lastWordInd], node->names[lastWordInd]);
    if(node->data.defaultData != NULL && node->data.defaultData->ptr != NULL){
        len += fprintf(file, " = ");
        len += printArgData(node->data.defaultData, file);
    }
    if(optional){
        len += fprintf(file, " ]");
    }
    return len;
}
int printParamsWrapper(map_t * map, MapNode * node){
    return printParams(node, stderr, true);
}

void printUsage(map_t * flagMap, map_t * paramMap, const char * progname){
    fprintf(stderr, "Usage:\t%s", progname);
    iterMapSingle(flagMap, printFlagsWrapper);
    iterMapSingle(paramMap, printParamsWrapper);
    fprintf(stderr, "\n");
}

void printHelp(map_t * flagMap, map_t * paramMap, char fmt[], ...){
    char * key = fmt;
    char * commaPos;
    char * helpText;
    MapNode * node;
    int printedLen;
    va_list args;
    va_start(args, fmt);
    for(commaPos = strchr(key, ','); ; key = commaPos + 1, commaPos = strchr(key, ',')){
        if(commaPos == NULL){
            commaPos = strchr(key, '\000');
        }
        helpText = va_arg(args, char *);
        node = getMapNode(paramMap, key, commaPos - key);
        if(node != NULL){
            //bool optional = (strstr(helpText, "optional") != NULL);
            printedLen = printParams(node, stderr, !node->data.required);
        }else{
            node = getMapNode(flagMap, key, commaPos - key);
            if(node == NULL){
                error(7, 0, "key '%.*s' does not exist in map", (int) (commaPos - key), key);
            }
            printedLen = printFlags(node, stderr);
        }
        fprintf(stderr, "%-*s %s\n", ARG_SPACE - printedLen - 1, "", helpText);
        if(commaPos[0] == '\000'){
            break;
        }
    }
    va_end(args);
}
