# Compiler-specific makefile: clang.
#
# Copyright (C) 2022 Gregory W. Chicares.
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

$(srcdir)/compiler_clang.make:: ;

# Ascertain compiler version.

include $(srcdir)/compiler_clang_version.make
$(srcdir)/compiler_clang_version.make:: ;

# Warnings.

include $(srcdir)/compiler_clang_warnings.make
$(srcdir)/compiler_clang_warnings.make:: ;

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
  -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow \

# Apparently '-fomit-frame-pointer' is a clang default. Turn it off.

ifeq (gprof,$(build_type))
  optimization_flag := -O0 -fno-omit-frame-pointer
  analyzer_flag := -pg
else ifeq (ubsan,$(build_type))
  analyzer_flag := $(ubsan_options) -O3 -fno-omit-frame-pointer
else ifeq (safestdlib,$(build_type))
  optimization_flag := -O0 -fno-omit-frame-pointer
else
  optimization_flag := -O2 -fno-omit-frame-pointer
endif

# Flags.

# Define uppercase FLAGS recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the lowercase
# (and often immediately-expanded) variables they're composed from.

debug_flag := -ggdb

# Compiler-and-linker flags.
#
# 'c_l_flags' are to be used in both compiler and linker commands.
# The gprof '-pg' flag is one example. Another is '-fPIC', which
# pc-linux-gnu requires for '-shared':
#   https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#DOCF1
# Yet another is 'debug_flag': the GNU Coding Standards
#   https://www.gnu.org/prep/standards/html_node/Command-Variables.html
# suggest including flags such as '-g' in $(CFLAGS) because they
# are "not required for proper compilation", but lmi supports
# multiple build types that transcend that "proper" notion.

c_l_flags := $(debug_flag) $(analyzer_flag)

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  c_l_flags += -fPIC
endif

# C and C++ compiler flags.

# clang-14.0.5-1 doesn't need all this rigmarole--see:
#   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00072.html

REQUIRED_COMPILER_FLAGS := \
  $(c_l_flags) \
  -Woverriding-t-option \
    -ffp-model=strict \
    -ffp-exception-behavior=ignore \
  -Wno-overriding-t-option \
  -fno-ms-compatibility \
  -fno-ms-extensions \

# C compiler flags.

REQUIRED_CFLAGS = -std=c99 $(C_WARNINGS) $(REQUIRED_COMPILER_FLAGS)

# Better to leave this unset, and specify optimization elsewhere?
CFLAGS = $(optimization_flag)

# C++ compiler flags.

REQUIRED_CXXFLAGS = -std=c++20 $(CXX_WARNINGS) $(REQUIRED_COMPILER_FLAGS)

# Write '-Wno' options at the end, with a rationale here.
#
# -Wstring-plus-int: false negatives and no true positives in lmi.

CXXFLAGS = $(optimization_flag) -Wno-string-plus-int

# Linker flags.

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
  $(c_l_flags) \
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(EXTRA_LIBS) \

LDFLAGS = -Wl,-Map,$@.map \

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  LDFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code
endif

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
