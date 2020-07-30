#!/bin/sh

# Test that saving changes that don't really change the underlying file
# leave the original file intact

root=$abs_top_builddir/build/test-idempotent
hosts=$root/etc/hosts

rm -rf $root
mkdir -p $(dirname $hosts)

cat <<EOF > $hosts
127.0.0.1 localhost
EOF
touch -r $0 $hosts
cp -p $hosts $hosts.stamp

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.1.1
set /files/etc/hosts/1/ipaddr 127.0.0.1
save
EOF

[ $hosts -nt $hosts.stamp ] && exit 1

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.1.1
save
set /files/etc/hosts/1/ipaddr 127.0.0.1
save
EOF

[ $hosts -nt $hosts.stamp ] || exit 1

# Test that --createifnomatch allows /*[expr]/ to create a node, and is idempotent
touch -r $0 $hosts

# Add a new ip+host to $hosts using --createifnomatch
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses --createifnomatch > /dev/null <<EOF
set /files/etc/hosts/*[ipaddr='8.8.8.8']/ipaddr 8.8.8.8
set /files/etc/hosts/*[ipaddr='8.8.8.8']/canonical dns1
save
EOF

[ $hosts -nt $hosts.stamp ] || exit 1
[ $(grep -E -c '8.8.8.8\s+dns1' $hosts) = 1 ] || exit 1

touch -r $0 $hosts

# Test that --createifnomatch is idempotent
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses --createifnomatch > /dev/null <<EOF
set /files/etc/hosts/*[ipaddr='8.8.8.8']/ipaddr 8.8.8.8
set /files/etc/hosts/*[ipaddr='8.8.8.8']/canonical dns1
save
EOF

[ $hosts -nt $hosts.stamp ] && exit 1

[ $(grep -E -c '8.8.8.8\s+dns1' $hosts) = 1 ] || exit 1
