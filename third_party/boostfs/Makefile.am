# Master automake makefile, run automake from the top directory
# to generate Makefile from it (this # is also done by autogen.sh script)
#
# Copyright (C) 2005, 2006, 2007 Vadim Zeitlin.
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

# $Id$


if LMI_BOOSTFS_BUILD
noinst_LTLIBRARIES=libboost_filesystem.la
else
noinst_LTLIBRARIES=
endif

# look for boost sources in the directory determined by configure
vpath %.cpp $(LMI_BOOSTFS_SRC)

# we do not want the sources to be included into the distribution
nodist_libboost_filesystem_la_SOURCES = \
    convenience.cpp \
    exception.cpp \
    operations_posix_windows.cpp \
    path_posix_windows.cpp 
libboost_filesystem_la_SOURCES = 
libboost_filesystem_la_LDFLAGS = -module -no-undefined

