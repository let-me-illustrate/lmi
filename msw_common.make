# Platform specifics: msw, shared by all subplatforms.
#
# Copyright (C) 2005 Gregory W. Chicares.
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

# $Id: msw_common.make,v 1.4 2005-02-23 12:41:23 chicares Exp $

################################################################################

EXEEXT := .exe
SHREXT := .dll

################################################################################

# Libraries.

# There is no universal standard way to install free software on this
# platform; the directories given here merely reflect locations chosen
# haphazardly by GWC. TODO ?? Make better choices where possible, and
# eliminate the corresponding workarounds here.

# Prefer to use shared-library versions of libxml2 and wx: they link
# faster and rarely need to be changed.

# Path to wxWindows.
# TODO ?? Probably the wx version should be a separate variable,
# defined elsewhere for all platforms.

wx_dir := /wx-cvs-20050216/wxWidgets

wx_platform_dir = \
  $(wx_dir)/lib/gcc_dll/mswd \

platform_defines := \
  -DLIBXML_USE_DLL \
  -DSTRICT \
  -DWXUSINGDLL \

# TODO ?? If this is just the FHS location, then define it globally.
platform_libxml2_libraries = \
  -L $(system_root)/usr/local/lib -lxml2_dll \

platform_mpatrol_libraries := \
  -limagehlp \

# TODO ?? Can the 'msw' part be factored out regularly? Else need to
# do something for GNU/Linux: e.g. copy everything to /usr/local ?
#
platform_wx_libraries := \
  -lwx_new \
  -L $(wx_dir)/lib/gcc_dll -lwxmsw25d \

################################################################################

# Paths to external libraries.

# TODO ?? Instead, try to use whatever locations FHS prescribes, and
# write subplatform-specific workarounds for any locations that can't
# conform for some good reason.

# Path to libraries from www.boost.org . Most required boost libraries
# are implemented exclusively in headers. It seems common in the *nix
# world to leave those headers in the subdirectory of /usr/local/src/
# to which the boost distribution is extracted, probably because boost
# does not put all its headers in an include/ subdirectory and it
# would be tedious to find and copy them all to /usr/local/include .
boost_dir    := $(system_root)/usr/local/src/boost_1_31_0

# Path to GNU cgicc.
cgicc_include_dir := $(system_root)/usr/local/include
cgicc_source_dir  := $(system_root)/usr/local/src/cgicc

# Path to libxml2.
libxml2_include_dir  := $(system_root)/usr/local/include/libxml2

# Path to xmlwrapp.
xmlwrapp_include_dir := $(system_root)/usr/local/include
xmlwrapp_source_dir  := $(system_root)/usr/local/src/libxml

# HTML server's cgi-bin directory.
#cgi_bin_dir := $(system_root)/unspecified/cgi-bin

