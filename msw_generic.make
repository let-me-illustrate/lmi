# Platform specifics: msw generic with a bourne-compliant shell.
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

# $Id: msw_generic.make,v 1.10 2007-03-10 18:23:55 chicares Exp $

################################################################################

# TODO ?? Much of this file is obsolete. It supports an archaic
# installation of GNU tools ported to msw here:
#   ftp://ftp.franken.de/pub/win32/develop/gnuwin32/mingw32/porters/Mikey/
# and is provided only as a transitional aid for developers who still
# use those ancient ports.

################################################################################

system_root := C:

################################################################################

# Path to each compiler's root directory, i.e. bin/.. .
mingw_dir  := $(system_root)/MinGW
cygwin_dir := $(system_root)/cygwin
como_dir   := $(system_root)/como433

# Path to each compiler's main include directory. This is where
# patches would be applied for noncompliant compilers.

mingw_main_include_dir  := $(mingw_dir)
cygwin_main_include_dir := $(cygwin_dir)/usr
como_main_include_dir   := $(como_dir)

################################################################################

# Compiler, linker, and so on.

AR  := $(mingw_dir)/bin/ar
CC  := $(mingw_dir)/bin/gcc
CPP := $(mingw_dir)/bin/cpp
CXX := $(mingw_dir)/bin/g++
LD  := $(mingw_dir)/bin/g++
RC  := $(mingw_dir)/bin/windres

################################################################################

# Standard utilities.

gnu_utils_dir  := $(system_root)/gnu

# Required in /bin (if anywhere) by FHS-2.2 .

CP      := $(gnu_utils_dir)/cp
# TODO ?? Build this from gnu.org sources and put it in the same
# directory as everything else.
DATE    := $(gnu_utils_dir)/date
ECHO    := $(gnu_utils_dir)/echo
GZIP    := $(gnu_utils_dir)/gzip
LS      := $(gnu_utils_dir)/ls
MKDIR   := $(gnu_utils_dir)/mkdir
MV      := $(gnu_utils_dir)/mv
RM      := $(gnu_utils_dir)/rm
SED     := $(gnu_utils_dir)/sed
TAR     := $(gnu_utils_dir)/tar

# FHS-2.2 would put these in /usr/bin .

BZIP2   := $(gnu_utils_dir)/bzip2
DIFF    := $(gnu_utils_dir)/diff
GREP    := $(gnu_utils_dir)/grep
MD5SUM  := $(gnu_utils_dir)/md5sum
PATCH   := $(gnu_utils_dir)/patch
SORT    := $(gnu_utils_dir)/sort
TOUCH   := $(gnu_utils_dir)/touch
TR      := $(gnu_utils_dir)/tr
WC      := $(gnu_utils_dir)/wc
WGET    := $(gnu_utils_dir)/wget
XMLLINT := $(gnu_utils_dir)/xmllint

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

