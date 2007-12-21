#! /bin/bash

DATADIR=$(dirname $0)
GRAMMARS=${DATADIR}/grammars/reject/*.aug
AUGPARSE=${top_builddir-${DATADIR}/..}/src/augparse

echo "--------------------------------"
echo "Checking grammar rejection"
echo "   grammars from grammars/reject"
echo

if [ ! -x $AUGPARSE ] ; then
    echo "Failed to find executable augparse"
    echo "  looked for $AUGPARSE"
    exit 127
fi

result=0
for g in $GRAMMARS; do
    if [[ ! -r "$g" ]]; then
        echo "Grammar file $g is not readable"
        exit 19
    fi
    printf "%-30s ... " $(basename $g)
    ${AUGPARSE} $g > /dev/null 2>&1
    ret=$?
    if [[ $ret -eq 0 ]]; then
        echo FAIL
        result=1
    elif [[ $ret -eq 1 ]]; then
        echo PASS
    else
        echo ERROR
        result=19
    fi
done

echo

exit $result
