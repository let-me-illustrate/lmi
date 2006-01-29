#! /bin/sed -f

# Transform flat text to C strings.
# Motivating example:
#   char const c_string[] =
#   #include "output_of_this_script"
#   ;

# Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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

# $Id: text_to_strings.sed,v 1.2 2006-01-29 14:48:55 chicares Exp $

# Escape all double-quote characters in the input. Add a C newline
# representation at the end of each line and surround the line in
# double quotes.
s|"|\\"|g
s|^|"|
s|$|\\n"|

