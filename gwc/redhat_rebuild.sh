#!/bin/sh

set -evx

# Before running this script, ensure that the proprietary repository
# maintained outside any chroot is up to date--i.e., that all changes
# committed in any earlier chroot have been pushed.

case "$(cat /proc/version)" in
    (*"Red Hat"*)
                    echo 'check_certificate = off' >> ~/.wgetrc
        sudo sh -c "echo 'check_certificate = off' >> ~/.wgetrc"
        ;;
    (*)
        ;;
esac

cd /srv/cache_for_lmi/logs || printf 'Cannot cd\n'
wget -N -nv https://git.savannah.nongnu.org/cgit/lmi.git/plain/lmi_setup_00.sh
# wget -N -nv https://github.com/let-me-illustrate/lmi/raw/master/lmi_setup_00.sh
chmod +x lmi_setup_00.sh
# It would be undesirable for root to own the output file:
# shellcheck disable=SC2024
sudo ./lmi_setup_00.sh >log_"$(date -u +"%Y%m%dT%H%MZ")" 2>&1

# This script was originally intended for use only on a corporate
# server whose administrators give lmi developers NOPASSWD access
# in '/etc/sudoers'. On a personal debian system, if that access
# is given to the current user, then this script may be run thus:
#   $ rm nohup.out; nohup /wherever/gwc/redhat_rebuild.sh & disown; exit
# where '/wherever/' is some up-to-date git directory, e.g.:
#   /srv/chroot/lmi_bookworm_5x/opt/lmi/src/lmi/
# in a freshly-opened terminal for testing.
