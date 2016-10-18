#!/usr/bin/env bash

set -e
set -o pipefail

start_fold_marker() {
  echo "travis_fold:start:script.$1"
}

end_fold_marker() {
  echo "travis_fold:end:script.$1"
}

print_error_log() {
  file=$1
  maximuxsize=100
  actualsize=$(du -k "$file" | cut -f 1)

  if [ $actualsize -ge $maximuxsize ]; then
    echo "Error log file '$file' is bigger than 100kb. Please inspect test failures locally."
  else
    cat $file
  fi
}

start_fold_marker 'configure'
./configure --enable-debug=yes 2>&1 | colout2 -t configure
end_fold_marker 'configure'

start_fold_marker 'compilation'
make -j 2
end_fold_marker 'compilation'

start_fold_marker 'test'
set +e
make check -j 2
exit_status=$?
end_fold_marker 'test'

if [[ $exit_status -ne 0 ]];
then
start_fold_marker 'error'
start_fold_marker 'error.gnulib'
print_error_log gnulib/tests/test-suite.log
end_fold_marker 'error.gnulib'
start_fold_marker 'error.augeas'
print_error_log tests/test-suite.log
end_fold_marker 'error.augeas'
end_fold_marker 'error'
exit $exit_status
fi

set -e
start_fold_marker 'test.lenses'
./src/try valgrind 2>&1 | colout2 -t valgrind
end_fold_marker 'test.lenses'
