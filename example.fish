#!/usr/bin/env fish

make || exit
set spec 'flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth;'
echo $spec
# print out the raw output
echo raw output:
set vars "$(echo $spec | ./clparser -- $argv)"
set ec $status
echo $vars
if test $ec != 0
    echo exit code: $ec
    exit $ec
end
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
