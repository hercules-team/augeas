#! /bin/bash

# Test that we don't follow symlinks when writing to .augnew

ROOT=$abs_top_builddir/build/test-put-symlink-augnew
LENSES=$abs_top_srcdir/lenses

HOSTS=$ROOT/etc/hosts
HOSTS_AUGNEW=${HOSTS}.augnew

ATTACK_FILE=$ROOT/other/attack

rm -rf $ROOT
mkdir -p $(dirname $HOSTS)
mkdir -p $(dirname $ATTACK_FILE)

cat <<EOF > $HOSTS
127.0.0.1 localhost
EOF
touch $ATTACK_FILE

(cd $(dirname $HOSTS) && ln -s ../other/attack $(basename $HOSTS).augnew)

HOSTS_SUM=$(sum $HOSTS)

augtool --nostdinc -I $LENSES -r $ROOT --new > /dev/null <<EOF
set /files/etc/hosts/1/alias myhost
save
EOF

if [ ! -f $HOSTS ] ; then
    echo "/etc/hosts is no longer a regular file"
    exit 1
fi
if [ ! "x${HOSTS_SUM}" = "x$(sum $HOSTS)" ]; then
    echo "/etc/hosts has changed"
    exit 1
fi

if [ ! -f $HOSTS_AUGNEW ] ; then
    echo "/etc/hosts.augnew is still a symlink, should be unlinked"
    exit 1
fi
if ! grep myhost $HOSTS_AUGNEW >/dev/null; then
    echo "/etc/hosts does not contain the modification"
    exit 1
fi

if [ -s $ATTACK_FILE ]; then
    echo "/other/attack now contains data, should be blank"
    exit 1
fi
