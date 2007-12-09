#!/bin/sh

# For msw, download and build lmi and required libraries.

# Copyright (C) 2007 Gregory W. Chicares.
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
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: install_msw.sh,v 1.6 2007-12-09 22:08:16 chicares Exp $

set -v

# To get the latest version of this script:
#
# wget -N 'http://cvs.savannah.gnu.org/viewvc/*checkout*/lmi/lmi/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi /cygdrive/c/var/opt/lmi
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
umount "/opt"
mount --force "C:/opt/lmi" "/opt/lmi"

[ -z "$restore_opt_mount" ] || sh -c $restore_opt_mount

mkdir --parents /tmp/lmi_cache

cygcheck -s -v -r

java -version

cd /opt/lmi/src

# Avoid bogus "failed to open /home/wherever/.cvspass for reading".

touch ~/.cvspass

export CVS_RSH="ssh"
export CVSROOT=":pserver:anonymous@cvs.savannah.nongnu.org:/sources/lmi"
cvs -z3 checkout -P lmi

cd /opt/lmi/src/lmi

# A "Replacing former MinGW_ mount:" message probably means that this
# mount was set by an earlier lmi installation; that can be ignored.
# It seems quite unlikely that anyone who's building lmi would have
# any other need for a mount with that name.

restore_MinGW_mount=`mount --mount-commands |grep '"/MinGW_"'`
[ -z "$restore_MinGW_mount" ] \
  || echo $restore_MinGW_mount |grep --silent '"C:/opt/lmi/MinGW-20050827"' \
  || echo -e "Replacing former MinGW_ mount:\n  $restore_MinGW_mount" >/dev/tty
mount --force "C:/opt/lmi/MinGW-20050827" "/MinGW_"
rm --force --recursive scratch
rm --force --recursive /MinGW_
make prefix=/MinGW_ cache_dir=/tmp/lmi_cache -f install_mingw.make

make -f install_miscellanea.make clobber
make -f install_miscellanea.make

make -f install_libxml2_libxslt.make

make -f install_wx.make

find /tmp/lmi_cache -type f |xargs md5sum

export PATH=/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH

make system_root= PATH_GCC=/MinGW_/bin/ mingw_dir=/MinGW_ wx_dir=/opt/lmi/wx-scratch/wxWidgets-2.8.6/gcc344/ wx_build_dir=/opt/lmi/local/bin wx_config_check
make system_root= PATH_GCC=/MinGW_/bin/ mingw_dir=/MinGW_ wx_dir=/opt/lmi/wx-scratch/wxWidgets-2.8.6/gcc344/ wx_build_dir=/opt/lmi/local/bin show_flags
make system_root= PATH_GCC=/MinGW_/bin/ mingw_dir=/MinGW_ wx_dir=/opt/lmi/wx-scratch/wxWidgets-2.8.6/gcc344/ wx_build_dir=/opt/lmi/local/bin install

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
  <print_directory>/var/opt/lmi/spool</print_directory>
  <skin_filename>xml_notebook.xrc</skin_filename>
  <spreadsheet_file_extension>.dif</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
  <xsl_fo_command>CMD /c /opt/lmi/third_party/fop-0.20.5/fop</xsl_fo_command>
</configurable_settings>
EOF

# TODO ?? The program shouldn't require this directory to exist.
pushd /cygdrive/c; mkdir --parents var/opt/lmi/spool; popd

date -u +'%Y%m%dT%H%MZ'

echo Finished building lmi. >/dev/tty

