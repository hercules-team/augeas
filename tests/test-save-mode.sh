#! /bin/bash

# Test manipulating the save flags in /augeas/save

root=$abs_top_builddir/build/test-save-mode
hosts=$root/etc/hosts
augopts="--nostdinc -r $root -I $abs_top_srcdir/lenses"

run_augtool() {
    exp=$1
    shift
    augtool $augopts "$@" > /dev/null
    status=$?
    if [ "x$exp" = ok -a $status -ne 0 ] ; then
        echo "augtool failed"
        exit 1
    elif [ "x$exp" = fail -a $status -eq 0 ] ; then
        echo "augtool succeeded but should have failed"
        exit 1
    fi
}

assert_ipaddr() {
    exp="/files/etc/hosts/1/ipaddr = $1"
    act=$(augtool $augopts get /files/etc/hosts/1/ipaddr)

    if [ "$act" != "$exp" ] ; then
        printf "Expected: %s\n" "$exp"
        printf "Actual  : %s\n" "$act"
        exit 1
    fi
}

assert_file_exists() {
    if [ ! -f "$1" ] ; then
        echo "File $1 does not exist, but should"
        exit 1
    fi
}

assert_file_exists_not() {
    if [ -f "$1" ] ; then
        echo "File $1 exists, but should not"
        exit 1
    fi
}

setup() {
#    echo $*
    rm -rf $root
    mkdir -p $(dirname $hosts)
    cat > $hosts <<EOF
127.0.0.1 localhost
EOF
}

setup "No /augeas/save"
run_augtool fail <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
rm /augeas/save
save
EOF
assert_ipaddr 127.0.0.1

setup "Invalid /augeas/save"
run_augtool fail <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
set /augeas/save "not a valid flag"
save
EOF
assert_ipaddr 127.0.0.1

setup "noop"
run_augtool fail <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
set /augeas/save noop
save
EOF
assert_ipaddr 127.0.0.1
assert_file_exists_not $hosts.augnew
assert_file_exists_not $hosts.augsave

setup "newfile"
run_augtool ok <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
set /augeas/save newfile
save
EOF
assert_ipaddr 127.0.0.1
assert_file_exists $hosts.augnew
assert_file_exists_not $hosts.augsave

setup "overwrite"
run_augtool ok <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
set /augeas/save overwrite
save
EOF
assert_ipaddr 127.0.0.2
assert_file_exists_not $hosts.augnew
assert_file_exists_not $hosts.augsave

setup "backup"
run_augtool ok <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
set /augeas/save backup
save
EOF
assert_ipaddr 127.0.0.2
assert_file_exists_not $hosts.augnew
assert_file_exists $hosts.augsave


augopts="${augopts} --autosave"

setup "autosave"
run_augtool ok <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
EOF
assert_ipaddr 127.0.0.2
assert_file_exists_not $hosts.augnew
assert_file_exists_not $hosts.augsave

setup "autosave command line"
run_augtool ok set /files/etc/hosts/1/ipaddr 127.0.0.2
assert_ipaddr 127.0.0.2
assert_file_exists_not $hosts.augnew
assert_file_exists_not $hosts.augsave
