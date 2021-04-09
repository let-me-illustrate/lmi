# Makefile: automatic dependencies.
#
# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

################################################################################

# Configuration.

include $(srcdir)/configuration.make
$(srcdir)/configuration.make:: ;

################################################################################

# 'include' this in another makefile to create automatic dependencies.

# The 'automake' technique is discussed here:
#   https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
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
#   -D _M_IX86=300 -D __BORLANDC__=0x550 -D _WIN32
# for borland C++ 5.5 . Specifying the language explicitly
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

# Common gcc flags for autodependencies, to which an option in
#   {-M, -MD, -MM, -MMD}
# is to be added.
#
# Specify both '-MT $@' and '-MF $(@:.o=.d)' to make a target like
#   %.rc.o : %.rc
# produce 'FOO.rc.d' containing prerequisites of 'FOO.rc.o', with
# both files containing '.rc.'.
#
# Using '-MT $@' with '-MD' or '-MMD' causes duplication of the object
# file name in the '.d' file, e.g., giving
#   FOO.o FOO.o: /src/FOO.cpp
# where
#   FOO.o: /src/FOO.cpp
# would be expected. That's been known for many years:
#   http://gcc.gnu.org/ml/gcc-patches/2001-02/msg00481.html
# | Note the way cpp gives double -o test.o below
# Apparently the real reason why that odd behavior is still present
# after all this time is that it's harmless.

common_autodependency_flags = -MP -MT $@ -MF $(@:.o=.d)

# For gcc, simply add these flags when invoking the preprocessor
# routinely via the compiler driver; then autodependency files are
# produced as a side effect, saving a costly extra invocation.

MAKEDEPEND_FLAGS = \
  -MMD $(common_autodependency_flags) \

# For toolchains other than gcc, invoke this command as an extra step
# after compiling.
#
# Always use this command for gnu 'windres'. Although it's part of the
# gcc toolchain, it offers no way to specify incremental preprocessor
# flags. It's possible to override all preprocessor flags:
#   --preprocessor='gcc -E -xc-header -DRC_INVOKED -MMD -MP...'
# but that fragile enormity isn't worth the minuscule savings that
# '-MMD' offers, because msw resource files are typically small,
# simple, and few.

MAKEDEPEND_NON_GCC_COMMAND = \
  $(GNU_CPP) \
    -x c++ \
    $(ALL_CPPFLAGS) \
    $(compiler_impersonation_cppflags) \
    -MM $(common_autodependency_flags) \
    $< \

# A problem can arise if cpp fails abnormally:
#   http://mail.gnu.org/archive/html/help-make/2004-01/msg00065.html
# In that case, empty dependency files may be created, which prevent
# make from working correctly. I know of no better method to handle
# this than to check for zero-byte '.d' files explicitly.

-include *.d
*.d:: ;
