#!/bin/sh
make || exit
args='flags: f,flag g qwerty h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
# print out the raw output
echo raw output:
vars="$(echo "$args" | ./clparser "$@")" || { ec=$?; echo exit code: $ec; exit $ec; }
echo "$vars"
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
