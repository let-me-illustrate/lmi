#!/bin/sh

# Installer for wxPdfDocument library.
#
# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

set -vxe

# Configurable settings ########################################################

wxpdfdoc_skip_clean=${wxpdfdoc_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

MAKE=${MAKE:-"make $coefficiency"}

[ -n "$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER" && exit 1; }
[ -n "$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"  && exit 2; }

# Variables that normally should be left alone #################################

mingw_dir=/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw

prefix=/opt/lmi/local
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"

srcdir=$(dirname "$(readlink --canonicalize "$0")")
wxpdfdoc_dir="$srcdir/third_party/wxpdfdoc"

build_type=$(/usr/share/misc/config.guess)

# Script commands ##############################################################

case "$build_type" in
    (*-*-cygwin*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

wxpdfdoc_cc_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math -fsignaling-nans'
wxpdfdoc_cxx_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math -fsignaling-nans'

config_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --with-wx-prefix=$prefix
  --with-wx-exec-prefix=$exec_prefix
  --disable-dependency-tracking
"

# Provide the host and the build type only when cross-compiling,
# because specifying host for native builds, e.g.:
#   wx-config --host=$(/usr/share/misc/config.guess)
# fails. See:
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00039.html
#   https://trac.wxwidgets.org/ticket/12698
if [ "$LMI_TRIPLET" != "$build_type" ]
then
    config_options="
      $config_options
      --build=$build_type
      --host=$LMI_TRIPLET
    "
fi

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

cd "$wxpdfdoc_dir"
autoreconf --verbose

if [ -z "$gcc_version" ]; then
    gcc_version=$(make --no-print-directory --directory="$srcdir" show_gcc_version)
fi
build_dir="$exec_prefix/wxpdfdoc-ad_hoc/lmi-$LMI_COMPILER-$gcc_version"

if [ "$wxpdfdoc_skip_clean" != 1 ]
then
    rm --force --recursive "$build_dir"
    rm --force --recursive "$exec_prefix/lib/*wxcode*pdfdoc*"
    rm --force --recursive "$exec_prefix/src/pdf*.inc"
fi

mkdir --parents "$build_dir"

cd "$build_dir"
# 'config_options' must not be double-quoted
# shellcheck disable=SC2086
"$wxpdfdoc_dir"/configure $config_options \
    CFLAGS="$wxpdfdoc_cc_flags" \
  CXXFLAGS="$wxpdfdoc_cxx_flags" \

$MAKE
$MAKE install
# autotools: 'make install' doesn't respect group permissions--see:
#   https://lists.gnu.org/archive/html/automake/2019-01/msg00000.html
chmod -R g=u "$prefix"/include/wx*
chmod -R g=u "$exec_prefix"/bin
chmod -R g=u "$exec_prefix"/lib
# This should find zero files:
find "$prefix" -perm -200 -not -perm -020

exit 0
