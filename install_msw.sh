#!/bin/sh

# For msw, download and build lmi and required libraries.

# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

set -v

# To get the latest version of this script:
#
# wget -N 'http://git.savannah.gnu.org/cgit/lmi.git/plain/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

# '--jobs=4': big benefit for multicore, no penalty for single core.
# '--output-sync=recurse' is also used, passim, to facilitate log
# comparison.
export coefficiency='--jobs=4'

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
umount "/opt"
mount --force "C:/opt/lmi" "/opt/lmi"

[ -z "$restore_opt_mount" ] || sh -c $restore_opt_mount

# Read this entire thread for $CYGCHECK rationale:
#   https://cygwin.com/ml/cygwin/2012-02/threads.html#00910
#   https://cygwin.com/ml/cygwin/2012-03/threads.html#00005
# Cf.:
#   https://lists.nongnu.org/archive/html/lmi/2016-01/msg00092.html
export CYGCHECK=`cygpath --mixed /usr/bin/cygcheck`
cmd /c $CYGCHECK -s -v -r | tr --delete '\r'

java -version

cd /opt/lmi/src
# Favor http over git's own protocol only because corporate firewalls
# in lmi's target industry tend to block the latter.
git clone http://git.savannah.nongnu.org/r/lmi.git
# Use git's own wherever possible.
# git clone git://git.savannah.nongnu.org/lmi.git

cd /opt/lmi/src/lmi

# A "Replacing former [...] mount:" message probably means that this
# mount was set by an earlier lmi installation; that can be ignored.
# It seems quite unlikely that anyone who's building lmi would have
# any other need for mounts with the names used here.

restore_MinGW_mount=`mount --mount-commands 2>/dev/null | grep '"/MinGW_"'`
[ -z "$restore_MinGW_mount" ] \
  || echo $restore_MinGW_mount | grep --silent '"C:/opt/lmi/MinGW-4_9_1"' \
  || echo -e "Replacing former MinGW_ mount:\n $restore_MinGW_mount" >/dev/tty
mount --force "C:/opt/lmi/MinGW-4_9_1" "/MinGW_"

restore_cache_mount=`mount --mount-commands 2>/dev/null | grep '"/cache_for_lmi"'`
[ -z "$restore_cache_mount" ] \
  || echo $restore_cache_mount | grep --silent '"C:/cache_for_lmi"' \
  || echo -e "Replacing former cache mount:\n  $restore_cache_mount" >/dev/tty
mount --force "C:/cache_for_lmi" "/cache_for_lmi"

# Downloaded archives are kept in /cache_for_lmi/downloads/ because
# they are costly to download and some host might be temporarily
# unavailable.

mkdir --parents /cache_for_lmi/downloads

mount

md5sum $0
find /cache_for_lmi/downloads -type f | xargs md5sum

rm --force --recursive scratch
rm --force --recursive /MinGW_
make $coefficiency --output-sync=recurse -f install_mingw.make

make $coefficiency --output-sync=recurse -f install_miscellanea.make clobber
make $coefficiency --output-sync=recurse -f install_miscellanea.make

make $coefficiency --output-sync=recurse -f install_libxml2_libxslt.make

make $coefficiency --output-sync=recurse -f install_wx.make

make $coefficiency --output-sync=recurse -f install_wxpdfdoc.make

find /cache_for_lmi/downloads -type f | xargs md5sum

export         PATH=/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH
export minimal_path=/opt/lmi/local/bin:/opt/lmi/local/lib:/usr/bin:/bin:/usr/sbin:/sbin

make $coefficiency --output-sync=recurse PATH=$minimal_path wx_config_check
make $coefficiency --output-sync=recurse PATH=$minimal_path show_flags
make $coefficiency --output-sync=recurse PATH=$minimal_path install

# No lmi binary should depend on any Cygwin library.

for z in /opt/lmi/bin/*; \
  do cmd /c $CYGCHECK $z 2>&1 | grep --silent cygwin \
    && echo -e "\ncygcheck $z" && cmd /c $CYGCHECK $z; \
  done

echo -n "2450449 2458849"                          >/opt/lmi/data/expiry
echo    "0efd124fac6b15e6a9cd0b3dd718eea5  expiry" >/opt/lmi/data/validated.md5
echo -n "8fa614e38dde6f7ab0f9fade87dfa2e3"         >/opt/lmi/data/passkey

# Tailored to msw; for POSIX, s|C:|| and s|CMD /c|/bin/sh| (e.g.).

cat >/opt/lmi/data/configurable_settings.xml <<EOF
<?xml version="1.0"?>
<configurable_settings version="2">
  <calculation_summary_columns/>
  <cgi_bin_log_filename>cgi_bin.log</cgi_bin_log_filename>
  <custom_input_0_filename>custom.ini</custom_input_0_filename>
  <custom_input_1_filename>custom.inix</custom_input_1_filename>
  <custom_output_0_filename>custom.out0</custom_output_0_filename>
  <custom_output_1_filename>custom.out1</custom_output_1_filename>
  <default_input_filename>C:/etc/opt/lmi/default.ill</default_input_filename>
  <libraries_to_preload/>
  <offer_hobsons_choice>0</offer_hobsons_choice>
  <print_directory>C:/opt/lmi/bin</print_directory>
  <seconds_to_pause_between_printouts>10</seconds_to_pause_between_printouts>
  <skin_filename>skin.xrc</skin_filename>
  <spreadsheet_file_extension>.tsv</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
  <xsl_fo_command>CMD /c C:/opt/lmi/third_party/fop-0.20.5/fop</xsl_fo_command>
</configurable_settings>
EOF

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(expr $(date '+%s' -d $stamp1) - $(date '+%s' -d $stamp0))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"

echo Finished building lmi. >/dev/tty

