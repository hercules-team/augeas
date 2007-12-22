#! /bin/bash

DATADIR=$(dirname $0)
GRAMMARS=${DATADIR}/grammars
AUGPARSE=${top_builddir-${DATADIR}/..}/src/augparse

function run_grammars {
    ret_succ=$1
    ret_fail=$(( 1 - $ret_succ ))
    action=$2
    shift
    shift

    for g in $*; do
        if [[ ! -r "$g" ]]; then
            echo "Grammar file $g is not readable"
            exit 19
        fi
        printf "$action %-30s ... " $(basename $g)
        ${AUGPARSE} $g > /dev/null 2>&1
        ret=$?
        if [[ $ret -eq $ret_fail ]]; then
            echo FAIL
            result=1
        elif [[ $ret -eq $ret_succ ]]; then
            echo PASS
        else
            echo ERROR
            result=19
        fi
    done
}

if [ ! -x $AUGPARSE ] ; then
    echo "Failed to find executable augparse"
    echo "  looked for $AUGPARSE"
    exit 127
fi

echo "--------------------------------"
echo "Checking grammars"
echo

result=0
run_grammars 1 reject $GRAMMARS/reject/*.aug
run_grammars 0 accept $GRAMMARS/accept/*.aug

exit $result
