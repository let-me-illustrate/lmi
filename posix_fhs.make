# Platform specifics: GNU/Linux and systems that equivalently comply
# with POSIX and the Filesystem Hierarchy Standard.
#
# Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: posix_fhs.make,v 1.16 2007-03-10 18:23:55 chicares Exp $

################################################################################

system_root := /
EXEEXT :=
SHREXT := .so

platform_boost_libraries := \
  -lboost_filesystem-gcc \

platform_gnome_xml_libraries := \
  $(shell xml2-config --libs) \
  $(shell xslt-config --libs) \

# Let the user override this on the make command line to use a
# non-default wx configuration.
WXCONFIG := wx-config

platform_wx_libraries := $(shell $(WXCONFIG) --libs)
wx_cxxflags := $(shell $(WXCONFIG) --cxxflags) -DwxUSE_STD_STRING

AR      := ar
CC      := gcc
CPP     := cpp
CXX     := g++
LD      := g++

# Don't set RC to anything: elsewhere, this is taken as indicating
# that msw resources aren't used on this platform.
RC      :=

# Required in /bin (if anywhere) by FHS-2.2 .

CP      := cp
DATE    := date
ECHO    := echo
GZIP    := gzip
LS      := ls
MKDIR   := mkdir
MV      := mv
RM      := rm
SED     := sed
TAR     := tar

# FHS-2.2 would put these in /usr/bin .

BZIP2   := bzip2
DIFF    := diff
GREP    := grep
MD5SUM  := md5sum
PATCH   := patch
SORT    := sort
TOUCH   := touch
TR      := tr
WC      := wc
WGET    := wget
XMLLINT := xmllint

