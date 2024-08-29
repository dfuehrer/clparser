#!/usr/bin/env fish

make || exit
set spec 'flags: f,flag=-g=-h g=-h=-qwerty qwerty=-flag=-h=-g h,help; parameters: q,asdf u=defval nothing zzz,z,Z=someth; positionals: asdf test;'
set helpmsg '
help = print this help message
flag = a random flag
g    = i dunno, its nonsense
qwerty = a keyboard layout
asdf = q param
u    = something
zzz  = a bunch of zs
'
# print out the raw output
echo raw output:
set vars "$(echo $spec | ./clparser -e --help-msg $helpmsg -- $argv)"
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
echo positional params:
echo asdf = "$asdf"
echo test = "$test"
#printf 'positional args: %s\n' "$*"
printf 'positional args:'
printf " '%s'" $args
printf '\n'
