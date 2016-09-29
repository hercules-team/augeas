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
make check -j 2
end_fold_marker 'compilation'
start_fold_marker 'test'
./src/try valgrind
end_fold_marker 'test'
