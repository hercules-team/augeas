#! /bin/bash

# Check that saving preserves mode and ownership; for this test to make
# much sense (if any) the user running it should have at least one
# supplementary group

root=$abs_top_builddir/build/preserve
hosts=$root/etc/hosts

rm -rf $root
mkdir -p $(dirname $hosts)

echo -e '127.0.0.1\tlocalhost' > $hosts

chmod 0600 $hosts
group=$(groups | tr ' ' '\n' | tail -n 1)
chgrp $group $hosts

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
set /files/etc/hosts/1/alias alias.example.com
save
EOF
if [ $? != 0 ] ; then
    echo "augtool failed on existing file"
    exit 1
fi

act_group=$(ls -l $hosts | cut -d ' ' -f 4)
act_mode=$(ls -l $hosts | cut -d ' ' -f 1)

if [ "x$group" != "x$act_group" ] ; then
    echo "Expected group $group but got $act_group"
    exit 1
fi

if [ x-rw------- != "x$act_mode" ] ; then
    echo "Expected mode 0600 but got $act_mode"
    exit 1
fi

# Check that we create new files without error
rm -rf $root
mkdir -p $(dirname $hosts)
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.1
set /files/etc/hosts/1/canonical host.example.com
save
EOF
if [ $? != 0 ] ; then
    echo "augtool failed on new file"
    exit 1
fi
