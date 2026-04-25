#!/bin/bash

# Test that augprint produces the expected output, and that the output
# can be used to reconstruct the original file


test_augprint_files=$abs_top_srcdir/tests/test-augprint

export AUGEAS_ROOT=$abs_top_builddir/build/test-augprint
export AUGEAS_LENS_LIB=$abs_top_srcdir/lenses

# ------------- /etc/hosts ------------
mkdir -p $AUGEAS_ROOT/etc
cp --no-preserve=mode --recursive $test_augprint_files/etc/ $AUGEAS_ROOT/

output=$AUGEAS_ROOT/etc.hosts.augprint
AUGEAS_ROOT=$test_augprint_files augprint --pretty --verbose /etc/hosts > $output
augtool --load-file=/etc/hosts -f  $output --autosave 1>/dev/null
# Check that output matches expected output
diff -bu    $test_augprint_files/etc.hosts.pretty.augprint $output || exit 1
# Check that file is unchanged
diff -bu -B $test_augprint_files/etc/hosts $AUGEAS_ROOT/etc/hosts || exit 1

AUGEAS_ROOT=$test_augprint_files augprint --regexp=2 /etc/hosts > $output
augtool --load-file=/etc/hosts -f  $output --autosave 1>/dev/null
# Check that output matches expected output
diff -bu    $test_augprint_files/etc.hosts.regexp.augprint $output || exit 1
# Check that file is unchanged
diff -bu -B $test_augprint_files/etc/hosts $AUGEAS_ROOT/etc/hosts || exit 1

# ------------- /etc/squid/squid.conf and /etc/pam,d/systemd-auth  ------------
for test_file in /etc/squid/squid.conf /etc/pam.d/system-auth ; do
	mkdir -p $(dirname $AUGEAS_ROOT/$test_file)
  output=${test_file//\//.}
  output=${AUGEAS_ROOT}/${output#.}.augprint
	AUGEAS_ROOT=$test_augprint_files augprint $test_file > $output
	augtool --load-file=$test_file -f $output --autosave 1>/dev/null
	# Check that file is unchanged
	diff -bu -B $test_augprint_files/$test_file $AUGEAS_ROOT/$test_file || exit 1
done

