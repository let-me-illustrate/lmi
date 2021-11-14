#!/bin/sh

set -evx

# Before running this script, ensure that the proprietary repository
# maintained outside any chroot is up to date. For example, if:
#  - chroot 'lmi_bullseye_3' has a current proprietary repository;
#  - '/srv/cache_for_lmi/blessed/proprietary' is out of date; and
#  - a new chroot is to be created;
# then run these commands:
#   pushd /srv/cache_for_lmi/blessed/proprietary
#   git fetch /srv/chroot/lmi_bullseye_3/opt/lmi/blessed/proprietary master:master
# If, in the current chroot ('lmi_bullseye_3' in the example above),
# the "blessed" repository is significantly behind the current working
# copy, then first of all log into that chroot and do 'git push'.

            echo 'check_certificate = off' >> ~/.wgetrc
sudo sh -c "echo 'check_certificate = off' >> ~/.wgetrc"

cd /srv/cache_for_lmi/logs || printf 'Cannot cd\n'
wget -N -nv https://github.com/let-me-illustrate/lmi/raw/master/lmi_setup_00.sh
chmod +x lmi_setup_00.sh
# It would be undesirable for root to own the output file:
# shellcheck disable=SC2024
sudo ./lmi_setup_00.sh >log_"$(date -u +"%Y%m%dT%H%MZ")" 2>&1
