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

# $Id: msw_msys.make,v 1.1 2005-01-28 01:34:43 chicares Exp $

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

# Compiler, linker, and so on.

AR  := /mingw/bin/ar
CC  := /mingw/bin/gcc
CPP := /mingw/bin/cpp
CXX := /mingw/bin/g++
LD  := /mingw/bin/g++
RC  := /mingw/bin/windres

################################################################################

# Standard utilities.

# Required in /bin (if anywhere) by FHS-2.2 .

# A more modern 'sed' than MSYS provides as of 2005-01 is needed.
# Get the GNU sources, for sed-4.0.7 or later; build a new msw binary
# with MSYS; and define $(SED) to point to it.

CP     := /bin/cp
DATE   := /bin/date
ECHO   := /bin/echo
LS     := /bin/ls
MKDIR  := /bin/mkdir
MV     := /bin/mv
RM     := /bin/rm
SED    := $(system_root)/usr/bin/sed
TAR    := /bin/tar

# FHS-2.2 would put these in /usr/bin .

BZIP2  := /usr/bin/bzip2
DIFF   := /usr/bin/diff
GREP   := /usr/bin/grep
MD5SUM := /usr/bin/md5sum
PATCH  := /usr/bin/patch
TOUCH  := /usr/bin/touch
TR     := /usr/bin/tr
WC     := /usr/bin/wc

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

