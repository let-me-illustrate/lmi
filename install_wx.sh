#!/bin/sh

# Install msw-native wx with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

wx_skip_clean=${wx_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

MAKE=${MAKE:-"make $coefficiency"}

[ -n "$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER" && exit 1; }
[ -n "$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"  && exit 2; }

# Variables that normally should be left alone #################################

mingw_dir=/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw

prefix=/opt/lmi/local
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"

srcdir=$(dirname "$(readlink --canonicalize "$0")")
wx_dir="$srcdir/third_party/wx"

build_type=$(/usr/share/misc/config.guess)

# Script commands ##############################################################

case "$build_type" in
    (*-*-cygwin*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

# Distinguish wx dll by host type, compiler version, and wx SHA1.
if [ -z "$gcc_version" ]; then
    gcc_version=$(make --no-print-directory --directory="$srcdir" show_gcc_version)
fi
vendor=${LMI_TRIPLET}-$gcc_version-$(git rev-parse --short HEAD:third_party/wx)

# Configuration reference:
#   https://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html

wx_cc_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math -fsignaling-nans'
wx_cxx_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math -fsignaling-nans'

config_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --build=$build_type
  --host=$LMI_TRIPLET
  --disable-apple_ieee
  --disable-aui
  --disable-compat30
  --disable-dependency-tracking
  --disable-fswatcher
  --disable-gif
  --disable-mediactrl
  --disable-precomp-headers
  --disable-propgrid
  --disable-ribbon
  --disable-richtext
  --disable-stc
  --disable-sys-libs
  --disable-webview
  --enable-monolithic
  --enable-option-checking
  --enable-shared
  --enable-stl
  --enable-vendor=$vendor
  --with-cxx=11
  --without-opengl
  --without-subdirs
"

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

build_dir="$exec_prefix/wx-ad_hoc/lmi-$LMI_COMPILER-$gcc_version"

if [ "$wx_skip_clean" != 1 ]
then
    rm --force --recursive "$build_dir"
    # This incidentally removes wxPdfDoc, but it's a good idea to rebuild that
    # whenever wx is upgraded anyway.
    rm --force --recursive "$exec_prefix/bin/wx*"
    rm --force --recursive "$exec_prefix/include/wx*"
    rm --force --recursive "$exec_prefix/lib/wx*"
    rm --force --recursive "$exec_prefix/lib/libwx*"
fi

mkdir --parents "$build_dir"

cd "$build_dir"
printf 'Building %s with %s for %s.\n' "wx" "$LMI_COMPILER" "$LMI_TRIPLET"
# 'config_options' must not be double-quoted
# shellcheck disable=SC2086
"$wx_dir"/configure $config_options \
  CPPFLAGS="-I$prefix/include" \
    CFLAGS="$wx_cc_flags" \
  CXXFLAGS="$wx_cxx_flags" \
   LDFLAGS="-L$exec_prefix/lib" \

$MAKE
$MAKE install
printf 'Built %s with %s for %s.\n' "wx" "$LMI_COMPILER" "$LMI_TRIPLET"

# autotools: 'make install' doesn't respect group permissions--see:
#   https://lists.gnu.org/archive/html/automake/2019-01/msg00000.html
chmod -R g=u "$prefix"/include/wx*
chmod -R g=u "$exec_prefix"/lib/wx*
chmod -R g=u "$exec_prefix"/lib/libwx*
chmod -R g=u "$prefix"/share
# This should find zero files:
find "$prefix" -perm -200 -not -perm -020

exit 0
