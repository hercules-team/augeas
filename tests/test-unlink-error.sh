#! /bin/bash

# Make sure we don't delete files simply because there was an error reading
# them in

root=$abs_top_builddir/build/test-unlink-error
xinetd=$root/etc/xinetd.conf

rm -rf $root
mkdir -p $(dirname $xinetd)

cat > $xinetd <<EOF
intentional garbage
EOF

augtool --nostdinc -r $root -I $abs_top_srcdir/lenses > /dev/null <<EOF
clear /files
save
EOF

if [ ! -f $xinetd ] ; then
  echo "Deleted xinetd.conf"
  exit 1
fi
