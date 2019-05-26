#!/bin/sh this-script-must-be-sourced-not-run

foo()
{
echo "'$LMI_IN' --> '$LMI_OUT1', '$LMI_OUT2' : entering 'set.sh'"

export LMI_OUT1="$LMI_IN"
export LMI_OUT2="$LANG"

case "$LMI_IN" in
    (Mongolia) LMI_OUT2="mn_MN" ;;
    (Russia)   LMI_OUT2="ru_RU" ;;
    (*) ;;
esac

echo "'$LMI_IN' --> '$LMI_OUT1', '$LMI_OUT2' : leaving 'set.sh'"
}

foo

unset -f foo
