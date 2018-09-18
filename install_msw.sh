#!/bin/sh

# For msw, download and build lmi and required libraries.

# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

# To get the latest version of this script:
#
# wget -N 'https://git.savannah.gnu.org/cgit/lmi.git/plain/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

# '--jobs=4': big benefit for multicore, no penalty for single core
#   (but don't force it to 4 if it's already set).
# '--output-sync=recurse' is also used, passim, to facilitate log
#   comparison.
if [ -z "$coefficiency" ]
then
    export coefficiency='--jobs=4'
fi

export platform
case $(uname) in
    CYGWIN*)
        platform=CYGWIN
        ;;
esac

if [ "CYGWIN" = "$platform" ]
then
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
    # 'mount --mount-commands' gives commands to replicate all mounts,
    # with 'device|directory' arguments in double quotes; this script
    # no longer supports that obsolete command option.
    #
    # Cygwin-1.7 stores all permanent mounts in files:
    #   /etc/fstab for 'system' mounts
    #   /etc/fstab.d/[user-name] for 'user' mounts
    # 'mount' adds temporary mounts that vanish when the session ends; it
    #   does not affect 'system' mounts (unless 'override' is specified,
    #   which is never done here).
    # 'mount --mount-entries' gives mount arguments to replicate all
    # mounts, with no quotes around arguments (hence the trailing space
    # in the 'grep' commands below).
    #
    # Regardless of version, only system mounts are wanted here, and they
    # are never overridden.

    restore_opt_mount=$(mount --mount-entries | grep '/opt/lmi ')

    umount "/opt"
    umount "/opt/lmi"
    mkdir /opt
    mount --force "C:/opt" "/opt"
    mkdir --parents /opt/lmi/src/lmi
    umount "/opt"
    mount --force "C:/opt/lmi" "/opt/lmi"

    [ -z "$restore_opt_mount" ] || sh -c mount "$restore_opt_mount"

    # Read this entire thread for $CYGCHECK rationale:
    #   https://cygwin.com/ml/cygwin/2012-02/threads.html#00910
    #   https://cygwin.com/ml/cygwin/2012-03/threads.html#00005
    # Cf.:
    #   https://lists.nongnu.org/archive/html/lmi/2016-01/msg00092.html
    CYGCHECK=$(cygpath --mixed /usr/bin/cygcheck)
    export CYGCHECK
    cmd /c "$CYGCHECK" -s -v -r | tr --delete '\r'

    # 'core.fileMode' rationale:
    #   https://lists.nongnu.org/archive/html/lmi/2017-11/msg00018.html

    git config --global core.fileMode false
fi

java -version

mkdir --parents /opt/lmi/src
cd /opt/lmi/src || print "Cannot cd"

# Preserve any preexisting source directory, moving it aside so that
# 'git clone' will install a pristine working copy.

cp --archive lmi lmi-moved-"$stamp0"
rm -rf /opt/lmi/src/lmi

# Use git's own protocol wherever possible. In case that's blocked
# by a corporate firewall, fall back on https. In case a firewall
# inexplicably blocks the gnu.org domain, try Vadim's github clone
# as a last resort.

git clone git://git.savannah.nongnu.org/lmi.git \
  || git clone https://git.savannah.nongnu.org/r/lmi.git \
  || git clone https://github.com/vadz/lmi.git

cd /opt/lmi/src/lmi || print "Cannot cd"

./check_git_setup.sh

if [ "CYGWIN" = "$platform" ]
then
    # A "Replacing former [...] mount:" message probably means that this
    # mount was set by an earlier lmi installation; that can be ignored.
    # It seems quite unlikely that anyone who's building lmi would have
    # any other need for mounts with the names used here.

    restore_MinGW_mount=$(mount --mount-entries | grep '/MinGW_ ')
    [ -z "$restore_MinGW_mount" ] \
      || printf '%s\n' "$restore_MinGW_mount" | grep --silent 'C:/opt/lmi/MinGW-7_3_0' \
      || printf 'Replacing former MinGW_ mount:\n %s\n' "$restore_MinGW_mount" >/dev/tty
    mount --force "C:/opt/lmi/MinGW-7_3_0" "/MinGW_"

    restore_cache_mount=$(mount --mount-entries | grep '/cache_for_lmi ')
    [ -z "$restore_cache_mount" ] \
      || printf '%s\n' "$restore_cache_mount" | grep --silent 'C:/cache_for_lmi' \
      || printf 'Replacing former cache mount:\n  %s\n' "$restore_cache_mount" >/dev/tty
    mount --force "C:/cache_for_lmi" "/cache_for_lmi"
