#!/bin/zsh
#
# 'lmi_toolchains' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
  lmi_toolchains="clang_gnu64 gcc_gnu64 gcc_msw64"
export LMI_COMPILER
export LMI_TRIPLET
# shellcheck disable=SC2043
for toolchain in ${=lmi_toolchains} ;
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
prefix=/opt/lmi
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"
log_dir="$exec_prefix"/logs
mkdir --parents "$log_dir"
{
printf 'toolchain: %s\n' "${LMI_COMPILER}_${LMI_TRIPLET}" > /dev/tty
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
