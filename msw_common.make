# Platform specifics: msw, shared by all subplatforms.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

# $Id$

################################################################################

EXEEXT := .exe
SHREXT := .dll

################################################################################

# Libraries and associated options.

# There is no universal standard way to install free software on this
# platform, so copy libraries and their headers to /usr/local as FHS
# prescribes.

# Prefer to use a shared-library version of libxml2: it links faster
# and rarely needs to be updated.

platform_defines := \
  -DLIBXML_USE_DLL \
  -DSTRICT \

platform_gui_ldflags := -mwindows

platform_gnome_xml_libraries := \
  -lexslt.dll \
  -lxslt.dll \
  -lxml2.dll \

platform_mpatrol_libraries := \
  -limagehlp \

platform_wx_libraries := \
  -lwx_new \
  -lwxmsw25d \

# In addition to other reasons for treating these libraries as mere
# collections of source files, there's a specific problem with boost.
# The build system provided by boost produces library names such as
# 'libboost_filesystem-mgw.a', which violates the rule here
#   http://www.boost.org/more/lib_guide.htm#Directory_structure
# because it contains a hyphen, which causes problems with borland
# tools, apparently because it looks like an option prefix.

platform_boost_libraries :=

platform_cgicc_libraries :=

platform_xmlwrapp_libraries :=

################################################################################

# HTML server's cgi-bin directory. Not used yet. Eventually, an
# 'install' target might copy cgi-bin binaries thither.
#
#cgi_bin_dir := $(system_root)/unspecified/cgi-bin

