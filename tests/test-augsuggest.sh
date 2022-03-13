#!/bin/bash

# Test that augsuggest produces the expected output, and that the output
# can be used to reconstruct the original file

root=$abs_top_builddir/build/test-augsuggest
augeas_root=$abs_top_builddir/tests/test-augsuggest

export AUGEAS_LENS_LIB=$abs_top_builddir/lenses
# ------------- /etc/hosts ------------
mkdir -p $root/etc
AUGEAS_ROOT=$augeas_root augsuggest --pretty --verbose /etc/hosts > $root/etc.hosts.augtool
AUGEAS_ROOT=$root augtool --load-file=/etc/hosts -f  $root/etc.hosts.augtool --noload --autosave 1>/dev/null
diff -bu    $augeas_root/etc.hosts.pretty.augtool $root/etc.hosts.augtool || exit 1
diff -bu -B $augeas_root/etc/hosts $root/etc/hosts || exit 1

AUGEAS_ROOT=$augeas_root augsuggest --regexp=2 /etc/hosts > $root/etc.hosts.augtool
AUGEAS_ROOT=$root augtool --load-file=/etc/hosts -f  $root/etc.hosts.augtool --noload --autosave 1>/dev/null
diff -bu    $augeas_root/etc.hosts.regexp.augtool $root/etc.hosts.augtool || exit 1
diff -bu -B $augeas_root/etc/hosts $root/etc/hosts || exit 1

# ------------- /etc/squid/squid.conf and /etc/pam,d/systemd-auth  ------------
for test_file in /etc/squid/squid.conf /etc/pam.d/system-auth ; do
	mkdir -p $(dirname $root/$test_file)
  output=${test_file//\//.}
  output=${output#.}
	AUGEAS_ROOT=$augeas_root augsuggest $test_file > $root/${output}.augtool
	AUGEAS_ROOT=$root augtool --load-file=$test_file -f  $root/${output}.augtool --noload --autosave 1>/dev/null
	diff -bu -B $augeas_root/$test_file $root/$test_file || exit 1
done

