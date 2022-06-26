# Compiler-specific makefile: gcc.
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

AR      := $(gcc_proclitic)ar
CC      := $(gcc_proclitic)gcc
CPP     := $(gcc_proclitic)cpp
CXX     := $(gcc_proclitic)g++
LD      := $(gcc_proclitic)g++
# For GNU/Linux, $(RC) is never invoked.
RC      := $(gcc_proclitic)windres

# GNU tools (or workalikes) for special purposes.
#
# For testing physical closure and generating autodependencies, use
# either GNU tools or closely compatible equivalents such as clang.
# This obviates figuring out how other toolchains support these needs.
#
# Override these definitions to specify GNU tools when using an
# incompatible toolchain.

GNU_CPP := $(CPP)
GNU_CXX := $(CXX)

################################################################################

# Identify run-time libraries for redistribution. See:
#   https://lists.nongnu.org/archive/html/lmi/2017-05/msg00046.html
# Perhaps gcc's '-print-sysroot' would be more suitable, but that
# option returns an empty string with debian cross compilers.
#
# It might seem more robust to write something like
#   compiler_sysroot := $(shell readlink -fn /usr/lib/gcc/$(LMI_TRIPLET)/*-win32)
# but that would actually weaken makefile portability, and there
# is no guarantee that this directory will be named similarly in
# future debian releases, much less on other OSs.

ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
compiler_sysroot := $(dir $(shell $(CXX) -print-libgcc-file-name))

compiler_runtime_files := \
  $(wildcard $(compiler_sysroot)/libgcc*.dll) \
  $(wildcard $(compiler_sysroot)/libstdc++*.dll) \

endif

################################################################################

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

# Compiler version.

# $(subst): workaround for debian, whose MinGW-w64 identifies its
# version 7.x.0 as "7.x-win32".

ifeq (gcc,$(LMI_COMPILER))
  gcc_version   := $(subst -win32,.0,$(shell $(CXX)     -dumpversion))
endif

# These are defined even for toolchains other than gcc.

gnu_cpp_version := $(subst -win32,.0,$(shell $(GNU_CPP) -dumpversion))
gnu_cxx_version := $(subst -win32,.0,$(shell $(GNU_CXX) -dumpversion))

ifeq      (3.4.4,$(gnu_cpp_version))
else ifeq (3.4.5,$(gnu_cpp_version))
else ifeq (4.9.1,$(gnu_cpp_version))
else ifeq (4.9.2,$(gnu_cpp_version))
else ifeq (6.3.0,$(gnu_cpp_version))
else ifeq (7.2.0,$(gnu_cpp_version))
else ifeq (7.3.0,$(gnu_cpp_version))
else ifeq (8,$(gnu_cpp_version))
else ifeq (8.1.0,$(gnu_cpp_version))
else ifeq (8.2.0,$(gnu_cpp_version))
else ifeq (8.3.0,$(gnu_cpp_version))
else ifeq (9,$(gnu_cpp_version))
else ifeq (9.3.0,$(gnu_cpp_version))
else ifeq (10,$(gnu_cpp_version))
else ifeq (10.0,$(gnu_cpp_version))
else ifeq (11,$(gnu_cpp_version))
else ifeq (11.0,$(gnu_cpp_version))
else
  $(warning Untested $(GNU_CPP) version '$(gnu_cpp_version)')
endif

ifeq      (3.4.4,$(gnu_cxx_version))
else ifeq (3.4.5,$(gnu_cxx_version))
else ifeq (4.9.1,$(gnu_cxx_version))
else ifeq (4.9.2,$(gnu_cxx_version))
else ifeq (6.3.0,$(gnu_cxx_version))
else ifeq (7.2.0,$(gnu_cxx_version))
else ifeq (7.3.0,$(gnu_cxx_version))
else ifeq (8,$(gnu_cxx_version))
else ifeq (8.1.0,$(gnu_cxx_version))
else ifeq (8.2.0,$(gnu_cxx_version))
else ifeq (8.3.0,$(gnu_cxx_version))
else ifeq (9,$(gnu_cxx_version))
else ifeq (9.3.0,$(gnu_cxx_version))
else ifeq (10,$(gnu_cxx_version))
else ifeq (10.0,$(gnu_cxx_version))
else ifeq (11,$(gnu_cxx_version))
else ifeq (11.0,$(gnu_cxx_version))
else
  $(warning Untested $(GNU_CXX) version '$(gnu_cxx_version)')
