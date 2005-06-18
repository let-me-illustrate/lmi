# Platform specifics: msw generic with a bourne-compliant shell.
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

# $Id: msw_generic.make,v 1.3 2005-06-18 04:46:41 chicares Exp $

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

gnu_utils_dir := $(system_root)/gnu

# Required in /bin (if anywhere) by FHS-2.2 .

CP     := $(gnu_utils_dir)/cp
# TODO ?? Build this from FSF sources and put it in the same
# directory as everything else.
#DATE   := $(gnu_utils_dir)/date
DATE   ?= $(cygwin_dir)/bin/date
ECHO   := $(gnu_utils_dir)/echo
LS     := $(gnu_utils_dir)/ls
MKDIR  := $(gnu_utils_dir)/mkdir
MV     := $(gnu_utils_dir)/mv
RM     := $(gnu_utils_dir)/rm
SED    := $(gnu_utils_dir)/sed

# FHS-2.2 would put these in /usr/bin .

BZIP2  := $(gnu_utils_dir)/bzip2
DIFF   := $(gnu_utils_dir)/diff
GREP   := $(gnu_utils_dir)/grep
MD5SUM := $(gnu_utils_dir)/md5sum
PATCH  := $(gnu_utils_dir)/patch
TAR    := $(gnu_utils_dir)/tar
TOUCH  := $(gnu_utils_dir)/touch
TR     := $(gnu_utils_dir)/tr
WC     := $(gnu_utils_dir)/wc

################################################################################

# Configuration shared by all msw subplatforms.

include $(src_dir)/msw_common.make
$(src_dir)/msw_common.make:: ;

