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

# $Id: msw_common.make,v 1.3 2005-02-23 12:37:20 chicares Exp $

################################################################################

EXEEXT := .exe
SHREXT := .dll

################################################################################

# TODO ?? Comment blocks marked '[expunge]' explain why some things
# that had never been documented well inline are now suppressed. They
# will be expunged after the next cvs commit, so that they leave a
# trace which can be found e.g. with 'cvs annotate'.

# Removed: no evidence that these are needed. [expunge]
#
# Maybe these were once useful for the old wx build system:
# http://sf.net/tracker/index.php?func=detail&aid=652491&group_id=9863&atid=309863
# but that was replaced quite a while ago.
#
# WINDOWS_REQUIRED_CPPFLAGS := \
#   -DWINVER=0x0400 \
#   -D__WIN95__ \
#   -DHAVE_W32API_H \
#   -D__WXMSW__ \
#   -D__WINDOWS__ \

# Removed: part of $(REQUIRED_CPPFLAGS) at least for now. [expunge]
#
#  -D__WXDEBUG__ \

# Removed: now part of $(platform_defines). [expunge]
#
#  -DSTRICT \
#  -DWXUSINGDLL \

# Removed: now used in target-specific contexts. [expunge]
#
#  -DDLL_NEW_USING_DLL \

# Removed: fixed in gcc-3.4.x . [expunge]
#
#  -DNO_GCC_PRAGMA \
#
# Reference: GWC message Mon, 22 Mar 2004 00:02:29 -0500:
# http://article.gmane.org/gmane.comp.lib.wxwidgets.devel/45251
#
# On Sun, 20 Feb 2005 12:56:55 +0100, Mattia Barbon
# <mattia.barbon@libero.it> wrote:
# http://article.gmane.org/gmane.comp.lib.wxwindows.general/25373
# > It is a MinGW bug; it is fixed in GCC 3.4.x.
# > A workaround for the test case I had is not using
# > #pragma interface/implementation.

# Removed: this had always been a bad idea. [expunge]
#
#  -DwxUSE_STL \
#
# On 2004-12-15 10:19 AM, Vadim Zeitlin <vadim@tt-solutions.com>
# wrote [personal communication]:
# > This is not necessary (you get wxUSE_STL value from setup.h)
# > and could actually be harmful as you need to do -DwxUSE_STL=1
# > because it is tested with #if, not #ifdef.

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

# TODO ?? Removed: duplicative and unreferenced. [expunge]
# The 'xrc' library has been integrated with the base wx library, so
# it need no longer be specified separately.
#
# wx_libraries = \
#   -lwx_new \
#   -L $(wx_dir)/lib/gcc_dll -lwxmsw25d_xrc -lwxmsw25d \

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

# Removed: not necessary. [expunge]
#
# SUSPICIOUS_LIBS := \
#   -lstdc++ -lgcc -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 \
#   -lcomctl32 -lctl3d32 -lodbc32 -ladvapi32 -lodbc32 -lwsock32 \
#   -lopengl32 -lglu32 -lole32 -loleaut32 -luuid \

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

