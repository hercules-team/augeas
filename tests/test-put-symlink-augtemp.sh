#! /bin/bash

# Test that we don't follow .augnew symlinks (regression test)

ROOT=$abs_top_builddir/build/test-put-symlink-augtemp
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

# Test the normal save code path which would use a temp augnew file
augtool --nostdinc -I $LENSES -r $ROOT > /dev/null <<EOF
set /files/etc/hosts/1/alias myhost1
save
EOF

if [ -h $HOSTS ] ; then
    echo "/etc/hosts is now a symlink, pointing to" $(readlink $HOSTS)
    exit 1
fi
if ! grep myhost1 $HOSTS >/dev/null; then
    echo "/etc/hosts does not contain the modification"
    exit 1
fi

if [ ! -h $HOSTS_AUGNEW ] ; then
    echo "/etc/hosts.augnew is not a symbolic link"
    exit 1
fi
LINK=$(readlink $HOSTS_AUGNEW)
if [ "x$LINK" != "x../other/attack" ] ; then
    echo "/etc/hosts.augnew no longer links to ../other/attack"
    exit 1
fi

if [ -s $ATTACK_FILE ]; then
    echo "/other/attack now contains data, should be blank"
    exit 1
fi
