#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "parseargs.h"
#include "printargs.h"

#define MAX_SHELL_LEN   10
#define MAX_PROCCOMM_LEN    (6 + 10 + 5 + 1)    // /proc/ + ppid + /comm    + \000
#define MAX_PROCCMDLINE_LEN (6 + 10 + 8 + 1)    // /proc/ + ppid + /cmdline + \000




// TODO add multiple args support like nargs=+ from python argparse
// TODO update return values to be unique (maybe overlap with map errors)

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
// 2 flags and parameter not defined
// 3 bad definitions
// use structure echo "$spec" | clparser --args-for-parser -- "$@"
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
    // TODO add better support for positional parameters:
    //  1.  flag to specify "stop parsing" (count rest of args as positional) if any positional arg found
    //  2.  flag to specify "stop parsing" (count rest of args as positional) if any arg found that is not in the specification
    //      - could encapsulate 1. if you explicitly specify 0 positional args (probably better this way)
    //  3.  add something specific for positional parameters to be able to look up values
    //      - eg: script file1 file2 [options] setting file1 to first arg and file2 to setcond arg rather than just an array
    //      - need some way of specifying which position the arg should be filled by
    //      - need to support both named positional args and existing array style
    //      - maybe have parseArgs take in an array (NULL terminated) of MapData pointers of nodes of the positional args map, in the order of positional args
    //          - get map nodes from return of addMapMembers, just add to array at end
    //          - spec parser just parse like parameters but then create array (TODO should i make a new function or work around the assumptions of the existing one)
    MapData * helpNode         = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("help"              ), "h", 1);
    MapData * maintainArgvNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("maintain-argv"     ), "a", 1);
    MapData * overrideArgvNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("override-argv"     ), "A", 1);
    MapData * helpExitsNode    = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("help-exits"        ), "e", 1);
    MapData * useNamespaceNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("namespace"         ), "n", 1);
    MapData *  noNamespaceNode = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("no-namespace"      ), "N", 1);
    MapData * noOutEmptyNode   = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssd"  , STRVIEW("no-output-empty"   ), "E", 1);
    MapData * unkIsPosNode     = addMapMembers(& flagMap_loc, &flagFalse, BOOL, false, "Ssdsd", STRVIEW("unknown-positional"), "u", 1, "P", 1);
    MapData * helpMsgNode      = addMapMembers(&paramMap_loc, NULL      , STR , false, "Ssdsd", STRVIEW("help-msg"          ), "H", 1, "m", 1);
    MapData * shellNode        = addMapMembers(&paramMap_loc, "default" , STR , true , "Ssdsd", STRVIEW("shell"             ), "S", 1, "s", 1);
    MapData * progNameNode     = addMapMembers(&paramMap_loc, "default" , STR , true , "Ssd"  , STRVIEW("prog-name"         ), "p", 1);
    setNodeNegation(maintainArgvNode, overrideArgvNode);
    setNodeNegation(overrideArgvNode, maintainArgvNode);
    setNodeNegation(useNamespaceNode,  noNamespaceNode);
    setNodeNegation( noNamespaceNode, useNamespaceNode);
    //helpMsgNode->data.required = false;     // this isnt really super required

    // TODO should clparser parse given input args if they don't match clparser args?
    //  - this would sorta be nice but would just be way to error prone that it wouldn't really be worth it
    Errors retVal = parseArgs(argc - 1, argv + 1, &flagMap_loc, &paramMap_loc, NULL, &passedArgs, false);
    if(retVal != Success){
        fprintf(stderr, "error parsing args: %d\n", retVal);
        return retVal;
    }

    // set the default value for the shell if not given
    const char * shellStr = getNodeData(   shellNode);
    const char * progName = getNodeData(progNameNode);
    char parentCommandStr[MAX_SHELL_LEN] = "";
    bool allocatedProgName = false;
    pid_t ppid = getppid();
    if(strcmp(shellStr, "default") == 0){
        // TODO figure out a general way of getting the calling process name
        char pcmdFilename[MAX_PROCCOMM_LEN];
        snprintf(pcmdFilename, MAX_PROCCOMM_LEN, "/proc/%d/comm", ppid);
        FILE * pcmdFile = fopen(pcmdFilename, "r");
        fgets(parentCommandStr, MAX_SHELL_LEN, pcmdFile);
        int comlen = strlen(parentCommandStr);
        if( parentCommandStr[comlen-1] == '\n'){
            parentCommandStr[comlen-1] =  '\000';
        }
        // TODO xonsh seems to be more of a pain (appimage super doesnt work like this, actual install puts part of the script name in comm?)
        shellStr = parentCommandStr;
        shellNode->data.defaultData->ptr = shellStr;
    }
    if(strcmp(progName, "default") == 0){
        // TODO figure out a general way of getting the calling script
        char pcmdFilename[MAX_PROCCMDLINE_LEN];
        snprintf(pcmdFilename, MAX_PROCCMDLINE_LEN, "/proc/%d/cmdline", ppid);
        FILE * pcmdFile = fopen(pcmdFilename, "r");
        size_t n = 0;
        char * cmdline = NULL;
        // call getline twice to get the first arg (script filename)
        getdelim(&cmdline, &n, '\000', pcmdFile);
        getdelim(&cmdline, &n, '\000', pcmdFile);
        progName = cmdline;
        progNameNode->data.defaultData->ptr = progName;
        allocatedProgName = progName != NULL;
    }

    if(getNode_bool(helpNode)){
        // TODO have the help info not print out to stderr
        printUsage(&flagMap_loc, &paramMap_loc, NULL, argv[0]);
        fprintf(stderr, "\tclparser takes in command line arguments to parse as command line arguments and a specification\n");
        fprintf(stderr, "\tfor which command line arguments should exist from stdin, and outputs shell code to set variables\n");
        fprintf(stderr, "\tset from the command line arguments to stdout.  Command line arguments to be parsed should be\n");
        fprintf(stderr, "\tgiven after --.  This will prevent the command line arguments to be parsed from being interpreted\n");
        fprintf(stderr, "\tas clparser command line arguments.  See the man page or README.md for more details\n");
        printHelp(&flagMap_loc, &paramMap_loc, NULL,
                "help               = print this help message\n\
                 maintain-argv      = do not override argv\n\
                 override-argv      = do     override argv\n\
                 namespace          = namespace args by using associative arrays if available or prepending flags_ or params_ to the variable names\n\
                 no-namespace       = do not use associative arrays or prefixes on variable names\n\
                 no-output-empty    = don't output parameters that aren't given\n\
                 unknown-positional = if an unknown argument is seen, treat all following arguments as positional arguments\n\
                 shell              = which shell syntax to use (sh, bash, zsh, ksh, csh, fish, xonsh) (default to calling shell)\n\
                 prog-name          = name of program to display in --help usage message (default to path of script called (first arg of cmdline of calling shell))\n\
                 help-exits         = exit after printing the help message\n\
                 help-msg           = some sort of string of help messages for args");
        return 1;
    }

    for(passedArgc = 0; passedArgs[passedArgc] != NULL && passedArgc <= argc; ++passedArgc);

    const char * helpMessage = getNodeData(helpMsgNode);

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

    bool helpExits =  getNode_bool(helpExitsNode   );
    bool useArgv   =  getNode_bool(overrideArgvNode);
    if( !useArgv &&  !getNode_bool(maintainArgvNode) && shell == SH){
        // if didnt set override or maintain argv, then default to maintain unless using SH since it has no arrays
        // (so the only way to maintain whitespace in sh is to use argv)
        useArgv = true;
    }
    bool useNamespace =   getNode_bool(useNamespaceNode);
    if( !useNamespace && !getNode_bool( noNamespaceNode)){
        // if didnt set use/don't use namespace, then default to use namespace unless using SH, CSH, or FISH since they have no associative arrays
        switch(shell){
            case CSH:
            case FISH:
            case SH:
                useNamespace = false;
                break;
            case BASH:
            case ZSH:
            case KSH:
            case XONSH:
            default:
                useNamespace = true;
                break;
        }
    }
    ParsePrintOptions parseOpts = {
        .shell=shell,
        .useArgv=useArgv,
        .useNamespace=useNamespace,
        .noOutputEmpty=getNode_bool(noOutEmptyNode),
        .unknownPositional=getNode_bool(unkIsPosNode),
    };

    // free the maps, should be done with the info now
    freeMap(&flagMap_loc);
    freeMap(&paramMap_loc);

    char * cbuf = NULL;
    size_t n = 0;
    ssize_t len;
    // TODO should i just use getdelim with \0?
    len = getline(&cbuf, &n, stdin);
    // TODO check that there wasnt an error
    if(len == -1){
        perror("read spec");
        return 2;
    }
    map_t flagMap, paramMap, posMap;
    initMap(&flagMap);
    initMap(&paramMap);
    initMap(&posMap);

    // let me think about what im doing here
    // ferr is a pointer to the char after the ; ending the flags or whatever error happened
    char * ce = cbuf + len;
    // find all flag params, make the default value "0" (false)
    char * ferr = parseArgSpec(cbuf, &flagMap, "flags:", (void *)&flagFalse, BOOL, false, NULL);
    //printMap(&flagMap);

    //addMapMembers(&flagMap, (void *)&flagFalse, BOOL, "sdsd", "help", 4, "h", 1);

    // if the ferr is before or after the buff then we know theres no flags
    bool noflag = false;
    bool noparam = false;
    if(ferr == NULL){
        fprintf(stderr, "error reading flags\n");
        return 3;
    } else if(ferr <= cbuf || ferr > ce){
        noflag = true;
    }

    // puts("params now");
    char * perr = parseArgSpec(cbuf, &paramMap, "parameters:", NULL, STR, true, NULL);
    //printMap(&paramMap);
    // if didnt find params and no flag then return 1 this is bad
    if(perr == NULL){
        // if params errored on parsing then exit
        fprintf(stderr, "error reading parameters\n");
        return 3;
    } else if(perr <= cbuf || perr > ce){
        noparam = true;
    }

    MapData * * positionalParamNodes = NULL;
    perr = parseArgSpec(cbuf, &posMap, "positionals:", NULL, STR, true, &positionalParamNodes);
    //printMap(&paramMap);
    // if didnt find params and no flag then return 1 this is bad
    if(perr == NULL){
        // if params errored on parsing then exit
        fprintf(stderr, "error reading positionals\n");
        return 3;
    } else if(perr <= cbuf || perr > ce){
        // if didnt get pos params, then free it rather than having explicit 0 positionals defined
        free(positionalParamNodes);
        positionalParamNodes = NULL;
        if(noflag && noparam){
            fprintf(stderr, "found neither flags nor parameters nor positionals\n");
            return 2;
        }
    }

    retVal = parseArgsPrint(passedArgc, passedArgs, &flagMap, &paramMap, positionalParamNodes, &parseOpts);
    // TODO do some stuffs and figure out the errors
    //  - probably exit with this exit code specifically
    //if(retVal != Success){
    //    fprintf(stderr, "error parsing args: %d\n", retVal);
    //}

    //bool help = getMapMember_bool(&flagMap, "help", 4);
    helpNode = getMapNode(&flagMap, "help", 4);
    if(helpNode != NULL && getNode_bool(helpNode)){
    //if(hasNode(&flagMap, "help", 4) && getMapMember_bool(&flagMap, "help", 4)){
        printUsage(&flagMap, &paramMap, (const MapData **) positionalParamNodes, progName);
        if(helpMessage != NULL){
            printHelp(&flagMap, &paramMap, (const MapData **) positionalParamNodes, helpMessage);
        }
        if(helpExits){
            printExit(shell);
        }
        return 0;
    }

    freeMap(&flagMap);
    freeMap(&paramMap);
    freeMap(&posMap);
    if(positionalParamNodes != NULL){
        free(positionalParamNodes);
    }

    free(cbuf);
    if(allocatedProgName){
        free((void *)progName);
    }

    return retVal;
}




