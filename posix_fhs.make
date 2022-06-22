# Platform specifics: GNU/Linux and systems that equivalently comply
# with POSIX and the Filesystem Hierarchy Standard.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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
# https://savannah.nongnu.org/projects/lmi
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

################################################################################

SHREXT := .so

EXTRA_LIBS := -ldw -lunwind -ldl

AR      := ar
CC      := gcc
CPP     := cpp
CXX     := g++
LD      := g++

# Don't set RC to anything: elsewhere, this is taken as indicating
# that msw resources aren't used on this platform.
RC      :=

# Required in /bin (if anywhere) by FHS-2.2 .

CHMOD   := chmod
CP      := cp
DATE    := date
ECHO    := echo
GZIP    := gzip
INSTALL := install
LS      := ls
MKDIR   := mkdir
MV      := mv
RM      := rm
SED     := sed
TAR     := tar

# FHS-2.2 would put these in /usr/bin .

BSDTAR  := bsdtar
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
