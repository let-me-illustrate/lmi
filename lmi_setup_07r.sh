#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
assert_not_chrooted

# Suppress a nuisance: rh-based distributions provide a default
# zsh logout file that clears the screen.
sed -e'/^[^#]/s/^/# SUPPRESSED # /' -i /etc/zlogout

# Make a more modern 'git' available via 'scl'. This is not needed
# if all real work is done in a debian chroot.
#yum --assumeyes install centos-release-scl
#yum-config-manager --enable rhel-server-rhscl-7-rpms
#yum --assumeyes install devtoolset-8 rh-git218
# In order to use the tools on the three preceding lines, do:
#   scl enable devtoolset-8 rh-git218 $SHELL
# and then they'll be available in that environment.

# Fix weird errors like "Problem with the SSL CA cert (path? access rights?)".
# Try 'update' before 'install' as described here:
#   https://lists.nongnu.org/archive/html/lmi/2020-02/msg00003.html
yum --assumeyes update  ca-certificates curl nss-pem wget
yum --assumeyes install ca-certificates curl nss-pem wget

# Install "EPEL", which is required to install 'debootstrap' and
# 'schroot' later on RHEL--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00037.html
yum --assumeyes install https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm || true
# This command is recommended for RHEL-7, as EPEL packages may depend
# on it; but it's allowed to fail, because of a corporate server that
# allows access only to its own customized RHEL stuff:
subscription-manager repos --enable "rhel-*-optional-rpms" --enable "rhel-*-extras-rpms"  --enable "rhel-ha-for-rhel-*-server-rpms" || true
yum --assumeyes install epel-release

stamp=$(date -u +'%Y%m%dT%H%M%SZ')
echo "$stamp $0: Installed EPEL."  | tee /dev/tty
