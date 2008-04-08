# Makefile: automatic dependencies.
#
# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: autodependency.make,v 1.13 2008-04-08 19:03:44 chicares Exp $

################################################################################

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

################################################################################

# 'include' this in another makefile to create automatic dependencies.

# The 'automake' technique is discussed here:
#   http://make.paulandlesley.org/autodep.html
# Its 'sed' processing is no longer necessary because of '-MP':
#   http://makelinux.org/make3/make3-CHP-8-SECT-3.html
# | This completely eliminates the messy five-part sed expression in
# | our make-depend function. It seems that the automake developers
# | who invented the phony target technique caused this option to be
# | added to gcc.
# As that page further notes,
# | Even when proprietary compilers are used it may be possible to use
# | gcc for dependency management.
# To do so, tell GNU cpp to impersonate the other toolchain. Two kinds
# of flags are required: one set to say it's not gcc
#   -undef -nostdinc
#   -U __GNUC__ -U __GNUG__ -U __GNUC_MINOR__ -U __GNUC_PATCHLEVEL__
# ('-no-gcc' is no longer available:
#   http://gcc.gnu.org/ml/gcc-patches/2003-03/msg00919.html
# so gcc macros must be explicitly undefined); and another set to
# mimic the other toolchain, such as
#   -D _M_IX86=300 -D __BORLANDC__=0x500 -D _WIN32 -D __FLAT__
# for borland C++ 5.02 . Specifying the language explicitly
#   -x c++
# may be necessary in exotic cases like msw "resource" files.
# TODO ?? But it's done promiscuously below.]

# Maintainers should note that
#   http://gcc.gnu.org/ml/gcc-bugs/2003-05/msg00161.html
# | the behaviour of the -M options is complex and convoluted
# My measurements show that '-MMD' improves build speed by five or ten
# percent over '-MD'. It would be dangerous to use any preprocessor
# older than gcc-3.x's: the documentation for gcc-2.95's preprocessor
# says it treats
#   #include <my_own_header_included_with_angle_brackets.hpp>
# as a system file, which would be most undesirable because lmi uses
# that syntax for third-party headers (as does boost).

# TODO ?? Investigate and resolve this issue:
# 2003-05-04 GWC changed command option
#    -M
# to
#    -M -MT $@ \
# for the gcc-3.x+ preprocessor. Tested with gcc-3.2.3, but not with
# any earlier 3.x version.
#
# Purpose: to overcome a change in preprocessor behavior between
# gcc-2.95x and gcc-3.x that had caused a problem in situations like:
#
# .SUFFIXES:
# %.rc.o : %.rc
#         $(MAKEDEPEND)
#         $(RC) $(ALL_RCFLAGS) $< -o $@
#
# 2.95
#   $/gcc-2.95.2-1/bin/cpp --version
#   2.95.2
#   $/gcc-2.95.2-1/bin/cpp -M resource.rc
#   resource.rc.o: resource.rc header.hpp
# Note the 'rc' in target name 'resource.rc.o'...
#
# 3.2.3
#   $/MinGW/bin/cpp --version
#   cpp.EXE (GCC) 3.2.3 (mingw special 20030425-1) [...]
#   $/MinGW/bin/cpp -M resource.rc
#   resource.o: resource.rc header.hpp
# Oops, no more 'rc' in target name
#
#   $/MinGW/bin/cpp -MT resource.rc.o -M resource.rc
#   resource.rc.o: resource.rc header.hpp
# 3.x specific, but it works.
#
# With this modification, the gcc-2.95.2-1 preprocessor reports
# 'unrecognized option' for '-MT', which seems benign, but the
# filename that follows may cause grief--so use '-MT' only with the
# gcc-3.x preprocessor.

# For gcc, simply add these flags when invoking the preprocessor
# routinely via the compiler driver; then autodependency files are
# produced as a side effect, saving a costly extra invocation.

MAKEDEPEND_FLAGS = \
  -MMD -MP -MF $*.d \

# For toolchains other than gcc, invoke this command as an extra step
# after compiling.

MAKEDEPEND_NON_GCC_COMMAND = \
  $(GNU_CPP) \
    -x c++ \
    $(ALL_CPPFLAGS) \
    $(comp_cpp_pretend_flags) \
    $< -MM -MP -MF $*.d \

-include *.d
*.d:: ;

