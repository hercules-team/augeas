#!/usr/bin/env bash

start_fold_marker() {
  echo "travis_fold:start:install.$1"
}

end_fold_marker() {
  echo "travis_fold:end:install.$1"
}

start_fold_marker 'dependencies'
if [[ $CC = "gcc"* ]];
then
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
# elif [[ $CC = 'clang'* ]]; then
#   sudo apt-add-repository 'deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-${CC#clang-} main'
#   wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
fi
sudo add-apt-repository ppa:jonathonf/binutils -y &&\
  sudo add-apt-repository ppa:ondrej/autotools -y

sudo apt-get -qq update

sudo apt-get install aria2
git clone https://github.com/ilikenwf/apt-fast /tmp/apt-fast --depth 1
sudo cp /tmp/apt-fast/apt-fast /usr/bin
sudo chmod +x /usr/bin/apt-fast
sudo cp /tmp/apt-fast/apt-fast.conf /etc

sudo apt-fast -qq install --only-upgrade automake autoconf binutils -y
sudo apt-fast -qq install python3 python3-pip libselinux1-dev $CC libxml2-dev libreadline-dev valgrind -y

sudo pip3 install -q colout2
end_fold_marker 'dependencies'

start_fold_marker 'autogen'
./autogen.sh 2>&1 | colout2 -t autogen
exit_status=$?
end_fold_marker 'autogen'

if [[ $exit_status -ne 0 ]];
then
  start_fold_marker 'error'
  echo "An error occured while running autogen. Exit status is $exit_status."
  start_fold_marker 'error.autogen'
  cat config.log
  end_fold_marker 'error.autogen'
  end_fold_marker 'error'
  exit $exit_status
fi

sed -i '41s/ENOENT/ENOENT || errno == EINVAL/' gnulib/tests/test-readlink.h
