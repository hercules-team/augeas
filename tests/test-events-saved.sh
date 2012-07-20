#! /bin/bash

# Check that saving preserves mode and ownership; for this test to make
# much sense (if any) the user running it should have at least one
# supplementary group

run_augtool() {
savemode=$1
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses <<EOF
set /augeas/save $savemode
set /files/etc/hosts/1/ipaddr 127.0.1.1
set /files/boot/grub/menu.lst/default 3
set /files/etc/inittab/1/action fake
rm /files/etc/puppet/puppet.conf
set /files/etc/yum.repos.d/fedora.repo/fedora/enabled 0
save
match /augeas/events/saved
EOF
}

root=$abs_top_builddir/build/test-events-saved

for savemode in overwrite backup newfile noop; do
    rm -rf $root
    mkdir -p $root
    cp -pr $abs_top_srcdir/tests/root/* $root
    chmod -R u+w $root

    saved=$(run_augtool $savemode | grep ^/augeas/events/saved | cut -d ' ' -f 3 | sort | tr '\n' ' ')
    exp="/files/boot/grub/menu.lst /files/etc/hosts /files/etc/inittab /files/etc/puppet/puppet.conf /files/etc/yum.repos.d/fedora.repo "

    if [ -f "$root/etc/puppet/puppet.conf" -a "$savemode" != "noop" ]
    then
        echo "Save mode: $savemode"
        echo "File /etc/puppet/puppet.conf should have been deleted"
        exit 1
    fi

    if [ "$saved" != "$exp" ]
    then
        echo "Unexpected entries in /augeas/events/saved:"
        echo "Expected: \"$exp\""
        echo "Actual:   \"$saved\""
        echo "Save mode: $savemode"
        exit 1
    fi
done
