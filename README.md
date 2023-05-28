# clparser
* This is supposed to be a basic parser of command line arguments for shell scripts
* The idea is that you give it a spec in a certain format and command line arguments that were inputted to the script and it parses them
    by turning them into simple `variablename=value`
* This makes it so that you can just call it with 
```sh
eval "$(echo "$spec" | clparser -- "$@")"
```
* This way it will call the `variablename=value` and you can go on without having to worry about positioning or anything

* where `spec='flags: f,flag g qwerty h,hist; parameters: q,asdf u nothing zzz,z;'`
and prints out
```sh
flag=false
f=false
h=false
help=false
g=false
qwerty=true
asdf="${2}"
q="${2}"
Z='test'
zzz='test'
z='test'
u='defval'
nothing="${8}"
set -- "${4}" "${6}" "${10}" "${11}"
```
if called with arguments `-fgq whatever --qwerty 'i dunno' --zzz=test words --nothing else -- --test hi`
which results in
    f,flag = false
    g = false
    qwerty = true
    h,help = false
    q,asdf = whatever
    u = defval
    nothing = else
    zzz,z,Z = test
    positional args: i dunno words --test hi

- example scripts have been added showing off the features
    - try running the scripts to see the output and how it can be used
    - for `zsh` and `ksh`, see the `example.bash` script, it uses the same syntax
    - for `xonsh`, probably just use argparse, its built in and very good

- if `clparser` sees `--` in the args, it will stop processing the args and just output everything after as positional args
    - this is used to let `clparser` know all the remaining args should be passed through
    - this is how clparser knows the difference between it's own args to parse and the scripts args
    - any args not starting with - and not after a parameter will be turned into a positional arg
- `clparser` uses direct references to the input positional arguments as much as possible to avoid issues with whitespace or other symbol expansion
- flags are returned as true or false
    - use in `sh` like `"$force" && rm -f`, running the `true` and `false` commands rather than testing the string
```sh
if "$force"; then
    echo force
fi
```
- `bash`, `zsh`, `ksh`, and `xonsh` use associative arrays for flags and params (see example.* scripts for examples)
    - `sh`, `csh`, and `fish` all use plain variables because they don't have associative arrays and variables are sufficient
- `bash`, `zsh`, `ksh`, `xonsh`, `fish`, and `csh` use arrays for the positional arguments
    - `sh` will override argv to it can preserve whitespace in the input arguments
    - `--override-argv` will force overriding  argv in all shells
    - `--maintain-argv` will force maintaining argv in all shells
- the shell syntax language is determined by the calling process
    - NOTE: this doesn't seem to work for xonsh, at least while installed through the Appimage
    - `--shell` specifies a specific shell to use
- `clparser --help` will show all the options
    - NOTE: currently `--help-msg` is unimplemented

# C library
- `clparser` was written so it can be used as a C lib
    - `clparser` uses the library as intended for its own arguments
- `#include "process.h"` and add process.o and map.o to the compilation command
