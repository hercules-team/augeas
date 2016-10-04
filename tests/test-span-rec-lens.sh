# This test checks that https://github.com/hercules-team/augeas/issues/397 is
# fixed. It would otherwise lead to a segfault in augtool

if [ -z "$abs_top_builddir" ]; then
    echo "abs_top_builddir is not set"
    exit 1
fi

if [ -z "$abs_top_srcdir" ]; then
    echo "abs_top_srcdir is not set"
    exit 1
fi

ROOT=$abs_top_builddir/build/test-span-rec-lens
LENSES=$abs_top_srcdir/lenses

FILE=$ROOT/etc/default/im-config

rm -rf $ROOT
mkdir -p $(dirname $FILE)
cat <<EOF > $FILE
if [ 1 ]; then
# K
else
# I
fi
EOF

# If bug 397 is not fixed, this will abort because of memory corruption
augtool --nostdinc -I $LENSES -r $ROOT --span rm /files >/dev/null
