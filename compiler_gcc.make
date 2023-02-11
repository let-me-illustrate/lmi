# Compiler-specific makefile: gcc.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

# Other makefiles included; makefiles not to be remade.

# Don't remake this makefile.

$(srcdir)/compiler_gcc.make:: ;

# Ascertain compiler version.

include $(srcdir)/compiler_gcc_version.make
$(srcdir)/compiler_gcc_version.make:: ;

# Warnings.

include $(srcdir)/compiler_gcc_warnings.make
$(srcdir)/compiler_gcc_warnings.make:: ;

# EXTRA variables.
#
# /\w*EXTRA_/ variables such as $(EXTRA_LDFLAGS) are set by other
# makefiles, and used here without modification.

# C preprocessor flags.
#
# wx sets _FILE_OFFSET_BITS to 64; this definition is explicitly
# repeated below to make assurance doubly sure--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00039.html
# et seq.

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(lmi_include_directories)) \
  $(addprefix -isystem , $(sys_include_directories)) \
  $(lmi_wx_new_so_attributes) \
  $(platform_defines) \
  $(wx_predefinitions) \
  -D_FILE_OFFSET_BITS=64 \

ifneq (,$(USE_SO_ATTRIBUTES))
  REQUIRED_CPPFLAGS += -DLMI_USE_SO_ATTRIBUTES $(lmi_so_attributes)
endif

ifeq (safestdlib,$(build_type))
  REQUIRED_CPPFLAGS += \
    -D_GLIBCXX_DEBUG \
    -D_GLIBCXX_DEBUG_PEDANTIC \
    -D_GLIBCXX_SANITIZE_VECTOR \

endif

# Overriding options--simply expanded, and empty by default.

tutelary_flag :=

# MinGW-w64's SEH defectively requires '-fomit-frame-pointer' until
# gcc-10.2.1 (but as of 20220701 debian provides 10.0.0 only). See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00026.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00000.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00002.html
#   https://lists.nongnu.org/archive/html/lmi/2021-03/msg00000.html
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99234

ifneq (,$(filter $(gcc_version), 10 10.0))
  ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_TRIPLET)))
    tutelary_flag := -fomit-frame-pointer
  endif
endif

# Build type governs
#  - optimization flags
#  - gprof
#  - libstdc++ debugging macros

# Options for undefined-behavior sanitizer.
#
# These:
#   pointer-compare,pointer-subtract
# aren't "production-ready"--see:
#   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00037.html

ubsan_options := \
  -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow,bounds-strict \
  -Wno-duplicated-branches \
  -fno-var-tracking \
  -fno-var-tracking-assignments \
  --param max-gcse-memory=1000000 \

# Flags.

# Define uppercase FLAGS recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the lowercase
# (and often immediately-expanded) variables they're composed from.

debug_flag := -ggdb

# '-fomit-frame-pointer' is an infelicitous default. Turn it off,
# as it makes debugging difficult and has no measurable benefit.

analyzer_flag :=
optimization_flag := -fno-omit-frame-pointer

ifeq (gprof,$(build_type))
  analyzer_flag += -pg
  optimization_flag += -O0
else ifeq (ubsan,$(build_type))
  analyzer_flag += $(ubsan_options)
  optimization_flag += -O3
else ifeq (safestdlib,$(build_type))
  optimization_flag += -O0
else
  optimization_flag += -O2
endif

# Historical workarounds for product-file-generating binaries.
#
# These workarounds are no longer necessary as of 2022-07-02. They are
# retained as comments in case similar problems ever arise again.
#
# An overriding version of 'my_prod.cpp', which is used to create a
# nondistributable binary, contains so many large strings that, after
# consuming more than one CPU minute and 1 MiB of RAM, MinGW gcc-3.4.5
# produced a diagnostic such as
#   warning: NULL pointer checks disabled:
#   39933 basic blocks and 167330 registers
# which was historically prevented by specifying '-O0'. In 2019-02,
# however, using gcc-7.3, with many more products in 'my_*.cpp' files,
# building with '-O0' caused a stack overflow at run time with 32-bit
# msw, which can be prevented by specifying any optimization option
# except '-O0'. That stack overflow seemed to be caused by adding
# twenty-one 2017 CSO products to 'my_db.cpp' in particular; however,
# after the same new products were added to 'my_prod.cpp' and that
# file was compiled with '-O0', the stack overflow occurred no matter
# what flags 'my_db.cpp' was compiled with--presumably because all
# linked objects share the same stack and it's their total size that
# matters.
#
# The '-fno-var-tracking-assignments' flag avoids a specific compiler
# diagnostic in 'my_prod.cpp', which contains a very large number of
# lengthy strings. That flag is specified explicitly here to ensure
# that it's still used even if the '-O' flag is someday changed, even
# though gcc currently documents that any optimization flag other than
# '-O0' implies it.
#
# For simplicity and robustness, the same options are used for all
# 'my_*.cpp' files.
#
# product_file_sources := my_db.o my_fund.o my_prod.o my_rnd.o my_tier.o
#
# $(product_file_sources): debug_flag += -fno-var-tracking-assignments
# $(product_file_sources): optimization_flag += -Os

