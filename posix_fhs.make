# Platform specifics: GNU/Linux and systems that equivalently comply
# with POSIX and the Filesystem Hierarchy Standard.
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

# $Id: posix_fhs.make,v 1.4 2005-06-17 01:46:14 zeitlin Exp $

################################################################################

system_root := /
EXEEXT :=
SHREXT := .so

platform_libxml2_libraries = $(shell xml2-config --libs)

# Let the user override this on the make command line to use a
# non-default wx configuration.
WXCONFIG := wx-config

platform_wx_libraries := $(shell $(WXCONFIG) --libs)
wx_cxxflags := $(shell $(WXCONFIG) --cxxflags) -DwxUSE_STD_STRING

AR     := ar
CC     := gcc
CPP    := cpp
CXX    := g++
LD     := g++

# don't set RC to anything, this means that we don't use resources on this
# platform
RC     :=

CP     := cp
DATE   := date
ECHO   := echo
LS     := ls
MKDIR  := mkdir
MV     := mv
RM     := rm
SED    := sed
TAR    := tar

BZIP2  := bzip2
DIFF   := diff
GREP   := grep
MD5SUM := md5sum
PATCH  := patch
TOUCH  := touch
TR     := tr
WC     := wc

