#!/bin/sh

cd /srv/cache_for_lmi/logs
wget -nv https://github.com/vadz/lmi/raw/master/lmi_setup_00.sh
chmod +x lmi_setup_00.sh
sudo ./lmi_setup_00.sh >log_$(date -u +"%Y%m%dT%H%MZ" -d "$stamp0") 2>&1
