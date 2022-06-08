#!/bin/zsh

unit_test_stderr_clutter='
/.*\/test_coding_rules_test\.sh$/d
/^Testing .test_coding_rules.\.$/d
/^  This message should appear on stderr\.$/d
/^Integrity check failed for .coleridge.$/d
/^Please report this: culminate() not called\.$/d
/^sh: 1: xyzzy: not found$/d
'

# 'triplets' really is used, but in a zsh-specific way
# shellcheck disable=SC2034
  triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32"
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
  if [ "x86_64-pc-linux-gnu" = "$LMI_TRIPLET" ]
  then
    printf '\n  unit tests with UBSan\n\n'
    # shellcheck disable=SC3001
    (setopt nomultios; \
      ( \
        (make "$coefficiency" --output-sync=recurse unit_tests \
          build_type=ubsan UBSAN_OPTIONS=print_stacktrace=1 \
        | tee \
          >(grep '\*\*\*') \
          >(grep \?\?\?\?) \
          >(grep '!!!!' --count | xargs printf '%d tests succeeded\n') \
        >"$log_dir"/unit_tests_ubsan_stdout \
        ) \
        3>&1 1>&2 2>&3 \
        | tee "$log_dir"/unit_tests_ubsan_stderr \
          | sed -e "$unit_test_stderr_clutter" \
          | sed -e's/^/UBSan: /' \
      ) 3>&1 1>&2 2>&3 \
    );
  fi
}
printf "\n  Done.\n"
done
