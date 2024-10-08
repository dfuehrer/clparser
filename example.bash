#!/usr/bin/env bash

# NOTE this file can be run with zsh and ksh just fine (clparser uses the same syntax for all 3)
make || exit
spec='flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth; positionals: asdf test;'
helpmsg='
help = print this help message
flag = a random flag
g    = i dunno, its nonsense
qwerty = a keyboard layout
asdf = q param
u    = something
zzz  = a bunch of zs
'
#$help && exit
# print out the raw output
echo raw output:
vars="$(echo "$spec" | ./clparser -e --help-msg "$helpmsg" -- "$@")"
ec=$?
echo "$vars"
if [ "$ec" -ne 0 ]; then
    echo exit code: $ec
    exit $ec
fi
# actually run it and show results
echo variable results:
eval "$vars"
echo f,flag  = "${flags[flag]}"
echo g       = "${flags[g]}"
echo qwerty  = "${flags[qwerty]}"
echo h,help  = "${flags[help]}"
echo q,asdf  = "${params[asdf]}"
echo u       = "${params[u]}"
echo nothing = "${params[nothing]}"
echo zzz,z,Z = "${params[Z]}"
echo positional params:
echo asdf = "${positionals[asdf]}"
echo test = "${positionals[test]}"
printf 'positional args:'
printf " '%s'" "${args[@]}"
printf '\n'
