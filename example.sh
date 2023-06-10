#!/usr/bin/env sh
make || exit
spec='flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
helpmsg='
help = print this help message
flag = a random flag
g    = i dunno, its nonsense
qwerty = a keyboard layout
asdf = q param
u    = something
zzz  = a bunch of zs
'
#tr '\0' '\n' < /proc/$$/cmdline | head -n2
#cat /proc/$$/comm
#cat -v /proc/$$/cmdline
#echo
#wc /proc/$$/cmdline
# print out the raw output
echo raw output:
vars="$(echo "$spec" | ./clparser --no-output-empty -e --help-msg "$helpmsg" -- "$@")"
ec=$?
printf '%s\n' "$vars"
if [ "$ec" -ne 0 ]; then
    echo exit code: $ec
    exit $ec
fi
# actually run it and show results
echo variable results:
eval "$vars"
echo f,flag = "$flag"
echo g = "$g"
echo qwerty = "$qwerty"
echo h,help = "$help"
echo q,asdf = "$asdf"
echo u = "$u"
echo nothing = "$nothing"
echo zzz,z,Z = "$Z"
printf 'default vals: %s\n' "$*"
#printf 'default vals: %s\n' "$args"
