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

# $Id: msw_common.make,v 1.2 2005-02-17 23:27:21 chicares Exp $

################################################################################

EXEEXT := .exe
SHREXT := .dll

################################################################################

# There is no universal standard way to install free software on this
# platform; the directories given here merely reflect locations chosen
# haphazardly by GWC. TODO ?? Make better choices where possible, and
# eliminate the corresponding workarounds here.

WINDOWS_REQUIRED_CPPFLAGS := \
  -DWINVER=0x0400 \
  -D__WIN95__ \
  -DSTRICT \
  -DHAVE_W32API_H \
  -D__WXMSW__ \
  -D__WINDOWS__ \
  -D__WXDEBUG__ \
  -DWXUSINGDLL \
  -DDLL_NEW_USING_DLL \
  -DNO_GCC_PRAGMA \
  -DwxUSE_STL \

################################################################################

# Libraries.

# Prefer to use a shared-library version of libxml2 because it links
# faster and rarely needs to be changed.

platform_defines := \
  -DLIBXML_USE_DLL \

platform_libxml2_libraries = \
  -L $(system_root)/usr/local/lib -lxml2_dll \

platform_mpatrol_libraries := \
  -limagehlp

wx_dll_define := -DWXUSINGDLL

# Path to wxWindows.
# TODO ?? Probably the wx version should be a separate variable,
# defined elsewhere for all platforms.

wx_dir := /wxWindows-2.5.1

wx_platform_dir = \
  $(wx_dir)/lib/gcc_dll/mswd \

wx_libraries = \
  -lwx_new \
  -L $(wx_dir)/lib/gcc_dll -lwxmsw25d_xrc -lwxmsw25d \

# TODO ?? Libraries suspected of being useful for wx.

SUSPICIOUS_LIBS := \
  -lstdc++ -lgcc -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 \
  -lcomctl32 -lctl3d32 -lodbc32 -ladvapi32 -lodbc32 -lwsock32 \
  -lopengl32 -lglu32 -lole32 -loleaut32 -luuid \

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

