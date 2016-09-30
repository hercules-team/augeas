#!/usr/bin/env bash

set -e

start_fold_marker() {
  echo "travis_fold:start:script.$1"
}

end_fold_marker() {
  echo "travis_fold:end:script.$1"
}

start_fold_marker 'configure'
./configure --enable-debug=yes
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
cat gnulib/tests/test-suite.log
cat tests/test-suite.log
end_fold_marker 'error'
exit $exit_status
fi

set -e

start_fold_marker 'test.lenses'
./src/try valgrind
end_fold_marker 'test.lenses'
