#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

. ./lmi_setup_inc.sh
# shellcheck disable=SC1091
. /tmp/schroot_env

set -evx

assert_su
assert_chrooted

# Add i386 before installing wine, so that wine can run 32-bit .exe's .

dpkg --add-architecture i386

# Prevent daemons from starting in the chroot; work around an
# 'ischroot' defect. See:
#   https://wiki.debian.org/chroot#Configuration
# and, for file permissions:
#   https://lists.nongnu.org/archive/html/lmi/2020-02/msg00015.html

cat >/usr/sbin/policy-rc.d <<EOF
#!/bin/sh
exit 101
EOF

chmod 755 /usr/sbin/policy-rc.d

dpkg-divert --divert /usr/bin/ischroot.debianutils --rename /usr/bin/ischroot
ln -s /bin/true /usr/bin/ischroot

# For now at least, ignore this warning:
#   dpkg-divert: warning: diverting file '/usr/bin/ischroot' from an Essential
#   package with rename is dangerous, use --no-rename
# because the debian.org wiki cited above still recommends this diversion
# as of 2019-03, and the underlying defect is still unresolved:
#   https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=685034

# Suppress a nuisance: debian-based distributions provide a default
# bash logout file that clears the screen.
sed -e'/^[^#]/s/^/# SUPPRESSED # /' -i /etc/skel/.bash_logout

# Historical notes on various distros' package names.
#
# redhat names some packages differently:
#   pkgconfig ShellCheck libxml2 libxslt
#   vim-X11 vim-common vim-enhanced vim-minimal
#   mingw32-gcc-c++ mingw64-gcc-c++
#   java-1.8.0-openjdk
# It provides 'xsltproc' as part of libxslt, though not as a
# separate package:
#   https://bugzilla.redhat.com/show_bug.cgi?id=965996
#
# Instead of worrying about such gratuitous differences, this suite
# of scripts installs a debian chroot on any host system, and builds
# only in that chroot.

logdir=/srv/cache_for_lmi/logs
mkdir -p "${logdir}"
apt-get update
apt-get --assume-yes install \
  automake \
  bc \
  bsdmainutils \
  bzip2 \
  curl \
  cvs \
  default-jre \
  g++-mingw-w64 \
  g++-multilib \
  git \
  jing \
  libarchive-tools \
  libc6-dbg \
  libdw-dev \
  libgtk-3-dev \
  libtool \
  libxml2-utils \
  libunwind-dev \
  libxslt1-dev \
  make \
  patch \
  pkg-config \
  rsync \
  shellcheck \
  sudo \
  trang \
  unzip \
  vim-doc \
  vim-fugitive \
  vim-gtk \
  wget \
  wine \
  xsltproc \
  zsh \
 >"${logdir}/${CHRTNAME}"-apt-get-log 2>&1

# This command should produce little output:
< "${logdir}/${CHRTNAME}"-apt-get-log tr -d '\r' | sed \
 -e'0,/^Preconfiguring/d' \
 -e'/^Fetched\|^Preparing\|^Unpacking\|^Configuring\|^Selecting/d' \
 -e'/^Setting up\|^Processing\|^Adding\|^update-alternatives\|^[Dd]one./d' \
 -e'/^(Reading database\|^Linking\|^Moving old\|^Regenerating/d' \
 -e'/^Creating config\|^Updating certificates\|^Running hooks/d' \
 -e'/^Running in chroot, ignoring request.$/d' \
 -e'/^update-rc.d: warning: start and stop actions are no longer supported; falling back to defaults$/d' \
 -e'/^invoke-rc.d: policy-rc.d denied execution of start.$/d' \
 -e'/^Warning: The home dir \/run\/uuidd you specified can.t be accessed: No such file or directory$/d' \
 -e'/^Not creating home directory .\/run\/uuidd..$/d' \
 -e'/^No schema files found: doing nothing.$/d' \
 -e'/^[0-9][0-9]* added, [0-9][0-9]* removed; done.$/d' \

#
# Don't worry about messages like the following--see:
#   https://lists.nongnu.org/archive/html/lmi/2016-09/msg00025.html
#
#   update-rc.d: warning: start and stop actions are no longer supported; falling back to defaults
#   invoke-rc.d: policy-rc.d denied execution of start.
#   No schema files found: doing nothing.
#   Warning: The home dir /run/uuidd you specified can't be accessed: No such file or directory
#   Not creating home directory `/run/uuidd'.

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Installed debian packages."  | tee /dev/tty
