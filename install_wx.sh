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

set -vxe

# A repository is cached in /cache_for_lmi/vcs/, where it can be kept
# up to date and reused cheaply--whereas cloning it from a remote
# host takes considerable time and bandwidth, and fails if internet
# connectivity is lost, or the host is temporarily unavailable, or
# it is blocked by a corporate firewall.

# Configurable settings ########################################################

remote_host_url=${remote_host_url:-"https://github.com/wxWidgets/wxWidgets.git"}

wx_commit_sha=${wx_commit_sha:-"e38866d3a603f600f87016458260f73593627348"}

wx_skip_clean=${wx_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=4"}

MAKE=${MAKE:-"make $coefficiency"}

# Variables that normally should be left alone #################################

mingw_dir=/MinGW_

prefix=/opt/lmi/local
exec_prefix="$prefix"

# Script commands ##############################################################

proxy_parent_dir="/cache_for_lmi/vcs"
mkdir --parents "$proxy_parent_dir"

proxy_wx_dir="$proxy_parent_dir"/wxWidgets

# Create a local mirror if it doesn't already exist.
if [ ! -d "$proxy_wx_dir" ]
then
    cd "$proxy_parent_dir"
    git clone "$coefficiency" --recurse-submodules "$remote_host_url"
fi

cd "$proxy_wx_dir"

# Fetch desired commit from remote host if missing.
if ! git rev-parse --quiet --verify "$wx_commit_sha^{commit}" >/dev/null
then
    git fetch "$proxy_wx_dir"
fi

git checkout "$wx_commit_sha"

# Get any new submodules that may have been added, even if nested.
git submodule update "$coefficiency" --recursive --init

if [ "$wx_skip_clean" = 1 ]
then
    rm --force --recursive "$build_dir"
    # This incidentally removes wxPdfDoc, but it's a good idea to rebuild that
    # whenever wx is upgraded anyway.
    rm --force --recursive $exec_prefix/bin/wx*
    rm --force --recursive $exec_prefix/include/wx*
    rm --force --recursive $exec_prefix/lib/wx*
    rm --force --recursive $exec_prefix/lib/libwx*
fi

build_type=$("$proxy_wx_dir"/config.guess)
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

build_dir="$prefix"/../wx-scratch/lmi-gcc-$gcc_version
mkdir --parents "$build_dir"

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
  --without-opengl
  --without-subdirs
  CPPFLAGS=-I$prefix/include
  CFLAGS=$wx_cc_flags
  CXXFLAGS=$wx_cxx_flags
  LDFLAGS=-L$exec_prefix/lib
"

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

cd "$build_dir"
"$proxy_wx_dir"/configure $config_options
$MAKE
$MAKE install

exit 0
