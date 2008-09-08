#! /bin/bash

# Run the tests in lenses/tests through valgrind and report which ones leak
set -e

TOPDIR=$(cd $(dirname $0)/.. && pwd)
[[ -n "$top_builddir" ]] || top_builddir=$TOPDIR
[[ -n "$top_srcdir" ]] || top_srcdir=$TOPDIR


AUGPARSE="libtool --mode=execute valgrind -q --leak-check=full ${top_builddir}/src/augparse"
LENS_DIR=${top_srcdir}/lenses
TESTS=$LENS_DIR/tests/test_*.aug

for t in $TESTS
do
  echo Run $(basename $t .aug)
  set +e
  ${AUGPARSE} --nostdinc -I $LENS_DIR $t
done
