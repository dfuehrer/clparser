#!/usr/bin/env tcsh

make || exit
set spec='flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
set helpmsg='\
help = print this help message\
flag = a random flag\
g    = i dunno, its nonsense\
qwerty = a keyboard layout\
asdf = q param\
u    = something\
zzz  = a bunch of zs\
'
# print out the raw output
echo raw output:
#echo "$spec:q" | ./clparser --help-msg "$helpmsg:q" -- $argv:q
#set vars=`echo $spec | ./clparser -- $argv[*]`
set vars=`echo $spec:q | ./clparser -e --help-msg $helpmsg:q -- $argv:q`
#set vars="`eval echo '$spec:q' | ./clparser -- $argv:q`"
set ec=$?
echo vars len: $#vars
printf '%s\n' "$vars:q"
if ( "$ec" != 0 ) then
    echo exit code: $ec
    exit $ec
endif
# actually run it and show results
echo variable results:
eval "$vars:q"
#eval `echo "$spec:q" | ./clparser -e --help-msg "$helpmsg:q" -- $argv:q`
$help && exit
echo f,flag  = "$flag"
echo g       = "$g"
echo qwerty  = "$qwerty"
echo h,help  = "$help"
echo q,asdf  = "$asdf"
echo u       = "$u"
echo nothing = "$nothing"
echo zzz,z,Z = "$Z"
#printf 'positional args: %s\n' "$*"
printf 'positional args: %s\n' "$args"
