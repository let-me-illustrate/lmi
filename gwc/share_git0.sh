#!/bin/sh

# Warning: if you have anything valuable in /tmp/eraseme/, this
# script will eradicate it.

# Attempt to set up a git repository to be shared by multiple users.
# The bare zlib repository at the wx github site is used for this
# demonstration because it is tiny. The 'audio' group is used as
# the repository's owner because it is likely to exist on any
# GNU/Linux desktop machine and the normal user as well as 'pulse'
# are likely to belong to that group.

set -v

# expect 022 here:
umask
# There would be no problem below if umask were 002, so one option
# is to execute
#   umask 002
# here. That would affect only the shell in which this script runs,
# but it would have a persistent effect if run at the command line.
# However, that seems to be the simplest thing that actually works.

cd /tmp || exit
rm -rf /tmp/eraseme
mkdir -p /tmp/eraseme
cd /tmp/eraseme || exit
# expect drwxr-xr-x 2 greg greg here:
ls -ld .
chgrp audio .
# expect drwxr-xr-x 2 greg audio here:
#                          ^^^^^
ls -ld .
chmod g+s .
# expect drwxr-sr-x 2 greg audio here:
#              ^
ls -ld .

# Even
#   chmod g+ws .
# above wouldn't prevent the problems below.

# Important: '--shared' would not set 'core.SharedRepository' here,
# because the '--shared' option in these two commands:
#   git init --shared
#   git clone --shared
# doesn't mean the same thing. Adding '--shared' to this git-clone
# command seems to do nothing at all, presumably because github.com
# is not part of any local filesystem.
#
# This setting:
#   core.SharedRepository=0660
# doesn't enable group members to use the repository; neither does
#   core.SharedRepository=world
git clone --jobs=32 --bare --config core.SharedRepository=group https://github.com/wxWidgets/zlib.git
git -C zlib.git config --list | grep 'core\.'
# expect drwxr-sr-x 7 greg audio here:
ls -ld zlib.git
# expect similar permissions here:
ls -l zlib.git
# this succeeds when run by owner:
git -C zlib.git fetch
# expect group users to include 'pulse' and 'greg':
getent group audio
# this fails because group lacks write permissions:
sudo --user=pulse git -C zlib.git fetch
# this doesn't add group write permissions:
git -C zlib.git init --shared
ls -ld zlib.git
ls -l zlib.git
# even though this does set group write permissions:
git init --bare --shared ./eraseme.git
ls -ld eraseme.git

# The usual advice is either to
#  - run 'git init --shared', and then either
#    - rm the contents of the .git directory and git clone [...] .
#      where the '.' at the end is very important, or
#    - manually emulate git-clone by running commands such as:
#        git remote add origin <repository-url>
#        git pull origin master
# or to
#  - run 'git clone', and then some set of 'find ... chmod ...'
#    commands that cannot be understood by mere mortals.
# Instead, this process:
#   cd some/parent/directory
#   umask 002
#   chgrp audio .
#   chmod g+s .
#   git clone --bare --config core.SharedRepository=group some_url
# seems simpler and more comprehensible.
