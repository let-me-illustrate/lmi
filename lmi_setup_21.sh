#!/bin/sh

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

set -vx

. ./lmi_setup_inc.sh

assert_su
assert_chrooted

# Add a normal user, and a corresponding group.
#
# No attempt is made to set a real password, because that can't be
# done securely in a script. A better password can be set later,
# interactively, if desired. Forcing the dummy password to expire
# immediately, thus (e.g.):
#   chage -d 0 greg
# may seem like a good idea, but invoking schroot with that userid
# doesn't prompt for a password change.
groupadd --gid=1000 greg
useradd --gid=1000 --groups=sudo --uid=1000 --create-home --shell=/bin/zsh \
  --password="$(openssl passwd -1 expired)" greg

# Add an 'lmi' group, which may be useful in a multi-user chroot.
groupadd --gid=1001 lmi
usermod -aG lmi greg

mkdir -p /opt/lmi
chown greg:lmi /opt/lmi
mkdir -p /etc/opt/lmi
chown greg:lmi /etc/opt/lmi
mkdir -p /var/opt/lmi
chown greg:lmi /var/opt/lmi
mkdir -p /cache_for_lmi
chown greg:lmi /cache_for_lmi

chsh -s /bin/zsh greg

# Repair /usr/share/libtool/.../ltmain.sh as indicated here:
#   https://lists.gnu.org/archive/html/libtool-patches/2011-06/msg00001.html
# Do this as root because root owns the file.

cat >/home/greg/ltmain.sh.patch <<EOF
--- /usr/share/libtool/build-aux/ltmain.sh.orig 2016-08-20 12:34:31.000000000 +0000
+++ /usr/share/libtool/build-aux/ltmain.sh 2017-08-10 13:10:28.466155965 +0000
@@ -5555,7 +5555,7 @@
 /* declarations of non-ANSI functions */
 #if defined __MINGW32__
 # ifdef __STRICT_ANSI__
-int _putenv (const char *);
+_CRTIMP int _putenv (const char *);
 # endif
 #elif defined __CYGWIN__
 # ifdef __STRICT_ANSI__
EOF

patch --dry-run --strip=0 --directory=/ </home/greg/ltmain.sh.patch \
 && patch --strip=0 --directory=/ </home/greg/ltmain.sh.patch

# Configure zsh, for root as well as the user configured above.

wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/gwc/.zshrc'
mv .zshrc ~
cp -a ~/.zshrc /home/greg/.zshrc
chown greg:greg /home/greg/.zshrc

# Configure vim. Rather than trying to split its contents between
# '~/.vimrc' and '/etc/vim/vimrc.local', just copy it everywhither.

wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/gwc/.vimrc'
mv .vimrc ~
cp -a ~/.vimrc /etc/vim/vimrc.local
cp -a ~/.vimrc /home/greg/.vimrc
chown greg:greg /home/greg/.vimrc

# Without this, 'zg' gives an error message; with it, vim creates a
# spellfile the first time 'zg' is used, if none already exists.
mkdir ~/.vim
mkdir /home/greg/.vim
chown greg:greg /home/greg/.vim
# It's a much better idea to copy a mature spellfile hither:
wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/gwc/.vim/spell/en.utf-8.add'
mkdir ~/.vim/spell
mv en.utf-8.add ~/.vim/spell/en.utf-8.add
mkdir /home/greg/.vim/spell
chown greg:greg /home/greg/.vim/spell
cp -a ~/.vim/spell/en.utf-8.add /home/greg/.vim/spell/en.utf-8.add
chown greg:greg /home/greg/.vim/spell/en.utf-8.add
# and then (imperatively) run this command:
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'
# which will be repeated below in the user chroot.

# Enable stable and security upgrades:

cat >/etc/apt/sources.list <<EOF
deb http://deb.debian.org/debian/ ${CODENAME} main
deb http://deb.debian.org/debian/ ${CODENAME}-updates main
deb http://security.debian.org/   ${CODENAME}/updates main
EOF

# Apply any available upgrades:

apt-get --assume-yes update
apt-get --assume-yes dist-upgrade
