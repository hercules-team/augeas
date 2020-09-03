#!/bin/sh

# Test that changing the value of a node causes the function modified() to return true

root=$abs_top_builddir/build/test-function-modified
hosts=$root/etc/hosts

rm -rf $root
mkdir -p $(dirname $hosts)

cat <<EOF > $hosts
127.0.0.1 localhost
::1       localhost6
EOF

touch -r $0 $hosts

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF >$root/output.1
set /files/etc/hosts/1/ipaddr 127.0.0.4
match /files/etc/hosts//*[modified()]
EOF

[ "$(cat $root/output.1)" = '/files/etc/hosts/1/ipaddr = 127.0.0.4' ] || exit 1

# An empty value should return true, even if none of the child nodes are modified
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF >$root/output.2
set /files/etc/hosts/1 x
set /files/etc/hosts/1
match /files/etc/hosts//*[modified()]
EOF

[ "$(cat $root/output.2)" = '/files/etc/hosts/1 = (none)' ] || exit 1

# Test that changing a value does not change the parent node
# Test that adding a new node also marks the (new) parent node.
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF >$root/output.3
set /files/etc/hosts/1/ipaddr 127.1.1.1
set /files/etc/hosts/3/ipaddr 127.3.3.3
match /files/etc/hosts//*[modified()]
EOF

diff -q $root/output.3  -  <<EOF || exit 1
/files/etc/hosts/3 = (none)
/files/etc/hosts/1/ipaddr = 127.1.1.1
/files/etc/hosts/3/ipaddr = 127.3.3.3
EOF

