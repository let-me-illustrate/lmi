#!/bin/sh

# For posix, download and build lmi and required libraries.

# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

set -vx

# To get the latest version of this script:
#
# wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/install_posix.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

lmi_build_type=$(/usr/share/libtool/build-aux/config.guess)

# This should work with a rather minimal path.

minimal_path=${MINIMAL_PATH:-"/usr/bin:/bin:/usr/sbin:/sbin"}

export PATH="$minimal_path"

# '--jobs=': big benefit for multicore (but can be overridden).
# '--output-sync=recurse' is used passim to facilitate log comparison.
if [ -z "$coefficiency" ]
then
    coefficiency="--jobs=$(nproc)"
    export coefficiency
fi

# Install and upgrade all packages if any is missing.
packages_list='autoconf automake bsdtar curl dos2unix doxygen
  gdb git libgtk-3-dev libtool make patch pkg-config rsync unzip wget
  zip zsh'

# Disable shellcheck warning about the need to double quote $packages_list:
# it can't be done here and we really want word splitting to happen here.
# shellcheck disable=SC2086
missing_packages_count=$(dpkg-query -W -f='${Status}\n' $packages_list 2>&1 | \
  grep -v -c 'install ok installed')

if [ "$missing_packages_count" -gt 0 ]
then
    echo "$missing_packages_count"
    # For now at least, only show the commands, without running them:
    echo sudo apt update
    # shellcheck disable=SC2086
    echo sudo apt install -y $packages_list
fi

if [ "/opt/lmi/src/lmi" = "$PWD" ]
then
    inhibit_git_clone=1
    printf 'Running in lmi srcdir, so inhibiting git clone.\n'
    printf 'Eviscerating...\n'
    make eviscerate || true
fi

mkdir --parents /opt/lmi/src
cd /opt/lmi/src || printf 'Cannot cd\n'

# Set 'inhibit_git_clone=1' to test uncommitted changes.
if [ "$inhibit_git_clone" != 1 ]
then
    # Preserve any preexisting source directory, moving it aside so
    # that 'git clone' will install a pristine working copy.

    cp --archive lmi lmi-moved-"$stamp0"
    rm -rf /opt/lmi/src/lmi

    # Use git's own protocol wherever possible. In case that's blocked
    # by a corporate firewall, fall back on https. In case a firewall
    # inexplicably blocks the gnu.org domain, try Vadim's github clone
    # as a last resort.
    clone_opts="$coefficiency --recurse-submodules"
    # 'git clone' options must not be double-quoted
    # shellcheck disable=SC2086
    git clone      $clone_opts git://git.savannah.nongnu.org/lmi.git \
      || git clone $clone_opts https://git.savannah.nongnu.org/r/lmi.git \
      || git clone $clone_opts https://github.com/let-me-illustrate/lmi.git
fi

cd /opt/lmi/src/lmi || printf 'Cannot cd\n'

./check_git_setup.sh

# Get any new submodules that may have been added, even if nested.
git submodule update "$coefficiency" --recursive --init

# Downloaded archives are kept in /srv/cache_for_lmi/downloads/ because
# they are costly to download and some host might be temporarily
# unavailable.

mkdir --parents /srv/cache_for_lmi/downloads

mount

md5sum "$0"
find /srv/cache_for_lmi/downloads -type f -print0 | xargs --null md5sum

make "$coefficiency" --output-sync=recurse -f install_miscellanea.make clobber
make "$coefficiency" --output-sync=recurse -f install_miscellanea.make

# This for-loop can iterate over as many toolchains as desired.
# Make sure the current production architecture is built last, so that
# it's the one installed to /opt/lmi/bin/ when this script ends.
export LMI_COMPILER=gcc
export LMI_TRIPLET
# shellcheck disable=SC2043
for LMI_TRIPLET in x86_64-pc-linux-gnu ;
do
    # Set a minimal path for makefiles and scripts that are
    # designed to be independent of lmi's runtime path.
    export PATH="$minimal_path"

    ./install_xml_libraries.sh

    ./install_wx.sh
    ./install_wxpdfdoc.sh

    find /srv/cache_for_lmi/downloads -type f -print0 | xargs --null md5sum

    # Source this script only for commands that depend upon it.
    . ./set_toolchain.sh

    make "$coefficiency" --output-sync=recurse wx_config_check
    make "$coefficiency" --output-sync=recurse show_flags
    make "$coefficiency" --output-sync=recurse clean
    make "$coefficiency" --output-sync=recurse uninstall
    make "$coefficiency" --output-sync=recurse install
