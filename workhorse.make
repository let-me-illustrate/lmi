# Main lmi makefile, invoked by 'GNUmakefile'.
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

# $Id: workhorse.make,v 1.3 2005-02-03 16:03:37 chicares Exp $

###############################################################################

# This is not a top-level makefile.

ifeq (0,$(MAKELEVEL))
  $(error Don't use this as a top-level makefile)
endif

###############################################################################

# Directives and such.

# The GNU make manual says:
#   "This is almost always what you want make to do,
#   but it is not historical practice; so for
#   compatibility, you must explicitly request it."

.DELETE_ON_ERROR:

################################################################################

# Default target.

# The default target is the first target by definition. But the
# dependencies desired here in turn depend on $(EXEEXT), which is
# defined in a configuration makefile that's included below, and that
# makefile has a target to prevent attempting to build it. To resolve
# that circularity, this target depends on another target that's
# defined after other makefiles are included.

.PHONY: all
all: effective_default_target

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

$(src_dir)/workhorse.make:: ;

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

# Automatic dependencies.

include $(src_dir)/autodependency.make
$(src_dir)/autodependency.make:: ;

# Objects.

include $(src_dir)/objects.make
$(src_dir)/objects.make:: ;

################################################################################

# Effective default target (described above under "Default target").

default_targets := \
  shared_demo$(EXEEXT) \
  static_demo$(EXEEXT) \
  antediluvian_cgi$(EXEEXT) \
  antediluvian_cli$(EXEEXT) \

.PHONY: effective_default_target
effective_default_target: $(default_targets)

################################################################################

# Location of include files and prerequisites.

# Prefer the 'vpath' directive to the $(VPATH) variable because
#  - it permits finer control; and
#  - $(VPATH) requires a platform-dependent path separator, which
#    makes it harder to write a cross-platform makefile.

# Treat certain external libraries as collections of source files to
# be compiled and linked explicitly here, instead of building them
# separately and linking them as normal libraries. Rationale:
#
# boost: the build system provided is not free.
#
# cgicc: './configure && make' fails in the MSYS environment.
#
# xmlwrapp: avoid the nonstandard build system provided.

all_include_directories := \
  $(src_dir) \
  $(boost_dir) \
  $(cgicc_dir) \
  $(libxml2_dir)/include/libxml2 \
  $(wx_platform_dir) \
  $(wx_dir)/include \
  $(wx_dir)/contrib/include \
  $(xmlwrapp_dir)/src/libxml \
  $(xmlwrapp_dir)/include \

all_source_directories := \
  $(src_dir) \
  $(src_dir)/products \
  $(boost_dir)/libs/filesystem/src \
  $(cgicc_dir)/cgicc \
  $(xmlwrapp_dir)/src/libxml \

vpath lib%.a          $(CURDIR)
vpath %.o             $(CURDIR)
vpath %.c             $(all_source_directories)
vpath %.cpp           $(all_source_directories)
vpath %.cxx           $(all_source_directories)
vpath %.h             $(all_source_directories)
vpath %.hpp           $(all_source_directories)
vpath %.tpp           $(all_source_directories)
vpath %.xpp           $(all_source_directories)
vpath quoted_gpl      $(src_dir)
vpath quoted_gpl_html $(src_dir)

################################################################################

# Warning options for gcc.

gcc_warnings := \
  -pedantic \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated \
  -Wimport \
  -Wnon-template-friend \
  -Woverloaded-virtual \
  -Wpmf-conversions \
  -Wpointer-arith \
  -Wsign-compare \
  -Wsynth \
  -Wwrite-strings \

# Some boost libraries treat 'long long' as part of the language,
# which it probably soon will be, so permit it now.

gcc_warnings += -Wno-long-long

# WX!! The wx library triggers many warnings with these flags:
#  -W \
#  -Wcast-qual \

gcc_extra_warnings := \
  -W \
  -Wcast-qual \
  -Wctor-dtor-privacy \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wendif-labels \
  -Winvalid-offsetof \
  -Wmultichar \
  -Wpacked \
  -Wredundant-decls \

# Too many warnings on correct code, e.g. exact comparison to zero:
#  -Wfloat-equal \

# Too many warnings for various boost libraries:
#  -Wold-style-cast \
#  -Wshadow \

# Boost deems that no one should use this warning flag:
#   http://aspn.activestate.com/ASPN/Mail/Message/boost/1822550

#  -Wundef \

# Too many warnings for libstdc++:
#  -Wunreachable-code \

# Since at least gcc-3.4.2, -Wmissing-prototypes is deprecated as
# being redundant for C++.

# At least until we integrate wx, all these warnings are OK:

gcc_warnings += $(gcc_extra_warnings)

C_WARNINGS         := $(gcc_warnings) -std=c99 -Wmissing-prototypes
CXX_WARNINGS       := $(gcc_warnings) -std=c++98

C_EXTRA_WARNINGS   := $(gcc_extra_warnings)
CXX_EXTRA_WARNINGS := $(gcc_extra_warnings)

################################################################################

# Build type governs optimization flags and use of mpatrol.

# TODO ?? Also consider defining these libstdc++ macros:
#   _GLIBCXX_DEBUG_ASSERT
#   _GLIBCXX_DEBUG_PEDASSERT
#   _GLIBCXX_DEBUG_VERIFY

ifeq (mpatrol,$(findstring mpatrol,$(build_type)))
  optimization_flag := -O0
  MPATROL_LIBS := -lmpatrol -lbfd -liberty $(platform_mpatrol_libraries)
else
  optimization_flag := -O2
  MPATROL_LIBS :=
endif

################################################################################

# Required libraries.

LIBXML2_LIBS := \
  $(platform_libxml2_libraries)

################################################################################

# Flags.

# Define these variables recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the variables
# they're composed from.

# Use '-g' instead of '-ggdb'. MinGW gcc-3.4.2 writes dwarf2 debug
# records if '-ggdb' is specified, but the version of gdb packaged
# with it expects stabs format.

CFLAGS = \
  -g $(optimization_flag) \

CXXFLAGS = \
  -g $(optimization_flag) \

# INELEGANT !! Define BOOST_DEPRECATED until code that uses the
# deprecated boost::bind library is rewritten.

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(all_include_directories)) \
  $(platform_defines) \
  -DBOOST_DEPRECATED \

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \

