# Platform specifics: msw using MSYS.
#
# Copyright (C) 2005 Gregory W. Chicares.
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: msw_msys.make,v 1.6 2005-09-09 04:36:22 wboutin Exp $

################################################################################

# MSYS !! MSYS-1.0.10 of 2004-05, the latest version as of 2005-01,
# mangles several sed commands used in lmi makefiles. Details:
#   http://sf.net/mailarchive/message.php?msg_id=10668423
# Here's another example:
#   http://sf.net/mailarchive/message.php?msg_id=9879502
# The defect is apparently in the MSYS fork of bash. The most
# important makefile targets work more or less correctly despite this
# defect, but it breaks autodependency generation.
#
# TODO ?? These instructions, however:
# # Get the GNU sources, for sed-4.0.7 or later; build a new msw binary
# # with MSYS; and define $(SED) to point to it.
# don't fix the problem. More work is needed.

################################################################################

system_root := /c

################################################################################

# Set normal system paths explicitly unless $(USE_STD_PATHS) is specified.
# Reason: many problems reported on mailing lists are due to users mixing
# MSYS and cygwin tools by setting $(PATH) incorrectly; but people who set
# $(USE_STD_PATHS) are assumed to know what they're doing. These paths are
# slash-terminated so that setting them to empty strings works, too.

# MSYS mounts /usr/bin/ as an alias for /bin/ , so it's irrelevant that
# it places files that belong in /usr/bin/ physically in its /bin/ . This
# has nothing to do with $(PATH).

ifeq (,$(USE_STD_PATHS))
  PATH_BIN     := /bin/
  PATH_GCC     := /mingw/bin/
  PATH_USR_BIN := /usr/bin/
endif

################################################################################

# Compiler, linker, and so on.

AR     := $(PATH_GCC)ar
CC     := $(PATH_GCC)gcc
CPP    := $(PATH_GCC)cpp
CXX    := $(PATH_GCC)g++
LD     := $(PATH_GCC)g++
RC     := $(PATH_GCC)windres

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

CP     := $(PATH_BIN)cp
DATE   := $(PATH_BIN)date
ECHO   := $(PATH_BIN)echo
LS     := $(PATH_BIN)ls
MKDIR  := $(PATH_BIN)mkdir
MV     := $(PATH_BIN)mv
RM     := $(PATH_BIN)rm

# TODO ?? Override MSYS's ancient sed. This is an experimental change.
# If testing confirms that it works, then it should be explained better,
# and instructions for building a more modern sed should be given.
#
# Vadim--I merely downloaded the GNU project's sources for sed-4.0.7
# and built them in MSYS in the normal './configure && make' way, then
# copied the resulting binary to "C:\usr\bin\sed.exe". Maybe it should
# go in a special directory in the lmi hierarchy instead. Just be sure
# you don't copy it into MSYS's own /bin/ directory: that's likely to
# cause horrible problems.

# SED    := $(PATH_BIN)sed
SED    := /c/usr/bin/sed

# FHS-2.2 would put these in /usr/bin .

BZIP2  := $(PATH_USR_BIN)bzip2
DIFF   := $(PATH_USR_BIN)diff
GREP   := $(PATH_USR_BIN)grep
GZIP   := $(PATH_USR_BIN)gzip
MD5SUM := $(PATH_USR_BIN)md5sum
PATCH  := $(PATH_USR_BIN)patch
TAR    := $(PATH_USR_BIN)tar
TOUCH  := $(PATH_USR_BIN)touch
TR     := $(PATH_USR_BIN)tr
WC     := $(PATH_USR_BIN)wc
WGET   := $(PATH_USR_BIN)wget

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

