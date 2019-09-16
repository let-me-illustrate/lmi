#!/bin/zsh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

# Add i386 before installing wine, so that wine can run 32-bit .exe's .

dpkg --add-architecture i386

# Prevent daemons from starting in the chroot; work around an
# 'ischroot' defect. See:
#   https://wiki.debian.org/chroot#Configuration

cat >/usr/sbin/policy-rc.d <<EOF
#!/bin/sh
exit 101
EOF

chmod a+x /usr/sbin/policy-rc.d

dpkg-divert --divert /usr/bin/ischroot.debianutils --rename /usr/bin/ischroot
ln -s /bin/true /usr/bin/ischroot

# For now at least, ignore this warning:
#   dpkg-divert: warning: diverting file '/usr/bin/ischroot' from an Essential
#   package with rename is dangerous, use --no-rename
# because the debian.org wiki cited above still recommends this diversion
# as of 2019-03, and the underlying defect is still unresolved:
#   https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=685034

# This being a "plain" schroot, mount essential directories:
mount -t devpts -o rw,nosuid,noexec,relatime,mode=600 devpts /dev/pts
mount -t proc -o rw,nosuid,nodev,noexec,relatime proc /proc

# If the chroot is to be permanent, consider adding those mounts to /etc/fstab .
# If the chroot is ever to be eradicated, be sure to unmount first:
# ...inside chroot:
#   umount ./dev/pts
#   umount ./proc
# ...or outside chroot:
#   umount /srv/chroot/${CHRTNAME}/dev/pts/
#   umount /srv/chroot/${CHRTNAME}/proc/
# Then, and only then:
#   rm -rf /srv/chroot/${CHRTNAME}/
#   rm /etc/schroot/chroot.d/${CHRTNAME}.conf

# Notes on various distros' package names.
#
# redhat provides 'xsltproc' as part of libxslt, though not as a
# separate package:
#   https://bugzilla.redhat.com/show_bug.cgi?id=965996

apt-get update
apt-get --assume-yes install wget g++-mingw-w64 automake libtool make \
 pkg-config git cvs zsh bzip2 unzip sudo wine default-jre jing trang \
 g++-multilib libxml2-utils libxslt1-dev vim-gtk vim-doc shellcheck \
 bc bsdtar xsltproc \
 >${CHRTNAME}-apt-get-log 2>&1

# This command should produce little output:
<${CHRTNAME}-apt-get-log tr -d '\r' | sed \
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