REQUIRED_ARFLAGS = \
  -rus

# Prefer to invoke GNU 'ld' through the compiler frontends 'gcc' and
# 'g++' because that takes care of linking the required libraries for
# each language. Accordingly, pass GNU 'ld' options with '-Wl,'.

# The link command promiscuously mentions every library that might be
# wanted for any target. Measurements show that this costs one-tenth
# of a second on reasonable hardware, and it saves the trouble of
# maintaining a list of which targets require which libraries.

REQUIRED_LDFLAGS = \
  -L . \
  -L /usr/local/lib \
  $(LIBXML2_LIBS) \
  $(MPATROL_LIBS) \
  -Wl,-Map,$@.map \

# The '--use-temp-file' windres option seems to be often helpful and
# never harmful.

REQUIRED_RCFLAGS = \
  $(ALL_CPPFLAGS) \
  --use-temp-file \

# To create msw import libraries, use '-Wl,--out-implib,$@.a'. There
# seems to be no advantage to that anymore as of 2005-01, but keeping
# the ancient incantation in a comment here does no harm.

ALL_ARFLAGS  = $(REQUIRED_ARFLAGS)  $(ARFLAGS)
ALL_CPPFLAGS = $(REQUIRED_CPPFLAGS) $(CPPFLAGS)
ALL_CXXFLAGS = $(REQUIRED_CXXFLAGS) $(CXXFLAGS)
ALL_CFLAGS   = $(REQUIRED_CFLAGS)   $(CFLAGS)
ALL_LDFLAGS  = $(REQUIRED_LDFLAGS)  $(LDFLAGS)
ALL_RCFLAGS  = $(REQUIRED_RCFLAGS)  $(RCFLAGS)

################################################################################

# Pattern rules.

%.o: %.c
	$(CC) $(MAKEDEPEND_0) -c $(ALL_CPPFLAGS) $(ALL_CFLAGS) $< -o$@
	$(MAKEDEPEND_1)

%.o: %.cpp
	$(CXX) $(MAKEDEPEND_0) -c $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) $< -o$@
	$(MAKEDEPEND_1)

%.o: %.cxx
	$(CXX) $(MAKEDEPEND_0) -c $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) $< -o$@
	$(MAKEDEPEND_1)

%.rc.o: %.rc
	$(RC) -o $@ $(ALL_RCFLAGS) -i $<

%$(EXEEXT):
	$(LD) -o $@ $^ $(ALL_LDFLAGS)

%$(SHREXT):
	$(LD) -o $@ -shared $^ $(ALL_LDFLAGS)

%.a:
	$(AR) $(ALL_ARFLAGS) $@ $^

# TODO ?? Should target-specific dependencies reside in 'objects.make'
# instead?

# TODO ?? Replace 'DLL_NEW_BUILDING_DLL' e.g. with something like
# 'LMI_WXNEW_DLLFLAG': 'LMI' should be a lexical element, and it
# ought to come first.

wx_new$(SHREXT): REQUIRED_CPPFLAGS += -DDLL_NEW_BUILDING_DLL
wx_new$(SHREXT): wx_new.o

# TODO ?? Reconsider writing $(LMI_DLLFLAG) in $(ALL_CPPFLAGS). It's
# no longer needed for gcc-3.x, but remains useful for other toolsets;
# and gcc-4.x can use it for the new ELF 'visibility' support. Oddly
# enough, though, it's harmful for msw, the platform for which that
# feature was originally defined: if it's never used on msw, then
# static and shared libraries can be built from the same object files.

lib%.a              : LMI_DLLFLAG :=
lib%$(SHREXT)       : LMI_DLLFLAG := -DLMI_BUILD_DLL
shared_demo$(EXEEXT): LMI_DLLFLAG := -DLMI_USE_DLL

liblmi.a liblmi$(SHREXT): $(lmi_common_objects)
libantediluvian.a libantediluvian$(SHREXT): $(antediluvian_common_objects)

