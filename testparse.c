#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE    1000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter
#define OUTPUT_SIZE 2000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter

//char * sepBuff(char * buf, char * slBuf, char * wBuf, char type[], char ** link);
char * link(char * buf, pllist * head, char [] type);

typedef struct pllist_t{
    // int slen;
    // int singleLetter;
    char * str;
    struct pllist_t * headSame;
    struct pllist_t * nextSame;
    struct pllist_t * next;
    //struct pllist_t * prev;
} pllist;
const pllist defVal;
const pllist * const defaultValNULL = &defVal;


// so the idea is that youll send in configuration stuffs over stdin and then have it take in the clargs as clargs and then itll output things that i guess are helpful based on the config stuffs sent in over stdin
// theres no way that this will be overcomplicated or impossible to use or write at all
// ill define that this can have flags and options that are set to values and a default field that everything else falls into
// ill do it in the gnuish i think style where there can be words that use the --, single letters use - and can be combined 
// if the single letter - options arent flags then the next arguments will be used as the options for that parameter thingy i dont know howw to explain
// it should go in order of the options it gets so if they contradict then itll be the last option
// the word parameters can use a space or an = with no spaces
// the stdin should be of the form:
// "flags: f,flag g qwerty h,hist; parameters: q,asdf u nothing zzz,z;"
// TODO figure out if i want it to implement default values and then have them set to those values if not set manually (probably -- just have it be like q,asdf=12)
// where things that are in the same word separated by commas are the same flag or parameter (if its only 1 char its a single char - else its a -- word)
// things after flags are flags and the stuff after parameters are the options with values to be set
// this should return a string like:
//  f=1
//  flag=1
//  g=1
//  qwerty=1
//  h=1
//  hist=1
//  q=whatever
//  asdf=whatever
//  nothing=sure
//  defaults="who even cares"
// if called with arguments "-fgq whatever --qwerty --nothing=sure who even cares
// theoretically im thinking this way you run it like $(echo "..." | parse $@) and itll set the variables accordingly so you can use them without doing any parsing
// error codes:
// 1 not alphanumeric flags or parameter
// 2 flags or parameter not defined
// 3 bad definitions
// TODO im considering changing the structure to make it so command line args are possible for this program
// if it works like echo $@ | parser "deffs..." --arg-for-parser then that would work
// otherwise i need to figure out a way to signal that the arguemtn is just for this parser and not what its working with
//  that could work like echo "deffs..." | parser --args-for-parser -- $@
//  in this case the arguments after -- are what needs to be parsed which makes the -- necessary (could make it something like -p or --parse instead of just --)
//  (could have it assume to parse everything if there is no -- but that could be confusing if someone passes in a -- that wasnt exected and things are parsed wrong or it errors cause thats how that works)
// im not sure which is more elegant, i think the first one but the problem is that makes me have to rewrite stuffs and makes it so i work with 2 big strings rather that a big string and an array
// i think if it hits a -- with no word after it should take that to mean everything after is default since that seems to be how most things would use it (this is less flexible in some ways but i dont want to deal with it)
// also TODO make sure that in the words that if allows alphanumeric and - and also when checking if its in the words, make sure it isnt a substring of another word by checking the characters on either side (for the left side need to check if the first letter is in the first position of the whole string)
int main(int argc, char ** argv){
    // this is just for debugging
    for(int i = 1; i < argc; i++){
        printf("%s\n", argc[i]);
    }
    char cbuf[BUF_SIZE];
    fgets(cbuf, BUF_SIZE, stdin);
    printf("%s\n", cbuf);

    //char slFlags[BUF_SIZE];
    //char slParams[BUF_SIZE];
    //char wFlags[BUF_SIZE];
    //char wParams[BUF_SIZE];

    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-' && argv[i][1] != '-'){
            // this is a single letter flag so loop through all the next letters
            for(char * f = argv[i][1]; *f; f++){
                if(!isalnum(*f)){
                    return 1;   // errror if f not alphanumeric
                // TODO make it look specifically for the single letter flags of parameters not just in everything cause then it could find this letter in a word
                // TODO consider making a large string and then sprintf to it and print it at the end so it doesnt stop halfway through on an error
                }else if(strchr(flags,  *f) != NULL){
                    printf("%c=1\n", *f);
                }else if(strchr(params, *f) != NULL){
                    printf("%c=%s\n", *f, argv[++i]);
                }else{
                    return 2;   // error if f not in flags or params
                }
            }
        }
    }

    return 0;
}


