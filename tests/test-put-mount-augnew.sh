#! /bin/bash

# Test that we can write into a bind mount placed at PATH.augnew with the
# copy_if_rename_fails flag.
# This requires that EXDEV or EBUSY is returned from rename(2) to activate the
# code path, so set up a bind mount on Linux.

if [ $UID -ne 0 -o "$(uname -s)" != "Linux" ]; then
    echo "Test can only be run as root on Linux to create bind mounts"
    exit 77
fi

ROOT=$abs_top_builddir/build/test-put-mount-augnew
LENSES=$abs_top_srcdir/lenses

HOSTS=$ROOT/etc/hosts
HOSTS_AUGNEW=${HOSTS}.augnew
TARGET=$ROOT/other/real_hosts

rm -rf $ROOT
mkdir -p $(dirname $HOSTS)
mkdir -p $(dirname $TARGET)

echo 127.0.0.1 localhost > $HOSTS
touch $TARGET $HOSTS_AUGNEW

mount --bind $TARGET $HOSTS_AUGNEW
Exit() {
    umount $HOSTS_AUGNEW
    exit $1
}

HOSTS_SUM=$(sum $HOSTS)

augtool --nostdinc -I $LENSES -r $ROOT --new <<EOF
set /augeas/save/copy_if_rename_fails 1
set /files/etc/hosts/1/alias myhost
save
print /augeas//error
EOF

if [ ! -f $HOSTS ] ; then
    echo "/etc/hosts is no longer a regular file"
    Exit 1
fi
if [ ! "x${HOSTS_SUM}" = "x$(sum $HOSTS)" ]; then
    echo "/etc/hosts has changed"
    Exit 1
fi
if [ ! "x${HOSTS_SUM}" = "x$(sum $HOSTS)" ]; then
    echo "/etc/hosts has changed"
    Exit 1
fi

if [ ! -s $HOSTS_AUGNEW ]; then
    echo "/etc/hosts.augnew is empty"
    Exit 1
fi
if [ ! -s $TARGET ]; then
    echo "/other/real_hosts is empty"
    Exit 1
fi

if ! grep myhost $TARGET >/dev/null; then
    echo "/other/real_hosts does not contain the modification"
    Exit 1
fi

Exit 0
