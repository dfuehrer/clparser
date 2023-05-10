#!/bin/sh
make && echo 'flags: f,flag g qwerty h,hist; parameters: q,asdf u=defval nothing zzz,z,Z=someth;' | ./clparser "$@" || echo $?
