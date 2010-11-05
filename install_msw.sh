#!/bin/sh

# For msw, download and build lmi and required libraries.

# Copyright (C) 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

# $Id$

set -v

# To get the latest version of this script:
#
# wget -N 'http://svn.sv.nongnu.org/svn/lmi/lmi/trunk/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi
#
# Downloaded archives are kept in /cache_for_lmi/downloads/ because
# they are costly to download and some host might be temporarily
# unavailable.

date -u +'%Y%m%dT%H%MZ'

mkdir --parents /cache_for_lmi/downloads

mount

# Establish mounts carefully.
#
# A command such as
#   mkdir --parents /cygdrive/c/opt/lmi/src/lmi
# has the perverse effect of creating 'C:\cygwin\cygdrive'. To avoid
# that problem, '/opt' is first mounted, then unmounted.
#
# Don't remove the '/opt' directory from Cygwin's filesystem. Other
# programs may want to use it. Furthermore, if it were removed, then
# shell completion, e.g., '/op' [tab], wouldn't work.

# The 'mount' command differs greatly between Cygwin versions.
#
# Cygwin-1.5 stores all mounts in the msw registry:
#   HKLM for 'system' mounts
#   HKCU for 'user' mounts
# 'mount' adds 'system' mounts by default.
# 'mount --mount-commands' gives commands to replicate all mounts.
#
# Cygwin-1.7 stores all permanent mounts in files:
#   /etc/fstab for 'system' mounts
#   /etc/fstab.d/[user-name] for 'user' mounts
# 'mount' adds temporary mounts that vanish when the session ends; it
#   does not affect 'system' mounts (unless 'override' is specified,
#   which is never done here).
# 'mount --mount-commands' is invalid; errors resulting from this
#   obsolete option are discarded here.
#
# Regardless of version, only system mounts are wanted here, and they
# are never overridden.

restore_opt_mount=`mount --mount-commands 2>/dev/null | grep '"/opt"'`

umount "/opt"
umount "/opt/lmi"
mkdir /opt
mount --force "C:/opt" "/opt"
mkdir --parents /opt/lmi/src/lmi
mkdir --parents /opt/lmi/share/doc/lmi
umount "/opt"
mount --force "C:/opt/lmi" "/opt/lmi"

[ -z "$restore_opt_mount" ] || sh -c $restore_opt_mount

cygcheck -s -v -r | tr --delete '\r'

java -version

# Avoid bogus "failed to open /home/wherever/.cvspass for reading".

touch ~/.cvspass

# Use 'pserver' unless 'ssh-agent' is active. This helps developers
# who are behind a firewall that blocks port 2401 but allows 'ssh'
# access. It is assumed that they have pointed $CVSROOT to
#   cvs.sv.gnu.org:/sources/lmi
# as is usual, and that they will switch back there manually after
# running this script.
#
# As a last resort, a screen scraper could be used, e.g.
#   http://downloads.sourceforge.net/cvsgrab/cvsgrab-2.3.tar.gz
# with a command line such as this:
#   /cvsgrab-2.3/cvsgrab.sh \
#     -url http://cvs.sv.gnu.org/viewvc/lmi/lmi/ \
#     -destDir cvsgrab \
#     -webInterface ViewVC1_0
# Unfortunately, that 'cvsgrab' command adds '-kb' to every file, so
# developers should take care not to use it.

export CVS_RSH="ssh"
echo $CVSROOT
ps -ef | grep --quiet ssh-agent \
  || export CVSROOT=":pserver:anonymous@cvs.savannah.nongnu.org:/sources/lmi"

echo $CVSROOT
cd /opt/lmi/src
# Prefer http to svn's own protocol because it works better from
# behind corporate firewalls.
#svn checkout svn://svn.sv.nongnu.org/lmi/lmi/trunk lmi
svn checkout http://svn.sv.nongnu.org/svn/lmi/lmi/trunk lmi

