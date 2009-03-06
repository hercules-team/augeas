#! /bin/bash

aug_mv() {
opts='--nostdinc -r /dev/null'
(augtool $opts | grep -v '/augeas\|/files\|augtool' | tr '\n' ' ') <<EOF
set /a/b/c value
mv $1 $2
print
EOF
}

assert_eq() {
    msg=$1
    if [ "$ACT" != "$EXP" ] ; then
        echo "Failed: aug_mv $msg"
        echo "Expected: <$EXP>"
        echo "Actual  : <$ACT>"
        exit 1
    fi

}

ACT=$(aug_mv /a/b/c /x)
EXP='/a /a/b /x = "value" '
assert_eq /x

ACT=$(aug_mv /a/b/c /x/y)
EXP='/a /a/b /x /x/y = "value" '
assert_eq /x/y

ACT=$(aug_mv /a/b/c /a/x)
EXP='/a /a/b /a/x = "value" '
assert_eq /a/x

# Check that we don't move into a descendant
ACT=$(aug_mv /a/b/c /a/b/c/d)
EXP='Failed /a /a/b /a/b/c = "value" /a/b/c/d '
assert_eq /a/b/c/d

ACT=$(aug_mv /a/b/c /a/b/d)
EXP='/a /a/b /a/b/d = "value" '
assert_eq /a/b/d

ACT=$(aug_mv /a /x/y)
EXP='/x /x/y /x/y/b /x/y/b/c = "value" '
assert_eq "/a to /x/y"
