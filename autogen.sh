#!/bin/sh

# Script to execute to initialize lmi build system after checking out pristine
# sources from a version control system: this script creates all generated
# files which are needed for the build but not stored under version control.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Vadim Zeitlin.
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

if [ ! -f configure.ac ] || [ ! -f Makefile.am ] || [ ! -f lmi.png ]; then
    echo "Please run this script from the lmi source directory."
    exit 2
fi

# use --foreign with automake because we lack standard GNU NEWS and AUTHOR
# files, if they're added we can "upgrade" to (default) GNU strictness.
# use --copy to allow simultaneous use on windows under mingw and cygwin platforms.
# Symlinking of files under mingw does not work out for cygwin and vice-versa.
echo "Setting up build system for lmi:"
echo " - aclocal " && aclocal -I aclocal && \
echo " - libtoolize " && libtoolize --copy --automake && \
echo " - autoconf " && autoconf && \
echo " - autoheader " && autoheader && \
echo " - automake " && automake --add-missing --copy --foreign && \
echo "Build setup successful, type \"configure\" to configure lmi now." && \
exit 0

echo "Automatic build files setup failed!"

exit 1
