#include <stdio.h>
#include <string.h>
//#include <ctype.h>
//#include <stdlib.h>
#include "process.h"


// TODO clean up the new section and put it in a function thing
// TODO prolly add in lists by separating by commas or something
//  not exactly sure if this is helpful or anything but
//  also lists arent really a thing in posix shell so i dont know what id expand them to
//  using argparse in python lists are done with separate arguments so that might be a better way if i actually had a use for lists
// TODO maybe make option to not override $@




// TODO go through this list of things and correct the things that are wrong and add features and things that i want
// so the idea is that youll send in configuration stuffs over stdin and then have it take in the clargs as clargs and then itll output things that i guess are helpful based on the config stuffs sent in over stdin
// theres no way that this will be overcomplicated or impossible to use or write at all
// ill define that this can have flags and options that are set to values and a default field that everything else falls into
// ill do it in the gnuish i think style where there can be words that use the --, single letters use - and can be combined 
// if the single letter - options arent flags then the next arguments will be used as the options for that parameter thingy i dont know howw to explain
// it should go in order of the options it gets so if they contradict then itll be the last option
// the word parameters can use a space or an = with no spaces
// the stdin should be of the form:
// spec="flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;"
// set default values for args using `a,arg=default_val`
// set other flags to negate the current flag using `f,flag=-other-flag`
//  - this way if other-flag is set after flag, then flag will be reset, no logic needed outside
//  - additionally this means that the last switch is always the most important (useful for overriding aliases)
// where things that are in the same word separated by commas are the same flag or parameter (if its only 1 char its a single char - else its a -- word)
// things after flags are flags and the stuff after parameters are the options with values to be set
// this should return a string like:
//  flag='false'
//  f='false'
//  h='false'
//  help='false'
//  g='false'
//  qwerty='true'
//  asdf="${2}"
//  q="${2}"
//  Z='test'
//  zzz='test'
//  z='test'
//  u='defval'
//  nothing="${7}"
//  set -- "${5}" "${9}" "${10}"
// if called with arguments `-fgq whatever --qwerty --zzz=test words --nothing else -- --test hi`
// run it like eval "$(echo "$spec" | parse "$@")" and itll set the variables accordingly so you can use them without doing any parsing
// error codes:
// 1 not alphanumeric flags or parameter
// 2 flags or parameter not defined
// 3 bad definitions
// TODO im considering changing the structure to make it so command line args are possible for this program
// i need to figure out a way to signal that the arguemnt is just for this parser and not what its working with
//  that could work like echo "$spec" | parser --args-for-parser -- "$@"
//  in this case the arguments after -- are what needs to be parsed which makes the -- necessary (could make it something like -p or --parse instead of just --)
//  (could have it assume to parse everything if there is no -- but that could be confusing if someone passes in a -- that wasnt exected and things are parsed wrong or it errors cause thats how that works)
// im not sure which is more elegant, i think the first one but the problem is that makes me have to rewrite stuffs and makes it so i work with 2 big strings rather that a big string and an array
// if it hits a -- with no word after it should take that to mean everything after is default since that seems to be how most things would use it (this is less flexible in some ways but i dont want to deal with it)
// ive decided this needs a help system so probably that would be like giving one of the names and then the help for it in a big string and then passing that string into the parser with a like --helpmsgs
// this thing needs a help and other arguments in general probably
int main(int argc, const char * const argv[]){
    // this is just for debugging
    // for(int i = 0; i < argc; i++){
    //     //printf("%s\n", argv[i]);
    //     puts(argv[i]);
    // }

    char * cbuf = NULL;
    size_t n = 0;
    ssize_t len;
    len = getline(&cbuf, &n, stdin);
    // TODO check that there wasnt an error
    map_t flagMap, paramMap;
    initMap(&flagMap);
    initMap(&paramMap);

    // TODO do something like this (until proper help support is added) if they dont include a -h/--help
    /* for(char ** argp = argv; argp < argv + argc; argp++){ */
    /*     if(!(strcmp(*argp, "--help") && strcmp(*argp, "-h"))){ */
    /*         printf("%s", cbuf); */
    /*         return 0; */
    /*     } */
    /* } */

    // printf("printint things now\n");
    // let me think about what im doing here
    // ferr is a pointer to the char after the ; ending the flags or whatever error happened
    char * ce = cbuf + len;
    // find all flag params, make the default value "0" (false)
    char * ferr = parseArgSpec(cbuf, &flagMap, "flags:", (void *)&flagFalse, BOOL, false);
    //printMap(&flagMap);

    //addMapMembers(&flagMap, (void *)&flagFalse, BOOL, "sdsd", "help", 4, "h", 1);

    // TODO dont error yet, not finding is only bad if we find neither
    // if the ferr is before or after the buff then we know theres no flags
    bool noflag = false;
    if(ferr == NULL){
        return 3;
    } else if(ferr <= cbuf || ferr > ce){
        noflag = true;
    }
    char * pcbuf;

    // find the last ; and see if ferr is there or not
    for(pcbuf = ce-1; (*pcbuf != ';') && (*pcbuf != '\0') && (pcbuf > cbuf); --pcbuf);
    // if ferr (end of flags) at pcbuf (last ;) then params should be at the beginning, otherwise start at ferr
    pcbuf = (pcbuf+1 == ferr || noflag) ? cbuf : ferr;

    // puts("params now");
    //char * perr = linkParams(pcbuf, &paramHead, "parameters:");
    char * perr = parseArgSpec(pcbuf, &paramMap, "parameters:", NULL, STR, true);
    //printMap(&paramMap);
    // if didnt find params and no flag then return 1 this is bad
    if(perr == NULL){
        // if params errored on parsing then exit
        // TODO print error message
        return 3;
    } else if((perr <= cbuf || perr > ce) && noflag){
        // TODO figure out error
        fprintf(stderr, "ill figure this error out later\n");
        return 2;
    }

    Errors retVal = parseArgsPrint(argc, argv, &flagMap, &paramMap);
    // TODO do some stuffs and figure out the errors
    //  - probably exit with this exit code specifically
    //if(retVal != Success){
    //    fprintf(stderr, "error parsing args: %d\n", retVal);
    //}

    //bool help = getMapMember_bool(&flagMap, "help", 4);
    MapNode * helpNode = getMapNode(&flagMap, "help", 4);
    // TODO figure out whether we should do this
    if(helpNode != NULL && *(const bool *)helpNode->data.ptr){
    //if(hasNode(&flagMap, "help", 4) && getMapMember_bool(&flagMap, "help", 4)){
        printUsage(&flagMap, &paramMap, argv[0]);
        //printHelp(&flagMap, &paramMap, "help,flag,qwerty,asdf,zzz", "print this help message", "i dunno, doesnt matter", "other message", "something whatever", "i dunno, something optional");
        // TODO figure out how to get user defined help here
        printHelp(&flagMap, &paramMap, "help", "print this help message");
        //printf("exit");
        return 1;
    }

    // TODO make a freeing function to free data
    freeMap(&flagMap);
    freeMap(&paramMap);

    free(cbuf);

    return retVal;
}




