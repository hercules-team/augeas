#! /bin/sh

TOP_DIR=$(cd $(dirname $0)/.. && pwd)
TOP_BUILDDIR="$abs_top_builddir"
[ -z "$TOP_BUILDDIR" ] && TOP_BUILDDIR="$TOP_DIR"
TOP_SRCDIR="$abs_top_srcdir"
[ -z "$TOP_SRCDIR" ] && TOP_SRCDIR="$TOP_DIR"

TEST_DIR="$TOP_SRCDIR/tests"

export PATH="$TOP_BUILDDIR/src:${PATH}"

export AUGEAS_ROOT="$TOP_BUILDDIR/build/test-augtool"
export AUGEAS_LENS_LIB="$TOP_SRCDIR/lenses"

fail() {
    [ -z "$failed" ] && echo FAIL
    failed=yes
    echo "$@"
    result=1
}

# Without args, run all tests
if [ $# -eq 0 ] ; then
    args="$TEST_DIR/test-augtool/*.sh"
else
    args="$@"
fi

result=0

for tst in $args; do
    unset failed

    printf "%-40s ... " $(basename $tst .sh)

    # Read in test variables. The variables we understand are
    # echo              - echo augtool commands if set to some value
    # commands          - the commands to send to augtool
    # lens              - the lens to use
    # file              - the file that should be changed
    # diff              - the expected diff
    # refresh           - print diff in a form suitable for cut and paste
    #                     into the test file if set to some value

    unset echo commands lens file diff refresh
    . $tst

    # Setup test root from root/
    [ -d "$AUGEAS_ROOT" ] && rm -rf "$AUGEAS_ROOT"
    dest_dir="$AUGEAS_ROOT"$(dirname $file)
    mkdir -p $dest_dir
    cp -p "$TEST_DIR"/root/$file $dest_dir

    [ -n "$echo" ] && echo="-e"

    commands="set /augeas/load/Test/lens $lens
set /augeas/load/Test/incl $file
load
$commands
save
quit"
    echo "$commands" | augtool $echo --nostdinc --noautoload -n || fail "augtool failed"

    abs_file="$AUGEAS_ROOT$file"
    if [ ! -f "${abs_file}.augnew" ]; then
        fail "Expected file $file.augnew"
    else
        act=$(diff -u "$abs_file" "${abs_file}.augnew" \
            | sed -r -e "s/^ $//;s!^(---|\+\+\+) ${AUGEAS_ROOT}($file(\.augnew)?)(.*)\$!\1 \2!;s/\\t/\\\\t/g")

        if [ "$act" != "$diff" ] ; then
            fail "$act"
        fi
    fi
    other_files=$(find "$AUGEAS_ROOT" -name \*.augnew -not -path "$abs_file.augnew")
    [ -n "$other_files" ] && fail "Unexpected file(s) $other_files"
    [ -z "$failed" ] && echo OK
done

exit $result
