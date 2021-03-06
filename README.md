# clparser
* This is supposed to be a basic parser of command line arguments for bash scripts
* The idea is that you give it a spec in a certain format and and command line arguments that were inputted to the script and it parses them 
    by turning them into simple ```variablename=value```
* This makes it so that you can just call it with 
```bash
eval $(echo 'spec' | clparser "$@")
```
* This way it will call the ```variablename=value``` and you can go on without having to worry about possitioning or anything

* where spec looks something like
"flags: f,flag g qwerty h,hist; parameters: q,asdf u nothing zzz,z;"
and results in
```bash
f=1
flag=1
g=1
qwerty=1
h=1
hist=1
q=whatever
asdf=whatever
nothing=sure
defaults="who even cares"
```
if called with arguments "-fgq whatever --qwerty --nothing=sure who even cares"

* I know i could have done this in awk or bash builtin stuff like anyone else
    (or used someone elses)
    but I wanted to write my own
* it is a good note that this isnt really for public use but if it works for you thats great


## TODO