fi

# Downloaded archives are kept in /cache_for_lmi/downloads/ because
# they are costly to download and some host might be temporarily
# unavailable.

mkdir --parents /cache_for_lmi/downloads

mount

md5sum "$0"
find /cache_for_lmi/downloads -type f | xargs md5sum

rm --force --recursive scratch

if [ "CYGWIN" = "$platform" ]
then
    # For Cygwin, install and use this msw-native compiler.
    rm --force --recursive /MinGW_
    make $coefficiency --output-sync=recurse -f install_mingw.make
else
    # For real *nix, set LMI_HOST to specify a cross compiler.
    export LMI_HOST=i686-w64-mingw32
fi

make $coefficiency --output-sync=recurse -f install_miscellanea.make clobber
make $coefficiency --output-sync=recurse -f install_miscellanea.make

make $coefficiency --output-sync=recurse -f install_libxml2_libxslt.make

./install_wx.sh

./install_wxpdfdoc.sh

find /cache_for_lmi/downloads -type f | xargs md5sum

export         PATH=/opt/lmi/local/bin:/opt/lmi/local/lib:$PATH
export minimal_path=/opt/lmi/local/bin:/opt/lmi/local/lib:/usr/bin:/bin:/usr/sbin:/sbin

make $coefficiency --output-sync=recurse PATH=$minimal_path wx_config_check
make $coefficiency --output-sync=recurse PATH=$minimal_path show_flags
make $coefficiency --output-sync=recurse PATH=$minimal_path clean
make $coefficiency --output-sync=recurse PATH=$minimal_path install

if [ "CYGWIN" = "$platform" ]
then
    # No lmi binary should depend on any Cygwin library.

    for z in /opt/lmi/bin/*; \
      do cmd /c "$CYGCHECK $z" 2>&1 | grep --silent cygwin \
        && printf '\ncygcheck %s\n' "$z" && cmd /c "$CYGCHECK $z"; \
      done
fi

# To regenerate authentication files:
# cd /opt/lmi/data
# printf '2450449 2472011'             >expiry
# printf '%s\n' "$(md5sum expiry)"     >validated.md5
# [wine] /opt/lmi/bin/generate_passkey >passkey
printf '2450449 2472011'                            >/opt/lmi/data/expiry
printf '5fc68a795c9c60da1b32be989efc299a  expiry\n' >/opt/lmi/data/validated.md5
printf '391daa5cbc54e118c4737446bcb84eea'           >/opt/lmi/data/passkey

# Tailored to msw; for POSIX, s|C:|| and s|CMD /c|/bin/sh| (e.g.).

cat >/opt/lmi/data/configurable_settings.xml <<EOF
<?xml version="1.0"?>
<configurable_settings version="2">
  <calculation_summary_columns/>
  <census_paste_palimpsestically>1</census_paste_palimpsestically>
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

# Remove "C:" for wine: it designates the "wine prefix" directory.
# "Z:" could be used instead, because that's where wine maps the
# apparent root, but that wouldn't work with posix builds. Instead,
# therefore, symlink the directories lmi uses as described in
# 'README.schroot'.

if [ "CYGWIN" != "$platform" ]
then
    sed -i /opt/lmi/data/configurable_settings.xml -e's/C://g'
fi

# Restore any preexisting source directory that had been preserved
# above, renaming the pristine checkout that had replaced it.
#
# Simply running 'git pull' should make the two directories identical
# except for any local changes or additions, which are presumably not
# to be discarded, and any differences in the '.git' subdirectory,
# which are presumably important to keep.

if [ -d /opt/lmi/src/lmi-moved-"$stamp0" ]
then
cd /opt/lmi/src && mv lmi lmi-new-"$stamp0" && mv lmi-moved-"$stamp0" lmi
fi

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(expr $(date '+%s' -d "$stamp1") - $(date '+%s' -d "$stamp0"))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"

echo Finished building lmi. >/dev/tty