// return a char pointer, with NULL being a failure, buf+strlen(buf) being it didnt find and otherwise return typeptr
// link will have a pointer for every char in slBuf that points to the corresponding word in wBuf, if there isnt a link then its NULL
// slBuf will just be a string of chars with a comma separating single letters that are equivelent
// wBuf is a string that has words separated by spaces if different and by commas if equivalent
// buf is the initial buffer inputed with the stuffs
// type should be either flags: or parameters:
char * sepBuff(char * buf, char * slBuf, char * wBuf, char type[], char ** link){
    memset(slBuf, 0, BUF_SIZE);     // assuming the buffers are BUF_SIZE long, it should be fine since this is only for use in this program
    memset(wBuf,  0, BUF_SIZE);
    int typeLen = strlen(type);
    char * typeptr  = strstr(cbuf, type) + typeLen;
    if(typeptr != NULL){
        char * end = strchr(typeptr, ';');
        if(end == NULL) return end;   // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
        /* char tempBuf[BUF_SIZE]; */
        /* memset(tempBuf, 0, BUF_SIZE); */
        /* strncpy(tempBuf, flagptr, end - flagptr); */
        char * c = typeptr, ce = c+1;
        char * sl = slBuf, w = wBuf;
        // so i think i need to have some variables to store the position in link and in wBuf so that the linking can be done after its found the whole string
        // ex: if its r,R,recurse you cant set the link for r till passing over R to recurse
        // probably do in a do while sorta thing id guess
        // separate out checking for ',' vs for ' ' or '=' because one means end and the other means more equivalent inputs
        //
        // thinking about it.  i need a pointer to where im at in the sl and w buffs.  the link array keeps up with the sl buff so it will keep its beginning position
        // the wbuf will need a pointer for its beginning but if the value of link is set first then this wont be an issue
        for(; ce < end; ce = c+1){
            if(*c == ' ' || *c == ',' || *c == '=')     continue;
            while(*ce == ','){
                // c is single letter
                *sl = *c;   sl++;
                                
            }
            while(*ce != ' ', *ce != '='){
                // c on a word
            }

        }
    }else{
        //return buf + strlen(buf);   // didnt find this type (not necessarily a fault but if both are missing then it doesnt work)
        // TODO i like this one better but i dont know if strchr will actually allow it so i needa test cause if it returns NULL then thats bad
        return strchr(buf, '\0');   // didnt find this type (not necessarily a fault but if both are missing then it doesnt work)
    }
    return typeptr;
}


// so the idea of this is that it will go through buff and add pointers to a linked list to dostuffs
// so im gonna need to make linked list functions to do all the linked list things like swapping pointers and stuff
// also i might want to make it sort the linked list as it isbeing created and stuff but im not sure
// cause sorting the list means i have to keep looping through it and that probably wont save me much inwhat i need to do later
// i may also want to specify if itis a single letter
// the other thing i could do is make a separate list for the single letters and words
// i think i want separate lists for the parameters and words and have this do them separately
// also this should probably replace spaces and commas with \0 so that it looks like a full string then it can be easily compared
char * link(char * buf, pllist ** headptr, char [] type){
    // create all the links and variables
    // also figure out dynamically creating all these linked list node things
    //int typeLen = strlen(type);
    //char * typeptr = strstr(cbuf, type) + typeLen;
    char * typeptr = strstr(cbuf, type);
    if(typeptr == NULL) return strchr(buf, '\0');   // didnt find this type (not necessarily a fault but if both are missing then it doesnt work)
    typeptr += strlen(type);
    char * end = strchr(typeptr, ';');
    if(end == NULL) return end;   // there has to be a semicolon to end the definition    (should check for a NULL to err immediately)
    char * c = typeptr;
    for(; *c == ' '; c++);
    char * ce = c + 1;
    pllist * lptr = *headptr;
    // i guess for this i want different values like
    // 1 was a space so were on   a  new set of values
    // 2 was a comma so were on the same set of values
    // 3 was an equals so were looking at a default val
    // 4 was a semicolon so were done
    // 0 means error
    int state = 1, nextState = 0;
    // loop throguh everything and make all the linked list stuffs
    for(; state != 4; c++){
        for(; isalnum(ce); ce++);
        // set next state based on the upcoming symbol
        nextState = setState(ce);
        // if the next state is 0 then its an error and exit
        if(!nextState)  return NULL;
        // add a new node onto the linked list
        lptr = addParam(&lptr, c, state);
        state = nextState;  // set state for next iteration
    }

    return typeptr
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

pllist * addParam(pllist ** lp, char * c, int state){
    // if done (hit semicolon) then set next NULL and exit
    if(state == 4){
        (*lp)->next = NULL;
        return (*lp)->next;
    }
    // allocate memory for next node and point at it with pp
    pllist * pp = (pllist *) malloc(sizeof pllist);
    //pp->prev = *lp;   // this works if the head starts out NULL cause it makes the head's prev NULL or makes a normal node the last pointer
    // if lp is NULL then its the head i guess so set the head now
    if(*lp == NULL){
        *lp = pp;
    }else{  // otherwise set the next thing
        (*lp)->next = pp;
    }
    pp->str = c;
    // if state 1 then its the first one so set the headSame to pp, if 2 or 3 then its just part so set the headSame to the last headSame
    switch(state){
        case 1:
            pp->headSame = pp;
            if((*lp)->nextSame != defaultValNULL)   (*lp)->nextSame = NULL;
            break;
        case 2:
        case 3:
            pp->headSame = (*lp)->headSame;
            (*lp)->nextSame = pp;
            break;
    }
    if(state == 3)  pp->nextSame = defaultValNULL;

    return pp;
}



            



