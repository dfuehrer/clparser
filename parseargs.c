#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <error.h>
#include <stdarg.h>
#include "parseargs.h"

#define ARG_SPACE   (50)
#define ARRAY_ALLOC_INCR    (10)


typedef struct nllist_s {
    StringView sv;
    MapData * origNode;
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
// TODO maybe move this to the printing side cause it doesnt make sense to use this here
char * parseArgSpec(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType, bool allowDefaults, MapData * * * const positionalArray){
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
    size_t posArrLen  = 0;
    size_t posArrSize = 0;
    char * c = typeptr;
    for( ; *c == ' '; c++);
    char * ce = c + 1;
    if(*c == ';'){
        if(positionalArray != NULL){
            *positionalArray = (MapData * *) calloc(1, sizeof (MapData *));
            (*positionalArray)[0] = NULL;
        }
        return c;
    }
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
                fprintf(stderr, "spec got bad value '%c' before end with ';'\n", *ce);
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
        MapData * mapNode = addMapMembers_fromList(map, defaultVal, type, head, sum, haveDefault);
        if(!allowDefaults){
            // if no default then check if there are negations
            for(nllist_t * negNode = negHead; negNode != NULL && negNode->origNode == NULL; negNode = negNode->next){
                negNode->origNode = mapNode;
            }
        }
        if(positionalArray != NULL){
            if(posArrLen + 1 >= posArrSize){
                // allocate larger array when full
                posArrSize += ARRAY_ALLOC_INCR;
                *positionalArray = (MapData **) realloc(*positionalArray, posArrSize * sizeof (MapData *));
                if(*positionalArray == NULL){
                    perror("allocating positional node array");
                    return NULL;
                }
                // clear out the rest of the array
                memset(*positionalArray + posArrLen, 0, (posArrSize - posArrLen) * sizeof (MapData *));
            }
            (*positionalArray)[posArrLen] = mapNode;
            ++posArrLen;
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
        MapData * negMapNode = getMapNode(map, node->sv.str, node->sv.len);
        if(negMapNode == NULL){
            // if didnt find the negation then error
            fprintf(stderr, "could not find '%.*s' defined for negation for %.*s\n", node->sv.len, node->sv.str, node->origNode->nameLens[0], node->origNode->names[0]);
            return NULL;
        }
        // add negation node negation to this node's list so the negation node will negate this node
        setNodeNegation(node->origNode, negMapNode);

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
            if(isspace(*c)){
                state = Space;
                break;
            }
            // TODO figure out if i want to have it replace the character if its not one of these cause if so then put the *c = '\0' after the switch-case cause its all the same
            // it probably doesnt matter really but the : on the second section would be replaced and then that would be a tad weird
            state = Error;
            break;
    }
    return state;
}

Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], const char * * positionalParams_ptr[], bool unknownPositional, bool print){

    // positionalParams_ptr is a ptr to an array of positional args
    // TODO maybe try to accomodate if positionalParams_ptr is NULL
    *positionalParams_ptr = (const char **) calloc((argc+1), sizeof (const char *));
    if(positionalParams_ptr == NULL){
        error(4, 0, "couldnt allocate positional param array");
    }
    memset(*positionalParams_ptr, (long) NULL, argc+1);
    const char ** thisDef = *positionalParams_ptr;
    MapData ** thisPosNode = positionalNodes;

    //bool unknownPositional = opts->unknownPositional;
    //bool print = opts->print;
    bool checkFlags;
    bool allPositional = false;

    //for(int i = 1; i < argc; ++i){
    for(int i = 0; i < argc; ++i){
        //printf("on arg '%s'\n", argv[i]);
        checkFlags = true;
        // if arg starts with - and has other characters, then its a single letter / word param
        if(!allPositional && argv[i][0] == '-' && argv[i][1] != '\000'){
            // if second char is not - then its a single letter params
            if(argv[i][1] != '-'){
                // this is a single letter flag so loop through all the next letters
                //fprintf(stderr, "sing let:\t'%s'\n", argv[i]);
                for(const char * f = argv[i] + 1; *f; f++){
                    //printf("let:\t'%c'\n", *f);
                    if(!isalnum(*f)){
                        fprintf(stderr, "char '%c' in %s not allowed\n", *f, argv[i]);
                        return NotAlnum;   // error if f not alphanumeric
                    // if this is last char and the next param doesnt have a - then this is probably a parameter
                    }else if(((f[1]) == '\000') && ((i+1 < argc) && (argv[i+1][0] != '-'))){
                        MapData * node = getMapNode(paramMap, f, 1);
                        if(node != NULL){
                            ++i;
                            if(print){
                                // set node value to this argv ind to use for $i var
                                int * varnum = (int *) calloc(1, sizeof (int));
                                *varnum = i + 1;
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
                        MapData * node = getMapNode(flagMap, f, 1);
                        if(node == NULL){
                            if(unknownPositional){
                                allPositional = true;
                                --i;
                                continue;
                            }
                            //puts("didnt find");
                            fprintf(stderr, "did not find '%c' as either a flag or param\n", *f);
                            return DidNotFind;       // didnt find it
                        }
                        node->data.ptr  = &flagTrue;
                        node->data.type = BOOL;
                        for(mnllist_t * negNode = node->data.negations; negNode != NULL; negNode = negNode->next){
                            negNode->data->data.ptr  = &flagFalse;
                            negNode->data->data.type = BOOL;
                        }
                    }
                }
            }else if(argv[i][2] != '\000'){
                // this is a word thign
                const char * word = argv[i] + 2;
                const char * val  = NULL;
                int wordlen = -1;
                //fprintf(stderr, "word:\t'%s'\n", word);
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
                //}else if((i+1 < argc) && (argv[i+1][0] != '-')){   // then this is probably a parameter
                }else if(val != NULL){   // then this is probably a parameter
                    MapData * node = getMapNode(paramMap, word, wordlen);
                    if(node != NULL){
                        if(val == argv[i+1]){
                            ++i;
                        }
                        if(print){
                            if(val == argv[i]){
                                // set node value to this argv ind to use for $i var
                                int * varnum = (int *) calloc(1, sizeof (int));
                                *varnum = i + 1;
                                node->data.ptr  = varnum;
                                node->data.type = INT;
                            }else{
                                // TODO maybe see if theres a way we can just grab it from the arg like ${i#--arg} is sh
                                // - this has the issue that the user could put a ' in the string and it would screw up the variable definition
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
                    MapData * node = getMapNode(flagMap, word, wordlen);
                    if(node == NULL){
                        if(unknownPositional){
                            allPositional = true;
                            --i;
                            continue;
                        }
                        //puts("didnt find");
                        fprintf(stderr, "did not find '%s' as either a flag or param\n", word);
                        return DidNotFind;       // didnt find it
                    }
                    node->data.ptr  = &flagTrue;
                    // TODO this shouldnt be necessary
                    node->data.type = BOOL;
                    for(mnllist_t * negNode = node->data.negations; negNode != NULL; negNode = negNode->next){
                        negNode->data->data.ptr  = &flagFalse;
                        negNode->data->data.type = BOOL;
                    }
                }
            }else{
                // if this arg is just '--', then all the rest of the args are just default vals, dont parse
                //for(++i; i < argc; ++i, ++thisDef){
                //    *thisDef = argv[i];
                //}
                //break;
                allPositional = true;
                continue;
            }
        }else{
            // this is a default
            //fprintf(stderr, "default:\t %s\n", argv[i]);
            *thisDef = argv[i];
            if(thisPosNode != NULL){
                if(*thisPosNode != NULL){
                    if(print){
                        // set node value to this argv ind to use for $i var
                        int * varnum = (int *) calloc(1, sizeof (int));
                        *varnum = i + 1;
                        (*thisPosNode)->data.ptr  = varnum;
                        (*thisPosNode)->data.type = INT;
                    }else{
                        if((*thisPosNode)->data.type == INT){
                            int * num = (int *) calloc(1, sizeof (int));
                            *num = strtol(*thisDef, NULL, 0);
                            (*thisPosNode)->data.ptr = num;
                        }else{
                            (*thisPosNode)->data.ptr  = *thisDef;
                            (*thisPosNode)->data.type = STR;
                        }
                    }
                    ++thisPosNode;
                //}else if(!allPositional && unknownPositional && positionalNodes[0] == NULL){
                }else if(!allPositional && unknownPositional){
                    // if the positional nodes array is present (not NULL) but empty (first element is NULL)
                    // then this is an unknown argument, so treat rest of args as positional
                    allPositional = true;
                }
            }
            //puts(*thisDef);
            ++thisDef;
        }
    }

    return Success;
}


// define function to parse args for C lib (no printing)
Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, MapData * positionalNodes[], const char * * positionalParams_ptr[], bool unknownPositional){
    return parseArgsBase(argc, argv, flagMap, paramMap, positionalNodes, positionalParams_ptr, unknownPositional, false);
}


int printFlag(const MapData * node, FILE * file){
    int len = fprintf(file, " [ ");
    char sep = '|';
    for(int i = 0; i < node->namesLen; ++i){
        if(i == node->namesLen - 1){
            sep = ' ';
        }
        if(node->nameLens[i] == 1){
            len += fprintf(file, "-%c%c", node->names[i][0], sep);
        }else{
            len += fprintf(file, "--%.*s%c", node->nameLens[i], node->names[i], sep);
        }
    }
    len += fprintf(file, "]");
    return len;
}
int printFlagWrapper(map_t * map, MapData * node, void * input){
    FILE * outFile = (FILE *) input;
    (void) map;
    return printFlag(node, outFile);
}

int printParam(const MapData * node, FILE * file){
    int i;
    int lastWordInd = 0;
    int len  = fprintf(file, " ");
    const bool optional = !node->data.required || node->data.defaultData != NULL;
    if(optional){
        len += fprintf(file, "[ ");
    }
    char sep = '|';
    for(i = 0; i < node->namesLen; ++i){
        if(i == node->namesLen - 1){
            sep = ' ';
        }
        // TODO print | between the args instead of spaces
        if(node->nameLens[i] == 1){
            len += fprintf(file, "-%c%c", node->names[i][0], sep);
        }else{
            len += fprintf(file, "--%.*s%c", node->nameLens[i], node->names[i], sep);
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
int printParamWrapper(map_t * map, MapData * node, void * input){
    FILE * outFile = (FILE *) input;
    (void) map;
    return printParam(node, outFile);
}

int printPositionalParam(const MapData * node, FILE * file){
    int i;
    int firstWordInd = 0;
    int len  = fprintf(file, " ");
    const bool optional = !node->data.required || node->data.defaultData != NULL;
    if(optional){
        len += fprintf(file, "[ ");
    }
    for(i = 0; i < node->namesLen; ++i){
        if(node->nameLens[i] > 1){
            firstWordInd = i;
            break;
        }
    }
    len += fprintf(file, "%.*s", node->nameLens[firstWordInd], node->names[firstWordInd]);
    if(node->data.defaultData != NULL && node->data.defaultData->ptr != NULL){
        len += fprintf(file, " = ");
        len += printArgData(node->data.defaultData, file);
    }
    if(optional){
        len += fprintf(file, " ]");
    }
    return len;
}

int printUsage(const map_t * flagMap, const map_t * paramMap, const MapData * positionalParams[], const char * progname){
    FILE * outFile = stderr;
    //FILE * outFile = stdout;
    int len = 0;
    if(progname == NULL){
        error(3, 0, "Must specify progname for printing usage");
    }
    len += fprintf(outFile, "Usage:\t%s", progname);
    // just casting these const maps the (map_t *) because i know the print functions dont modify the map (and i dont feel like copying the map function just to have a const and non-const version of it
    len += iterMapSingle((map_t *)flagMap , printFlagWrapper , outFile);
    len += iterMapSingle((map_t *)paramMap, printParamWrapper, outFile);
    // TODO do positional args normally go before or after flags/params (of course its totally arbitrary with my parser)
    if(positionalParams != NULL){
        for(const MapData ** posParam = positionalParams; *posParam != NULL; ++posParam){
            len += printPositionalParam(*posParam, outFile);
        }
    }
    len += fprintf(outFile, "\n");
    return len;
}

// TODO make this function format the strings (just line wrapping nicely)
//  - include the terminal width in this formatting too
int printHelp(const map_t * flagMap, const map_t * paramMap, const MapData * positionalParams[], const char * helpMessages){
    FILE * outFile = stderr;
    //FILE * outFile = stdout;
    const char * key;
    const char * msg;
    const char * lastSpace;
    int keyLen;
    int msgLen;
    int len = 0;
    bool wasSpace = false;
    for(const char * c = helpMessages; *c != '\000'; ++c){
        // skip past leading whitespace
        for( ; isspace(*c); ++c);
        if(*c == '\000'){
            break;
        }
        // get key (next word)
        key = c;
        // TODO check for null
        for( ; !isspace(*c); ++c){
            if(*c == '\000'){
                return 0;
            }
        }
        keyLen = c - key;
        // skip past trailing whitespace
        for( ; isspace(*c); ++c);
        if(*c != '='){
            // TODO error
            return 0;
        }
        ++c;
        // skip past leading whitespace
        for( ; isspace(*c); ++c);
        if(*c == '\000'){
            return 0;
        }
        // get msg (from now)
        msg = c;
        for( ; *c != '\n' && *c != '\000'; ++c){
            if(!isspace(*c)){
                wasSpace = false;
            }else if(!wasSpace){
                wasSpace  = true;
                lastSpace = c;
            }
        }
        if(!wasSpace){
            lastSpace = c;
        }
        msgLen = lastSpace - msg;
        // get node from key
        const MapData * node = getMapNode(paramMap, key, keyLen);
        int printedLen = 0;
        // print node arg
        if(node != NULL){
            printedLen = printParam(node, outFile);
        }else if((node = getMapNode(flagMap, key, keyLen)) != NULL){
            printedLen = printFlag(node, outFile);
        }else{
            if(positionalParams == NULL){
                const MapData ** node_ptr;
                for(node_ptr = positionalParams; *node_ptr != NULL; ++node_ptr){
                    if(keyInNode(*node_ptr, key, keyLen)){
                        node = *node_ptr;
                        break;
                    }
                }
            }
            if(node == NULL){
                error(7, 0, "key '%.*s' does not exist in map", keyLen, key);
            }
        }
        // print help message
        len += printedLen;
        len += fprintf(outFile, "%-*s %.*s\n", ARG_SPACE - printedLen - 1, "", msgLen, msg);
        if(*c == '\000'){
            break;
        }
    }

    return len;
}
