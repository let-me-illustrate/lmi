# Makefile: automatic dependencies.
#
# Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# http://savannah.nongnu.org/projects/lmi
# email: <chicares@cox.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: autodependency.make,v 1.1 2005-01-28 01:34:43 chicares Exp $

################################################################################

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

################################################################################

# 'include' this in another makefile to create automatic dependencies.

# The technique is discussed here
#   http://make.paulandlesley.org/autodep.html
# by Paul D. Smith, who credits Tom Tromey with its invention.

# In 2000, with refinements in later years, Gregory W. Chicares
# adapted it to work on the msw platform as well as with free
# operating systems, and with compilers other than gcc. See my
# 2000-11-15T19:11:59-0500 post to the mingw mailing list
#   http://sourceforge.net/mailarchive/message.php?msg_id=600022
# and the further discussion here:
#   http://mail.gnu.org/archive/html/make-w32/2002-07/msg00008.html
# Any defect should not reflect on Paul D. Smith or Tom Tromey's
# reputations.
#
# The most important changes are discussed below; the attempts to make
# this work with ms shells are of historical interest only, and may be
# taken as reasons to use a bourne-compatible shell on msw.

# First, tell GNU cpp to pretend it's whatever compiler we're creating
# automatic dependencies for. For gcc, this just means setting
#   comp_cpp_pretend_flags := -gcc
# For other compilers, we need two kinds of flags: one set to say it's not gcc
#   -undef -no-gcc -nostdinc
# and another set to mimic macros the other compiler predefines, such as
#   -D _M_IX86=300 -D __BORLANDC__=0x500 -D _WIN32 -D __FLAT__
# for borland C++ 5.02 . In either case, for a C++ project add
#   -xc++
# to let the preprocessor know it's handling C++. Note that
#   -x c++
# with an embedded space should be OK, and msw2000's CMD.EXE accepts it,
# but msw95's COMMAND.COM does not.
#
# [Due to later revisions, don't assume that this method works with
# any msw command interpreter. Get a bourne-compatble shell instead.]
#
# I changed Paul's sed commands to work with msw almost as well as
# with free operating systems. Here's a line by line explanation:
#
# Concatenate all lines continued with '\'
#               -e :a -e '/\\$$/N; s/\\\n//; ta' \
# Of course, sed commands in a makefile must double any dollar sign.
#
# Paul's sed script copies the original output of cpp to a file, then
# appends to it with >> . The worst problem here is that msw creates
# the file even if sed fails. Therefore, we'll store what we've got so
# far in the hold space with 'h', then later append it with 'G', and
# write it with 'w'.
#
# ['w' later changed--see below.]
#
#               -e 'h' \
#
# Here is what Paul does to prevent ugly errors when a file previously
# in the dependency list has been erased or renamed. Remove the target
# at the beginning,
#               -e 's/^[^:]*: *//' \
# and place a colon at the end. We can skip his steps to delete blank
# lines because we started out by making the whole expression one
# single line.
#               -e 's/$$/ :/' \
#
# As described above, append the contents of the hold space that we
# saved earlier,
#               -e 'G' \
# then write the whole thing to the target file (stem plus '.d'
# extension). If sed terminated early due to an error, the 'w' command
# is not reached, and no file is written. [See revisions below.]
#
# Make the dependency files in a separate step, e.g.
#
# %.o : %.cpp
#   $(MAKEDEPEND)
#   $(CXX) -c $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) $< -o$@
#
# If we were never going to use any other compiler than gcc, then we
# could combine this with the compile command, e.g., with '-MD',
# paying careful attention to the complex interactions among '-M'
# options:
#   http://gcc.gnu.org/ml/gcc-bugs/2003-05/msg00161.html

# A problem can arise if cpp fails abnormally:
#   http://mail.gnu.org/archive/html/help-make/2004-01/msg00065.html
# In that case, empty dependency files may be created, which prevent
# make from working correctly. I know of no better method to handle
# this than to check for zero-byte '.d' files explicitly.

# Changed 2001-12-03 by GWC: Instead of using sed's 'w' option, write
# the dependency file by redirecting sed's output:
#    > $*.d
# If the sed script failed, then 'sh' creates a zero-byte file.
#
# A zero-byte file is preferable to no file. We can find all zero-byte
# '.d' files more easily than we can determine which '.d' files should
# have been created but were not.
#
# It seems that writing a zero-byte file was already the behavior with
# Paul's original technique anyway.

# GWC 2001: Temporary (I hope) fix for cygwin. With the latest cygwin
# as of 2001-06-15 (I find no version number in the download),
# 'cpp -M' has two problems. First, it assumes that '/usr' maps to
# '/Cygwin/usr', which doesn't work in shells provided by msw. Second,
# it refers to some headers as being in '/usr/lib', whereas they are
# actually in Cygwin's '/lib', which is '/Cygwin/lib'. We use sed to
# fix these problems.
#
# 2003-04-05 GWC moved the cygwin workaround
#   comp_autodependency_kludge := \
#     -e 's_/usr/include_/cygwin/usr/include_g' \
#     -e 's_/usr/lib_/cygwin/lib_g' \
# into a cygwin-specific include file, referencing it here simply as
#   $(comp_autodependency_kludge)
# TODO ?? That other file was replaced. Put this workaround in its
# replacement if testing shows that cygwin still has this problem.

# 2001-12-03 GWC replaced single quote with double quote throughout.
# Required for msw2000; not tested with msw95. This change was later
# reverted: the protean quoting behavior of msw shells is undocumented
# and no simple solution can work for all of them, so just use a port
# of a bourne-compatible shell on the msw platform.

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
# gcc-3.x preprocessor. If you're still using a preprocessor older
# than gcc-3.x's, change '-M -MT $@' to '-M' in $(MAKEDEPEND) below.

autodependency_sed_commands = \
  -e :a -e '/\\$$/N; s/\\\n//; ta' \
  -e 'h' \
  -e 's/^[^:]*: *//' \
  -e 's/$$/ :/' \
  -e 'G' \
  $(comp_autodependency_kludge) \

MAKEDEPEND = \
  $(GNU_PREPROCESSOR) \
    -xc++ \
    $(ALL_CPPFLAGS) \
    $(comp_cpp_pretend_flags) \
    -M -MT $@ $< \
    | $(SED) $(autodependency_sed_commands) \
    > $*.d

# 2005-01-15 GWC Split $(MAKEDEPEND) in two to support this more
# natural use with gcc:
#
#   %.o: %.cpp
#       $(CXX) $(MAKEDEPEND_0) -c $(CPPFLAGS) $(CXXFLAGS) $< -o$@
#       $(MAKEDEPEND_1)
#
# TODO ?? For gcc, this is an improvement because it avoids invoking
# the preprocessor twice. But it breaks autodependency generation for
# other compilers. Can that problem be fixed by moving autodependency
# variables out of the pattern rules and into the compiler-specific
# definitions of $(CC) and $(CXX)? Is it possible to eliminate the
# intermediate '.d0' file?

MAKEDEPEND_0 = \
  -MD -MF $*.d0 \

MAKEDEPEND_1 = \
  $(SED) $(autodependency_sed_commands) < $*.d0 > $*.d; \
  $(RM) $*.d0 \

-include *.d
*.d: ;

