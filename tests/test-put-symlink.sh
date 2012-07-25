#! /bin/bash

# Test that we correctly preserve symlinks when saving a file

ROOT=$abs_top_builddir/build/test-put-symlink
LENSES=$abs_top_srcdir/lenses
HOSTS=$ROOT/etc/hosts
REAL_HOSTS=$ROOT/other/hosts

rm -rf $ROOT
mkdir -p $(dirname $HOSTS)
mkdir -p $(dirname $REAL_HOSTS)

cat <<EOF > $REAL_HOSTS
127.0.0.1 localhost
EOF

(cd $(dirname $HOSTS) && ln -s ../other/hosts $(basename $HOSTS))

augtool --nostdinc -I $LENSES -b -r $ROOT > /dev/null <<EOF
set /files/etc/hosts/1/alias myhost
save
EOF

HOSTS_AUGSAVE=${HOSTS}.augsave
if [ ! -f $HOSTS_AUGSAVE ] ; then
    echo "Missing /etc/hosts.augsave"
    exit 1
fi
if [ -h $HOSTS_AUGSAVE ] ; then
    echo "The file /etc/hosts.augsave is a symlink"
    exit 1
fi
if [ ! -h $HOSTS ] ; then
    echo "/etc/hosts is not a symbolic link"
    exit 1
fi

LINK=$(readlink $HOSTS)
if [ "x$LINK" != "x../other/hosts" ] ; then
    echo "/etc/hosts does not link to ../other/hosts"
    exit 1
fi

if ! grep myhost $REAL_HOSTS >/dev/null; then
    echo "/other/hosts does not contain the modification"
    exit 1
fi
