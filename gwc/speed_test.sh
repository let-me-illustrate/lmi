#!/bin/sh

  lmi_toolchains="clang_gnu64 gcc_gnu64 gcc_msw64"
export LMI_COMPILER
export LMI_TRIPLET
for toolchain in ${lmi_toolchains} ;
do
case "$toolchain" in
    (clang_gnu64)
        LMI_COMPILER="clang"
        LMI_TRIPLET="x86_64-pc-linux-gnu"
        ;;
    (gcc_gnu64)
        LMI_COMPILER="gcc"
        LMI_TRIPLET="x86_64-pc-linux-gnu"
        ;;
    (gcc_msw64)
        LMI_COMPILER="gcc"
        LMI_TRIPLET="x86_64-w64-mingw32"
        ;;
    (*)
        printf 'Unknown toolchain "%s".\n' "$toolchain"
        return 1;
        ;;
esac
{
printf 'toolchain = "%s"\n' "$toolchain" > /dev/tty || true
# shellcheck disable=SC2154
  make "$coefficiency" cli_timing > /dev/null
}
printf "\n  Done.\n"
done
