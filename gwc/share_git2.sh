#!/bin/sh

# Attempt to set up a git repository to be shared by multiple users.
#
# FETCH_HEAD doesn't get the right permissions, and that's a git
# defect--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-03/msg00016.html
#   https://public-inbox.org/git/20200319010321.18614-1-vz-git@zeitlins.org/T/#u

# Like 'share_git1.sh', but creates a non-bare repository.

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

# First method: emulate git-clone as three git commands, with
# a single 'chgrp' call at exactly the right spot.

mkdir nonbare
# Need to run 'chmod' because git doesn't create this directory.
chmod g+sw nonbare

# The crux of this method is 'git init':
git init --shared nonbare
chgrp -R audio nonbare
git -C nonbare remote add origin https://github.com/wxWidgets/zlib.git
git -C nonbare fetch origin

find ./nonbare ! -perm -g=w |sed -e'/objects\/pack/d'
# Oops: FETCH_HEAD doesn't have group permissions:
ls -l ./nonbare/.git/*HEAD

# This isn't really necessary; it just makes the result look more like
# that of the second method, below.
git -C nonbare pack-refs --all

# Generate index and worktree:
git -C nonbare checkout master

# This succeeds when run by owner:
git -C nonbare fetch
# this fails:
sudo --user=pulse git -C nonbare fetch
# but it succeeds if FETCH_HEAD's permissions are fixed:
chmod g+w nonbare/.git/FETCH_HEAD
sudo --user=pulse git -C nonbare fetch

# Need to do this after fetching, for the worktree.
chmod -R g=u nonbare

# Second method: git-clone, and fix up permissions manually

# The crux of this method is 'git clone':
git clone --jobs=32 --config core.SharedRepository=group https://github.com/wxWidgets/zlib.git
chgrp -R audio zlib
# This is better than 'chmod -R g+s' (it affects only directories):
find zlib -type d -exec chmod g+s {} +
# Specifying 's' here causes many 'S' occurrences in 'ls' output:
# chmod -R g+swX zlib
# Specifying 'g+w' here would cause pack files to be group writable:
chmod -R g=u zlib
# Why doesn't 'g=u' override the earlier 'g+s'?

find ./zlib ! -perm -g=w |sed -e'/objects\/pack/d'

# This succeeds when run by owner:
git -C zlib fetch

find ./zlib ! -perm -g=w |sed -e'/objects\/pack/d'
# Oops: FETCH_HEAD doesn't have group permissions:
ls -l ./zlib/.git/*HEAD

# This fails:
sudo --user=pulse git -C zlib fetch
# but it succeeds if FETCH_HEAD's permissions are fixed:
chmod g+w zlib/.git/FETCH_HEAD
sudo --user=pulse git -C zlib fetch

# The two methods produce somewhat similar results. Sizes:
du -sb zlib nonbare/
# are almost the same. Small differences:
#  - HEAD is
#      refs/heads/master  [in nonbare]
#      refs/heads/wx      [in zlib]
#    though both seem to point to the same SHA1
# git-fsck makes no complaint about either:
git -C zlib fsck
git -C nonbare fsck

# List all files' permissions for comparison, e.g.:
#   meld /srv/chroot/bullseye0/tmp/eraseme/ls-* &
cd /tmp/eraseme/nonbare && ls -alR >/tmp/eraseme/ls-nonbare
cd /tmp/eraseme/zlib    && ls -alR >/tmp/eraseme/ls-zlib
