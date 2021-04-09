#!/bin/sh

  triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32 i686-w64-mingw32"
export LMI_COMPILER=gcc
export LMI_TRIPLET
for LMI_TRIPLET in ${triplets} ;
do
{
printf 'LMI_TRIPLET = "%s"\n' "$LMI_TRIPLET" > /dev/tty
# shellcheck disable=SC2154
  make "$coefficiency" cli_timing > /dev/null
}
printf "\n  Done.\n"
done
