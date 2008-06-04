#! /bin/sh

set -e

VERBOSE=n
if [ "x$1" = "x-v" ]; then
    VERBOSE=y
fi

TOPDIR=$(cd $(dirname $0)/.. && pwd)
[ -n "$abs_top_srcdir" ] || top_srcdir=$TOPDIR


LENS_DIR=$abs_top_srcdir/lenses
TESTS=$LENS_DIR/tests/test_*.aug

LOG=$(mktemp /tmp/test-lenses.XXXXXX)
trap 'rm "$LOG"' EXIT

for t in $TESTS
do
  printf "%-30s ... " $(basename "$t" .aug)
  set +e
  augparse -I "$LENS_DIR" "$t" > "$LOG" 2>&1
  ret=$?
  set -e
  if [ ! $ret -eq 0 ]; then
    echo FAIL
    result=1
  elif [ $ret -eq 0 ]; then
    echo PASS
  fi
  if [ "$VERBOSE" = "y" ] ; then
     cat "$LOG"
  fi
done
