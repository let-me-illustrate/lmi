#!/bin/sh

# Installer for wxPdfDocument library.
#
# Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

# A repository is cached in /srv/cache_for_lmi/vcs/, where it can be
# kept up to date and reused cheaply--whereas cloning it from a remote
# host takes considerable time and bandwidth, and fails if internet
# connectivity is lost, or the host is temporarily unavailable, or
# it is blocked by a corporate firewall.

# Configurable settings ########################################################

remote_host_url=${remote_host_url:-"https://github.com/vadz/wxpdfdoc.git"}

wxpdfdoc_commit_sha=${wxpdfdoc_commit_sha:-"acbd019d18e991cca46a80e1be58e637774d5d3b"}

wxpdfdoc_skip_clean=${wxpdfdoc_skip_clean:-"0"}

coefficiency=${coefficiency:-"--jobs=$(nproc)"}

MAKE=${MAKE:-"make $coefficiency"}

[ -n "$LMI_COMPILER" ] || { printf '%s\n' "no LMI_COMPILER" && exit 1; }
[ -n "$LMI_TRIPLET"  ] || { printf '%s\n' "no LMI_TRIPLET"  && exit 2; }

# Variables that normally should be left alone #################################

mingw_dir=/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw

prefix=/opt/lmi/local
exec_prefix="$prefix/${LMI_COMPILER}_${LMI_TRIPLET}"

repo_name="wxpdfdoc"

# Script commands ##############################################################

proxy_parent_dir="/srv/cache_for_lmi/vcs"
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

# Reset in case git-checkout would fail. See:
#   https://lists.nongnu.org/archive/html/lmi/2020-07/msg00053.html
git reset --hard

git checkout "$wxpdfdoc_commit_sha"

build_type=$("$proxy_wxpdfdoc_dir"/admin/build-aux/config.guess)

case "$build_type" in
    (*-*-cygwin*)
        mingw_bin_dir=$mingw_dir/bin/
        ;;
esac

wxpdfdoc_cc_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math'
wxpdfdoc_cxx_flags='-fno-ms-extensions -fno-omit-frame-pointer -frounding-math'

config_options="
  --prefix=$prefix
  --exec-prefix=$exec_prefix
  --with-wx-prefix=$prefix
  --with-wx-exec-prefix=$exec_prefix
  --build=$build_type
  --host=$LMI_TRIPLET
  --disable-dependency-tracking
"

[ -n "$mingw_bin_dir" ] && export PATH="$mingw_bin_dir:${PATH}"

cd "$proxy_wxpdfdoc_dir"
autoreconf --verbose

build_dir="$exec_prefix/wxpdfdoc-ad_hoc/wxpdfdoc-$wxpdfdoc_commit_sha"

if [ "$wxpdfdoc_skip_clean" != 1 ]
then
    rm --force --recursive "$build_dir"
    rm --force --recursive "$exec_prefix/lib/*wxcode*pdfdoc*"
    rm --force --recursive "$exec_prefix/src/pdf*.inc"
fi

mkdir --parents "$build_dir"

cd "$build_dir"
# 'configure' options must not be double-quoted
# shellcheck disable=SC2086
"$proxy_wxpdfdoc_dir"/configure $config_options CFLAGS="$wxpdfdoc_cc_flags" CXXFLAGS="$wxpdfdoc_cxx_flags"
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
