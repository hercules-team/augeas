#!/bin/sh 
# create some examples of operations on finite automaton

export PATH=./:$PATH

dest=/tmp/fadot-examples
mkdir -p $dest

fadot -f $dest/sample.dot     -o show         "[a-z]*"
fadot -f $dest/concat.dot     -o concat       "[a-b]" "[b-c]"
fadot -f $dest/union.dot      -o union        "[a-b]" "[b-c]"
fadot -f $dest/intersect.dot  -o intersect    "[a-b]" "[b-c]"
fadot -f $dest/complement.dot -o complement   "[a-z]"
fadot -f $dest/minus.dot      -o minus        "[a-z]" "[a-c]" 


for i in $(ls -1 $dest/*.dot|cut -d. -f1); do
    dot -Tpng -o $i.png $i.dot
done

echo "Example compilation complete. Results are available in directory $dest"
