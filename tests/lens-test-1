#! /bin/sh
# Run one lens test.
# Derive names of inputs from the name of this script.

[ -n "$abs_top_srcdir" ] || abs_top_srcdir=$TOPDIR
LENS_DIR=$abs_top_srcdir/lenses

me=`echo "$0"|sed 's,.*/lens-\(.*\)\.sh$,\1,'`

t=$LENS_DIR/tests/test_$me.aug

if [ -n "$VALGRIND" ] ; then
  exec $VALGRIND $AUGPARSE --nostdinc -I "$LENS_DIR" "$t"
else
  exec augparse --nostdinc -I "$LENS_DIR" "$t"
fi
