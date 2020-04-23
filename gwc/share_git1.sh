#!/bin/sh

# Attempt to set up a git repository to be shared by multiple users.
#
# FETCH_HEAD doesn't get the right permissions, and that's a git
# defect--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-03/msg00016.html
#   https://public-inbox.org/git/20200319010321.18614-1-vz-git@zeitlins.org/T/#u

# Like 'share_git2.sh', but creates a bare repository.

set -v

# Start with a fresh throwaway directory.
cd /tmp || exit
rm -rf /tmp/eraseme
mkdir -p /tmp/eraseme
cd /tmp/eraseme || exit

# Expect group users to include 'pulse' as well as normal user:
getent group audio

# Get this over with early. Reason: if script is piped into 'less',
# type the password before the screen fills and password characters
# are treated as 'less' commands.
sudo --user=pulse true

# There would be no problem below if umask were 002, so one option
# is to execute
#   umask 002
# so that FETCH_HEAD's permissions don't have to be changed below.

# First method: emulate git-clone as three git commands, with
# a single 'chgrp' call at exactly the right spot.

# The crux of this method is 'git init':
git init --bare --shared manual.git
chgrp -R audio manual.git
git -C manual.git remote add origin https://github.com/wxWidgets/zlib.git
git -C manual.git fetch origin

find ./manual.git ! -perm -g=w |sed -e'/objects\/pack/d'
# Oops: FETCH_HEAD doesn't have group permissions:
ls -l ./manual.git/FETCH_HEAD

# This isn't really necessary; it just makes the result look more like
# that of the second method, below.
git -C manual.git pack-refs --all

# This succeeds when run by owner:
git -C manual.git fetch
# this fails:
sudo --user=pulse git -C manual.git fetch
# but it succeeds if FETCH_HEAD's permissions are fixed:
chmod g+w manual.git/FETCH_HEAD
sudo --user=pulse git -C manual.git fetch

# Second method: git-clone --bare --config core.SharedRepository=group

# expect 022 here:
umask
# There is no problem below if umask is 002, so one option
# is to execute
umask 002
# here. That affects only the shell in which this script runs,
# though it has a persistent effect if run at the command line.

chgrp audio .
chmod g+ws .

# The crux of this method is 'git clone':
git clone --jobs=32 --bare --config core.SharedRepository=group https://github.com/wxWidgets/zlib.git
# This succeeds when run by owner:
git -C zlib.git fetch
# this succeeds (but not without 'umask 002' above):
sudo --user=pulse git -C zlib.git fetch

# The two methods produce somewhat similar results. Sizes:
du -sb zlib.git manual.git
# are almost the same. Small differences:
#  - manual.git/config has this extra line under [remote "origin"]:
#      fetch = +refs/heads/*:refs/remotes/origin/*
#    (which is just a default)
#  - HEAD is
#      refs/heads/master  [in manual.git]
#      refs/heads/wx      [in zlib.git]
#    though both seem to point to the same SHA1
#  - git-fsck complains about an unborn branch, in manual.git only:
git -C zlib.git fsck
git -C manual.git fsck

# List all files' permissions for comparison, e.g.:
#   meld /srv/chroot/bullseye0/tmp/eraseme/ls-* &
cd /tmp/eraseme/manual.git && ls -alR >/tmp/eraseme/ls-manual.git
cd /tmp/eraseme/zlib.git   && ls -alR >/tmp/eraseme/ls-zlib.git
