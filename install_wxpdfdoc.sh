#!/bin/sh

# Installer for wxPdfDocument library.
#
# Copyright (C) 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

remote_host_url=${remote_host_url:-"https://github.com/vadz/wxpdfdoc.git"}

wxpdfdoc_commit_sha=${wxpdfdoc_commit_sha:-"55366f01eed8549ca704f9d9825127868858aafb"}

wxpdfdoc_skip_clean=${wxpdfdoc_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=4"}

MAKE=${MAKE:-"make $coefficiency"}

host_type=${LMI_HOST:-"i686-w64-mingw32"}

# Variables that normally should be left alone #################################

mingw_dir=/MinGW_

prefix=/opt/lmi/local
exec_prefix="$prefix"

repo_name="wxpdfdoc"

# Script commands ##############################################################

proxy_parent_dir="/cache_for_lmi/vcs"
mkdir --parents "$proxy_parent_dir"

proxy_wxpdfdoc_dir="$proxy_parent_dir"/$repo_name

# Create a local mirror if it doesn't already exist.
if [ ! -d "$proxy_wxpdfdoc_dir" ]
then
    cd "$proxy_parent_dir"
    git clone "$coefficiency" "$remote_host_url" $repo_name
fi

cd "$proxy_wxpdfdoc_dir"

# Fetch desired commit from remote host if missing.
if ! git rev-parse --quiet --verify "$wxpdfdoc_commit_sha^{commit}" >/dev/null
then
    git fetch origin
fi

git checkout "$wxpdfdoc_commit_sha"

build_type=$("$proxy_wxpdfdoc_dir"/admin/build-aux/config.guess)

case "$build_type" in
    (*-*-cygwin*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

wxpdfdoc_cc_flags=-fno-omit-frame-pointer
wxpdfdoc_cxx_flags=-fno-omit-frame-pointer

config_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --with-wx-prefix=$prefix
  --with-wx-exec-prefix=$exec_prefix
  --build=$build_type
  --host=$host_type
  --disable-dependency-tracking
  CFLAGS=$wxpdfdoc_cc_flags
  CXXFLAGS=$wxpdfdoc_cxx_flags
"

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

cd "$proxy_wxpdfdoc_dir"
autoreconf --verbose

build_dir="$prefix"/../wxpdfdoc-ad_hoc/wxpdfdoc-$wxpdfdoc_commit_sha

if [ "$wxpdfdoc_skip_clean" != 1 ]
then
    rm --force --recursive "$build_dir"
    rm --force --recursive $exec_prefix/lib/*wxcode*pdfdoc*
    rm --force --recursive $exec_prefix/src/pdf*.inc
fi

mkdir --parents "$build_dir"

cd "$build_dir"
"$proxy_wxpdfdoc_dir"/configure $config_options
$MAKE
$MAKE install

exit 0
