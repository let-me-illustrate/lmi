#!/bin/sh this-script-must-be-sourced-not-run

foo()
{
# $LMI_ENV_FILE is defined by the makefile that sources this script.
# shellcheck disable=SC2154
echo "LMI_ENV_FILE in 'set.sh': $LMI_ENV_FILE"
echo "entering 'set.sh': LMI_IN $LMI_IN; LMI_OUT1 $LMI_OUT1; LMI_OUT2 $LMI_OUT2"
export LMI_OUT1="$LMI_IN"
export LMI_OUT2="$LANG"

case "$LMI_IN" in
    (Mongolia) LMI_OUT2="mn_MN" ;;
    (Russia)   LMI_OUT2="ru_RU" ;;
    (*) ;;
esac

echo "leaving 'set.sh': LMI_IN $LMI_IN; LMI_OUT1 $LMI_OUT1; LMI_OUT2 $LMI_OUT2"
if [ -n "$LMI_ENV_FILE" ]; then
    {
    echo "export LMI_OUT1 := $LMI_OUT1"
    echo "export LMI_OUT2 := $LMI_OUT2"
    } > "$LMI_ENV_FILE"
fi
}

foo

unset -f foo
