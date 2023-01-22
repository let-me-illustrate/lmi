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
