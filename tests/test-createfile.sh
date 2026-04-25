#!/bin/bash
#
# Test that augeas can create a file based on new paths added to the tree
# The tree should retain the paths on a subsequent load operation
#

root=${abs_top_builddir:-.}/build/test-createfile
lenses=${abs_top_srcdir:-.}/lenses

sysctl_file=/etc/sysctl.d/newfile1.conf

rm -rf $root
mkdir -p $(dirname $root/$sysctl_file)

expected_match="/files/etc/sysctl.d/newfile1.conf/net.ipv4.ip_nonlocal_bind = 1"
expected_content='net.ipv4.ip_nonlocal_bind = 1'

output=$(augtool --nostdinc -r $root -I $lenses <<EOF | grep "$expected_match"
set /files/etc/sysctl.d/newfile1.conf/net.ipv4.ip_nonlocal_bind 1
save
match /files/etc/sysctl.d/newfile1.conf/net.ipv4.ip_nonlocal_bind
EOF
)

if [[ ! -e $root/$sysctl_file ]]; then
  echo "Failed to create file $sysctl_file under $root"
  exit 1
elif ! diff -bq $root/$sysctl_file <(echo "$expected_content") 1>/dev/null 2>&1; then
  echo "Contents of $root/sysctl_file are incorrect"
  cat  $root/$sysctl_file
  echo '-- end of file --'
  echo "Expected:"
  echo "$expected_content"
  echo '-- end of file --'
  exit 1
elif [[ -z "$output" ]]; then
  echo "Missing /files/$sysctl_file in tree after save"
  exit 1
else
  echo "Successfully created $sysctl_file with content:"
  cat  $root/$sysctl_file
  echo '-- end of file --'
fi

sysctl_file=/etc/sysctl.d/newfile2.conf

expected_match="/files/etc/sysctl.d/newfile2.conf/net.ipv4.ip_forward = 1"
expected_content='net.ipv4.ip_forward = 1'

output=$(augtool --nostdinc -r $root -I $lenses <<EOF | grep "$expected_match"
set /files/etc/sysctl.d/newfile2.conf/net.ipv4.ip_forward 1
save
load
match /files/etc/sysctl.d/newfile2.conf/*
save
EOF
)

if [[ ! -e $root/$sysctl_file ]]; then
  echo "Failed to create file $sysctl_file under $root"
  exit 1
elif ! diff -bq $root/$sysctl_file <(echo "$expected_content") 1>/dev/null 2>&1; then
  echo "Contents of $root/sysctl_file are incorrect"
  cat  $root/$sysctl_file
  echo '-- end of file --'
  echo "Expected:"
  echo "$expected_content"
  echo '-- end of file --'
  exit 1
elif [[ -z "$output" ]]; then
  echo "Missing /files/$sysctl_file in tree after save"
  exit 1
else
  echo "Successfully created $sysctl_file with content:"
  cat  $root/$sysctl_file
  echo '-- end of file --'
fi
