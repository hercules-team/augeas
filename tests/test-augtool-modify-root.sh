#! /bin/sh

# Make sure changing the value of root works

exp="/ = root"

act=$(augtool --noautoload 2>&1 <<EOF
set / root
get /
quit
EOF
)
result=$?

if [ $result -ne 0 ]; then
    echo "augtool failed"
    exit 1
fi

if [ "$act" != "$exp" ]; then
    echo "augtool produced unexpected output:"
    echo "Expected:"
    echo "$exp"
    echo "Actual:"
    echo "$act"
    exit 1
fi