shared_demo$(EXEEXT): library_demo.o alert_cli.o liblmi$(SHREXT)

static_demo$(EXEEXT): library_demo.o alert_cli.o $(lmi_common_objects)

antediluvian_cgi$(EXEEXT): $(antediluvian_cgi_objects) libantediluvian.a

antediluvian_cli$(EXEEXT): $(antediluvian_cli_objects) libantediluvian.a

################################################################################

# Tests.

.PHONY: test
test: unit_tests cgi_tests cli_tests

################################################################################

# Unit tests.

# Add extra warnings for unit tests.

# TODO ?? Treat C the same way here as C++.

$(unit_test_targets): REQUIRED_CXXFLAGS += $(CXX_EXTRA_WARNINGS)

.PHONY: unit_tests
unit_tests: $(unit_test_targets) run_unit_tests

.PHONY: unit_tests_not_built
unit_tests_not_built:
	@$(ECHO) "List of unit-test targets that did not build successfully:"
	@$(ECHO) $(filter-out $(wildcard *$(EXEEXT)),$(unit_test_targets))
	@$(ECHO) "List ends."

.PHONY: run_unit_tests
run_unit_tests: unit_tests_not_built $(addsuffix -run,$(unit_test_targets))

# Create 'mpatrol.log' if it doesn't already exist, so that it can be
# parsed for diagnostics unconditionally when unit tests are run.

mpatrol.log:
	$(TOUCH) $@

%$(EXEEXT)-run: mpatrol.log
	@$(ECHO) "\nRunning $*:"
	@-./$* --accept
	@test -e mpatrol.log \
	  && <mpatrol.log $(SED) \
	  -e'/^total warnings\|^total errors/!d' \
	  -e's/^\(.*$$\)/  mpatrol: \1/' \

################################################################################

# Test command-line interface.

# Run the test once, and throw away the results, just to get the
# program into the disk cache. Then run it again and report the
# results.

.PHONY: cli_tests
cli_tests:
	@$(ECHO) Test command line interface:
	@./antediluvian_cli$(EXEEXT) --accept --selftest > /dev/null;
	@./antediluvian_cli$(EXEEXT) --accept --selftest

################################################################################

# Test common gateway interface.

# This lightweight test emulates what a webserver would do.

.PHONY: cgi_tests
cgi_tests: antediluvian_cgi$(EXEEXT)
	@$(ECHO) Test common gateway interface:
	@./antediluvian_cgi$(EXEEXT) --write_content_string > /dev/null
	@<$(src_dir)/expected.cgi.out \
	  $(SED)  -e'/^[0-9. ]*$$/!d' -e'/[0-9]/!d' \
	  > cgi_touchstone
	@./antediluvian_cgi$(EXEEXT) --enable_test <cgi.test.in \
	  | $(SED)  -e'/^[0-9. ]*$$/!d' -e'/[0-9]/!d' \
	  | $(DIFF) -w - cgi_touchstone \
	  | $(WC)   -l \
	  | $(SED)  -e's/^/  /' -e's/$$/ errors/'

################################################################################

# Test idempotence of headers and template-instantiation files.

# When a file passes the idempotence test, create an empty file with
# suffix '.idempotent' in the build directory to record that success,
# and to prevent subsequent updates of this target from testing that
# file again unless it changes.

# TODO ?? Consider adding an autodependency mechanism--or is that
# gilding the lily? Changing one header may cause another to become
# nonidempotent, but the present test has not power to discover that.

# Treat '.h' files as C++. Some C++ headers use that suffix, though
# this project's do not. By default, g++ reports idempotence defects
# with this simple technique, but gcc does not, probably because it
# strives for compatibility with pre-standard C.

.PHONY: check_idempotence
check_idempotence: $(idempotent_files)

%.idempotent: %
	@$(CXX) \
	  -DLMI_IGNORE_PCH \
	  $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) \
	  -x c++ -w -O0 -fsyntax-only $<
	@$(TOUCH) $@

################################################################################

# Display selected variables.

.PHONY: show_flags
show_flags:
	@$(ECHO) ALL_CPPFLAGS            = '$(ALL_CPPFLAGS)'
	@$(ECHO) ALL_CFLAGS              = '$(ALL_CFLAGS)'
	@$(ECHO) ALL_CXXFLAGS            = '$(ALL_CXXFLAGS)'
	@$(ECHO) ALL_ARFLAGS             = '$(ALL_ARFLAGS)'
	@$(ECHO) ALL_LDFLAGS             = '$(ALL_LDFLAGS)'
	@$(ECHO) ALL_RCFLAGS             = '$(ALL_RCFLAGS)'
	@$(ECHO) src_dir                 = '$(src_dir)'
	@$(ECHO) boost_dir               = '$(boost_dir)'
	@$(ECHO) cgicc_dir               = '$(cgicc_dir)'
	@$(ECHO) libxml2_dir             = '$(libxml2_dir)'
	@$(ECHO) xmlwrapp_dir            = '$(xmlwrapp_dir)'
	@$(ECHO) all_include_directories = '$(all_include_directories)'
	@$(ECHO) all_source_directories  = '$(all_source_directories)'

