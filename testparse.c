#include <stdio.h>
#include <string.h>
#include "process.h"

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
            