endif

################################################################################

# Overriding options--simply expanded, and empty by default.

tutelary_flag :=

# Dialect and warning options for gcc.

# The default '-fno-rounding-math' means something like
#   #pragma STDC FENV ACCESS OFF
# which causes harm while bringing no countervailing benefit--see:
#   https://lists.nongnu.org/archive/html/lmi/2017-08/msg00045.html

c_standard   := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c99
cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++20

# Specify these:
#   $(gcc_version_specific_c_warnings)
#   $(gcc_version_specific_cxx_warnings)
# last, in order to override other options.

ifeq (3.4.4,$(gcc_version))
  # Suppress spurious gcc-3.4.4 warnings:
  #   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=22207
  gcc_version_specific_c_warnings   := -Wno-uninitialized
  gcc_version_specific_cxx_warnings := -Wno-uninitialized
  cxx_standard := -std=c++98
else ifeq (3.4.5,$(gcc_version))
  # Suppress spurious gcc-3.4.5 warnings:
  #   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=22207
  gcc_version_specific_c_warnings   := -Wno-uninitialized
  gcc_version_specific_cxx_warnings := -Wno-uninitialized
  # Fix "hello world":
  #   http://sourceforge.net/tracker/index.php?func=detail&aid=2373234&group_id=2435&atid=102435
  cxx_standard := -std=gnu++98
  # Use a correct snprintf() implementation:
  #   http://article.gmane.org/gmane.comp.gnu.mingw.user/27539
  cxx_standard += -posix
else ifneq (,$(filter $(gcc_version), 4.9.1 4.9.2))
  # See:
  #   https://lists.nongnu.org/archive/html/lmi/2015-12/msg00028.html
  #   https://lists.nongnu.org/archive/html/lmi/2015-12/msg00040.html
  gcc_version_specific_c_warnings := \
    -Wno-conversion \
    -Wno-unused-local-typedefs \
    -Wno-unused-variable \

  gcc_version_specific_cxx_warnings := \
    -Wno-conversion \
    -Wno-unused-local-typedefs \
    -Wno-unused-variable \

  cxx_standard := -std=c++11
else ifneq (,$(filter $(gcc_version), 6.3.0))
  gcc_version_specific_c_warnings   := -Wno-conversion
  gcc_version_specific_cxx_warnings := -Wno-conversion

  cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++17
else ifneq (,$(filter $(gcc_version), 7.2.0 7.3.0))
  gcc_version_specific_c_warnings   :=
  gcc_version_specific_cxx_warnings :=

  cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++17
else ifneq (,$(filter $(gcc_version), 8 8.1.0 8.2.0 8.3.0 9 9.3.0))
  gcc_version_specific_c_warnings   :=
  gcc_version_specific_cxx_warnings :=

  ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_TRIPLET)))
# See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00026.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00000.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00002.html
    tutelary_flag := -fomit-frame-pointer
  endif

  cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++2a
else ifneq (,$(filter $(gcc_version), 10 10.0))
  gcc_version_specific_c_warnings :=

  gcc_version_specific_cxx_warnings := \

  ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_TRIPLET)))
# See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00026.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00000.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00002.html
#   https://lists.nongnu.org/archive/html/lmi/2021-03/msg00000.html
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99234
# Fixed in gcc-10.2.1, but this makefile doesn't detect the last
# component of major.minor.patchlevel reliably.
    tutelary_flag := -fomit-frame-pointer
    ifneq (,$(filter $(gcc_version), 10 10.2.1))
      tutelary_flag := -fno-omit-frame-pointer
    endif
  endif

  cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++20
else ifneq (,$(filter $(gcc_version), 11 11.0))
  gcc_version_specific_c_warnings :=

# g++-11 warnings not recognized by g++-10
#
# The gcc manual says that '-Wenum-conversion' is for C only,
# but `make show_overlooked_cxx_warnings` disagreed.

  gcc_version_specific_cxx_warnings := \
    -Wctad-maybe-unsupported \
    -Wdeprecated-enum-enum-conversion \
    -Wdeprecated-enum-float-conversion \
    -Wenum-conversion \
    -Winvalid-imported-macros \

  cxx_standard := -fno-ms-extensions -frounding-math -fsignaling-nans -std=c++20
