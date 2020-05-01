# clparser
* This is supposed to be a basic parser of command line arguments for bash scripts
* The idea is that you give it a spec in a certain format and and command line arguments that were inputted to the script and it parses them 
    by turning them into simple ```variablename=value```
* This makes it so that you can just call it with 
<pre>bash
$(echo '<i>spec</i>' | clparser "$@")
</pre>
* This way it will call the ```variablename=value``` and you can go on without having to worry about possitioning or anything

* I know i could have done this in awk or bash like anyone else
    (or used someone elses)
    but I wanted to write my own
