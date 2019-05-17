#!/bin/sh this-script-must-be-sourced-not-run

foo()
{
echo "LMI_IN in 'set.sh': $LMI_IN"
echo "LMI_OUT entering 'set.sh': $LMI_OUT"
export LMI_OUT="$LMI_IN"
echo "LMI_OUT leaving 'set.sh': $LMI_OUT"
}

foo

unset -f foo
