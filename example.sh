#!/usr/bin/env sh
make || exit
args='flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
# print out the raw output
echo raw output:
vars="$(echo "$args" | ./clparser -- "$@")"
ec=$?
echo "$vars"
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
