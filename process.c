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
char * linkParams(char * buf, pllist ** headptr, char argType[]){
    // create all the links and variables
    // also figure out dynamically creating all these linked list node things
    //int typeLen = strlen(type);
    //char * typeptr = strstr(buf, type) + typeLen;
    char * typeptr = strstr(buf, argType);
    if(typeptr == NULL) return strchr(buf, '\0');   // didnt find this argType (not necessarily a fault but if both are missing then it doesnt work)
    typeptr += strlen(argType);
    //puts(typeptr);
    char * end = strchr(typeptr, ';');
    if(end == NULL) return end;   // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
    //puts(end);
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
    int state = 1, nextState = 1;
    // loop throguh everything and make all the linked list stuffs
    //printf("state = %d, nextState = %d\n", state, nextState);
    for(; state != 4; ce = (c = ce + 1) + 1){
        state = nextState;  // set state for next iteration
        for(; isalnum(*ce) || (*ce == '-') || (*ce == '_'); ce++);
        //printf("%s, %d\n", c, (int)(ce - c));
        // set next state based on the upcoming symbol
        nextState = setState(ce);
        //printf("nextState = %d\n", nextState);
        // if the next state is 0 then its an error and exit
        if(!nextState && (state != 4))  return NULL;
        // add a new node onto the linked list
        lpptr = addParam(lpptr, c, state);
        
    }

    puts("got through the linking");
    return end + 1;
}

int setState(char * c){
    int state;
    switch(*c){
        case ' ':
            state = 1;
            *c = '\0';
            break;
        case ',':
            state = 2;
            *c = '\0';
            break;
        case '=':
            state = 3;
            *c = '\0';
            break;
        case ';':
            state = 4;
            *c = '\0';
            break;
        default:
            // TODO figure out if i want to have it replace the character if its not one of these cause if so then put the *c = '\0' after the switch-case cause its all the same
            state = 0;
            break;
    }
    c++;
    for(; *c == ' '; c++);
    return state;
}

pllist ** addParam(pllist ** lp, char * c, int state){
    // if done (hit semicolon) then set next NULL and exit
    if(state == 4){
        (*lp)->next = NULL;
        return &(*lp)->next;
    }
    // allocate memory for next node and point at it with pp
    pllist * pp = (pllist *) malloc(sizeof(pllist));
    // i honestly dont want this but i think ill keep it cause its proabably safer
    if(pp == NULL){
        //fprintf(stderr, "couldnt allocate, this is bad\n");
        return pp;  // might have a problem here cause pp is NULL which makes it a single pointer
    }
    //puts("just allocated some memory");
    pp->str = c;
    // if state 1 then its the first one so set the headSame to pp, if 2 or 3 then its just part so set the headSame to the last headSame
    //printf("*lp = %x, pp = %x\n", *lp, pp);
    switch(state){
        case 1:
            pp->headSame = pp;
            //if(*lp)     printf("*lp->nextSame = %x\n", (*lp)->nextSame);
            //printf("pp->nextSame = %x\n", (pp)->nextSame);
            if((*lp) && ((*lp)->nextSame != defaultValNULL))   (*lp)->nextSame = NULL;
            break;
        case 2:
            //pp->nextSame = NULL;
        case 3:
            pp->headSame = (*lp)->headSame;
            (*lp)->nextSame = pp;
            if(state == 3)  pp->nextSame = defaultValNULL;
            break;
    }

    // if lp is NULL then its the head i guess so set the head now
    // since i changed this obviously get rid of the if cause theyre the same now
    /* if(*lp == NULL){ */
    /*     puts("i think this is the head"); */
    /*     *lp = pp; */
    /* }else{  // otherwise set the next thing */
    /*     //printf("*lp = %x\n", *lp); */
    /*     //(*lp)->next = pp; */
    /*     *lp = pp; */
    /* } */
    //pp->prev = *lp;   // this works if the head starts out NULL cause it makes the head's prev NULL or makes a normal node the last pointer
    *lp = pp;
    (*lp)->next = *lp;

    return &(*lp)->next;
}




//
//// return a char pointer, with NULL being a failure, buf+strlen(buf) being it didnt find and otherwise return typeptr
//// link will have a pointer for every char in slBuf that points to the corresponding word in wBuf, if there isnt a link then its NULL
//// slBuf will just be a string of chars with a comma separating single letters that are equivelent
//// wBuf is a string that has words separated by spaces if different and by commas if equivalent
//// buf is the initial buffer inputed with the stuffs
//// type should be either flags: or parameters:
//char * sepBuff(char * buf, char * slBuf, char * wBuf, char type[], char ** link){
//    memset(slBuf, 0, BUF_SIZE);     // assuming the buffers are BUF_SIZE long, it should be fine since this is only for use in this program
//    memset(wBuf,  0, BUF_SIZE);
//    int typeLen = strlen(type);
//    char * typeptr  = strstr(cbuf, type) + typeLen;
//    if(typeptr != NULL){
//        char * end = strchr(typeptr, ';');
//        if(end == NULL) return end;   // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
//        /* char tempBuf[BUF_SIZE]; */
//        /* memset(tempBuf, 0, BUF_SIZE); */
//        /* strncpy(tempBuf, flagptr, end - flagptr); */
//        char * c = typeptr, ce = c+1;
//        char * sl = slBuf, w = wBuf;
//        // so i think i need to have some variables to store the position in link and in wBuf so that the linking can be done after its found the whole string
//        // ex: if its r,R,recurse you cant set the link for r till passing over R to recurse
//        // probably do in a do while sorta thing id guess
//        // separate out checking for ',' vs for ' ' or '=' because one means end and the other means more equivalent inputs
//        //
//        // thinking about it.  i need a pointer to where im at in the sl and w buffs.  the link array keeps up with the sl buff so it will keep its beginning position
//        // the wbuf will need a pointer for its beginning but if the value of link is set first then this wont be an issue
//        for(; ce < end; ce = c+1){
//            if(*c == ' ' || *c == ',' || *c == '=')     continue;
//            while(*ce == ','){
//                // c is single letter
//                *sl = *c;   sl++;
//                                
//            }
//            while(*ce != ' ', *ce != '='){
//                // c on a word
//            }
//
//        }
//    }else{
//        //return buf + strlen(buf);   // didnt find this type (not necessarily a fault but if both are missing then it doesnt work)
//        // TODO i like this one better but i dont know if strchr will actually allow it so i needa test cause if it returns NULL then thats bad
//        return strchr(buf, '\0');   // didnt find this type (not necessarily a fault but if both are missing then it doesnt work)
//    }
//    return typeptr;
//}
//
