#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "process.h"


const pllist defVal = {};
const pllist * const defaultValNULL = &defVal;

// so the idea of this is that it will go through buff and add pointers to a linked list to dostuffs
// so im gonna need to make linked list functions to do all the linked list things like swapping pointers and stuff
// also i might want to make it sort the linked list as it isbeing created and stuff but im not sure
// cause sorting the list means i have to keep looping through it and that probably wont save me much inwhat i need to do later
// i may also want to specify if itis a single letter
// the other thing i could do is make a separate list for the single letters and words
// i think i want separate lists for the parameters and words and have this do them separately
// also this should probably replace spaces and commas with \0 so that it looks like a full string then it can be easily compared
char * linkParams(char * buf, pllist ** headptr, char argType[]){
    // create all the links and variables
    // also figure out dynamically creating all these linked list node things
    char * typeptr = strstr(buf, argType);
    if(typeptr == NULL) return strchr(buf, '\0');   // didnt find this argType (not necessarily a fault but if both are missing then it doesnt work)
    typeptr += strlen(argType);
    char * end = strchr(typeptr, ';');
    if(end == NULL) return end;   // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
    char * c = typeptr;
    for(; *c == ' '; c++);
    char * ce = c + 1;
    pllist ** lpptr = headptr;
    // i guess for this i want different values like
    // 1 was a space so were on   a  new set of values
    // 2 was a comma so were on the same set of values
    // 3 was an equals so were looking at a default val
    // 4 was a semicolon so were done
    // 0 means error
    State state = Space, nextState = Space;
    // loop throguh everything and make all the linked list stuffs
    //printf("state = %d, nextState = %d\n", state, nextState);
    // k im reading this and holy cow "ce = (c = ce + 1) + 1" is awful
    //for(; state != Semicolon; ce = (c = ce + 1) + 1){
    for(; state != Semicolon; c = ++ce, ++ce){
        state = nextState;  // set state for next iteration
        // TODO check for whitespace etc or decide its not allowed and then error gracefully
        for(; isalnum(*ce) || (*ce == '-') || (*ce == '_'); ++ce);
        // set next state based on the upcoming symbol
        nextState = setState(ce);
        // if the next state is 0 then its an error and exit unless were on the last line
        if(!nextState && (state != Semicolon))  return NULL;
        // add a new node onto the linked list
        lpptr = addParam(lpptr, c, state);
        // TODO check for error from addParam output and then return with death if so

    }

    return end + 1;
}

// TODO if after an equals dont change the state till you see a space
//  and then within that if there are quotes then wait till after the closing quote to quit from the space
//  if you wanted a ; you would also need to quote it
//  but this would allow you to put symbols and stuff in the default values so you dont have to set them later in bash cause who wants to do that
State setState(char * c){
    State state;
    switch(*c){
        case ' ':
            state = Space;
            *c = '\0';
            break;
        case ',':
            state = Comma;
            *c = '\0';
            break;
        case '=':
            state = Equals;
            *c = '\0';
            break;
        case ';':
            state = Semicolon;
            *c = '\0';
            break;
        default:
            // TODO figure out if i want to have it replace the character if its not one of these cause if so then put the *c = '\0' after the switch-case cause its all the same
            // it probably doesnt matter really but the : on the second section would be replaced and then that would be a tad weird
            state = Error;
            break;
    }
    //c++;
    //for(; *c == ' '; c++);
    return state;
}

// add a node to the linked list with all the pointers set and stuff
// base the pointers off the state, quit when state is 4
// pretty happy with this, only conditionals are from state dictating extra "Same" pointers and the malloc error checking
pllist ** addParam(pllist ** lp, char * c, State state){
    // if done (hit semicolon) then set next NULL and exit
    if(state == Semicolon){
        (*lp)->next = NULL;
        return &(*lp)->next;
    }
    // allocate memory for next node and point at it with pp
    pllist * pp = (pllist *) malloc(sizeof(pllist));
    if(pp == NULL){
        fprintf(stderr, "couldnt allocate, this is bad\n");
        return (pllist **) NULL;  // might have a problem here cause pp is NULL which makes it a single pointer
    }
    pp->str = c;
    // if state Space then its the first one so set the headSame to pp, if Comma or Equals then its just part so set the headSame to the last headSame
    switch(state){
        case Space:
            pp->headSame = pp;
            if((*lp) && ((*lp)->nextSame != defaultValNULL))   (*lp)->nextSame = NULL;
            break;
        case Comma:
        case Equals:
            pp->headSame = (*lp)->headSame;
            (*lp)->nextSame = pp;
            // TODO decide if i keep the warnings or if i take off the const part of defaltVallNULL because the contents are never used so right now im just casting it so the warning goes away
            //if(state == Equals)  pp->nextSame = defaultValNULL;
            if(state == Equals)  pp->nextSame = (pllist *) defaultValNULL;
            break;
        default:
            break;
    }

    //pp->prev = *lp;   // this works if the head starts out NULL cause it makes the head's prev NULL or makes a normal node the last pointer
    *lp = pp;       // set last dbl-ptr to current ptr
    //(*lp)->next = *lp;  // set last (current) next to last (current)
    pp->next = pp;  // set current next to current for use next time since returning next

    //return &(*lp)->next;  // return last (current) next dbl-ptr for use next time
    return &pp->next;
}

void clearMems(pllist * head){
    pllist * next;
    for(pllist * tmp = head; tmp; tmp = next){
        next = tmp->next;
        free(tmp);
    }
}



