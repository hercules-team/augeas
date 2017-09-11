#! /bin/bash

# Test that an attempt to save into a non-writable file does not leave a
# temporary file behind.
# See https://github.com/hercules-team/augeas/issues/479

if [ $UID -ne 0 -o "$(uname -s)" != "Linux" ]; then
    echo "Test can only be run as root on Linux as it uses chattr"
    exit 77
fi

root=$abs_top_builddir/build/test-nonwritable
hosts=$root/etc/hosts

rm -rf $root
mkdir -p $(dirname $hosts)

cat <<EOF > $hosts
127.0.0.1 localhost
EOF

chattr +i $hosts

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.2
save
EOF

chattr -i $hosts

if stat -t $hosts.* > /dev/null 2>&1
then
    echo "found a tempfile" $hosts.*
    exit 1
fi
