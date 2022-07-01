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
#
# -Wstring-plus-int: false negatives and no true positives in lmi.

treat_warnings_as_errors := -pedantic-errors -Werror

clang_common_warnings := \
  $(treat_warnings_as_errors) \
  -Wno-string-plus-int \

clang_c_warnings := \
  $(clang_common_warnings) \

clang_cxx_warnings := \
  $(clang_common_warnings) \

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
