#!/bin/sh

set -vx

rm -rf some_directory_that_does_not_exist_yet

cd /tmp
mkdir some_directory_that_does_not_exist_yet
cd some_directory_that_does_not_exist_yet
git clone git://git.savannah.nongnu.org/lmi.git \
  || git clone https://git.savannah.nongnu.org/r/lmi.git \
  || git clone https://github.com/vadz/lmi.git

cd ..
rm -rf some_directory_that_does_not_exist_yet
