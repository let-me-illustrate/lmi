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

# Aliases for tools used in targets elsewhere.

AR      := ar
CC      := clang
CPP     := cpp
CXX     := clang++
LD      := clang++
# For GNU/Linux, $(RC) is never invoked.
RC      := windres

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

REQUIRED_COMPILER_FLAGS := \
  $(c_l_flags) \
  -Woverriding-t-option \
    -ffp-model=strict \
    -ffp-exception-behavior=ignore \
  -Wno-overriding-t-option \
  -fno-ms-compatibility \
  -fno-ms-extensions \

# C compiler flags.

REQUIRED_CFLAGS = $(REQUIRED_COMPILER_FLAGS) -std=c99

# C++ compiler flags.

REQUIRED_CXXFLAGS = $(REQUIRED_COMPILER_FLAGS) -std=c++20

# Write '-Wno' options at the end, with a rationale here.
#
# -Wstring-plus-int: false negatives and no true positives in lmi.

CXXFLAGS = -Wno-string-plus-int

# Linker flags.

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
