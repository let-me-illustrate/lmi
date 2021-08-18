#!/bin/zsh

# 'triplets' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
  triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32 i686-w64-mingw32"
export LMI_COMPILER=gcc
export LMI_TRIPLET
# shellcheck disable=SC2043
for LMI_TRIPLET in ${=triplets} ;
do
prefix=/opt/lmi
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"
log_dir="$exec_prefix"/logs
{
printf 'LMI_TRIPLET = "%s"\n' "$LMI_TRIPLET" > /dev/tty
# shellcheck disable=SC2039,SC2154,SC3001
  make "$coefficiency" --output-sync=recurse unit_tests 2>&1 \
    | tee \
    >(grep '\*\*\*') \
    >(grep \?\?\?\?) \
    >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') \
    >"$log_dir"/unit_tests
}
printf "\n  Done.\n"
done
