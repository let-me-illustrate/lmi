# Parse compiler output for diagnostics.
#
# Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
# email: <gchicares@sbcglobal.net>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
#
# $Id$
#
  # Delete expected output from make.
/Circular.*dependency dropped/d
/Entering directory/d
/Leaving directory/d
/is up to date/d
/Nothing to be done for/d
/^GNUmakefile: MAKING/d
/^Makefile: MAKING/d
/submake[0-1]: MAKING/d
/submake[0-1]: with dependencies/d
/dependencies: /d
/^cp lmi/d
/^cp --preserve lmi/d
/^.:.gnu.cp --preserve lmi/d
/^mkdir /d
/^.:.gnu.mkdir /d
/^Begun at /d
/^Ended at /d
/^Elapsed time.*milliseconds/d
/^rm eraseme.o$/d
/^.:.gnu.rm eraseme.o$/d
/not remade because of errors\./d
/^These files are more recent than/d
/Built [0-9].*T[0-9].*Z\.$/d
/^make.*\[[0-9]*\]: warning: -jN forced in submake: disabling jobserver mode.$/d
/^make.*\[[0-9]*\]: \*\*\* \[.*\] Error [0-9]*$/d
/^make.*\[[0-9]*\]: \[.*\] Error [0-9]* (ignored)$/d
  # Delete expected output from gnu cpp.
/^[^ ]*CPP *-x/d
/^[^ ]*cpp *-x/d
  # Delete expected output from gcc build.
/^[^ ]*gcc.*-[Ico] /d
/^[^ ]*g++.*-[Ico] /d
/^[^ ]*sed -e.* /d
/^[^ ]*windres /d
/total time in link/d
/ld: data size /d
/^[^ ]*ar -[rus]* /d
/^[^ ]*ar: creating /d
/^Creating library file:.*.a$/d
/^< [a-z0-9].d0/d
  # Delete expected output from como build.
/^como  -c/d
/^como -o/d
/^Comeau/d
/^Copyright/d
/^MODE:.*C++/d
/^C++ prelinker:.*assigned to file [^ ]*o$/d
/^C++ prelinker:.*adopted by file [^ ]*o$/d
/^C++ prelinker:.*no longer needed in [^ ]*o$/d
/^C++ prelinker: executing:/d
  # Delete expected output from borland build.
/^[^ ]*bcc_cc --accept/d
/^Borland C++/d
/^Loaded pre-compiled headers\.$/d
/^[^ ]*bcc_ld --accept/d
/^Turbo Incremental Link/d
/^\\[/A-Za-z0-9_]*.cpp:$/d
  # Delete multiple blank lines.
/./,/^$/!d