endif

treat_warnings_as_errors := -pedantic-errors -Werror

# Write '-Wno' options at the end, with a rationale here.
#
# -Wabi: useful only for a special purpose
# -Wanalyzer-too-complex: not actually helpful
# -Wdate-time: only for "bit-wise-identical reproducible compilations"
# -Wfloat-equal: too many warnings on correct code, e.g.,
#   exact comparison to zero
# -Winline: 'inline' now refers to linkage, not inlining
# -Wmissing-declarations: for C++, this flags valid functions that
#   arguably belong in an unnamed namespace
# -Wpadded: useful only for low-level work
# -Wparentheses: its diagnostics are beyond pedantic
# -Wpsabi: undocumented--ARM only?
# -Wsign-conversion: [see note elsewhere in this file]
# -Wsuggest-attribute: too gcc-specific
# -Wsystem-headers: not generally useful
# -Wswitch-default: false positives for switches on enums that
#   include all cases (verified by -Wswitch-enum)

gcc_common_warnings := \
  $(treat_warnings_as_errors) \
  -Waggregate-return \
  -Wall \
  -Walloc-zero \
  -Walloca \
  -Warith-conversion \
  -Wcast-align=strict \
  -Wcast-function-type \
  -Wconversion \
  -Wdangling-else \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wdouble-promotion \
  -Wduplicated-branches \
  -Wduplicated-cond \
  -Wextra \
  -Wformat=2 \
  -Wformat-nonliteral \
  -Wformat-overflow=2 \
  -Wformat-security \
  -Wformat-signedness \
  -Wformat-truncation \
  -Wformat-y2k \
  -Wimport \
  -Winit-self \
  -Winvalid-pch \
  -Wlogical-op \
  -Wmissing-include-dirs \
  -Wmultichar \
  -Wnull-dereference \
  -Wpacked \
  -Wpointer-arith \
  -Wredundant-decls \
  -Wrestrict \
  -Wshadow \
  -Wshift-overflow=2 \
  -Wsign-compare \
  -Wstack-protector \
  -Wstrict-overflow \
  -Wstringop-overflow=4 \
  -Wswitch-enum \
  -Wtrampolines \
  -Wundef \
  -Wunreachable-code \
  -Wunsafe-loop-optimizations \
  -Wunused-macros \
  -Wvector-operation-performance \
  -Wvla \
  -Wno-abi \
  -Wno-analyzer-too-complex \
  -Wno-date-time \
  -Wno-float-equal \
  -Wno-inline \
  -Wno-missing-declarations \
  -Wno-padded \
  -Wno-parentheses \
  -Wno-psabi \
  -Wno-sign-conversion \
  -Wno-suggest-attribute=pure \
  -Wno-suggest-attribute=const \
  -Wno-suggest-attribute=noreturn \
  -Wno-suggest-attribute=malloc \
  -Wno-suggest-attribute=format \
  -Wno-suggest-attribute=cold \
  -Wno-system-headers \
  -Wno-switch-default \

# Consider these later:
postponed_gcc_common_warnings := \

# Write '-Wno' options at the end, with a rationale here.
#
# -Wunsuffixed-float-constants: a migration aid to support an eventual
#   FLOAT_CONST_DECIMAL64 pragma, but only a nuisance for now

gcc_c_warnings := \
  $(c_standard) \
  $(gcc_common_warnings) \
  -Wbad-function-cast \
  -Wc++-compat \
  -Wjump-misses-init \
  -Wmissing-braces \
  -Wmissing-prototypes \
  -Wnested-externs \
  -Wold-style-definition \
  -Wstrict-prototypes \
  -Wtraditional-conversion \
  -Wwrite-strings \
  -Wno-unsuffixed-float-constants \

