#! /bin/bash

# Test the lua interpreter in augtool

ROOT=$abs_top_srcdir/tests/root
LENSES=$abs_top_srcdir/lenses

OUT=$(augtool --lua --nostdinc -I $LENSES -r $ROOT -f $abs_top_srcdir/tests/test.auglua 2>&1)

echo "$OUT"

[[ -z $OUT ]] || exit 1

