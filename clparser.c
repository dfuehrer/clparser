#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "process.h"

#define MAX_SHELL_LEN   10
#define MAX_PROCCOMM_LEN    (6 + 10 + 5 + 1)    // /proc/ + ppid + /comm    + \000
#define MAX_PROCCMDLINE_LEN  (6 + 10 + 8 + 1)    // /proc/ + ppid + /cmdline + \000

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
//  flag=false
//  f=false
//  h=false
//  help=false
//  g=false
//  qwerty=true
//  asdf="${2}"
//  q="${2}"
//  Z='test'
//  zzz='test'
//  z='test'
//  u='defval'
//  nothing="${8}"
//  set -- "${4}" "${6}" "${10}" "${11}"
// if called with arguments `-fgq whatever --qwerty 'i dunno' --zzz=test words --nothing else -- --test hi`
// run it like eval "$(echo "$spec" | clparser -- "$@")" and itll set the variables accordingly so you can use them without doing any parsing
// error codes:
// 1 not alphanumeric flags or parameter
// 2 flags or parameter not defined
// 3 bad definitions
// TODO im considering changing the structure to make it so command line args are possible for this program
// i need to figure out a way to signal that the arguemnt is just for this parser and not what its working with
//  that could work like echo "$spec" | clparser --args-for-parser -- "$@"
//  in this case the arguments after -- are what needs to be parsed which makes the -- necessary (could make it something like -p or --parse instead of just --)
//  (could have it assume to parse everything if there is no -- but that could be confusing if someone passes in a -- that wasnt exected and things are parsed wrong or it errors cause thats how that works)
// if it hits a -- with no word after it should take that to mean everything after is default since that seems to be how most things would use it (this is less flexible in some ways but i dont want to deal with it)
// ive decided this needs a help system so probably that would be like giving one of the names and then the help for it in a big string and then passing that string into the parser with a like --helpmsgs
// this thing needs a help and other arguments in general probably
int main(int argc, const char * const argv[]){

    // setup internal flags and params maps
    map_t flagMap_loc, paramMap_loc;
    const char ** passedArgs = NULL;
    int passedArgc;
    initMap(&flagMap_loc);
    initMap(&paramMap_loc);
    MapData * helpNode         = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("help"         ), "h", 1);
    MapData * maintainArgvNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("maintain-argv"), "a", 1);
    MapData * overrideArgvNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("override-argv"), "A", 1);
    MapData * helpExitsNode    = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("help-exits"   ), "e", 1);
    MapData * useNamespaceNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("namespace"    ), "n", 1);
    MapData * helpMsgNode      = addMapMembers(&paramMap_loc, NULL      , STR , false, "Ssdsd", STRVIEW("help-msg"     ), "H", 1, "m", 1);
    MapData * shellNode        = addMapMembers(&paramMap_loc, "default" , STR , true , "Ssdsd", STRVIEW("shell"        ), "S", 1, "s", 1);
    MapData * progNameNode     = addMapMembers(&paramMap_loc, "default" , STR , true , "Ssd"  , STRVIEW("prog-name"    ), "p", 1);
    setNodeNegation(maintainArgvNode, overrideArgvNode);
    setNodeNegation(overrideArgvNode, maintainArgvNode);
    //setNodeNegation(useNamespaceNode, maintainArgvNode);
    //helpMsgNode->data.required = false;     // this isnt really super required

    Errors retVal = parseArgs(argc - 1, argv + 1, &flagMap_loc, &paramMap_loc, &passedArgs);
    if(retVal != Success){
        fprintf(stderr, "error parsing args: %d\n", retVal);
        return retVal;
    }

    // set the default value for the shell if not given
    const char * shellStr =    shellNode->data.ptr;
    const char * progName = progNameNode->data.ptr;
    char parentCommandStr[MAX_SHELL_LEN] = "";
    bool allocatedProgName = false;
    pid_t ppid = getppid();
    if(strcmp(shellStr, "default") == 0){
        // TODO figure out a general way of getting the calling process name
        //char * pcmdFilename = NULL;
        char pcmdFilename[MAX_PROCCOMM_LEN];
        //asprintf(&pcmdFilename, "/proc/%d/comm", ppid);
        snprintf(pcmdFilename, MAX_PROCCOMM_LEN, "/proc/%d/comm", ppid);
        FILE * pcmdFile = fopen(pcmdFilename, "r");
        //free(pcmdFilename);
        fgets(parentCommandStr, MAX_SHELL_LEN, pcmdFile);
        int comlen = strlen(parentCommandStr);
        if( parentCommandStr[comlen-1] == '\n'){
            parentCommandStr[comlen-1] =  '\000';
        }
        // TODO xonsh seems to be more of a pain
        shellStr = parentCommandStr;
        shellNode->data.defaultData->ptr = shellStr;
    }
    if(strcmp(progName, "default") == 0){
        // TODO figure out a general way of getting the calling script
        //char * pcmdFilename = NULL;
        char pcmdFilename[MAX_PROCCMDLINE_LEN];
        //asprintf(&pcmdFilename, "/proc/%d/comm", ppid);
        snprintf(pcmdFilename, MAX_PROCCMDLINE_LEN, "/proc/%d/cmdline", ppid);
        FILE * pcmdFile = fopen(pcmdFilename, "r");
        //free(pcmdFilename);
        size_t n = 0;
        char * cmdline = NULL;
        // call getline twice to get the first arg (script filename)
        getdelim(&cmdline, &n, '\000', pcmdFile);
        getdelim(&cmdline, &n, '\000', pcmdFile);
        progName = cmdline;
        progNameNode->data.defaultData->ptr = progName;
        allocatedProgName = progName != NULL;
    }

    if(*(const bool *)helpNode->data.ptr){
        // TODO have the help info not print out to stderr
        printUsage(&flagMap_loc, &paramMap_loc, argv[0]);
        fprintf(stderr, "\tclparser takes in command line arguments to parse as command line arguments and a specification\n");
        fprintf(stderr, "\tfor which command line arguments should exist from stdin, and outputs shell code to set variables\n");
        fprintf(stderr, "\tset from the command line arguments to stdout.  Command line arguments to be parsed should be\n");
        fprintf(stderr, "\tgiven after --.  This will prevent the command line arguments to be parsed from being interpreted\n");
        fprintf(stderr, "\tas clparser command line arguments.  See the man page or README.md for more details\n");
        printHelp(&flagMap_loc, &paramMap_loc,
                "help          = print this help message\n\
                 maintain-argv = do not override argv\n\
                 override-argv = do     override argv\n\
                 shell         = which shell syntax to use (sh, bash, zsh, ksh, csh, fish, xonsh) (default to calling shell)\n\
                 help-exits    = exit after printing the help message\n\
                 help-msg      = some sort of string of help messages for args");
        return 1;
    }

    for(passedArgc = 0; passedArgs[passedArgc] != NULL && passedArgc <= argc; ++passedArgc);

    const char * helpMessage = helpMsgNode->data.ptr;

    Shell shell;
    if      (strcmp(shellStr,    "sh") == 0 || strcmp(shellStr, "dash") == 0 || strcmp(shellStr, "ash") == 0){
        shell = SH;
    }else if(strcmp(shellStr,  "bash") == 0){
        shell = BASH;
    }else if(strcmp(shellStr,   "zsh") == 0){
        shell = ZSH;
    }else if(strcmp(shellStr,   "ksh") == 0){
        shell = KSH;
    }else if(strcmp(shellStr,  "tcsh") == 0 || strcmp(shellStr, "csh") == 0){
        shell = CSH;
    }else if(strcmp(shellStr,  "fish") == 0){
        shell = FISH;
    }else if(strcmp(shellStr, "xonsh") == 0){
        shell = XONSH;
    }

    bool useNamespace =  *(const bool *)useNamespaceNode->data.ptr;
    bool helpExits =  *(const bool *)helpExitsNode   ->data.ptr;
    bool useArgv   =  *(const bool *)overrideArgvNode->data.ptr;
    if( !useArgv &&  !*(const bool *)maintainArgvNode->data.ptr && shell == SH){
        // if didnt set override or maintain argv, then default to maintain unless using SH since it has no arrays
        // so the only way to maintain whitespace is to use argv
        useArgv = true;
    }

    // free the maps, should be done with the info now
    freeMap(&flagMap_loc);
    freeMap(&paramMap_loc);

    char * cbuf = NULL;
    size_t n = 0;
    ssize_t len;
    len = getline(&cbuf, &n, stdin);
    // TODO check that there wasnt an error
    map_t flagMap, paramMap;
    initMap(&flagMap);
    initMap(&paramMap);

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

    // TODO pass in all these options in a struct
    retVal = parseArgsPrint(passedArgc, passedArgs, &flagMap, &paramMap, shell, useArgv);
    // TODO do some stuffs and figure out the errors
    //  - probably exit with this exit code specifically
    //if(retVal != Success){
    //    fprintf(stderr, "error parsing args: %d\n", retVal);
    //}

    //bool help = getMapMember_bool(&flagMap, "help", 4);
    helpNode = getMapNode(&flagMap, "help", 4);
    // TODO figure out whether we should do this
    if(helpNode != NULL && *(const bool *)helpNode->data.ptr){
    //if(hasNode(&flagMap, "help", 4) && getMapMember_bool(&flagMap, "help", 4)){
        // TODO get the script name
        // TODO have the help info not print out to stderr and instead give printf commands 
        //printf(";\n");
        printUsage(&flagMap, &paramMap, progName);
        if(helpMessage != NULL){
            printHelp(&flagMap, &paramMap, helpMessage);
        }
        if(helpExits){
            printExit(shell);
        }
        return 0;
    }

    freeMap(&flagMap);
    freeMap(&paramMap);

    free(cbuf);
    if(allocatedProgName){
        free((void *)progName);
    }

    return retVal;
}




