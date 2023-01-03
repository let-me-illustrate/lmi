# Limited support for Comeau C++ 4.3.x under msw only.

# Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

# Limited support for Comeau C++ version 4.3.10.1, using MinGW gcc as
# the underlying C compiler, with a Cygwin shell. Comeau C++ is useful
# because it conforms to the standard in some ways that gcc does not,
# so it may reveal defects not found with gcc alone.

# SOMEDAY !! Errors may arise from template-instantiation conflicts,
# e.g., when many targets are built in the same directory. Ideas:
#  - use '-T'
#  - routinely delete '.ii' files
#  - use a monolithic library for all code shared by unit tests
#  - use a distinct subdirectory for each target
# Cf.:
# http://www-d0.fnal.gov/KAI/doc/UserGuide/faq.html#multiple_template_instance

LMI_COMPILER := como

srcdir       := $(CURDIR)

gcc_version  :=

prefix       := /opt/lmi
exec_prefix  := $(prefix)
lmi_bin_dir  := $(exec_prefix)/bin

como_dir     := $(prefix)/como
como_bin_dir := $(como_dir)/bin

# Comeau C++ requires an underlying C compiler--here, a particular
# version of MinGW gcc-2.95.3-5, as discussed in 'install_como.sh'.

mingw_gcc2   := mingw_for_como

gcc2_dir     := $(como_dir)/$(mingw_gcc2)
gcc2_bin_dir := $(gcc2_dir)/bin

# Comeau C++ for msw requires $COMO_MIN_INCLUDE to specify the
# underlying C compiler's include directory.

gcc2_inc_dir := $(gcc2_dir)/include

# Use gcc-3.x for autodependencies and physical-closure testing.

# SOMEDAY !! Ideally, the preprocessor used for autodependencies would
# impersonate Comeau C++. The "UNUSED_" variable below is probably not
# complete, and special include paths probably would need to be set.
# This is not easy because the Comeau preprocessor is not free and has
# no option like GNU cpp's '-dM' to list the macros it predefines. Yet
# it is not very important either, because Comeau C++ isn't remarkably
# different from gcc wrt conformance, so there are few conditionals to
# differentiate them and gcc dependencies work well enough for now.

UNUSED_compiler_impersonation_cppflags := \
  -undef -nostdinc \
  -U __GNUC__ -U __GNUG__ -U __GNUC_MINOR__ -U __GNUC_PATCHLEVEL__ \
  -D LMI_IGNORE_PCH \
  -D __COMO__ \

# This makefile because unusable long ago, so this dependency has been
# renamed to avoid pointless maintenance updates.
gcc3_dir     := /opt/lmi/no_longer_usable
gcc3_bin_dir := $(gcc3_dir)/bin

# Comeau C++ for msw requires both its own bin/ directory and the
# underlying C compiler's bin/ directory to be on the path. They must
# precede Cygwin's system directories because Cygwin puts its own gcc
# in /usr/bin/ . Comeau C++ suggests a MinGW gcc-2.95.3 tarball that
# unfortunately comes bundled with its own obsolete 'make', which must
# be removed or renamed to keep it from clashing with Cygwin's 'make'
# (though this doesn't matter if 'install_como.sh' is used).

insidious_make := $(gcc2_bin_dir)/make.exe

ifneq (,$(wildcard $(insidious_make)))
  $(error Remove or rename '$(insidious_make)')
endif

# Comeau C++ does not automatically define the customary macros for
# identifying the msw-intel platform.

CPPFLAGS := \
  -D__WIN32__ \
  -D_X86_ \

CC                 := gcc
# Overridden below, but only after use for $(LD):
CXX                := como

C_WARNINGS         :=
CXX_WARNINGS       :=

C_EXTRA_WARNINGS   :=
CXX_EXTRA_WARNINGS :=

# Comeau C++'s '--strict' option is incompatible with MinGW system
# headers, and even its relaxed '--a' counterpart is incompatible with
# '__declspec'.
#
# '--display_error_number' shows numeric error codes that may be
# useful for choosing diagnostics to suppress for code that is not
# 'strictly conforming'.
#
# Diagnostic 161: unrecognized pragma: occurs frequently in wx.
#
# Diagnostic 654: supposed 'declspec' incompatibility: there seems to
# be no way to avoid such a warning here:
#   [declspec] void* operator new  (std::size_t, wx_allocator);
#   void* operator new(std::size_t bytes, wx_allocator) {...}
# Como requires that the decoration precede the return type in the
# declaration, but doesn't allow it anywhere in the definition. With
# '--a', this doesn't need to be suppressed, but neither does it hurt
# because '--a' treats any '__declspec' as an error.
#
# Diagnostics 1195 and 1200: emulated ms defective loop variable
# scoping.

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

