#!/bin/sh

# Tests for augmatch

TOPDIR=$(cd $(dirname $0)/.. && pwd)
[ -n "$abs_top_srcdir" ] || abs_top_srcdir=$TOPDIR

export AUGEAS_LENS_LIB=$abs_top_srcdir/lenses
export AUGEAS_ROOT=$abs_top_srcdir/tests/root

fail() {
    echo "failed: $*"
    exit 1
}

assert_eq() {
    if [ "$1" != "$2" ]; then
        shift 2
        fail $*
    fi
}

# print the tree for /etc/exports
act=$(augmatch /etc/exports)
assert_eq 23 $(echo "$act" | wc -l) "t1: expected 23 lines of output"

# show only the entry for a specific mount
act=$(augmatch -m 'dir["/home"]' /etc/exports)
assert_eq 9 $(echo "$act" | wc -l) "t2: expected 9 lines of output"

# show all the clients to which we are exporting /home
act=$(augmatch -eom 'dir["/home"]/client' /etc/exports)
exp=$(printf "207.46.0.0/16\n192.168.50.2/32\n")
assert_eq "$exp" "$act" "t3: expected '$exp'"

# report errors with exit code 2
augmatch -m '**' /etc/exports >/dev/null 2>&1
ret=$?
assert_eq 2 $ret "t4: expected exit code 2 but got $ret"

augmatch /etc >/dev/null 2>&1
ret=$?
assert_eq 2 $ret "t5: expected exit code 2 but got $ret"

# test --quiet
act=$(augmatch -q -m "dir['/local']" /etc/exports)
ret=$?
assert_eq '' "$act" "t6: expected no output"
assert_eq 0 $ret "t6: expected exit code 0 but got $ret"

act=$(augmatch -q -m "dir['/not_there']" /etc/exports)
ret=$?
assert_eq '' "$act" "t7: expected no output"
assert_eq 1 $ret "t7: expected exit code 1 but got $ret"
