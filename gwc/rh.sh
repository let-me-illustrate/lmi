#!/bin/do_not_run_this_as_a_script

# Notes

# * Root access

# Both Kim and I have been added to the 'sudoers' group. Normally,
# on my own machine I just do

#  /home/greg[0]$su
#  /root[0]#echo $USER
#  greg

# which runs /root/.zshrc, which is the same as /home/greg/.zshrc

# Maybe instead I should do

#  /home/greg[0]$su -
#  /root[0]#echo $USER
#  root

# which runs .zprofile as well; someone online said that `su`
# "impersonates" root, but with `su -` you "become" root, FWIW

# We've been running
#   sudo su -
# on this server, which is good enough.

# Needed to do this:
# %s,https://git.savannah.nongnu.org/cgit/lmi.git/plain,https://github.com/let-me-illustrate/lmi/raw/master,
# for a corporate server that enables github but, incomprehensibly, blocks nongnu.org

# * Multi-user access

# Set up an 'lmi' group...
sudo groupadd lmi
sudo usermod -a -G lmi ${USER}
# if root already owns /opt as is likely:
chown root:sudo /opt/lmi
chmod g+w /opt/lmi
ls -dl /opt/lmi

#
sudo yum-config-manager --enable rhel-server-rhscl-7-rpms
sudo yum install rh-git218
scl enable rh-git218 bash

# from lmi_setup_21.sh [do this as root]

# Repair /usr/share/libtool/.../ltmain.sh as indicated here:
#   https://lists.gnu.org/archive/html/libtool-patches/2011-06/msg00001.html
# Do this as root because root owns the file.

cat >/home/${USER}/ltmain.sh.patch <<EOF
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

patch --dry-run --strip=0 </home/${USER}/ltmain.sh.patch \
 && patch --strip=0 </home/${USER}/ltmain.sh.patch

# A related matter: use 'cp' here because it's too much work to
# edit every occurrence of the debian filepath throughout lmi.
# See:
#   https://lists.nongnu.org/archive/html/lmi/2019-09/msg00035.html

sudo cp -a /usr/share/libtool/config/config.guess /usr/share/misc/config.guess

# Configure zsh, for root as well as the user configured above.

wget -N -nv 'https://github.com/let-me-illustrate/lmi/raw/master/gwc/.zshrc'
mv .zshrc ~
cp -a ~/.zshrc /home/${USER}/.zshrc
chown ${USER}:${USER} /home/${USER}/.zshrc

# Configure vim. Rather than trying to split its contents between
# '~/.vimrc' and '/etc/vim/vimrc.local', use '~/.vimrc' for all
# customizations and copy that file for the normal user too.

wget -N -nv 'https://github.com/let-me-illustrate/lmi/raw/master/gwc/.vimrc'
mv .vimrc ~
cp -a ~/.vimrc /home/${USER}/.vimrc
chown ${USER}:${USER} /home/${USER}/.vimrc

# Without this, 'zg' gives an error message; with it, vim creates a
# spellfile the first time 'zg' is used, if none already exists.
mkdir ~/.vim
mkdir /home/${USER}/.vim
chown ${USER}:${USER} /home/${USER}/.vim
# It's a much better idea to copy a mature spellfile hither:
wget -N -nv 'https://github.com/let-me-illustrate/lmi/raw/master/gwc/.vim/spell/en.utf-8.add'
mkdir ~/.vim/spell
mv en.utf-8.add ~/.vim/spell/en.utf-8.add
mkdir /home/${USER}/.vim/spell
chown ${USER}:${USER} /home/${USER}/.vim/spell
cp -a ~/.vim/spell/en.utf-8.add /home/${USER}/.vim/spell/en.utf-8.add
chown ${USER}:${USER} /home/${USER}/.vim/spell/en.utf-8.add
# and then (imperatively) run this command:
vim -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'
# which will be repeated below in the user chroot.

# from lmi_setup_41.sh [do this as normal user]

# Rebuild vim spellfile (as was done above for root)
vim -es -c ':mkspell! ~/.vim/spell/en.utf-8.add' -c ':q'

# Configure git. See:
#   https://lists.nongnu.org/archive/html/lmi/2016-03/msg00006.html
git config --global color.ui auto
git config --global commit.cleanup scissors
git config --global core.pager "less -+F -+X"
git config --global diff.colormoved plain
git config --global log.date iso8601-strict-local
git config --global log.follow true
git config --global pull.ff only
git config --global push.default simple

# from lmi_setup_42.sh

# Install lmi for wine.

wget -N -nv 'https://github.com/let-me-illustrate/lmi/raw/master/install_msw.sh'
chmod +x install_msw.sh
./install_msw.sh >log 2>&1

# Now everything should work much as it does in native msw. To run an
# msw program, prefix its command line with 'wine'. Test the chroot by
# running the lmi binary built in the preceding step:

pushd /opt/lmi/bin
wine ./lmi_wx_shared.exe --ash_nazg --data_path=../data

# last command fails--wine cannot be configured without X?
# yet 'wineconsole' does work; is that enough?
