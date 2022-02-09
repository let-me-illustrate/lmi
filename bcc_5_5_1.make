# Limited support for a particular non-free compiler.

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

LMI_COMPILER := bcc

srcdir       := $(CURDIR)

gcc_version  :=

prefix       := /opt/lmi
exec_prefix  := $(prefix)
lmi_bin_dir  := $(exec_prefix)/bin

# Use '\\' instead of 'c:' in order to avoid autodependency problems.
bcc_dir      := \\Borland/BCC55
bcc_bin_dir  := $(bcc_dir)/Bin
bcc_inc_dir  := $(bcc_dir)/Include
bcc_lib_dir  := $(bcc_dir)/Lib

compiler_include_directory := -I $(bcc_inc_dir)
compiler_runtime_files     := $(bcc_bin_dir)/cc3250.dll

shadow_header_directory    := $(bcc_inc_dir)/shadow

# Use gcc-3.x for autodependencies and physical-closure testing.
#
# Define WIN32_LEAN_AND_MEAN in order to avoid borland msw headers
# that use '#define FOO /##/', which gcc properly rejects.

compiler_impersonation_cppflags := \
  -undef -nostdinc \
  -U __GNUC__ -U __GNUG__ -U __GNUC_MINOR__ -U __GNUC_PATCHLEVEL__ \
  -D LMI_IGNORE_PCH \
  -D _M_IX86=300 -D __BORLANDC__=0x550 -D _WIN32 \
  -D WIN32_LEAN_AND_MEAN \
  -I $(shadow_header_directory) \

# This makefile because unusable long ago, so this dependency has been
# renamed to avoid pointless maintenance updates.
gcc3_dir     := /opt/lmi/no_longer_usable
gcc3_bin_dir := $(gcc3_dir)/bin

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

# The borland compiler rejects some conforming code, generating many
# spurious and distracting diagnostics; it fails utterly to build
# these tests:

excluded_unit_test_targets := \
  actuarial_table_test \
  authenticity_test \
  commutation_functions_test \
  configurable_settings_test \
  global_settings_test \
  input_test \
  irc7702a_test \
  istream_to_string_test \
  loads_test \
  math_functions_test \
  mc_enum_test \
  mortality_rates_test \
  path_utility_test \
  product_file_test \
  regex_test \
  round_test \
  tn_range_test \
  xml_serialize_test \

GNU_CPP := $(gcc3_bin_dir)/cpp
GNU_CXX := $(gcc3_bin_dir)/g++

MAKEDEPEND_FLAGS   :=
MAKEDEPEND_COMMAND := MAKEDEPEND_NON_GCC_COMMAND

# Shadow all standard headers for autodependencies, because borland
# provides only '.h'-suffixed files. These header lists are specified
# in C++2003 17.4.1.2 .
#
# Copying the borland headers wouldn't work because they contain some
# nonstandard code that the GNU preprocessor rejects, so just create
# them as empty files.

cxx_library_headers := \
  algorithm  \
  bitset     \
  complex    \
  deque      \
  exception  \
  fstream    \
  functional \
  iomanip    \
  ios        \
  iosfwd     \
  iostream   \
  istream    \
  iterator   \
  limits     \
  list       \
  locale     \
  map        \
  memory     \
  new        \
  numeric    \
  ostream    \
  queue      \
  set        \
  sstream    \
  stack      \
  stdexcept  \
  strstream  \
  streambuf  \
  string     \
  typeinfo   \
  utility    \
  valarray   \
  vector     \

cxx_c_library_headers := \
  cassert    \
  cctype     \
  cerrno     \
  cfloat     \
  ciso646    \
  climits    \
  clocale    \
  cmath      \
  csetjmp    \
  csignal    \
  cstdarg    \
  cstddef    \
  cstdio     \
  cstdlib    \
  cstring    \
  ctime      \
  cwchar     \
  cwctype    \

MAKECMDGOALS ?= unit_tests

# Default target.
$(MAKECMDGOALS):

bcc_5_5_1.make:: ;

# Use '--jobs=1' to prevent 'make' parallelism, which appears to be
# incompatible with borland C++. Ignore any "disabling jobserver mode"
# warning that this engenders.

%: force
	+@[ -d $(shadow_header_directory) ] || mkdir $(shadow_header_directory)
	@touch \
	  $(addprefix \
	    $(shadow_header_directory)/, \
	    $(cxx_library_headers) $(cxx_c_library_headers) \
	    )
	@-$(MAKE) \
	  --file=$(srcdir)/GNUmakefile \
	  --jobs=1 \
	                             srcdir='$(srcdir)' \
	                       LMI_COMPILER='$(LMI_COMPILER)' \
	                        gcc_version='$(gcc_version)' \
	                         C_WARNINGS='$(C_WARNINGS)' \
	                       CXX_WARNINGS='$(CXX_WARNINGS)' \
	                   C_EXTRA_WARNINGS='$(C_EXTRA_WARNINGS)' \
	                 CXX_EXTRA_WARNINGS='$(CXX_EXTRA_WARNINGS)' \
	                           CPPFLAGS='$(CPPFLAGS)' \
	         compiler_include_directory='$(compiler_include_directory)' \
	             compiler_runtime_files='$(compiler_runtime_files)' \
	    compiler_impersonation_cppflags='$(compiler_impersonation_cppflags)' \
	                                CXX='$(CXX)' \
	                           CXXFLAGS='$(CXXFLAGS)' \
	                                 LD='$(LD)' \
	                            LDFLAGS='$(LDFLAGS)' \
	         excluded_unit_test_targets='$(excluded_unit_test_targets)' \
	                            GNU_CPP='$(GNU_CPP)' \
	                            GNU_CXX='$(GNU_CXX)' \
	                   MAKEDEPEND_FLAGS='$(MAKEDEPEND_FLAGS)' \
	                 MAKEDEPEND_COMMAND='$(MAKEDEPEND_COMMAND)' \
	  $(MAKECMDGOALS); \

force: ;
