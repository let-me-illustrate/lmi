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
# wget -N 'http://cvs.savannah.gnu.org/viewvc/*checkout*/lmi/lmi/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi
#
# Downloaded archives are left in /tmp/lmi_cache because they are
# costly to download and some host might be temporarily unavailable.

date -u +'%Y%m%dT%H%MZ'

md5sum $0
find /tmp/lmi_cache -type f |xargs md5sum

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

restore_opt_mount=`mount --mount-commands |grep '"/opt"'`

umount "/opt"
umount "/opt/lmi"
mkdir /opt
mount --force "C:/opt" "/opt"
mkdir --parents /opt/lmi/src/lmi
mkdir --parents /opt/lmi/share/doc/lmi
umount "/opt"
mount --force "C:/opt/lmi" "/opt/lmi"

[ -z "$restore_opt_mount" ] || sh -c $restore_opt_mount

mkdir --parents /tmp/lmi_cache

cygcheck -s -v -r

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
cvs -z3 checkout -P lmi

export CVSROOT=${CVSROOT%%/sources/lmi}/web/lmi
echo $CVSROOT
cd /opt/lmi/share/doc
cvs -z3 checkout -P lmi

cd /opt/lmi/src/lmi

# A "Replacing former MinGW_ mount:" message probably means that this
# mount was set by an earlier lmi installation; that can be ignored.
# It seems quite unlikely that anyone who's building lmi would have
# any other need for a mount with that name.

restore_MinGW_mount=`mount --mount-commands |grep '"/MinGW_"'`
[ -z "$restore_MinGW_mount" ] \
  || echo $restore_MinGW_mount |grep --silent '"C:/opt/lmi/MinGW-20090203"' \
  || echo -e "Replacing former MinGW_ mount:\n  $restore_MinGW_mount" >/dev/tty
mount --force "C:/opt/lmi/MinGW-20090203" "/MinGW_"
rm --force --recursive scratch
rm --force --recursive /MinGW_
make prefix=/MinGW_ cache_dir=/tmp/lmi_cache -f install_mingw.make

make -f install_miscellanea.make clobber
make -f install_miscellanea.make

make -f install_libxml2_libxslt.make

make -f install_wx.make

find /tmp/lmi_cache -type f |xargs md5sum

export         PATH=/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH
export minimal_path=/opt/lmi/local/bin:/opt/lmi/local/lib:/usr/bin:/bin:/usr/sbin:/sbin

make PATH=$minimal_path wx_config_check
make PATH=$minimal_path show_flags
make PATH=$minimal_path install

# No lmi binary should depend on any Cygwin library.

for z in /opt/lmi/bin/*; \
  do cygcheck $z 2>&1 |grep --silent cygwin \
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
  <spreadsheet_file_extension>.dif</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
  <xsl_fo_command>CMD /c /opt/lmi/third_party/fop-0.20.5/fop</xsl_fo_command>
</configurable_settings>
EOF

date -u +'%Y%m%dT%H%MZ'

echo Finished building lmi. >/dev/tty

