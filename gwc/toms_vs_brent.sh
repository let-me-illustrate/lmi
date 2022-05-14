#!/bin/zsh

# Compile and run 'zero_test' for
#  - all architectures
#  - Alefeld's (TOMS 748) algorithm 4.2, as well as Brent's
# then extract the results of the tests in the TOMS 748 paper and
# save them to distinct files.

# 'triplets' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
  triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32 i686-w64-mingw32"
export LMI_COMPILER=gcc
export LMI_TRIPLET
# shellcheck disable=SC2043
for LMI_TRIPLET in ${=triplets} ;
do
prefix=/opt/lmi
log_dir=$(dirname "$(readlink --canonicalize "$0")")
{
printf 'LMI_TRIPLET = "%s"\n' "$LMI_TRIPLET" > /dev/tty

make clean
# shellcheck disable=SC2039,SC2154,SC3001
  make "$coefficiency" unit_tests unit_test_targets=zero_test \
      CPPFLAGS="-DTEST_ALEFELD" 2>&1 \
    | sed -e'1,/--8<--/d' -e'/--8<--/,$d' \
    >"$log_dir"/"zero_a_$LMI_TRIPLET"

make clean
# shellcheck disable=SC2039,SC2154,SC3001
  make "$coefficiency" unit_tests unit_test_targets=zero_test \
    | sed -e'1,/--8<--/d' -e'/--8<--/,$d' \
    >"$log_dir"/"zero_b_$LMI_TRIPLET"
}
printf "\n  Done.\n"
done
