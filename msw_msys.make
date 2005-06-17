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

# $Id: msw_msys.make,v 1.2 2005-06-17 13:46:15 zeitlin Exp $

################################################################################

# MSYS !! MSYS-1.0.10 of 2004-05, the latest version as of 2005-01,
# mangles several sed commands used in lmi makefiles. Details:
#   http://sf.net/mailarchive/message.php?msg_id=10668423
# Here's another example:
#   http://sf.net/mailarchive/message.php?msg_id=9879502
# The defect is apparently in the MSYS fork of bash. The most
# important makefile targets work more or less correctly despite this
# defect, but it breaks autodependency generation.

################################################################################

system_root := /c

################################################################################

# set the paths if the tools are not already in the PATH
#
# NB: they must be slash-terminated so that it's possible to set them to empty
#     strings too
ifeq (,$(USE_STD_PATHS))
    PATH_MINGW := /mingw/bin/
    PATH_BIN := /bin/
    PATH_USR_BIN := /usr/bin/
endif

# Compiler, linker, and so on.

AR  := $(PATH_MINGW)ar
CC  := $(PATH_MINGW)gcc
CPP := $(PATH_MINGW)cpp
CXX := $(PATH_MINGW)g++
LD  := $(PATH_MINGW)g++
RC  := $(PATH_MINGW)windres

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

# A more modern 'sed' than MSYS provides as of 2005-01 is needed.
# Get the GNU sources, for sed-4.0.7 or later; build a new msw binary
# with MSYS; and define $(SED) to point to it.

CP     := $(PATH_BIN)cp
DATE   := $(PATH_BIN)date
ECHO   := $(PATH_BIN)echo
LS     := $(PATH_BIN)ls
MKDIR  := $(PATH_BIN)mkdir
MV     := $(PATH_BIN)mv
RM     := $(PATH_BIN)rm
TAR    := $(PATH_BIN)tar

# FHS-2.2 would put these in /usr/bin .

BZIP2  := $(PATH_USR_BIN)bzip2
DIFF   := $(PATH_USR_BIN)diff
GREP   := $(PATH_USR_BIN)grep
MD5SUM := $(PATH_USR_BIN)md5sum
PATCH  := $(PATH_USR_BIN)patch
SED    := $(PATH_USR_BIN)sed
TOUCH  := $(PATH_USR_BIN)touch
TR     := $(PATH_USR_BIN)tr
WC     := $(PATH_USR_BIN)wc

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