################################################################################

# Compiler-and-linker flags.
#
# 'c_l_flags' are to be used in both compiler and linker commands.
# The gprof '-pg' flag is one example. Another is '-fPIC', which
# pc-linux-gnu requires for '-shared':
#   https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#DOCF1
# Yet another is 'debug_flag': including it in $(LDFLAGS) seems
# to be historical practice, as seen in the GNU 'make' manual:
#   https://www.gnu.org/software/make/manual/make.html#Complex-Makefile
# | LDFLAGS = -g
# even though GNU 'ld' ignores '-g'; and the GNU Coding Standards
#   https://www.gnu.org/prep/standards/html_node/Command-Variables.html
# suggest including flags such as '-g' in $(CFLAGS) because they
# are "not required for proper compilation", but lmi deliberately
# sets default debugging flags, of necessity: with gcc-3.4.5 at
# least, the '-fno-var-tracking-assignments' debugging option was
# required for compiling product files correctly. Furthermore, lmi
# binary distributions are intended to be built with '-ggdb' so that
# any errors reported by end users can reliably be reproduced using
# debug builds. If it is desired to negate '-ggdb', the gcc manual
# suggests that '-ggdb0' should do that.

c_l_flags = $(debug_flag) $(analyzer_flag)

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  c_l_flags += -fPIC
endif

# C and C++ compiler flags.

# Around 2012 it was profitable to use flags such as
#   --param ggc-min-expand=25 --param ggc-min-heapsize=32768
# with older, more RAM-hungry versions of gcc, on older hardware
# that didn't have as much RAM per core as gcc wanted. Experiments
# showed that these flags cut gcc's RAM appetite by fifty percent,
# in return for a ten-percent speed penalty that could be overcome
# by increasing parallelism. They became unnecessary with gcc-4.x,
# which used less RAM. They might become useful again in future
# circumstances that cannot be foreseen.

# The default '-fno-rounding-math' means something like
#   #pragma STDC FENV ACCESS OFF
# which causes harm while bringing no countervailing benefit--see:
#   https://lists.nongnu.org/archive/html/lmi/2017-08/msg00045.html

REQUIRED_COMPILER_FLAGS = \
  $(c_l_flags) \
  $(optimization_flag) \
  -frounding-math \
  -fsignaling-nans \
  -fno-ms-extensions \

# C compiler flags.

REQUIRED_CFLAGS = -std=c99 $(C_WARNINGS) $(REQUIRED_COMPILER_FLAGS)

CFLAGS =

# C++ compiler flags.

REQUIRED_CXXFLAGS = -std=c++20 $(CXX_WARNINGS) $(REQUIRED_COMPILER_FLAGS)

CXXFLAGS =

# Linker flags.

# Prefer to invoke the linker through compiler frontends, because
# that takes care of linking each language's required libraries.
# Accordingly, pass linker options with '-Wl,'.

# Directories set in $(overriding_library_directories) are searched
# before any others except the current build directory. There seems
# to be no conventional name for such a variable: automake recommends
# $(LDADD) or a prefixed variant for both '-l' and '-L' options, but
# $(LDADD) can't do the right thing in all cases: e.g., to override a
# default mpatrol library with a custom build,
#   -L overrides must come at the beginning of a command, but
#   -l options must come at the end, so that mpatrol is linked last.
# That is, in the typical automake usage
#   $(LINK) $(LDFLAGS) $(OBJECTS) $(LDADD) $(LIBS)
# no single variable can be changed to produce
#   $(LINK) $(LDFLAGS) $(OBJECTS) -L custom_path $(LIBS) -l custom
# for a custom version of a library whose default version is already
# specified in $(LIBS). Thus, a distinct variable is necessary for
# path overrides, so distinct variables are necessary.

