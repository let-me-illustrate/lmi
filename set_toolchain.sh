#!/bin/sh this-script-must-be-sourced-not-run

# Set $PATH, $WINEPATH, and $PERFORM based on $LMI_COMPILER and $LMI_TRIPLET.

# Copyright (C) 2019, 2020 Gregory W. Chicares.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# Invoke as ". /opt/lmi/src/lmi/set_toolchain.sh" without the quotes.
#
# $LMI_COMPILER and $LMI_TRIPLET are set to default values currently
# used in production if they were unset or null beforehand. They can
# be overridden at the command line, e.g.:
#
#   LMI_COMPILER=gcc ; LMI_TRIPLET=x86_64-pc-linux-gnu ; . /opt/lmi/src/lmi/set_toolchain.sh
#   LMI_COMPILER=gcc ; LMI_TRIPLET=i686-w64-mingw32    ; . /opt/lmi/src/lmi/set_toolchain.sh
#   LMI_COMPILER=gcc ; LMI_TRIPLET=x86_64-w64-mingw32  ; . /opt/lmi/src/lmi/set_toolchain.sh
#
# Implemented as a function that runs and then erases itself, so that
# sourcing this script changes the environment only as intended. This
# depends on 'local', which isn't yet POSIX, though there's a proposal
# to add it:
#   http://austingroupbugs.net/view.php?id=767
# 'dash' and 'ash' already have 'local', so, while it's not yet POSIX,
# it's used here, and its 'shellcheck' warning is disabled:
# shellcheck disable=SC2039

# Unimplemented alternative: Variables such as $prefix could be made
# similarly overridable, along with other directories--perhaps even
# platform-specific ones like $mingw_bin_dir.

# Unimplemented alternative: use symlinks, e.g:
#   ln --symbolic --force --no-dereference \
#   /opt/lmi/"${LMI_COMPILER}_${LMI_TRIPLET}"/bin /opt/lmi/bin
# and likewise for all other directories. Depending on symlinks
# didn't seem like a good idea.

# This alternative for future consideration trades some complexity of
# implementation for mitigation of confusion and convenience of use.
#
# To cross-build autotoolized libraries, configure with:
#   --host=$LMI_TRIPLET
#   --exec_prefix=$prefix/$LMI_TOOLCHAIN
# where
#   LMI_TOOLCHAIN=${LMI_COMPILER}_${LMI_TRIPLET}
#
# Supported values:
#   LMI_COMPILER : gcc, clang
#   LMI_TRIPLET  : x86_64-pc-linux-gnu, i686-w64-mingw32, x86_64-w64-mingw32
# (clang not yet tested).
#
# Examples:
#
#  LMI_TOOLCHAIN  LMI_COMPILER  ----------LMI_TRIPLET----------
#                               cpu     -vendor -[kernel-]system
#
#      gcc_msw32       gcc      i686    -w64            -mingw32
#      gcc_msw64       gcc      x86_64  -w64            -mingw32
#    clang_gnu64      clang     x86_64  -pc     -linux  -gnu
#
# Rationale: Triplets are required for selecting cross compilers. GNU
# triplets are in widespread use, although debian uses its own:
#   https://wiki.debian.org/Multiarch/Tuples#Why_not_use_GNU_triplets.3F
# The main reason for lmi to establish its own is that the official
# triplets for msw are so poor. For example, "i686-w64-mingw32" has
# substrings "64" and "32". Both seem to specify the CPU's word size
# (contradicting each other), but actually neither does--the word size
# is determined by the "i686" field. See:
#   https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=622276

foo()
{
local   lmi_build_type
        lmi_build_type=$(/usr/share/misc/config.guess)

local      prefix="/opt/lmi"
local      bindir="$prefix/bin"
local localbindir="$prefix/local/${LMI_COMPILER}_${LMI_TRIPLET}/bin"
local locallibdir="$prefix/local/${LMI_COMPILER}_${LMI_TRIPLET}/lib"

# Running a command like this many times:
#   export PATH="$localbindir":"$locallibdir":"$PATH"
# would cause $PATH to grow without bound.
#
# The default non-lmi portion of this $PATH is, as always, debatable;
# debian's default in '/etc/login.defs' adds silly 'games' directories
# but omits '/usr/sbin' and '/sbin', for instance.

minimal_path=${MINIMAL_PATH:-"/usr/bin:/bin:/usr/sbin:/sbin"}
export PATH="$localbindir":"$locallibdir":"$minimal_path"

# It is okay to export these two variables unconditionally.

export WINEPATH
export PERFORM

# Are double quotes inside double quotes inside $() dubious? I.e.,
#  " $( "is this string quoted?" ) "
# Answer: yes, it is: everything inside '$()' is parsed separately.
#   https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#tag_18_03
# [2.3/5] "The characters found from the beginning of the
# substitution to its end, allowing for any recursion necessary
# to recognize embedded constructs, shall be included unmodified
# in the result token, including any embedded or enclosing
# substitution operators or quotes. The token shall not be
# delimited by the end of the substitution."
#
# 'winepath' doesn't DTRT with '/first/path:/second/path'--need to
#   run it against each component, and separate results with ';', not ':'
#
case "$lmi_build_type" in
    (*-*-linux*)
        case "$LMI_TRIPLET" in
            (x86_64-pc-linux-gnu)
                # Using LD_LIBRARY_PATH is not ideal, but it does work.
                export LD_LIBRARY_PATH
                LD_LIBRARY_PATH=.
                LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$locallibdir"
                LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$bindir"
                ;;
            (*-*-mingw32)
                w0="$(winepath -w "$localbindir" | sed -e's/\\/\\\\/g')"
                w1="$(winepath -w "$locallibdir" | sed -e's/\\/\\\\/g')"
                export WINEPATH="$w0;$w1"
                export  PERFORM="wine"
                ;;
        esac
        ;;
    (*) ;;
esac
}

# This script is to be sourced, so use 'return' because 'exit' would
# have a surprising effect.

export LMI_COMPILER
export LMI_TRIPLET
       LMI_COMPILER=${LMI_COMPILER:-"gcc"}
       LMI_TRIPLET=${LMI_TRIPLET:-"i686-w64-mingw32"}

case "$LMI_COMPILER" in
    (gcc) ;;
    (*)
        printf '%s\n' "Changed nothing because compiler '$LMI_COMPILER' is untested."
        return 2;
        ;;
esac

case "$LMI_TRIPLET" in
    (x86_64-pc-linux-gnu) ;;
    (i686-w64-mingw32)    ;;
    (x86_64-w64-mingw32)  ;;
    (*)
        printf '%s\n' "Changed nothing because host triplet '$LMI_TRIPLET' is untested."
        return 3;
        ;;
esac

foo

unset -f foo