done

# GID should be the same for all files.
find /opt/lmi/ -not -group "$(id -gn "$(logname)")" -print
# User and group write permissions should be the same.
find /opt/lmi -perm -200 -not -perm -020 -print0 | xargs --no-run-if-empty -0 ls -ld
# Show all distinct file modes. Expect something like:
#   00444 regular file
#   00555 regular file
#   00664 regular empty file
#   00664 regular file
#   00775 regular empty file
#   00775 regular file
#   00777 symbolic link
#   02775 directory
# User and group permissions differ (multi-user-ness is impaired)
# if the digits in the fourth and fifth columns differ. The second
# column should contain '2' for all directories.
find /opt/lmi/ -print0 | xargs -0 stat -c "%05a %F" | sort -u
# Ad hoc repairs--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-05/msg00001.html
# chgrp -R lmi /opt/lmi
# find /opt/lmi -type d -exec chmod g+s {} +
# chmod -R g=u /opt/lmi

mkdir --parents /opt/lmi/data

# To regenerate authentication files for production distributions:
# cd /opt/lmi/data
# printf '2450449 2472011'             >expiry
# printf '%s\n' "$(md5sum expiry)"     >validated.md5
# [wine] /opt/lmi/bin/generate_passkey >passkey
printf '2450449 2472011'                            >/opt/lmi/data/expiry
printf '5fc68a795c9c60da1b32be989efc299a  expiry\n' >/opt/lmi/data/validated.md5
printf '391daa5cbc54e118c4737446bcb84eea'           >/opt/lmi/data/passkey

# Copy proprietary graphics if available; else use generic surrogates.
graphics_dir_0=/opt/lmi/proprietary/graphics
graphics_dir_1=/opt/lmi/src/lmi/gwc
for z in company_logo.png group_quote_banner.png ; do
  if   [ -f "$graphics_dir_0"/$z ]
    then install -m 0664 "$graphics_dir_0"/$z /opt/lmi/data/
  elif [ -f "$graphics_dir_1"/$z ]
    then install -m 0664 "$graphics_dir_1"/$z /opt/lmi/data/
  else
    printf 'Graphics files not found.\n'
  fi
done

# Configurable settings.

mkdir --parents /opt/lmi/print

# Like std::filesystem::root_name().
root_name=

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
  <default_input_filename>${root_name}/etc/opt/lmi/default.ill</default_input_filename>
  <libraries_to_preload/>
  <offer_hobsons_choice>0</offer_hobsons_choice>
  <print_directory>${root_name}/opt/lmi/print</print_directory>
  <seconds_to_pause_between_printouts>10</seconds_to_pause_between_printouts>
  <skin_filename>skin.xrc</skin_filename>
  <spreadsheet_file_extension>.tsv</spreadsheet_file_extension>
  <use_builtin_calculation_summary>1</use_builtin_calculation_summary>
</configurable_settings>
EOF

# Restore any preexisting source directory that had been preserved
# above, renaming the pristine checkout that had replaced it.
#
# Simply running 'git pull' should make the two directories identical
# except for any local changes or additions, which are presumably not
# to be discarded, and any differences in the '.git' subdirectory,
# which are presumably important to keep.

if [ "$inhibit_git_clone" != 1 ]
then
    if [ -d /opt/lmi/src/lmi-moved-"$stamp0" ]
    then
    cd /opt/lmi/src && mv lmi lmi-new-"$stamp0" && mv lmi-moved-"$stamp0" lmi
    fi
fi

stamp1=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Finished: $stamp1"

seconds=$(($(date '+%s' -d "$stamp1") - $(date '+%s' -d "$stamp0")))
elapsed=$(date -u -d @"$seconds" +'%H:%M:%S')
echo "Elapsed: $elapsed"

echo Finished building lmi. >/dev/tty
