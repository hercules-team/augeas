#! /bin/bash

# Test that we don't follow .augsave symlinks

ROOT=$abs_top_builddir/build/test-put-symlink-augsave
LENSES=$abs_top_srcdir/lenses

HOSTS=$ROOT/etc/hosts
HOSTS_AUGSAVE=${HOSTS}.augsave

ATTACK_FILE=$ROOT/other/attack

rm -rf $ROOT
mkdir -p $(dirname $HOSTS)
mkdir -p $(dirname $ATTACK_FILE)

cat <<EOF > $HOSTS
127.0.0.1 localhost
EOF
HOSTS_SUM=$(sum $HOSTS)

touch $ATTACK_FILE
(cd $(dirname $HOSTS) && ln -s ../other/attack $(basename $HOSTS).augsave)

# Now ask for the original to be saved in .augsave
augtool --nostdinc -I $LENSES -r $ROOT --backup > /dev/null <<EOF
set /files/etc/hosts/1/alias myhost
save
EOF

if [ ! -f $HOSTS ] ; then
    echo "/etc/hosts is no longer a regular file"
    exit 1
fi
if [ ! -f $HOSTS_AUGNEW ] ; then
    echo "/etc/hosts.augsave is still a symlink, should be unlinked"
    exit 1
fi

if [ ! "x${HOSTS_SUM}" = "x$(sum $HOSTS_AUGSAVE)" ]; then
    echo "/etc/hosts.augsave has changed from the original /etc/hosts"
    exit 1
fi
if ! grep myhost $HOSTS >/dev/null; then
    echo "/etc/hosts does not contain the modification"
    exit 1
fi

if [ -s $ATTACK_FILE ]; then
    echo "/other/attack now contains data, should be blank"
    exit 1
fi
