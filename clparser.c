#include <stdio.h>
#include <string.h>
//#include <ctype.h>
//#include <stdlib.h>
#include "process.h"

#define BUF_SIZE    1000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter
#define OUTPUT_SIZE 2000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter

// TODO clean up the new section and put it in a function thing
// TODO itd be cool if there were negation switches that if you gave them they just negate the other automatically (they cant both have the same value)
//  this would make it so that i dont have to worry about logic of opposing flags in sh, its done automatically
//  additionally this means that the last switch is always the most important, not whatever is chosen in the program
//      this means you can alias something with flags and if a flag you give later contradicts it uses that flag
// TODO it seems to die if no flags in spec
// TODO it seems to die if only give defualts
// TODO give some sort of option to have the defaults split by something so that i can preserve whitespace
// TODO probably print out helpful errors when it dies
// TODO prolly add in lists by separating by commas or something (not exactly sure if this is helpful or anything but)




// TODO go through this list of things and correct the things that are wrong and add features and things that i want
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
// ive decided this needs a help system so probably that would be like giving one of the names and then the help for it in a big string and then passing that string into the parser with a like --helpmsgs
// this thing needs a help and other arguments in general probably
int main(int argc, char ** argv){
    // this is just for debugging
    // for(int i = 0; i < argc; i++){
    //     //printf("%s\n", argv[i]);
    //     puts(argv[i]);
    // }
    char cbuf[BUF_SIZE];
    fgets(cbuf, BUF_SIZE, stdin);
    //printf("%s\n", cbuf);
    // puts(cbuf);
    // printf("defaultValNULL = %x\n", defaultValNULL);

    // printf("printint things now\n");
    char * ce = cbuf + strlen(cbuf);
    pllist * flagHead = NULL, * paramHead = NULL;
    char * ferr = linkParams(cbuf, &flagHead, "flags:");
    // puts("now were gonna go through the string i guess");
    // for(char * c = cbuf; c < ce; c++){
    //     printf("%c", (*c == '\0')? '0' : *c);
    // }

    // puts("going through the linked list now");
    // char Ns[] = "NULL    ";
    // char ttp[10];
    // char * ns;
    // for(pllist * lp = flagHead; lp != NULL; lp = lp->next){
    //     if(!lp->nextSame)   ns = Ns;
    //     else{
    //         sprintf(ttp, "%x", lp->nextSame);
    //         ns = ttp;
    //     }
    //     printf("str = %s:\tme=%x, headSame=%x, nextSame=%s, next=%x\n", lp->str, lp, lp->headSame, ns, lp->next);
    // }
    
    // printf("ferr = %x, cbuf = %x, ce = %x\n", ferr, cbuf, ce);
    // TODO dont error yet, not finding is only bad if we find neither
    int noflag = 0;
    if(ferr < cbuf || ferr > ce)    noflag = 1;
    char * pcbuf;
    for(pcbuf = ce-1; (*pcbuf != ';') && (*pcbuf != '\0') && (pcbuf > cbuf); pcbuf--);
    if(*pcbuf == ';'){
        pcbuf = ferr;
    }else if(*pcbuf == '\0'){
        pcbuf = cbuf;
    }else{return 1;}
    // puts(pcbuf);

    // puts("params now");
    char * perr = linkParams(pcbuf, &paramHead, "parameters:");
    // if didnt find params and no flag then return 1 this is bad
    if((perr < cbuf || perr > ce) && noflag)    return 3;
    // puts("just finished with params");
    // for(char * c = cbuf; c < ce; c++){
    //     printf("%c", (*c == '\0')? '0' : *c);
    // }

    // puts("going through the linked list now again");
    // printf("defaultValNULL:\t%x\n", defaultValNULL);
    // for(pllist * lp = paramHead; lp != NULL; lp = lp->next){
    //     if(!lp->nextSame)   ns = Ns;
    //     else{
    //         sprintf(ttp, "%x", lp->nextSame);
    //         ns = ttp;
    //     }
    //     printf("str = %s:\tme=%x, headSame=%x, nextSame=%s, next=%x\n", lp->str, lp, lp->headSame, ns, lp->next);
    // }


    int retVal = parseArgs(argc, argv, flagHead, paramHead);

    clearMems(flagHead);
    clearMems(paramHead);

    return retVal;
}
            