# Write '-Wno' options at the end, with a rationale here.
#
# -Wabi-tag: useful only for a special purpose
# -Waggregate-return: generally incompatible with C++
# -Weffc++: obsolescent
# -Wmismatched-tags: not helpful--see:
#   https://lists.nongnu.org/archive/html/lmi/2016-05/msg00075.html
# -Wmultiple-inheritance: forbids a useful feature
# -Wnamespaces: forbids a useful feature
# -Wsign-promo: too many false positives--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00016.html
# -Wsuggest-final-methods, and
# -Wsuggest-final-types: use these only occasionally, like -Weffc++;
#   work with '-Wsuggest-final-types' first, because making a class
#   final may resolve '-Wsuggest-final-methods' suggestions for its
#   members; but expect many false positives
# -Wtemplates: forbids a useful feature
# -Wvirtual-inheritance: forbids a useful feature

gcc_cxx_warnings := \
  $(cxx_standard) \
  $(gcc_common_warnings) \
  -Wc++11-compat \
  -Wc++14-compat \
  -Wc++1z-compat \
  -Wcatch-value=3 \
  -Wcomma-subscript \
  -Wconditionally-supported \
  -Wctor-dtor-privacy \
  -Wdelete-non-virtual-dtor \
  -Wdeprecated \
  -Wdeprecated-copy-dtor \
  -Wextra-semi \
  -Wnoexcept \
  -Wnoexcept-type \
  -Wnon-template-friend \
  -Wnon-virtual-dtor \
  -Wold-style-cast \
  -Woverloaded-virtual \
  -Wplacement-new=2 \
  -Wpmf-conversions \
  -Wredundant-tags \
  -Wregister \
  -Wreorder \
  -Wstrict-null-sentinel \
  -Wsuggest-override \
  -Wsynth \
  -Wuseless-cast \
  -Wvolatile \
  -Wzero-as-null-pointer-constant \
  -Wno-abi-tag \
  -Wno-aggregate-return \
  -Wno-effc++ \
  -Wno-mismatched-tags \
  -Wno-multiple-inheritance \
  -Wno-namespaces \
  -Wno-sign-promo \
  -Wno-suggest-final-methods \
  -Wno-suggest-final-types \
  -Wno-templates \
  -Wno-virtual-inheritance \

# Consider these later:
postponed_gcc_cxx_warnings := \

gcc_common_extra_warnings := \
  -Wcast-qual \

bourn_cast_test.o: gcc_common_extra_warnings += \
  -Wno-double-promotion \

$(cgicc_objects): gcc_common_extra_warnings += \
  -Wno-conversion \
  -Wno-zero-as-null-pointer-constant \

ifeq (safestdlib,$(build_type))
  ifeq (3.4.5,$(gcc_version))
    expression_template_0_test.o: gcc_common_extra_warnings += -Wno-unused-parameter
  endif
endif

# XMLWRAPP !! Remove these workarounds after updating xmlwrapp. See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00018.html
# et seqq.:
$(xmlwrapp_objects): gcc_common_extra_warnings += \
  -Wno-conversion \

# SOMEDAY !! Address some of these '-Wconversion' issues.

wno_conv_objects := \
  rate_table.o \
  round_glibc.o \

$(wno_conv_objects): gcc_common_extra_warnings += -Wno-conversion -Wfloat-conversion

wno_sign_conv_objects := \
  $(wx_dependent_objects) \
  crc32.o \
  getopt.o \
  md5.o \
  round_glibc.o \

# '-Wsign-conversion' is generally unusable with C++ because the STL
# defectively uses unsigned types. It is useful to enable it from
# time to time and filter the output thus:
#   grep 'error:' | sed -e '/size_type/d'

$(wno_sign_conv_objects): gcc_common_extra_warnings += -Wno-sign-conversion

C_WARNINGS = \
  $(gcc_c_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_c_warnings) \

CXX_WARNINGS = \
  $(gcc_cxx_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_cxx_warnings) \

################################################################################

# Build type governs
#  - optimization flags
#  - gprof
#  - libstdc++ debugging macros

# Options for undefined-behavior sanitizer.
#
# These:
#   pointer-compare,pointer-subtract
# aren't necessarily usable with gcc--see:
#   https://lists.nongnu.org/archive/html/lmi/2022-06/msg00033.html

ubsan_options := \
  -fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow,bounds-strict \
  -Wno-duplicated-branches \
  -fno-omit-frame-pointer \
  -fno-var-tracking \
  -fno-var-tracking-assignments \
  -O3 \
  --param max-gcse-memory=1000000 \

