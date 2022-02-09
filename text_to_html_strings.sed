#!/bin/sed -f

# Transform flat text to html represented as C strings.
# Motivating example:
#   char const c_string[] =
#   #include "output_of_this_script"
#   ;

# Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

# Replace certain special characters with html entities as required
# for the output to be valid html. This script does not attempt to
# handle quotes or dashes more tastefully than its input: that would
# be complex and error prone.
s|&|\&amp;|g
s|<|\&lt;|g
s|>|\&gt;|g

# Expunge formfeeds; insert paragraph tags.
s|\f||g
s|^$|</p>\n\n<p>|

# Don't pretend that '`' is &lsquo;.
s|`|'|g

# Add a generic html header and footer. Implementation notes:
#
# The header and footer must be quoted for use as C strings. It seems
# unnatural to do that manually here instead of using the reliable
# commands applied below to the input. Therefore, sed's 'a' and 'i'
# commands cannot be used to insert the header and footer, because
# the text they add is queued directly for output to stdout and
# cannot be processed further.
#
# The header is inserted through substitutions on the first line.
# It would seem simpler to insert '\n' before each line of text
# inserted, but that would insert the lines in reverse order, and
# reversing the order in which they occur in this script to make it
# come out right would make the script difficult to read. Therefore,
# '\a' is used as a temporary sentry to mark the end of the text
# inserted at each step.
#
# Page break support using CSS2 requires html version 4. Version 4.01
# is used instead of 4.0, as w3c.org recommends.
#
# A title tag is required inside the head tag, but there's no good
# way to specify a non-empty title automatically.

# Add generic header.
1s|^|\a|
1s|\a|<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"\a|
1s|\a|\n"http://www.w3.org/TR/html4/strict.dtd">\a|
1s|\a|\n<html>\a|
1s|\a|\n<head>\a|
1s|\a|\n<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">\a|
1s|\a|\n<title></title>\a|
1s|\a|\n</head>\a|
1s|\a|\n<body>\a|
1s|\a|\n<p>\a|
1s|\a|\n|
# Add generic footer.
$s|$|\n</p>|
$s|$|\n</body>|
$s|$|\n</html>|

# Escape all double-quote characters in the input. Add a C newline
# representation at the end of each line and surround the line in
# double quotes. Treat embedded literal newlines that arose from
# inserting the header and footer compatibly.
s|"|\\"|g
s|^|"|
s|$|\\n"|
s|\n|\\n"\n"|g
