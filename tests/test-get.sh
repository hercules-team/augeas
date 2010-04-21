#! /bin/bash

# Check that reading the files in tests/root/ with augtool does not lead to
# any errors

TOPDIR=$(cd $(dirname $0)/.. && pwd)
[ -n "$abs_top_srcdir" ] || abs_top_srcdir=$TOPDIR

export AUGEAS_LENS_LIB=$abs_top_srcdir/lenses
export AUGEAS_ROOT=$abs_top_srcdir/tests/root

errors=$(augtool --nostdinc match '/augeas//error/descendant-or-self::*')

if [ "x$errors" != "x  (no matches)" ] ; then
    printf "get /augeas//error reported errors:\n%s\n" "$errors"
    exit 1
fi
