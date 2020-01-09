#!/bin/sh

# Install msw-native wx with options suitable for lmi.
#
# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

wx_commit_sha=${wx_commit_sha:-"f741031e69de73d5816cc56e99c9beba3ac820de"}

wx_skip_clean=${wx_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

MAKE=${MAKE:-"make $coefficiency"}

[ -n "$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER" && exit 1; }
[ -n "$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"  && exit 2; }

# Variables that normally should be left alone #################################

mingw_dir=/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw

prefix=/opt/lmi/local
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"

repo_name="wxWidgets"

# Script commands ##############################################################

proxy_parent_dir="/cache_for_lmi/vcs"
mkdir --parents "$proxy_parent_dir"

proxy_wx_dir="$proxy_parent_dir"/$repo_name

# Create a local mirror if it doesn't already exist.
if [ ! -d "$proxy_wx_dir" ]
then
    cd "$proxy_parent_dir"
    git clone "$coefficiency" --recurse-submodules "$remote_host_url" $repo_name
fi

cd "$proxy_wx_dir"

# Fetch desired commit from remote host if missing.
if ! git rev-parse --quiet --verify "$wx_commit_sha^{commit}" >/dev/null
then
    git fetch origin
fi

git checkout "$wx_commit_sha"

# Get any new submodules that may have been added, even if nested.
git submodule update "$coefficiency" --recursive --init

build_type=$("$proxy_wx_dir"/config.guess)

case "$build_type" in
    (*-*-cygwin*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

# Distinguish wx dll by host type, compiler version, and wx SHA1.
gcc_version=$("${mingw_bin_dir}${LMI_TRIPLET}-$LMI_COMPILER" -dumpversion|tr -d '\r')
vendor=${LMI_TRIPLET}-$gcc_version-$wx_commit_sha

# Configuration reference:
#   http://lists.nongnu.org/archive/html/lmi/2007-11/msg00001.html

wx_cc_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math'
wx_cxx_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math'

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
  LDFLAGS=-L$exec_prefix/lib
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
# 'configure' options must not be double-quoted
# shellcheck disable=SC2086
"$proxy_wx_dir"/configure $config_options CFLAGS="$wx_cc_flags" CXXFLAGS="$wx_cxx_flags"
$MAKE
$MAKE install

exit 0
