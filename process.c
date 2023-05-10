#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "process.h"


// so the idea of this is that it will go through buff and add pointers to a linked list to dostuffs
// so im gonna need to make linked list functions to do all the linked list things like swapping pointers and stuff
// also i might want to make it sort the linked list as it isbeing created and stuff but im not sure
// cause sorting the list means i have to keep looping through it and that probably wont save me much inwhat i need to do later
// i may also want to specify if itis a single letter
// the other thing i could do is make a separate list for the single letters and words
// i think i want separate lists for the parameters and words and have this do them separately
// also this should probably replace spaces and commas with \0 so that it looks like a full string then it can be easily compared
// TODO actually rework the linked list stuff to use the maps
// then parse the args by setting stuff in the maps and getting stuff from the maps
// TODO also make a handy function to easily add values from args to the list? (for C interface that doesnt need to parse a string)
char * linkParams(char * buf, map_t * map, char argType[], void * defaultValue, DataType defaultType){
    // create all the links and variables
    // also figure out dynamically creating all these linked list node things
    char * typeptr = strstr(buf, argType);
    if(typeptr == NULL) return buf;     // didnt find this argType (not necessarily a fault but if both are missing then it doesnt work), return buf to indicate just didnt find this
    typeptr += strlen(argType);
    char * end = strchr(typeptr, ';');
    if(end == NULL) return NULL;        // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
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
    llist_t * head = NULL;
    for( ; nextState != Semicolon; c = ++ce){
        void * defaultVal = defaultValue;
        DataType type = defaultType;
        llist_t * node = head;
        // loop through a param/flag list
        int sum = 0;
        for( ; ; c = ++ce){
            state = nextState;  // set state for next iteration
            // TODO check for whitespace etc or decide its not allowed and then error gracefully
            for( ; isalnum(*ce) || (*ce == '-') || (*ce == '_'); ++ce);
            // set next state based on the upcoming symbol
            nextState = setState(ce);
            // if the next state is 0 then its an error and exit unless were on the last line
            if(nextState == Error && (state != Semicolon))  return NULL;
            // add a new node onto the linked list
            // TODO check for issues in the format (like multiple =)
            //printf("param: '%.*s', state = %d, nextState = %d\n", (int)(ce - c), c, state, nextState);
            if(state == Space || state == Comma){
                ++sum;
                llist_t * n = node;
                // if we are at the end of the list, then just add a new at the beginning
                // we keep the list through all the params/flags and just override the data
                // NOTE this only works if the addMapMembers_fromList function uses the length of the list directly
                if(node == NULL){
                    //printf("allocating new node\n");
                    node = (llist_t *) calloc(1, sizeof (llist_t));
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
                // TODO create a stringview for this
                StringView * sv = (StringView *) calloc(1, sizeof (StringView));
                defaultVal = sv;
                sv->str = c;
                sv->len = (int) (ce - c);
                type = STRING_VIEW;
            }
            if(nextState == Space || nextState == Semicolon){
                break;
            }
        }
        addMapMembers_fromList(map, defaultVal, type, head, sum);
    }
    // cleanup the list
    for(llist_t * node = head ; node != NULL; ){
        llist_t * n = node->next;
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
void printKeyValues(const MapNode * node){
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
        switch(node->type){
            case STR:
                printf("%.*s='%s'\n", node->nameLens[i], str, (char *) node->data);
                break;
            case STRING_VIEW:
                printf("%.*s='%.*s'\n", node->nameLens[i], str, ((StringView *) node->data)->len, ((StringView *) node->data)->str);
                break;
            case BOOL:
                printf("%.*s='%s'\n", node->nameLens[i], str, (*(bool *) node->data) ? "true\0" : "false");
                break;
            case INT:
                printf("%.*s=\"$%d\"\n", node->nameLens[i], str, *(int *) node->data);
                break;
            default:
                // TODO this isnt great
                printf("%.*s=%p\n", node->nameLens[i], str, node->data);
                break;
        }
        if(tmpstr != NULL){
            // if created tmp str to replace - then free it
            free(tmpstr);
        }
    }
}


void printKeyValuesWrapper(map_t * map, MapNode * node){
    if(node->data != NULL){
        return printKeyValues(node);
    }
}

Errors parseArgsBase(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, void * flagValue, DataType flagType, const char * defaultValues[], bool print){

    defaultValues = (const char **) calloc((argc - 1), sizeof (const char *));
    memset(defaultValues, (long) NULL, argc);
    const char ** thisDef = defaultValues;

    bool checkFlags;

    for(int i = 1; i < argc; i++){
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
                        return NotAlnum;   // error if f not alphanumeric
                    // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                    // if this is last char and the next param doesnt have a - then this must be a parameter
                    }else if(((f[1]) == '\000') && ((i+1 < argc) && (argv[i+1][0] != '-'))){   // then this is a parameter
                        MapNode * node = getMapNode(paramMap, f, 1);
                        // TODO make the node data const..
                        if(node != NULL){
                            ++i;
                            if(print){
                                // set node value to this argv ind to use for $i var
                                node->data = &i;
                                node->type = INT;
                                printKeyValues(node);
                                // reset data to NULL so i dont print it out as a default val
                                node->data = NULL;
                            }else{
                                node->data = (void *) argv[i];
                                node->type = STR;
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
                            return DidNotFind;       // didnt find it
                        }
                        node->data = flagValue;
                        node->type = flagType;
                        if(print){
                            printKeyValues(node);
                        }
                    }
                }
            }else{
                // this is a word thign
                const char * word = argv[i] + 2;
                //printf("word:\t");
                //puts(word);
                bool notAlnum = false;
                for(const char * c = word; *c && !notAlnum; c++){
                    notAlnum = (!isalnum(*c) && (*c != '-') && (*c != '_'));
                }
                if(notAlnum){
                    //puts("well its gotta be alnum");
                    return NotAlnum;   // error if f not alphanumeric
                // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                }else if((i+1 < argc) && (argv[i+1][0] != '-')){   // then this is probably a parameter
                    MapNode * node = getMapNode(paramMap, word, strlen(word));
                    if(node != NULL){
                        ++i;
                        if(print){
                            // set node value to this argv ind to use for $i var
                            node->data = &i;
                            node->type = INT;
                            printKeyValues(node);
                            // reset data to NULL so i dont print it out as a default val
                            node->data = NULL;
                        }else{
                            node->data = (void *) argv[i];
                            node->type = STR;
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
                    MapNode * node = getMapNode(flagMap, word, 1);
                    if(node == NULL){
                        //puts("didnt find");
                        return DidNotFind;       // didnt find it
                    }
                    node->data = flagValue;
                    node->type = flagType;
                    if(print){
                        printKeyValues(node);
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

    // now print defaults values out
    // TODO only print out values that werent already found in the args
    if(print){
        iterMapSingle(paramMap, printKeyValuesWrapper);
    }

    // now print out the values without parameters (defaults)
    if(*defaultValues && print){
        // TODO I feel like defaults just isnt a good enough name here so figure something else out
        // make defaults replace the args using the set - notation below
        // - TODO should i keep defaults instead so you dont lose the original args?
        printf("set -");
        //printf("defaults='%s", *defaultValues);
        int i = 1;
        for(thisDef = defaultValues; *thisDef != NULL; ++thisDef){
            //printf(" '%s'", *thisDef);
            // find index of this string for printing out var to use
            for( ; i < argc && *thisDef != argv[i]; ++i);
            printf(" \"$%d\"", i);
            //printf(" %s", *thisDef);
        }
        printf("\n");
        //printf("'\n");
    }

    return Success;
}


// define function to parse args for C lib (no printing)
Errors parseArgs(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap, const char * defaultValues[]){
    // TODO is this guarenteed to always exist?
    static bool flagTrue = true;
    return parseArgsBase(argc, argv, flagMap, paramMap, &flagTrue, BOOL, defaultValues, false);
}
// define function to parse args for sh (print)
Errors parseArgsPrint(const int argc, const char * const * argv, map_t * flagMap, map_t * paramMap){
    const char ** defaultValues = NULL;
    Errors err = parseArgsBase(argc, argv, flagMap, paramMap, "1", STR, defaultValues, true);
    free(defaultValues);
    return err;
}