# Architecture-specific directories $(locallibdir) and $(localbindir)
# are placed on the link path in order to accommodate msw dlls, for
# which no canonical location is clearly specified by FHS, because
# they're both binaries and libraries in a sense. These two
# subdirectories seem to be the most popular choices, and usage
# varies, at least for msw:
#  - wx-2.7.0 built with autotools puts its dll in lib/
#  - libxml2 and libxslt put their dlls in bin/
# It is crucial to list these two subdirectories in exactly the order
# given. If they were specified in reverse order, then gnu 'ld' would
# find a dll before its import library, which latter would therefore
# be ignored--and that prevented mpatrol from working correctly.

all_library_directories := \
  . \
  $(overriding_library_directories) \
  $(locallibdir) \
  $(localbindir) \

EXTRA_LDFLAGS :=

REQUIRED_LDFLAGS = \
  -Wl,-Map,$@.map \
  $(c_l_flags) \
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(EXTRA_LIBS) \

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  REQUIRED_LDFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code
endif

# By infelicitous default, MinGW enables auto-import. See, e.g.:
#   https://sourceforge.net/p/mingw/mailman/message/16354653/
# Disabling it globally, thus:
#   REQUIRED_LDFLAGS += -Wl,--disable-auto-import
# worked with mingw.org's gcc, but the more recent MinGW-w64
# versions seem to require it unless $(USE_SO_ATTRIBUTES) is
# defined.

ifneq (,$(USE_SO_ATTRIBUTES))
  ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
    REQUIRED_LDFLAGS += -Wl,--disable-auto-import -static-libstdc++
  endif
endif

LDFLAGS =

# Archiver flags.

REQUIRED_ARFLAGS = -rus

# Resource compiler (msw) flags.
#
# The '--use-temp-file' windres option seems to be often helpful and
# never harmful.
#
# As seems customary, $(ALL_CPPFLAGS) is passed to 'windres', which
# doesn't recognize '-isystem'--hence the $(subst) workaround.

REQUIRED_RCFLAGS = \
  $(subst -isystem,--include-dir,$(ALL_CPPFLAGS)) \
  --use-temp-file \

# Only these /ALL_[A-Z]*FLAGS/ are both set here and used for targets.
# This casual command is useful for testing that:
#   git grep FLAGS workhorse.make| grep -o '\w*FLAGS' | sort -u
#
# The GNU Coding Standards
#   https://www.gnu.org/prep/standards/html_node/Command-Variables.html
# say
#   "Put CFLAGS last in the compilation command, after other
#    variables containing compiler options, so the user can
#    use CFLAGS to override the others."
# Going one step beyond that idea, lmi puts $(tutelary_flag) last,
# after even $(CFLAGS), for flags that must not be overridden.

ALL_CPPFLAGS = $(REQUIRED_CPPFLAGS) $(CPPFLAGS)
ALL_CFLAGS   = $(REQUIRED_CFLAGS)   $(CFLAGS)   $(tutelary_flag)
ALL_CXXFLAGS = $(REQUIRED_CXXFLAGS) $(CXXFLAGS) $(tutelary_flag)
ALL_LDFLAGS  = $(REQUIRED_LDFLAGS)  $(LDFLAGS)
ALL_ARFLAGS  = $(REQUIRED_ARFLAGS)  $(ARFLAGS)
ALL_RCFLAGS  = $(REQUIRED_RCFLAGS)  $(RCFLAGS)

# For the /dev/null rationale, see:
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=91011#c7

.PHONY: show_overlooked_cxx_warnings
show_overlooked_cxx_warnings:
	@$(CXX) $(ALL_CXXFLAGS) -Q --help=warning -xc++ /dev/null \
	  | $(GREP) '[[]disabled[]]' \
	  | $(SED) -e's/[ \t]*[[]disabled[]]//' -e's/^ *-W//' \
	  > eraseme
	@$(GREP) -of eraseme $(this_makefile) | $(GREP) -vxf - eraseme || true
	@$(RM) eraseme
