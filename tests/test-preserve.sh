#! /bin/bash

# Check that saving preserves mode and ownership; for this test to make
# much sense (if any) the user running it should have at least one
# supplementary group

root=$abs_top_builddir/build/preserve
hosts=$root/etc/hosts

init_dirs() {
rm -rf $root
mkdir -p $(dirname $hosts)
}

stat_inode() {
ls -il $1 | awk '{ print $1 }'
}

AUGTOOL="augtool --nostdinc -r $root -I $abs_top_srcdir/lenses"
init_dirs

printf '127.0.0.1\tlocalhost\n' > $hosts

chmod 0600 $hosts
group=$(groups | tr ' ' '\n' | tail -1)
chgrp $group $hosts

[ -x /usr/bin/chcon ] && selinux=yes || selinux=no
if [ $selinux = yes ] ; then
  /usr/bin/chcon -t etc_t $hosts > /dev/null 2>/dev/null || selinux=no
fi

$AUGTOOL >/dev/null <<EOF
set /files/etc/hosts/1/alias alias.example.com
save
EOF
if [ $? != 0 ] ; then
    echo "augtool failed on existing file"
    exit 1
fi

act_group=$(ls -l $hosts | sed -e 's/  */ /g' | cut -d ' ' -f 4)
act_mode=$(ls -l $hosts | cut -b 1-10)
if [ $selinux = yes ] ; then
  act_con=$(stat --format=%C $hosts | cut -d ':' -f 3)
fi
if [ "x$group" != "x$act_group" ] ; then
    echo "Expected group $group but got $act_group"
    exit 1
fi

if [ x-rw------- != "x$act_mode" ] ; then
    echo "Expected mode 0600 but got $act_mode"
    exit 1
fi

if [ $selinux = yes -a xetc_t != "x$act_con" ] ; then
    echo "Expected SELinux type etc_t but got $act_con"
    exit 1
fi

# Check that we create new files without error and with permissions implied
# from the umask
init_dirs

oldumask=$(umask)
umask 0002
$AUGTOOL > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.1
set /files/etc/hosts/1/canonical host.example.com
save
EOF
if [ $? != 0 ] ; then
    echo "augtool failed on new file"
    exit 1
fi
if [ ! -e $hosts ]; then
    echo "augtool didn't create new /etc/hosts file"
    exit 1
fi
act_mode=$(ls -l $hosts | cut -b 1-10)
if [ x-rw-rw-r-- != "x$act_mode" ] ; then
    echo "Expected mode 0664 due to $(umask) umask but got $act_mode"
    exit 1
fi
umask $oldumask

# Check that we create new files without error when backups are requested
init_dirs

$AUGTOOL -b > /dev/null <<EOF
set /files/etc/hosts/1/ipaddr 127.0.0.1
set /files/etc/hosts/1/canonical host.example.com
save
EOF
if [ $? != 0 ] ; then
    echo "augtool -b failed on new file"
    exit 1
fi

# Check that we preserve a backup file on request
printf '127.0.0.1\tlocalhost\n' > $hosts
exp_inode=$(stat_inode $hosts)

$AUGTOOL -b > /dev/null <<EOF
set /files/etc/hosts/1/alias alias.example.com
print /augeas/save
save
EOF

if [ ! -f $hosts.augsave ] ; then
  echo "Backup file was not created"
  exit 1
fi

act_inode=$(stat_inode $hosts.augsave)
if [ "x$act_inode" != "x$exp_inode" ] ; then
  echo "Backup file's inode changed"
  exit 1
fi

act_inode=$(stat_inode $hosts)
if [ "x$act_inode" = "x$exp_inode" ] ; then
  echo "Same inode for backup file and main file"
  exit 1
fi
