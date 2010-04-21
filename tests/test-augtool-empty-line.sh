#! /bin/bash

# Test for BZ 566844. Make sure we don't spew nonsense when the input
# contains empty lines
out=$(augtool --nostdinc 2>&1 <<EOF

quit
EOF
)
result=$?

if [ $result -ne 0 ]; then
    echo "augtool failed"
    exit 1
fi

if [ -n "$out" ]; then
    echo "augtool produced output:"
    echo "$out"
    exit 1
fi