# Comeau C++'s strict mode is incompatible with many useful libraries
# such as boost::filesystem (if built from source).

# To get rid of the ms nonsense and use the strictest mode available
# on msw, uncomment the next two lines.
# CPPFLAGS :=
# CXXFLAGS := --a

CXXFLAGS := $(nonstrict_cxxflags) --no_prelink_verbose

# For debugging on msw with gcc as the underlying C compiler, add
#  /g
# to $(CXXFLAGS): the slash is required, and '-g' doesn't work.
# SOMEDAY !! Test this; modify the CXX wrapper to handle it as a
# special case if necessary.

LD := $(CXX)

LDFLAGS :=

# Library order is crucial.
#
# Comeau C++ doesn't accept '-lxslt.dll -lxml2.dll', so required
# libraries are specified here as full filepaths. It treats leading
# slashes as option delimiters, so leading slashes in library names
# are changed here to doubled backslashes. SOMEDAY !! A 'como_ld'
# program could accept gcc syntax.

LDFLAGS := \
  \\opt/lmi/local/lib/libexslt.dll.a \
  \\opt/lmi/local/lib/libxslt.dll.a \
  \\opt/lmi/local/lib/libxml2.dll.a \

excluded_unit_test_targets := \
  regex_test \
  round_test \

GNU_CPP := $(gcc3_bin_dir)/cpp
GNU_CXX := $(gcc3_bin_dir)/g++

MAKEDEPEND_FLAGS   :=
MAKEDEPEND_COMMAND := MAKEDEPEND_NON_GCC_COMMAND

MAKECMDGOALS ?= lmi_cli_monolithic.exe

# Default target.
$(MAKECMDGOALS):

como_4_3_3.make:: ;

# SOMEDAY !! Comeau C++ should have its own wrapper. For now, the
# borland wrapper works.

CXX := \
  $(lmi_bin_dir)/bcc_cc \
  --accept --program como \

# Comeau C++ for msw requires both its own bin/ directory and the
# underlying C compiler's bin/ directory to be on the path, and an
# msw path to the underlying C compiler's include directory to be
# given in another environment variable. Specify environment changes
# explicitly here, before invoking $(MAKE).
#
# Use '--jobs=1' to prevent 'make' parallelism, which appears to be
# incompatible with Comeau C++. Ignore any "disabling jobserver mode"
# warning that this engenders.

%: force
	@export PATH=$(como_bin_dir):$(gcc2_bin_dir):$$PATH; \
	export COMO_MIN_INCLUDE=$(gcc2_inc_dir); \
	$(MAKE) \
	  --file=$(srcdir)/GNUmakefile \
	  --jobs=1 \
	                        gcc_version='$(gcc_version)' \
	                             srcdir='$(srcdir)' \
	                       LMI_COMPILER='$(LMI_COMPILER)' \
	                         C_WARNINGS='$(C_WARNINGS)' \
	                       CXX_WARNINGS='$(CXX_WARNINGS)' \
	                   C_EXTRA_WARNINGS='$(C_EXTRA_WARNINGS)' \
	                 CXX_EXTRA_WARNINGS='$(CXX_EXTRA_WARNINGS)' \
	                           CPPFLAGS='$(CPPFLAGS)' \
	    compiler_impersonation_cppflags='$(compiler_impersonation_cppflags)' \
	                                 CC='$(CC)' \
	                                CXX='$(CXX)' \
	                           CXXFLAGS='$(CXXFLAGS)' \
	                                 LD='$(LD)' \
	                            LDFLAGS='$(LDFLAGS)' \
	                   platform_defines='' \
	         excluded_unit_test_targets='$(excluded_unit_test_targets)' \
	                            GNU_CPP='$(GNU_CPP)' \
	                            GNU_CXX='$(GNU_CXX)' \
	                   MAKEDEPEND_FLAGS='$(MAKEDEPEND_FLAGS)' \
	                 MAKEDEPEND_COMMAND='$(MAKEDEPEND_COMMAND)' \
	  $(MAKECMDGOALS); \

force: ;
