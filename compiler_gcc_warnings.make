# Compiler-specific makefile: gcc warnings.
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

# Don't remake this makefile.

$(srcdir)/compiler_gcc_warnings.make:: ;

# Warnings for gcc.

gcc_version_specific_c_warnings :=
gcc_version_specific_cxx_warnings :=

ifneq (,$(filter $(gcc_version), 10 10.0))
# Nothing to do here.
else ifneq (,$(filter $(gcc_version), 11 11.0))

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

endif

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

treat_warnings_as_errors := -pedantic-errors -Werror

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

# Keep version-specific warnings last, so that they override others.

C_WARNINGS = \
  $(gcc_c_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_c_warnings) \

CXX_WARNINGS = \
  $(gcc_cxx_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_cxx_warnings) \