export CVSROOT=${CVSROOT%%/sources/lmi}/web/lmi
echo $CVSROOT
cd /opt/lmi/share/doc
# Not yet available via svn...
cvs -z3 checkout -P lmi

cd /opt/lmi/src/lmi

# A "Replacing former [...] mount:" message probably means that this
# mount was set by an earlier lmi installation; that can be ignored.
# It seems quite unlikely that anyone who's building lmi would have
# any other need for mounts with the names used here.

restore_MinGW_mount=`mount --mount-commands 2>/dev/null | grep '"/MinGW_"'`
[ -z "$restore_MinGW_mount" ] \
  || echo $restore_MinGW_mount | grep --silent '"C:/opt/lmi/MinGW-20090203"' \
  || echo -e "Replacing former MinGW_ mount:\n $restore_MinGW_mount" >/dev/tty
mount --force "C:/opt/lmi/MinGW-20090203" "/MinGW_"

restore_cache_mount=`mount --mount-commands 2>/dev/null | grep '"/cache_for_lmi"'`
[ -z "$restore_cache_mount" ] \
  || echo $restore_cache_mount | grep --silent '"C:/cache_for_lmi"' \
  || echo -e "Replacing former cache mount:\n  $restore_cache_mount" >/dev/tty
mount --force "C:/cache_for_lmi" "/cache_for_lmi"

mount

md5sum $0
find /cache_for_lmi/downloads -type f | xargs md5sum

# $coefficiency is intended for passing a '--jobs' option, e.g.
#   export coefficiency='--jobs=12'

rm --force --recursive scratch
rm --force --recursive /MinGW_
make $coefficiency prefix=/MinGW_ cache_dir=/cache_for_lmi/downloads -f install_mingw.make

make $coefficiency -f install_miscellanea.make clobber
make $coefficiency -f install_miscellanea.make

make $coefficiency -f install_libxml2_libxslt.make

make $coefficiency -f install_wx.make

find /cache_for_lmi/downloads -type f | xargs md5sum

export         PATH=/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH
export minimal_path=/opt/lmi/local/bin:/opt/lmi/local/lib:/usr/bin:/bin:/usr/sbin:/sbin

make $coefficiency PATH=$minimal_path wx_config_check
make $coefficiency PATH=$minimal_path show_flags
make $coefficiency PATH=$minimal_path install

# No lmi binary should depend on any Cygwin library.

for z in /opt/lmi/bin/*; \
  do cygcheck $z 2>&1 | grep --silent cygwin \
    && echo -e "\ncygcheck $z" && cygcheck $z; \
  done

echo -n "2450449 2458849"                          >/opt/lmi/data/expiry
echo    "0efd124fac6b15e6a9cd0b3dd718eea5  expiry" >/opt/lmi/data/validated.md5
echo -n "8fa614e38dde6f7ab0f9fade87dfa2e3"         >/opt/lmi/data/passkey

cat >/opt/lmi/bin/configurable_settings.xml <<EOF
<?xml version="1.0"?>
<configurable_settings>
  <calculation_summary_columns/>
  <cgi_bin_log_filename>cgi_bin.log</cgi_bin_log_filename>
  <custom_input_filename>sample.ini</custom_input_filename>
  <custom_output_filename>sample.out</custom_output_filename>
  <default_input_filename>/etc/opt/lmi/default.ill</default_input_filename>
  <libraries_to_preload/>
  <offer_hobsons_choice>0</offer_hobsons_choice>
  <print_directory>.</print_directory>
  <skin_filename>skin.xrc</skin_filename>
  <spreadsheet_file_extension>.tsv</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
  <xsl_fo_command>CMD /c /opt/lmi/third_party/fop-0.20.5/fop</xsl_fo_command>
</configurable_settings>
EOF

date -u +'%Y%m%dT%H%MZ'

echo Finished building lmi. >/dev/tty

