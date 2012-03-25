#! /bin/bash

# Test that we can write into a bind mount with the copy_if_rename_fails flag.
# This requires that EXDEV or EBUSY is returned from rename(2) to activate the
# code path, so set up a bind mount on Linux.

if [ $UID -ne 0 -o "$(uname -s)" != "Linux" ]; then
    echo "Test can only be run as root on Linux to create bind mounts"
    exit 77
fi

ROOT=$abs_top_builddir/build/test-put-mount
LENSES=$abs_top_srcdir/lenses

HOSTS=$ROOT/etc/hosts
TARGET=$ROOT/other/real_hosts

rm -rf $ROOT
mkdir -p $(dirname $HOSTS)
mkdir -p $(dirname $TARGET)

echo 127.0.0.1 localhost > $TARGET
touch $HOSTS

mount --bind $TARGET $HOSTS
Exit() {
    umount $HOSTS
    exit $1
}

HOSTS_SUM=$(sum $HOSTS)

augtool --nostdinc -I $LENSES -r $ROOT <<EOF
set /augeas/save/copy_if_rename_fails 1
set /files/etc/hosts/1/alias myhost
save
print /augeas//error
EOF

if [ ! "x${HOSTS_SUM}" != "x$(sum $HOSTS)" ]; then
    echo "/etc/hosts hasn't changed"
    Exit 1
fi

if [ ! "x${HOSTS_SUM}" != "x$(sum $TARGET)" ]; then
    echo "/other/real_hosts hasn't changed"
    Exit 1
fi

if ! grep myhost $TARGET >/dev/null; then
    echo "/other/real_hosts does not contain the modification"
    Exit 1
fi

Exit 0
