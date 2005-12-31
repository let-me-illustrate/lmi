# Limited support for a particular non-free compiler under msw only.

# Copyright (C) 2004, 2005 Gregory W. Chicares.
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

# $Id: como_4_3_3.make,v 1.9 2005-12-31 16:59:42 chicares Exp $

toolset := como

src_dir := $(CURDIR)

gcc_version :=

# Casual workarounds for como C++ version 4.3.3, using gcc as the
# underlying C compiler. It is casually supported through this
# makefile because it conforms to the standard in some ways that gcc
# does not, so using it at least for unit tests may reveal defects.

# This C++ compiler requires an underlying C compiler. To use gcc as
# that underlying compiler, it needs MinGW gcc-2.95.3-5; other
# versions or other msw ports of gcc won't do.
#
underlying_cc := /mingw-2.95.3-5

C_WARNINGS         :=
CXX_WARNINGS       :=

C_EXTRA_WARNINGS   :=
CXX_EXTRA_WARNINGS :=

# This compiler does not automatically define the customary macros for
# identifying the msw-intel platform.

CPPFLAGS := \
  -D__WIN32__ \
  -D_X86_ \

CC  := como
CXX := como

# This compiler's '--strict' option is incompatible with MinGW system
# headers, and even its relaxed '--a' counterpart is incompatible with
# '__declspec'.
#
# '--display_error_number' shows numeric error codes that may be
# useful for choosing diagnostics to suppress for code that is not
# 'strictly conforming'.
#
# Diagnostic 161: unrecognized pragma: frequent in wx.
#
# Diagnostic 654: supposed 'declspec' incompatibility: there seems to
# be no way to avoid such a warning here:
#   [declspec] void* operator new  (std::size_t, wx_allocator);
#   void* operator new(std::size_t bytes, wx_allocator) {...}
# Como requires that the decoration precede the return type in the
# declaration, but doesn't allow it anywhere in the definition. With
# '--a', this doesn't need to be suppressed, but it doesn't hurt either
# because '--a' treats any '__declspec' as an error.
#
# Diagnostics 1195 and 1200: emulated ms defective loop variable scoping.

nonstrict_cxxflags := \
  --diag_suppress=161 \
  --diag_suppress=654 \
  --diag_suppress=1195 \
  --diag_suppress=1200 \
  --display_error_number \
  --long_long \
  --no_microsoft_bugs \

strict_cxxflags := \
  --a \
  --display_error_number \
  --long_long \
  --no_microsoft_bugs \

# This compiler's strict mode is incompatible with many useful tools
# such as boost::filesystem (if built from source).

CXXFLAGS := $(nonstrict_cxxflags)

# For debugging on msw with gcc as the underlying C compiler, add
#  /g
# to $(CXXFLAGS): the slash is required, and '-g' won't work.

LD := $(CXX)

LDFLAGS :=

MAKEDEPEND_0 :=

MAKEDEPEND_1 :=

MPATROL_LIBS :=

# To get rid of the ms nonsense and use the strictest mode available
# on msw, uncomment the next two lines.
# CPPFLAGS :=
# CXXFLAGS := --a

all:

como_4_3_3.make:: ;

# Comeau C++ for msw expects its bin/ directory to be on the path and
# the environment to contain a path to gcc. Furthermore, it expects
# to invoke msw subshells. However, we prefer *nix shells and want to
# put nothing compiler specific in the environment.
#
# Accommodating these como idiosyncrasies by setting $(CXX) to the
# following shell script...
#
## #!/usr/bin/sh
## echo "Starting...\n"
## cmd="
##   ComSpec=$WindowsComSpec;
##   COMSPEC=$WindowsComSpec;
##   path=(/como433/bin /mingw-2.95.3-5/bin /usr/bin C:/WINNT/system32 C:/WINNT);
##   export COMO_MIN_INCLUDE=/mingw-2.95.3-5/include;
##   como $*;
##   exit;"
## /usr/bin/sh -c "echo cmd is $cmd; $cmd; echo done" || print Error executing command: $cmd
#
# ...gobbles compiler diagnostics. And enclosing the commands in
#
##   $(shell
##     [commands]
##   )
#
# gobbles newlines in the output. Therefore, we resort to the odd
# workaround of enclosing the commands in
#
##   sh -c "
##     [commands]
##   "
#
# instead.
#
# The result is an ugly workaround that's probably quite fragile.

%: force
	@sh -c " \
	  path=(/usr/bin /como433/bin/ $(underlying_cc)/bin/); \
	  export COMO_MIN_INCLUDE=$(underlying_cc)/include; \
	  ComSpec=C:\\\\WINDOWS\\\\SYSTEM32\\\\CMD.EXE; \
	  $(MAKE) \
	    -f $(src_dir)/GNUmakefile \
	            gcc_version='$(gcc_version)' \
	                src_dir='$(src_dir)' \
	                toolset='$(toolset)' \
	             C_WARNINGS='$(C_WARNINGS)' \
	           CXX_WARNINGS='$(CXX_WARNINGS)' \
	       C_EXTRA_WARNINGS='$(C_EXTRA_WARNINGS)' \
	     CXX_EXTRA_WARNINGS='$(CXX_EXTRA_WARNINGS)' \
	               CPPFLAGS='$(CPPFLAGS)' \
	                     CC='$(CC)' \
	                    CXX='$(CXX)' \
	               CXXFLAGS='$(CXXFLAGS)' \
	                     LD='$(LD)' \
                    LDFLAGS='$(LDFLAGS)' \
	           MAKEDEPEND_0='$(MAKEDEPEND_0)' \
	           MAKEDEPEND_1='$(MAKEDEPEND_1)' \
	           MPATROL_LIBS='$(MPATROL_LIBS)' \
	    unit_tests; \
	"

force: ;