# Since gcc version 4.6, '-fomit-frame-pointer' has apparently been
# the default. Don't use that because it makes debugging difficult.
# See:
#   https://lists.gnu.org/archive/html/lmi/2016-06/msg00091.html

ifeq (gprof,$(build_type))
  optimization_flag := -O0 -fno-omit-frame-pointer
  analyzer_flag := -pg
else ifeq (ubsan,$(build_type))
  analyzer_flag := $(ubsan_options)
else ifeq (safestdlib,$(build_type))
  optimization_flag := -O0 -fno-omit-frame-pointer
else
  optimization_flag := -O2 -fno-omit-frame-pointer
endif

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

# For simplicity and robustness, the same options are used for all
# 'my_*.cpp' files.

product_file_sources := my_db.o my_fund.o my_prod.o my_rnd.o my_tier.o

product_file_flags := -Os -fno-var-tracking-assignments -fno-omit-frame-pointer

$(product_file_sources): optimization_flag += $(product_file_flags)

# $(optimization_flag) is part of the default $(CXXFLAGS), but a
# target-specific assignment there isn't enough, because it is too
# easily overridden by specifying $(CXXFLAGS) on the command line.
# This flag overrides any such override:

$(product_file_sources): tutelary_flag += $(product_file_flags)

################################################################################

# Flags.

# Define uppercase FLAGS recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the lowercase
# (and often immediately-expanded) variables they're composed from.

debug_flag := -ggdb

# MinGW gcc-3.4.2 writes dwarf2 debug records if '-ggdb' is specified,
# but the version of gdb packaged with it expects stabs format.
#
ifeq (3.4.2,$(gcc_version))
  debug_flag := -g
  gcc_version_specific_c_warnings   := -Wno-uninitialized
  gcc_version_specific_cxx_warnings := -Wno-uninitialized
endif

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

# As this is written in 2012, lmi is often built on machines with less
# RAM per core than gcc wants. Experiments show that these flags cut
# gcc's RAM appetite by fifty percent, in return for a ten-percent
# speed penalty that can be overcome by increasing parallelism. There
# seems to be no need for them with gcc-4.x, which uses less RAM.

ifeq (gcc,$(LMI_COMPILER))
  ifeq (3.4.5,$(gcc_version))
    ggc_flags := --param ggc-min-expand=25 --param ggc-min-heapsize=32768
  endif
endif

CFLAGS = \
  $(ggc_flags) $(optimization_flag) $(c_l_flags) \

CXXFLAGS = \
  $(ggc_flags) $(optimization_flag) $(c_l_flags) \

LDFLAGS = $(c_l_flags) -Wl,-Map,$@.map \

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  LDFLAGS += -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code
endif

# Explicitly disable the infelicitous auto-import default. See:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/19758
#     [2006-05-18T11:38:01Z from Earnie Boyd]
# Do not disable it for MinGW-w64, which seems to require it.

ifeq (3.4.5,$(gcc_version))
  LDFLAGS += -Wl,--disable-auto-import
endif

ifneq (,$(USE_SO_ATTRIBUTES))
  ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
    LDFLAGS += -Wl,--disable-auto-import -static-libstdc++
  endif
endif

# C compiler flags.

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

# C++ compiler flags.

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \

# Archiver flags.

REQUIRED_ARFLAGS = \
  -rus

# Linker flags.

# Prefer to invoke GNU 'ld' through the compiler frontends 'gcc' and
# 'g++' because that takes care of linking the required libraries for
# each language. Accordingly, pass GNU 'ld' options with '-Wl,'.

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
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(EXTRA_LIBS) \

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

ALL_ARFLAGS  = $(REQUIRED_ARFLAGS)  $(ARFLAGS)
ALL_CPPFLAGS = $(REQUIRED_CPPFLAGS) $(CPPFLAGS)
ALL_CXXFLAGS = $(REQUIRED_CXXFLAGS) $(CXXFLAGS) $(tutelary_flag)
ALL_CFLAGS   = $(REQUIRED_CFLAGS)   $(CFLAGS)   $(tutelary_flag)
ALL_LDFLAGS  = $(REQUIRED_LDFLAGS)  $(LDFLAGS)
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
