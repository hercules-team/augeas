#! /bin/bash

# Check that reading the files in tests/root/ with augtool does not lead to
# any errors

TOPDIR=$(cd $(dirname $0)/.. && pwd)
[[ -n "$top_builddir" ]] || top_builddir=$TOPDIR
[[ -n "$top_srcdir" ]] || top_srcdir=$TOPDIR

export AUGEAS_LENS_LIB=${top_srcdir}/lenses
export AUGEAS_ROOT=${top_srcdir}/tests/root

augtool print '/augeas/files' | grep -q /error && ret=1 || ret=0
exit $ret
