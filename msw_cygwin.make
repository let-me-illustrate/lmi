# Platform specifics: msw using cygwin.
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

# $Id: msw_cygwin.make,v 1.6 2005-12-07 15:12:25 chicares Exp $

################################################################################

system_root := /cygdrive/c

################################################################################

# Set normal system paths explicitly unless $(USE_STD_PATHS) is specified.
# Reason: many problems reported on mailing lists are due to users mixing
# MSYS and cygwin tools by setting $(PATH) incorrectly; but people who set
# $(USE_STD_PATHS) are assumed to know what they're doing. These paths are
# slash-terminated so that setting them to empty strings works, too.

ifeq (,$(USE_STD_PATHS))
  PATH_BIN     := /bin/
  PATH_GCC     := /usr/local/bin/
  PATH_USR_BIN := /usr/bin/
endif

################################################################################

# Compiler, linker, and so on.

AR      := $(PATH_GCC)ar
CC      := $(PATH_GCC)gcc
CPP     := $(PATH_GCC)cpp
CXX     := $(PATH_GCC)g++
LD      := $(PATH_GCC)g++
RC      := $(PATH_GCC)windres

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

CP      := $(PATH_BIN)cp
DATE    := $(PATH_BIN)date
ECHO    := $(PATH_BIN)echo
GZIP    := $(PATH_BIN)gzip
LS      := $(PATH_BIN)ls
MKDIR   := $(PATH_BIN)mkdir
MV      := $(PATH_BIN)mv
RM      := $(PATH_BIN)rm
SED     := $(PATH_BIN)sed
TAR     := $(PATH_BIN)tar

# FHS-2.2 would put these in /usr/bin .

BZIP2   := $(PATH_USR_BIN)bzip2
DIFF    := $(PATH_USR_BIN)diff
GREP    := $(PATH_USR_BIN)grep
MD5SUM  := $(PATH_USR_BIN)md5sum
PATCH   := $(PATH_USR_BIN)patch
TOUCH   := $(PATH_USR_BIN)touch
TR      := $(PATH_USR_BIN)tr
WC      := $(PATH_USR_BIN)wc
WGET    := $(PATH_USR_BIN)wget
XMLLINT := $(PATH_USR_BIN)xmllint

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

