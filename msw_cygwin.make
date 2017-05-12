# Platform specifics: msw (cygwin) with MinGW-w64 toolchain.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

################################################################################

# Sanity checks.

cygdrive_diagnostic := \
  Current working directory '$(CURDIR)' contains '/cygdrive/'. \
  This is likely to cause problems with native tools such as the \
  MinGW-w64 compiler. Build from an identity-mount directory instead

ifeq (/cygdrive/,$(findstring /cygdrive/,$(CURDIR)))
  $(error $(cygdrive_diagnostic))
endif

ifeq (,$(wildcard /opt/lmi/*))
  $(warning Installation may be invalid: /opt/lmi/ missing or empty. )
endif

################################################################################

system_root := /cygdrive/c

################################################################################

# Use cygwin as a quasi-cross-compiler for an i686-pc-mingw32 target.

# For autotoolized libraries, pass these flags to 'configure':
# cross_compile_flags := --build=i686-pc-cygwin --host=i686-w64-mingw32

################################################################################

# These makefiles are designed to be independent of $PATH: they work
# correctly even if $PATH is empty. That seems desirable as a general
# principle; furthermore, many problems reported on mailing lists are
# due to users inadvertently mixing cygwin and other tools by setting
# $PATH incorrectly. OTOH, such problems arise infrequently with real
# *nix, and some fundamental tools are not necessarily located in any
# particular location: for example, neither FHS-2.2 nor FHS-3.0
# prescribes where 'grep' must reside.
#
# To force $PATH to be respected instead, set $(USE_STD_PATHS) to a
# nonempty string.
#
# These paths are slash-terminated so that setting them to empty
# strings does the right thing.

ifeq (,$(USE_STD_PATHS))
  PATH_BIN     := /bin/
  PATH_GCC     := /MinGW_/bin/
  PATH_USR_BIN := /usr/bin/
endif

################################################################################

# Compiler, linker, and so on.

# Oddly, MinGW-w64 provides prefixed versions of compilers:
#   i686-w64-mingw32-gcc.exe
#   i686-w64-mingw32-g++.exe
# but not of the other tools.

#host_prefix := i686-w64-mingw32-
host_prefix :=

AR      := $(PATH_GCC)$(host_prefix)ar
CC      := $(PATH_GCC)$(host_prefix)gcc
CPP     := $(PATH_GCC)$(host_prefix)cpp
CXX     := $(PATH_GCC)$(host_prefix)g++
LD      := $(PATH_GCC)$(host_prefix)g++
RC      := $(PATH_GCC)$(host_prefix)windres

# Identify run-time libraries for redistribution. See:
#   https://cygwin.com/ml/cygwin/2010-09/msg00553.html
# Of course manipulating an lmi user's $PATH is out of the question.

compiler_sysroot := /MinGW_/i686-w64-mingw32/lib

compiler_runtime_files := \
  $(compiler_sysroot)/libstdc++-6.dll \
  $(compiler_sysroot)/libgcc_s_sjlj-1.dll \

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

# FHS-2.2 would presumably put these in /usr/bin . However, debian
# puts 'bzip2' and 'grep' in /bin .

BZIP2   := $(PATH_USR_BIN)bzip2
DIFF    := $(PATH_USR_BIN)diff
GREP    := $(PATH_USR_BIN)grep
MD5SUM  := $(PATH_USR_BIN)md5sum
PATCH   := $(PATH_USR_BIN)patch
SORT    := $(PATH_USR_BIN)sort
TOUCH   := $(PATH_USR_BIN)touch
TR      := $(PATH_USR_BIN)tr
WC      := $(PATH_USR_BIN)wc
WGET    := $(PATH_USR_BIN)wget

# Programs for which FHS doesn't specify a location.

# Instead of requiring installation of Cygwin's libxml2:
#   XMLLINT := $(PATH_USR_BIN)xmllint
# use the one that lmi builds:
XMLLINT := /opt/lmi/local/bin/xmllint

################################################################################

# Configuration shared by all msw subplatforms.

include $(srcdir)/msw_common.make
$(srcdir)/msw_common.make:: ;

