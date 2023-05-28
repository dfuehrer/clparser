#!/usr/bin/env tcsh

make || exit
set spec='flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
# print out the raw output
echo raw output:
set args="`seq -f '"\"\$"argv[%.0f]"\""' $#`"
eval 'echo "'"$spec"'" | ./clparser -- '"$args"
#set vars=`echo $spec | ./clparser -- $argv[*]`
set vars=`eval 'echo "'"$spec"'" | ./clparser -- '"$args"`
set ec=$?
echo vars len: $#vars
echo "$vars"
if ( "$ec" != 0 ) then
    echo exit code: $ec
    exit $ec
endif
# actually run it and show results
echo variable results:
eval "$vars"
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
