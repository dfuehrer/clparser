#include <stdio.h>
#include <string.h>

#define BUF_SIZE    1000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter
#define OUTPUT_SIZE 2000   // yes this will probably cause me problems later, maybe ill even make it dynamic like a quitter

// so the idea is that youll send in configuration stuffs over stdin and then have it take in the clargs as clargs and then itll output things that i guess are helpful based on the config stuffs sent in over stdin
// theres no way that this will be overcomplicated or impossible to use or write at all
// ill define that this can have flags and options that are set to values and a default field that everything else falls into
// ill do it in the gnuish i think style where there can be words that use the --, single letters use - and can be combined 
// if the single letter - options arent flags then the next arguments will be used as the options for that parameter thingy i dont know howw to explain
// it should go in order of the options it gets so if they contradict then itll be the last option
// the word parameters can use a space or an = with no spaces
// the stdin should be of the form:
// "flags: f,flag g, qwerty h,hist; parameters: q,asdf u nothing zzz,z;"
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
int main(int argc, char ** argv){
    // this is just for debugging
    for(int i = 1; i < argc; i++){
        printf("%s\n", argc[i]);
    }
    char cbuf[BUF_SIZE];
    fgets(cbuf, BUF_SIZE, stdin);
    printf("%s\n", cbuf);

    char flags[BUF_SIZE];
    char params[BUF_SIZE];
    memset(flags, 0, BUF_SIZE);
    memset(params, 0, BUF_SIZE);
    char * flagptr = strstr(cbuf, "flags:");
    char * paramptr = strstr(cbuf, "parameters:");
#define FLAGS_LEN 6
#define PARAMS_LEN 11
    if(flagptr != NULL){
        char * end = strchr(cbuf + flagptr, ';');
        if(end == NULL) return 3;
        strncpy(flags, cbuf + FLAGS_LEN);   // dont need to worry about null terminating since the strings were set with memset to 0 anyways
    }
    if(paramptr != NULL){
        strcpy(params, cbuf + PARAMS_LEN);
    }

    for(int i = 1; i < argc; i++){
        if(argv[i] == '-'){
            // this is a single letter flag so loop through all the next letters
            for(char * f = argv[i][1]; *f; f++){
                if(!isalnum(*f)){
                    return 1;
                }else if(strchr(flags, *f) == NULL && strchr(params, *f) == NULL){
                    return 2;
                }
            }
        }
    }

    return 0;
}
