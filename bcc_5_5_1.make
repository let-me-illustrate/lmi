# Limited support for a particular non-free compiler.

# Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

# $Id: bcc_5_5_1.make,v 1.12 2008-04-01 15:57:32 chicares Exp $

toolset     := bcc

src_dir     := $(CURDIR)

gcc_version :=

prefix      ?= /opt/lmi
exec_prefix ?= $(prefix)
lmi_bin_dir := $(exec_prefix)/bin

bcc_dir     := C:/Borland/BCC55
bcc_bin_dir := $(bcc_dir)/Bin
bcc_inc_dir := $(bcc_dir)/Include
bcc_lib_dir := $(bcc_dir)/Lib

compiler_include_directory := -I $(bcc_inc_dir)
compiler_runtime_files     := $(bcc_bin_dir)/cc3250.dll

# Casual workarounds for borland C++ version 5.5.1 . The vendor calls
# this compiler "free", but it is not: it is merely gratis. It may
# violate the vendor's license to distribute binaries built with that
# compiler. It is casually supported through this makefile only
# because its implementation-specific behavior differs from gcc's, so
# using it at least for unit tests may reveal portability defects.

# This makefile requires wrappers for the vendor's command-line tools,
# because their syntax is convoluted.
#
# Options for borland compilers:
# -a   alignment (-a1 is byte [5.02 default]; -a4 is 4-byte [5.5.1 default])
# -g0  max warnings (0 means infinity)
# -H=  name of pch file
# -Hc  cache pch file
# -j0  max errors (0 means infinity)
# -Od  disable all optimizations
# -O2  optimize for speed
# -Q   extended error information (introduced with version 5.5x)
# -R   pollute object files with 'object browser' overhead
# -v   debug info in object files
# -WM  multithreaded
# -X   don't pollute object files with proprietary autodependency info
# -xd  call dtors when unwinding stack on exception (defective in 5.02!)
# -Z   suppress redundant loads
#
# Turn off the 'inl' and 'csu' warnings, there being no other apparent
# way to compile the borland/rw STL without warnings.
#
# Turn off the 'pck', 'pow', and 'onr' warnings because boost/config
# generates them in great profusion.
#
# Don't use borland's -O2 (optimize for speed) option because it
# enables following useful options
#   -Ob -Oe -Og -Ol -Om -Op -OS -Ov -Z
# and -Oi as well--but -Oi is broken. If -Oi is used, then e.g.
# std::strcpy() does not exist. This defect has been documented in the
# borland newsgroups.

C_WARNINGS         :=
CXX_WARNINGS       :=

C_EXTRA_WARNINGS   :=
CXX_EXTRA_WARNINGS :=

CXX := \
  $(lmi_bin_dir)/bcc_cc \
  --accept --program $(bcc_bin_dir)/bcc32 \

CXXFLAGS := \
  -D_RTLDLL \
  -a1 -g0 -H=lmi.csm -Hc -j0 -Q -R- -WM- \
  -w -w-inl -w-csu -w-onr -w-pck -w-pow \
  -Ob -Oe -Og -Ol -Om -Op -OS -Ov -v- \

LD := \
  $(lmi_bin_dir)/bcc_ld \
  --accept --program $(bcc_bin_dir)/ilink32 \

LDFLAGS := \
  -L$(bcc_lib_dir) /ap /c /E0 /m /Tpe /V4.0 /w /w-dup /w-dpl \
  --startup-file $(bcc_lib_dir)/c0x32.obj import32.lib cw32i.lib \

MAKEDEPEND_0 :=

MAKEDEPEND_1 :=

MPATROL_LIBS :=

# The borland linker fails with this library search path:
#   /wx-cvs-20050216/wxWidgets/lib/gcc_dll
# Perhaps it requires an msdos path. Passing an empty variable
# definition to the main makefile at least allows unit tests to link.

platform_wx_libraries :=

# The borland compiler rejects some conforming code, generating many
# spurious and distracting diagnostics. It fails to compile these
# tests as of 2005-02-21:
#   mc_enum_test$(EXEEXT)
#   value_cast_test$(EXEEXT)

all:

bcc_5_5_1.make:: ;

%: force
	@-$(MAKE) \
	  -f $(src_dir)/GNUmakefile \
	                       src_dir='$(src_dir)' \
	                       toolset='$(toolset)' \
	                   gcc_version='$(gcc_version)' \
	                    C_WARNINGS='$(C_WARNINGS)' \
	                  CXX_WARNINGS='$(CXX_WARNINGS)' \
	              C_EXTRA_WARNINGS='$(C_EXTRA_WARNINGS)' \
	            CXX_EXTRA_WARNINGS='$(CXX_EXTRA_WARNINGS)' \
	                      CPPFLAGS='$(CPPFLAGS)' \
	    compiler_include_directory='$(compiler_include_directory)' \
	        compiler_runtime_files='$(compiler_runtime_files)' \
	                           CXX='$(CXX)' \
	                      CXXFLAGS='$(CXXFLAGS)' \
	                            LD='$(LD)' \
	                       LDFLAGS='$(LDFLAGS)' \
	                  MAKEDEPEND_0='$(MAKEDEPEND_0)' \
	                  MAKEDEPEND_1='$(MAKEDEPEND_1)' \
	                  MPATROL_LIBS='$(MPATROL_LIBS)' \
	         platform_wx_libraries='$(platform_wx_libraries)' \
	  unit_tests \

force: ;

