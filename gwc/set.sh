#!/bin/sh this-script-must-be-sourced-not-run

foo()
{
echo "LMI_IN in 'set.sh': $LMI_IN"
echo "LMI_OUT1 entering 'set.sh': $LMI_OUT1"
echo "LMI_OUT2 entering 'set.sh': $LMI_OUT2"
export LMI_OUT1="$LMI_IN"
export LMI_OUT2="$LANG"
echo "LMI_OUT1 leaving 'set.sh': $LMI_OUT1"
echo "LMI_OUT2 leaving 'set.sh': $LMI_OUT2"
}

foo

unset -f foo
