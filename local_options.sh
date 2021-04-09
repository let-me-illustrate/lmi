#!/bin/sh

# Conveniently save commonly-used options.

# Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

# This sample usage:
#   ./local_options mingw_dir=/MinGW-20050120/ CFLAGS='-g -O0'
# creates a file 'local_options.make' that contains
#   local_options += mingw_dir='/MinGW-20050120/'
#   local_options += CFLAGS='-g -O0'
# which is included by 'GNUmakefile', which passes those options to
# submakefiles. This is purely a convenience: the same result would
# be obtained by invoking 'make' with the same arguments.
#
# Naturally, any option containing whitespace must be quoted.
#
# Any variable defined here is overridden by any definition of the
# same variable on the 'make' command line. Thus, in the example
# above, if make is invoked as
#   make CFLAGS='-Wall'
# then the value '-g -O0' in 'local_options.make' is ignored and
# '-Wall' is used instead.
#
# It is not suggested that this is a good way of managing options in
# general. The section "Variables from the Environment" in the GNU
# 'make' manual describes a substantially similar way of handling
# options; all the warnings given there apply to this technique, too.
#
# One reasonable use is to supply a persistent local definition of
# $(mingw_dir), in order to avoid the problem described here:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/14747
#     [2005-01-17T16:30:44Z from Greg Chicares]
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/14748
#     [2005-01-17T18:15:26Z from Aaron W. LaFramboise]
# when multiple versions of MinGW gcc are installed.

{
  echo "# Copyright (C) $(date -u +'%Y') [you]."
  echo "# https://savannah.nongnu.org/projects/lmi"
  echo ""
  echo "# Local options"
} > local_options.make
for z in "$@"
  do
    echo "$z" \
    | sed -e "/=/{s/=/='/;s/$/'/}" \
    | sed -e "s/^/local_options += /" \
    >> local_options.make
  done
