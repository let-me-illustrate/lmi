#!/bin/sh

# Attempt to set up a git repository to be shared by multiple users.
#
# FETCH_HEAD doesn't get the right permissions, and that's a git
# defect--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-03/msg00016.html
#   https://public-inbox.org/git/20200319010321.18614-1-vz-git@zeitlins.org/T/#u

set -v

# Like 'share_git1.sh', but creates a non-bare repository.

# Expect 022 here:
umask
# The FETCH_HEAD permission problem doesn't arise if umask is 002,
# so one option is to execute
#   umask 002
# That affects only the shell in which this script runs, but it has a
# persistent effect if run at the command line.

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

inited="inited_nonbare"

mkdir "$inited"
# Need to run 'chmod' because git doesn't create this directory.
chmod g+sw "$inited"

# The crux of this method is 'git init':
git init --shared "$inited"
chgrp -R audio "$inited"
git -C "$inited" remote add origin https://github.com/wxWidgets/zlib.git
git -C "$inited" fetch origin

find ./"$inited" ! -perm -g=w |sed -e'/objects\/pack/d'
# Oops: FETCH_HEAD doesn't have group permissions:
ls -l ./"$inited"/.git/*HEAD

# This isn't really necessary; it just makes the result look more like
# that of the second method, below.
git -C "$inited" pack-refs --all

# Generate index and worktree:
git -C "$inited" checkout master

# This succeeds when run by owner:
git -C "$inited" fetch
# this fails:
sudo --user=pulse git -C "$inited" fetch
# but it succeeds if FETCH_HEAD's permissions are fixed:
chmod g+w "$inited"/.git/FETCH_HEAD
sudo --user=pulse git -C "$inited" fetch

# Need to do this after fetching, for the worktree.
chmod -R g=u "$inited"

# Second method: git-clone, and fix up permissions manually

cloned="cloned_nonbare"

# The crux of this method is 'git clone':
git clone --jobs=32 --config core.SharedRepository=group https://github.com/wxWidgets/zlib.git "$cloned"
chgrp -R audio "$cloned"
# This is better than 'chmod -R g+s' (it affects only directories):
find "$cloned" -type d -exec chmod g+s {} +
# Specifying 's' here would cause many 'S' occurrences in 'ls' output:
#   chmod -R g+swX "$cloned"
# Specifying 'g+w' here would cause pack files to be group writable:
chmod -R g=u "$cloned"
# There, 'g=u' doesn't override the earlier 'g+s'--see:
#   https://lists.nongnu.org/archive/html/lmi/2020-03/msg00019.html

# Permissions seem to be okay...
find ./"$cloned" ! -perm -g=w |sed -e'/objects\/pack/d'
# ...but that's because FETCH_HEAD doesn't yet exist:
ls -l ./"$cloned"/.git/*HEAD

# This succeeds when run by owner:
git -C "$cloned" fetch

find ./"$cloned" ! -perm -g=w |sed -e'/objects\/pack/d'
# Oops: FETCH_HEAD doesn't have group permissions:
ls -l ./"$cloned"/.git/*HEAD

# This fails:
sudo --user=pulse git -C "$cloned" fetch
# but it succeeds if FETCH_HEAD's permissions are fixed:
chmod g+w "$cloned"/.git/FETCH_HEAD
sudo --user=pulse git -C "$cloned" fetch

# The two methods produce somewhat similar results. Sizes:
du -sb "$cloned" "$inited"
# are almost the same. Small differences:
#  - HEAD is
#      refs/heads/master  [in inited_nonbare]
#      refs/heads/wx      [in cloned_nonbare]
#    though both seem to point to the same SHA1
# git-fsck makes no complaint about either:
git -C "$cloned" fsck
git -C "$inited" fsck

# List all files' permissions for comparison, e.g.:
#   meld /srv/chroot/bullseye0/tmp/eraseme/ls-* &
cd /tmp/eraseme/"$inited" && ls -alR >/tmp/eraseme/ls-"$inited"
cd /tmp/eraseme/"$cloned" && ls -alR >/tmp/eraseme/ls-"$cloned"
