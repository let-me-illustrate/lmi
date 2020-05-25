#!/bin/sh

# Create a chroot for cross-building "Let me illustrate...".
#
# Copyright (C) 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

. ./lmi_setup_inc.sh
. /tmp/schroot_env

set -vx

assert_su
assert_chrooted

# Add a normal user, and a corresponding group.
#
# No attempt is made to set a real password, because that can't be
# done securely in a script. A better password can be set later,
# interactively, if desired. Forcing the dummy password to expire
# immediately, thus (e.g.):
#   chage -d 0 "${NORMAL_USER}"
# may seem like a good idea, but invoking schroot with that userid
# doesn't prompt for a password change.
#
# Hardcode the salt so that repeated openssl invocations yield
# identical results, to avoid gratuitous regressions when comparing
# successive logs.

groupadd --gid="${NORMAL_GROUP_GID}" "${NORMAL_GROUP}"
useradd \
  --gid="${NORMAL_GROUP_GID}" \
  --uid="${NORMAL_USER_UID}" \
  --create-home \
  --shell=/bin/zsh \
  --password="$(openssl passwd -1 --salt '' expired)" \
  "${NORMAL_USER}"

usermod -aG sudo "${NORMAL_USER}" || echo "Oops."

# Add an 'lmi' group, which may be useful in a multi-user chroot.
getent group 1001 || groupadd --gid=1001 lmi || echo "Oops."
usermod -aG lmi "${NORMAL_USER}" || echo "Oops."

mkdir --parents /opt/lmi
chmod g=u+s     /opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /opt/lmi
mkdir --parents /etc/opt/lmi
chmod g=u+s     /etc/opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /etc/opt/lmi
mkdir --parents /var/opt/lmi
chmod g=u+s     /var/opt/lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /var/opt/lmi
mkdir --parents /srv/cache_for_lmi
chmod g=u+s     /srv/cache_for_lmi
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /srv/cache_for_lmi

chsh -s /bin/zsh "${NORMAL_USER}"

# Repair /usr/share/libtool/.../ltmain.sh as indicated here:
#   https://lists.gnu.org/archive/html/libtool-patches/2011-06/msg00001.html
# Do this as root because root owns the file.

cat >/home/"${NORMAL_USER}"/ltmain.sh.patch <<EOF
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

patch --dry-run --strip=0 --directory=/ </home/"${NORMAL_USER}"/ltmain.sh.patch \
 && patch --strip=0 --directory=/ </home/"${NORMAL_USER}"/ltmain.sh.patch

# BEGIN ./lmi_setup_09.sh
# Configure zsh, for root and normal users.

wget -N -nv "${GIT_URL_BASE}"/gwc/.zshrc
mv .zshrc ~
cp -a ~/.zshrc /home/"${NORMAL_USER}"/.zshrc
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.zshrc

# Configure vim. Rather than trying to split its contents between
# '~/.vimrc' and '/etc/vim/vimrc.local', use '~/.vimrc' for all
# customizations and copy that file for the normal user too.

wget -N -nv "${GIT_URL_BASE}"/gwc/.vimrc
mv .vimrc ~
cp -a ~/.vimrc /home/"${NORMAL_USER}"/.vimrc
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vimrc

# Without this, 'zg' gives an error message; with it, vim creates a
# spellfile the first time 'zg' is used, if none already exists.
mkdir ~/.vim
mkdir /home/"${NORMAL_USER}"/.vim
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim
# It's a much better idea to copy a mature spellfile hither:
wget -N -nv "${GIT_URL_BASE}"/gwc/.vim/spell/en.utf-8.add
mkdir ~/.vim/spell
mv en.utf-8.add ~/.vim/spell/en.utf-8.add
mkdir /home/"${NORMAL_USER}"/.vim/spell
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim/spell
cp -a ~/.vim/spell/en.utf-8.add /home/"${NORMAL_USER}"/.vim/spell/en.utf-8.add
chown "${NORMAL_USER}":"${NORMAL_GROUP}" /home/"${NORMAL_USER}"/.vim/spell/en.utf-8.add
# and then (imperatively) run this command:
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'
# which will be repeated later for the normal user.
# END   ./lmi_setup_09.sh

# Enable stable and security upgrades--see:
#    https://www.debian.org/releases/stretch/amd64/apds03.html.en#idm4504
# for a 'stretch' example, and these messages:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00046.html
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00047.html
# for the '-security' line and for the reason why https is not used.

cat >/etc/apt/sources.list <<EOF
deb http://deb.debian.org/debian/ ${CODENAME} main
deb http://deb.debian.org/debian/ ${CODENAME}-updates main
deb http://security.debian.org/ ${CODENAME}-security main
EOF

# Apply any available upgrades:

apt-get --assume-yes update
apt-get --assume-yes dist-upgrade
