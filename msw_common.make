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

# $Id: msw_common.make,v 1.8 2005-07-24 23:13:50 chicares Exp $

################################################################################

EXEEXT := .exe
SHREXT := .dll

################################################################################

# Libraries.

# There is no universal standard way to install free software on this
# platform, so copy libraries and their headers to /usr/local as FHS
# prescribes.

# TODO ?? At least as a temporary expedient, I copied setup.h to
# /usr/local/include/wx from /wxXXX/lib/.../mswd; is that kosher?

# Prefer to use shared-library versions of libxml2 and wx: they link
# faster and rarely need to be changed.

platform_defines := \
  -DLIBXML_USE_DLL \
  -DSTRICT \
  -DWXUSINGDLL \

platform_boost_libraries := \
  -lboost_filesystem-mgw \

platform_libxml2_libraries := \
  -lxml2.dll \

platform_mpatrol_libraries := \
  -limagehlp \

platform_wx_libraries := \
  -lwx_new \
  -lwxmsw25d \

platform_gui_ldflags := -mwindows

################################################################################

# HTML server's cgi-bin directory. Not used yet. Eventually, an
# 'install' target might copy cgi-bin binaries thither.
#
#cgi_bin_dir := $(system_root)/unspecified/cgi-bin