// get the head for the flags or parameters and the string value to print
void printStuffs(char * str, pllist * member){
    if(str == NULL){
        // TODO decide if this is ok since it changes the original data
        for(pllist * tmp = member->headSame; tmp; tmp = tmp->nextSame){
            if(tmp->nextSame == defaultValNULL){
                str = tmp->str;
                break;
            }else if(tmp->nextSame == NULL){
                str = "1";
                //return;     // TODO this is an error, should return something prolly
            }
        }
    }
    for(pllist * tmp = member->headSame; (tmp != NULL); tmp = tmp->nextSame){
        // TODO decide if this is ok since it changes the original data
        // this could be bad if you gave 2 equivalent things because it might not match the second time if you did a something -r --remove-now
        // so probably allocate some memory temporarily or soemthing to change things in and then print that or just printf char by char
        // the other option is changing to added everything to a massive buffer to print at the end and just replace them all in there at the end
        // but what if i change the pointers on both at once
        // change - to _
        for(char * c = tmp->str; *c; c++)    if(*c == '-')       *c = '_';
        printf("%s='%s'\n", tmp->str, str);
        //printf("eval %s=%s\n", tmp->str, str);
        if(!tmp->nextSame){
            return;
        }else if(tmp->nextSame->nextSame == defaultValNULL){
            tmp->nextSame->nextSame = NULL;
            break;
        }
    }
}

// match a string to an entry in the list with head head
// and maybe add values to end of linked list as default vlaues cause thats something
pllist * mtchStr(char * str, pllist * head){
    pllist * tmp = head;
    for(; (tmp != NULL) && strcmp(str, tmp->str); tmp = tmp->next);
    return tmp;
}

// match a character to an entry in the list with head head
// and maybe add values to end of linked list as default vlaues cause thats something
pllist * mtchChr(char   c,   pllist * head){
    pllist * tmp = head;
    for(; (tmp != NULL) && (tmp->str[1] || (c != *tmp->str)); tmp = tmp->next);
    return tmp;
}



Errors parseArgs(int argc, char ** argv, pllist * flagHead, pllist * paramHead){

    char ** defs = (char **) malloc(sizeof(char *) * (argc - 1));
    memset(defs, (long) NULL, argc);
    char ** thisDef = defs;

    int checkFlags = 1;

    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-'){
            if(argv[i][1] != '-'){
                // this is a single letter flag so loop through all the next letters
                //printf("singlet:\t");
                //puts(argv[i]);
                for(char * f = argv[i] + 1; *f; f++){
                    //printf("let:\t%c\n", *f);
                    if(!isalnum(*f)){
                        return NotAlnum;   // errror if f not alphanumeric
                    // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                    }else if(!(*(f+1)) && ((i+1 < argc) && (argv[i+1][0] != '-'))){   // then this is a parameter
                        pllist * tmp = mtchChr(*f, paramHead);
                        //if(tmp == NULL){
                        if(tmp){
                            printStuffs(argv[++i], tmp);
                            //printf("wtharg:\t");
                            //puts(argv[i]);
                            checkFlags = 0;
                        }else{
                            //puts("didnt find");
                            //return DidNotFind;       // didnt find it
                        }
                    }
                    if(checkFlags){                                          // i think the only other option is its a flag
                        pllist * tmp = mtchChr(*f, flagHead);
                        //if(tmp == NULL){
                        if(!tmp){
                            //puts("didnt find");
                            return DidNotFind;       // didnt find it
                        }
                        printStuffs("1", tmp);
                    }
                }
            }else{
                // this is a word thign
                char * word = argv[i] + 2;
                //printf("word:\t");
                //puts(word);
                int notAlnum = 0;
                for(char * c = word; *c; c++){
                    notAlnum += (!isalnum(*c) && (*c != '-') && (*c != '_'));
                }
                if(notAlnum){
                    //puts("well its gotta be alnum");
                    return NotAlnum;   // error if f not alphanumeric
                // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                }else if((i+1 < argc) && (argv[i+1][0] != '-')){   // then this is probably a parameter
                    pllist * tmp = mtchStr(word, paramHead);
                    if(tmp){
                        printStuffs(argv[++i], tmp);
                        //printf("wtharg:\t");
                        //puts(argv[i]);
                        checkFlags = 0;
                    }else{
                        //puts("didnt find");
                        //return DidNotFind;       // didnt find it
                    }
                }
                if(checkFlags){                                          // i think the only other option is its a flag
                    pllist * tmp = mtchStr(word, flagHead);
                    if(!tmp){
                        //puts("didnt find");
                        return DidNotFind;       // didnt find it
                    }
                    printStuffs("1", tmp);
                }
            }
        }else{
            // this is a default
            //printf("default:\t");
            *thisDef = argv[i];
            //puts(*thisDef);
            thisDef++;
        }
        checkFlags = 1;
    }

    // now print defaults values out
    for(pllist * fp = paramHead; fp; fp = fp->next){
        if(fp->nextSame == defaultValNULL){
            printStuffs(fp->str, fp);
        }
    }

    // now print out the values without parameters (defaults)
    if(*defs){
        // TODO I feel like defaults just isnt a good enough name here so figure something else out
        // TODO this method with eval doesnt seem to work since there are spaces and i havent figured out how to do it
        //printf("eval ( \"defaults='%s", *defs);
        printf("defaults='%s", *defs);
        for(char ** defStr = defs+1; *defStr; defStr++){
            printf(" %s", *defStr);
        }
        //printf("'\" )\n");
        printf("'\n");
    }
    free(defs);

    return NoError;
}


