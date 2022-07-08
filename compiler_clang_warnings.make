# Compiler-specific makefile: clang warnings.
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

# Don't remake this makefile.

$(srcdir)/compiler_clang_warnings.make:: ;

# Warnings for clang.

clang_version_specific_c_warnings :=
clang_version_specific_cxx_warnings :=

# Write '-Wno' options at the end, with a rationale here.

treat_warnings_as_errors := -pedantic-errors -Werror

clang_common_warnings := \
  $(treat_warnings_as_errors) \
  -ferror-limit=0 \
  -Weverything \

# Write '-Wno' options at the end, with a rationale here.

clang_c_warnings := \
  $(clang_common_warnings) \
  -Wno-float-conversion \
  -Wno-float-equal \
  -Wno-reserved-macro-identifier \
  -Wno-sign-conversion \

# Write '-Wno' options at the end, with a rationale here.
#
# -W++20-compat: backward compatibility not desirable
# -W++98-compat-pedantic: backward compatibility not desirable
### [triple '#' marks warnings requiring investigation]
###  -Wcomma
###  -Wconditional-uninitialized
# -Wcovered-switch-default: better to leave "default:" in place
# -Wdate-time: only for "bit-wise-identical reproducible compilations"
###  -Wdeprecated-copy-with-dtor
###  -Wdisabled-macro-expansion
# -Wdocumentation: warns about comments
# -Wdocumentation-html: warns about comments
# -Wdocumentation-unknown-command: warns about comments
###  -Wdouble-promotion
###  -Wexit-time-destructors
###  -Wextra-semi-stmt
###  -Wfloat-conversion
# -Wfloat-equal: too many false positives, e.g., 0.0 == X
###  -Wglobal-constructors
###  -Wimplicit-fallthrough
# -Wlogical-op-parentheses: && is multiplicative; || is additive
# -Wmismatched-tags: stylistic freedom--not a defect
###  -Wmissing-noreturn
###  -Wmissing-prototypes
###  -Wmissing-variable-declarations
# -Wpadded: useful only for low-level work
###  -Wsign-conversion
# -Wstring-plus-int: false negatives and no true positives in lmi
###  -Wundefined-func-template
# -Wunreachable-code-break: pleonastic 'break' adds clarity
# -Wweak-template-vtables: same as '-Wweak-vtables'
# -Wweak-vtables: it's better to let the linker remove duplicates

clang_cxx_warnings := \
  $(clang_common_warnings) \
  -Wno-c++20-compat \
  -Wno-c++98-compat-pedantic \
  -Wno-comma \
  -Wno-conditional-uninitialized \
  -Wno-covered-switch-default \
  -Wno-date-time \
  -Wno-deprecated-copy-with-dtor \
  -Wno-disabled-macro-expansion \
  -Wno-documentation \
  -Wno-documentation-html \
  -Wno-documentation-unknown-command \
  -Wno-double-promotion \
  -Wno-exit-time-destructors \
  -Wno-extra-semi-stmt \
  -Wno-float-conversion \
  -Wno-float-equal \
  -Wno-global-constructors \
  -Wno-implicit-fallthrough \
  -Wno-logical-op-parentheses \
  -Wno-mismatched-tags \
  -Wno-missing-noreturn \
  -Wno-missing-prototypes \
  -Wno-missing-variable-declarations \
  -Wno-padded \
  -Wno-sign-conversion \
  -Wno-string-plus-int \
  -Wno-undefined-func-template \
  -Wno-unreachable-code-break \
  -Wno-weak-template-vtables \
  -Wno-weak-vtables \

# Target-specific modifications.

$(cgicc_objects): clang_common_extra_warnings += \
  -Wno-shorten-64-to-32 \
  -Wno-unknown-pragmas \
  -Wno-zero-as-null-pointer-constant \

# Keep version-specific warnings last, so that they override others.

C_WARNINGS = \
  $(clang_c_warnings) \
  $(clang_common_extra_warnings) \
  $(clang_version_specific_c_warnings) \

CXX_WARNINGS = \
  $(clang_cxx_warnings) \
  $(clang_common_extra_warnings) \
  $(clang_version_specific_cxx_warnings) \

# This file does not end in backslash-newline.
