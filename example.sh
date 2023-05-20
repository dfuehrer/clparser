#!/bin/sh
make || exit
args='flags: f,flag g qwerty h,hist; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
# print out the raw output
echo raw output:
echo "$args" | ./clparser "$@" || { ec=$?; echo exit code: $ec; exit $ec; }
# actually run it and show results
echo variable results:
eval "$(echo "$args" | ./clparser "$@")"
echo f,flag = "$flag"
echo g = "$g"
echo qwerty = "$qwerty"
echo h,hist = "$hist"
echo q,asdf = "$asdf"
echo u = "$u"
echo nothing = "$nothing"
echo zzz,z,Z = "$Z"
printf 'default vals: %s\n' "$*"
