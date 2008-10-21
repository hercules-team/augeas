#! /bin/bash

# Check that saving preserves mode and ownership; for this test to make
# much sense (if any) the user running it should have at least one
# supplementary group

run_augtool() {
augtool --nostdinc -r $root -I $abs_top_srcdir/lenses <<EOF
set /files/etc/hosts/1/ipaddr 127.0.1.1
set /files/etc/grub.conf/default 3
set /files/etc/inittab/1/action fake
save
match /augeas/events/saved
EOF
}

root=$abs_top_builddir/build/test-events-saved

rm -rf $root
mkdir -p $root/etc

for f in hosts grub.conf inittab; do
  cp -p $abs_top_srcdir/tests/root/etc/$f $root/etc
done

saved=$(run_augtool | grep ^/augeas/events/saved | cut -d ' ' -f 3 | sort | tr '\n' ' ')
exp="/files/etc/grub.conf /files/etc/hosts /files/etc/inittab "

if [ "$saved" != "$exp" ]
then
    echo "Unexpected entries in /augeas/events/saved:"
    echo "Expected: \"$exp\""
    echo "Actual:   \"$saved\""
    exit 1
fi
