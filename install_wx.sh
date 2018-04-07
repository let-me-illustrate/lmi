#!/bin/sh

# Install msw-native wx with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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
# http://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# Configurable settings ########################################################

wx_git_url=${wx_git_url:-https://github.com/wxWidgets/wxWidgets.git}

wx_commit_sha=${wx_commit_sha:-41045df7ea5f93e4c07c1bd846d7127a372705bd}

coefficiency=${coefficiency:---jobs=4}
MAKE=${MAKE:-make $coefficiency}

# Variables that normally should be left alone #################################

mingw_dir=/MinGW_

prefix=/opt/lmi/local
exec_prefix=${prefix}

wx_dir=/opt/lmi/third_party/src/wxWidgets

# Script commands ##############################################################

set -e

if [ ! -d $wx_dir ]
then
    wx_dir_parent=${wx_dir%/*}
    [ -d $wx_dir_parent ] || mkdir -p $wx_dir_parent
    cd $wx_dir_parent
    git clone "$wx_git_url" ${wx_dir##*/}
    cd $wx_dir
else
    cd $wx_dir
    git rev-parse --quiet --verify "$wx_commit_sha^{commit}" >/dev/null || git fetch "$wx_git_url"
fi

if [ $(git rev-parse HEAD) != "$wx_commit_sha" ]
then
    git checkout "$wx_commit_sha"
    git submodule update --init
fi

[ "$wx_skip_clean" = 1 ] || git clean -dfx

# This incidentally removes wxPdfDoc, but it's a good idea to rebuild that
# whenever wx is upgraded anyway.
rm --force --recursive $exec_prefix/bin/wx*
rm --force --recursive $exec_prefix/include/wx*
rm --force --recursive $exec_prefix/lib/wx*
rm --force --recursive $exec_prefix/lib/libwx*

build_type=$($wx_dir/config.guess)
host_type=i686-w64-mingw32

case $(uname) in
    CYGWIN*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

# Construct a vendor string for this build using the compiler name and version
# and the unique commit SHA-1.
gcc_version=$(${mingw_bin_dir}${host_type}-gcc -dumpversion|tr -d '\r')
vendor=gcc-$gcc_version-$wx_commit_sha

build_dir=$wx_dir/lmi-gcc-$gcc_version
mkdir -p "$build_dir"

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html

wx_cc_flags=-fno-omit-frame-pointer
wx_cxx_flags=-fno-omit-frame-pointer

config_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --build=$build_type
  --host=$host_type
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
  --disable-webview
  --enable-monolithic
  --enable-option-checking
  --enable-shared
  --enable-stl
  --enable-vendor=$vendor
  --with-cxx=11
  --with-expat=builtin
  --with-libpng=builtin
  --with-zlib=builtin
  --without-libjpeg
  --without-libtiff
  --without-opengl
  --without-subdirs
  CPPFLAGS=-I$prefix/include
  CFLAGS=$wx_cc_flags
  CXXFLAGS=$wx_cxx_flags
  LDFLAGS=-L$exec_prefix/lib
"

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

cd "$build_dir"
../configure "$config_options"
$MAKE
$MAKE install

exit 0
