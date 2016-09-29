#!/usr/bin/env bash

start_fold_marker() {
  echo "travis_fold:start:install.$1"
}

end_fold_marker() {
  echo "travis_fold:end:install.$1"
}

start_fold_marker 'dependencies'
sudo add-apt-repository ppa:jonathonf/binutils -y &&\
  sudo add-apt-repository ppa:ondrej/autotools -y &&\
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y

sudo apt-get update
sudo apt-get install --only-upgrade automake autoconf binutils -y
sudo apt-get install libselinux1-dev gcc-6 libxml2-dev libreadline-dev valgrind -y
end_fold_marker 'dependencies'

start_fold_marker 'autogen'
./autogen.sh
exit_status=$?
end_fold_marker 'autogen'

if [[ $exit_status -ne 0 ]];
then
  start_fold_marker 'error'
  echo "An error occured while running autogen. Exit status is $exit_status."
  cat config.log
  end_fold_marker 'error'
  exit $exit_status
fi

sed -i '41s/ENOENT/ENOENT || errno == EINVAL/' gnulib/tests/test-readlink.h
