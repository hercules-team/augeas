#! /bin/bash

# Test for bug https://fedorahosted.org/augeas/ticket/1
#
# Check that putting an invalid node into the tree and saving
# leads to failure, and therefore the original file being preserved

root=$abs_top_builddir/build/test-bug-1
file=$root/etc/logrotate.d/test

rm -rf $root
mkdir -p $(dirname $file)

cat > $file <<EOF
/myfile {
  size=5M
}
EOF
ln $file $file.orig

augtool --nostdinc -I $abs_top_srcdir/lenses -r $root > /dev/null <<EOF
ins invalid before /files/etc/logrotate.d/rpm/rule[1]
save
EOF

result=$?

if [ $result -eq 0 ] ; then
    echo "augtool succeeded, but should have failed"
    exit 1
fi

if [ ! $file -ef $file.orig ] ; then
    echo "File was changed, but should not have been"
    exit 1
fi
