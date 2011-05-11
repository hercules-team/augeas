#! /bin/bash

# Run test modules that make sure the interpreter fails/succeeds in
# various hairy situations.
#
# If run with option '-v', error output from each augparse run is printed

TOPDIR=$(cd $(dirname $0)/.. && pwd)
DATADIR=${abs_top_srcdir-${TOPDIR}}/tests
MODULES=${DATADIR}/modules
AUGPARSE=${abs_top_builddir-${DATADIR}/..}/src/augparse

set -e

VERBOSE=n
if [ "x$1" = "x-v" ]; then
    VERBOSE=y
fi

run_tests_plain ()
{
    ret_succ=$1
    ret_fail=$(( 1 - $ret_succ ))
    action=$2
    shift
    shift

    for g in $*; do
        if [ ! -r "$g" ]; then
            echo "Grammar file $g is not readable"
            exit 19
        fi
        printf "$action %-30s ... " $(basename $g .aug)
        set +e
        errs=$(augparse --nostdinc -I ${MODULES} $g 2>&1 > /dev/null)
        ret=$?
        set -e
        if [ $ret -eq $ret_fail ]; then
            echo FAIL
            result=1
        elif [ $ret -eq $ret_succ ]; then
            echo PASS
        else
            echo ERROR
            result=19
        fi
        if [ "$VERBOSE" = "y" ] ; then
            echo $errs
        fi
    done
}

run_tests_valgrind ()
{
    ret_succ=$1
    ret_fail=$(( 1 - $ret_succ ))
    action=$2
    shift
    shift

    for g in $*; do
        if [ ! -r "$g" ]; then
            echo "Grammar file $g is not readable"
            exit 19
        fi
        echo "========================================================="
        printf "$action %-30s \n" $(basename $g .aug)
        set +e
        $VALGRIND $AUGPARSE --nostdinc -I ${MODULES} $g
        ret=$?
        set -e
        if [ $ret -eq $ret_fail ]; then
            echo FAIL
            result=1
        elif [ $ret -eq $ret_succ ]; then
            echo PASS
        else
            echo ERROR
            result=19
        fi
        if [ "$VERBOSE" = "y" ] ; then
            echo $errs
        fi
        echo "---------------------------------------------------------"
    done
}

if [ ! -x $AUGPARSE ] ; then
    echo "Failed to find executable augparse"
    echo "  looked for $AUGPARSE"
    exit 127
fi

echo "--------------------------------"
echo "Running interpreter tests"
echo

result=0
[ -z "$VALGRIND" ] && flavor=plain || flavor=valgrind

run_tests_$flavor 1 reject $MODULES/fail_*.aug
run_tests_$flavor 0 accept $MODULES/pass_*.aug

exit $result
