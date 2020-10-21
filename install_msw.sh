#!/bin/sh

# For msw, download and build lmi and required libraries.

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
# wget -N 'https://git.savannah.nongnu.org/cgit/lmi.git/plain/install_msw.sh'

# To remove lmi prior to reinstalling with this script:
#
# rm --force --recursive /opt/lmi

stamp0=$(date -u +'%Y-%m-%dT%H:%M:%SZ')
echo "Started: $stamp0"

lmi_build_type=$(/usr/share/misc/config.guess)

# This should work with a rather minimal path.

minimal_path=${MINIMAL_PATH:-"/usr/bin:/bin:/usr/sbin:/sbin"}

case "$lmi_build_type" in
    (*-*-cygwin*)
        minimal_path="$minimal_path:$(cygpath --sysdir)"
        java -version
        ;;
    (*)
        java -version
        ;;
esac

export PATH="$minimal_path"

# '--jobs=': big benefit for multicore (but can be overridden).
# '--output-sync=recurse' is used passim to facilitate log comparison.
if [ -z "$coefficiency" ]
then
    coefficiency="--jobs=$(nproc)"
    export coefficiency
fi

case "$lmi_build_type" in
    (*-*-cygwin*)
        platform=Cygwin
        ;;
esac

# 'config.guess' might indicate something like "x86_64-pc-wsl"
# someday, but until then:
case "$(uname -r)" in
    (*Microsoft*)
        platform=WSL
        ;;
esac

if [ "Cygwin" = "$platform" ]
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
    cmd /c "$CYGCHECK" -s -v -r | tr --delete '\r'

    # 'core.fileMode' rationale:
    #   https://lists.nongnu.org/archive/html/lmi/2017-11/msg00018.html

    git config --global core.fileMode false
fi

if [ "WSL" = "$platform" ]
then
    # Install and upgrade all packages if any is missing.
    packages_list='autoconf automake bsdtar curl dos2unix doxygen
      gdb git libgtk-3-dev libtool make patch pkg-config rsync unzip wget
      zip zsh g++-mingw-w64-i686'

    # Disable shellcheck warning about the need to double quote $packages_list:
    # it can't be done here and we really want word splitting to happen here.
    # shellcheck disable=SC2086
    missing_packages_count=$(dpkg-query -W -f='${Status}\n' $packages_list 2>&1 | \
      grep -v -c 'install ok installed')

    if [ "$missing_packages_count" -gt 0 ]
    then
        sudo apt update
        # shellcheck disable=SC2086
        sudo apt install -y $packages_list
    fi

    # Mount /opt/lmi and /srv/cache_for_lmi directories.
    # Unfortunately sudo must be used because we don't have permissions to
    # write to /opt and / directories.
    mkdir --parents /mnt/c/opt/lmi/src/lmi
    restore_lmi_mount=$(mount | grep '/opt/lmi')
    if [ -z "$restore_lmi_mount" ]
    then
        [ ! -d /opt/lmi ] && sudo mkdir /opt/lmi
        sudo mount --bind /mnt/c/opt/lmi /opt/lmi
    fi
    mkdir --parents /mnt/c/srv/cache_for_lmi
    restore_cache_mount=$(mount | grep '/srv/cache_for_lmi')
    if [ -z "$restore_cache_mount" ]
    then
        [ ! -d /srv/cache_for_lmi ] && sudo mkdir /srv/cache_for_lmi
        sudo mount --bind /mnt/c/srv/cache_for_lmi /srv/cache_for_lmi
    fi
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

if [ "Cygwin" = "$platform" ]
then
    # A "Replacing former [...] mount:" message probably means that this
    # mount was set by an earlier lmi installation; that can be ignored.
    # It seems quite unlikely that anyone who's building lmi would have
    # any other need for mounts with the names used here.

    restore_cache_mount=$(mount --mount-entries | grep '/srv/cache_for_lmi ')
    [ -z "$restore_cache_mount" ] \
      || printf '%s\n' "$restore_cache_mount" | grep --silent 'C:/srv/cache_for_lmi' \
      || printf 'Replacing former cache mount:\n  %s\n' "$restore_cache_mount" >/dev/tty
    mount --force "C:/srv/cache_for_lmi" "/srv/cache_for_lmi"
fi

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
triplets="x86_64-w64-mingw32 i686-w64-mingw32"
if [ "Cygwin" != "$platform" ] && [ "WSL" != "$platform" ]
then
    triplets="x86_64-pc-linux-gnu x86_64-w64-mingw32 i686-w64-mingw32"
fi
export LMI_COMPILER=gcc
export LMI_TRIPLET
# shellcheck disable=SC2043
#for LMI_TRIPLET in i686-w64-mingw32 ;
for LMI_TRIPLET in ${triplets} ;
do
    # Set a minimal path for makefiles and scripts that are
    # designed to be independent of lmi's runtime path.
    export PATH="$minimal_path"

    # For Cygwin or WSL, install and use this msw-native compiler.
    # Install it for other build types, too, even if only for
    # validating the installation procedure.
    mingw_dir=/opt/lmi/${LMI_COMPILER}_${LMI_TRIPLET}/gcc_msw
    [ -d "$mingw_dir" ] && rm --force --recursive "$mingw_dir"
    if   [ "i686-w64-mingw32"   = "$LMI_TRIPLET" ]; then
      make "$coefficiency" --output-sync=recurse -f install_mingw32.make
    elif [ "x86_64-w64-mingw32" = "$LMI_TRIPLET" ]; then
      make "$coefficiency" --output-sync=recurse -f install_mingw.make
    else
      printf 'No MinGW compiler for this triplet.\n'
    fi

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

    if [ "Cygwin" = "$platform" ]
    then
        # No lmi binary should depend on any Cygwin library.
        for z in /opt/lmi/bin/* ;
          do cmd /c "$CYGCHECK $z" 2>&1 | grep --silent cygwin \
            && printf '\ncygcheck %s\n' "$z" && cmd /c "$CYGCHECK $z" ;
          done
    fi
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
root_name=C:

# Don't use "C:" for wine: it designates the "wine prefix" directory.
# "Z:" could be used instead, because that's where wine maps the
# apparent root, but that wouldn't work with posix builds. Instead,
# therefore, symlink the directories lmi uses as described in
# 'README.schroot'.

if [ "Cygwin" != "$platform" ] && [ "WSL" != "$platform" ]
then
    root_name=
fi

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
