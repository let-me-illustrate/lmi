#!/bin/sh this-script-must-be-sourced-not-run

foo()
{
# $LMI_TIMESTAMP is defined by the makefile that sources this script.
# shellcheck disable=SC2154
echo "LMI_TIMESTAMP in 'set.sh': $LMI_TIMESTAMP"
echo "LMI_IN in 'set.sh': $LMI_IN"
echo "LMI_OUT1 entering 'set.sh': $LMI_OUT1"
echo "LMI_OUT2 entering 'set.sh': $LMI_OUT2"
export LMI_OUT1="$LMI_IN"
export LMI_OUT2="$LANG"
echo "LMI_OUT1 leaving 'set.sh': $LMI_OUT1"
echo "LMI_OUT2 leaving 'set.sh': $LMI_OUT2"
{
    echo "export LMI_OUT1 := $LMI_OUT1"
    echo "export LMI_OUT2 := $LMI_OUT2"
} > env."$LMI_TIMESTAMP".eraseme
}

foo

unset -f foo
