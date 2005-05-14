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

# $Id: workhorse.make,v 1.25 2005-05-14 15:59:03 chicares Exp $

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
  wx_new$(SHREXT) \
  shared_demo$(EXEEXT) \
  static_demo$(EXEEXT) \
  antediluvian_cgi$(EXEEXT) \
  antediluvian_cli$(EXEEXT) \
  lmi_cli$(EXEEXT) \
  lmi_wx$(EXEEXT) \
  ihs_crc_comp$(EXEEXT) \
  product_files$(EXEEXT) \
  liblmi$(SHREXT) \

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

# Files whose names match 'my_%.cpp' are taken as product data files,
# which are overridden by customized files in a special directory.

all_include_directories := \
  $(src_dir) \
  $(system_root)/usr/local/include \

all_source_directories := \
  $(src_dir) \
  $(boost_dir)/libs/filesystem/src \
  $(cgicc_source_dir) \
  $(xmlwrapp_source_dir) \

vpath lib%.a          $(CURDIR)
vpath %.o             $(CURDIR)

vpath my_%.cpp        $(src_dir)/../products/src

vpath %.c             $(all_source_directories)
vpath %.cpp           $(all_source_directories)
vpath %.cxx           $(all_source_directories)
vpath %.h             $(all_source_directories)
vpath %.hpp           $(all_source_directories)
vpath %.tpp           $(all_source_directories)
vpath %.xpp           $(all_source_directories)

vpath %.rc            $(all_source_directories)

vpath quoted_gpl      $(src_dir)
vpath quoted_gpl_html $(src_dir)

# This rule must exist for the parent makefile to make these targets,
# apparently; TODO ?? but why?

quoted_gpl quoted_gpl_html:

################################################################################

# Warning options for gcc.

gcc_common_warnings := \
  -pedantic \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wendif-labels \
  -Wimport \
  -Wmultichar \
  -Wpacked \
  -Wpointer-arith \
  -Wsign-compare \
  -Wwrite-strings \

# Some boost libraries treat 'long long' as part of the language,
# which it probably soon will be, so permit it now.

gcc_common_warnings += -Wno-long-long

gcc_c_warnings := \
  $(gcc_common_warnings) \
  -std=c99 \
  -Wmissing-prototypes \

gcc_cxx_warnings := \
  $(gcc_common_warnings) \
  -std=c++98 \
  -Wctor-dtor-privacy \
  -Wdeprecated \
  -Wnon-template-friend \
  -Woverloaded-virtual \
  -Wpmf-conversions \
  -Wsynth \

# TODO ?? VZ reports that
#  -Winvalid-offsetof \
# gives an error with GNU/Linux gcc-3.3 and 3.4, though that seems
# to contradict the gcc manual. This should be investigated and
# possibly reported on gcc bugzilla.

# WX !! The wx library triggers many warnings with these flags:

gcc_common_extra_warnings := \
  -W \
  -Wcast-qual \
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

# TODO ?? How can these best be enabled for non-wx code?
# gcc_warnings += $(gcc_common_extra_warnings)

C_WARNINGS         := $(gcc_c_warnings)
CXX_WARNINGS       := $(gcc_cxx_warnings)

C_EXTRA_WARNINGS   := $(gcc_common_extra_warnings)
CXX_EXTRA_WARNINGS := $(gcc_common_extra_warnings)

################################################################################

# Build type governs optimization flags and use of mpatrol and gprof.

# TODO ?? Also consider defining these libstdc++ macros:
#   _GLIBCXX_DEBUG_ASSERT
#   _GLIBCXX_DEBUG_PEDASSERT
#   _GLIBCXX_DEBUG_VERIFY

MPATROL_LIBS :=

test_targets = unit_tests cgi_tests cli_tests av_tests

# TODO ?? With mpatrol, target 'av_tests' takes far too long to run.
# Investigate whether this indicates an optimization opportunity.

ifeq (mpatrol,$(findstring mpatrol,$(build_type)))
  optimization_flag := -O0
  MPATROL_LIBS := -lmpatrol -lbfd -liberty $(platform_mpatrol_libraries)
  test_targets = unit_tests cgi_tests cli_tests
else
  ifeq (gprof,$(findstring gprof,$(build_type)))
    optimization_flag := -O0
    gprof_flag := -pg
  else
    optimization_flag := -O2
  endif
endif

################################################################################

# Prevent license issues by providing dummy versions of problematic
# libraries that wx builds by default.

# TODO ?? Consider forcing this issue in case someone builds wx
# without prescribed changes to prevent making these libraries,
# in order to ensure that this application can never display any file
# that would use GPL-incompatible code provided with wx.

# C:/wx-lmi/lmi[0]$cp --preserve /lib/libm.a libregex.a
# C:/wx-lmi/lmi[0]$cp --preserve /lib/libm.a libpng.a
# C:/wx-lmi/lmi[0]$cp --preserve /lib/libm.a libjpeg.a
# C:/wx-lmi/lmi[0]$cp --preserve /lib/libm.a libzlib.a
# C:/wx-lmi/lmi[0]$cp --preserve /lib/libm.a libtiff.a

################################################################################

# Required libraries.
#
# The link command promiscuously mentions libxml2 for all targets.
# Measurements show that this costs one-tenth of a second on
# reasonable hardware, and it saves the trouble of maintaining a list
# of which targets require which libraries.
#
# TODO ?? Consider refining it anyway, because it's unclean: libxml2
# isn't actually required for all targets.

REQUIRED_LIBS := \
  $(platform_libxml2_libraries) \

################################################################################

# Flags.

# Define these variables recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the variables
# they're composed from.

# Use '-g' instead of '-ggdb'. MinGW gcc-3.4.2 writes dwarf2 debug
# records if '-ggdb' is specified, but the version of gdb packaged
# with it expects stabs format.

CFLAGS = \
  -g $(optimization_flag) $(gprof_flag) \

CXXFLAGS = \
  -g $(optimization_flag) $(gprof_flag) \

LDFLAGS = \
  $(gprof_flag) \
  -Wl,-Map,$@.map \

# INELEGANT !! Define BOOST_DEPRECATED until code that uses the
# deprecated boost::bind library is rewritten.

# TODO ?? Is there a better way to handle __WXDEBUG__, such as
# #including some wx configuration header?

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(all_include_directories)) \
  $(lmi_wx_new_dllflag) \
  $(platform_defines) \
  -DBOOST_DEPRECATED \
  -D__WXDEBUG__ \

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

# TODO ?? Define $(wx_cxxflags) for msw as well as posix.

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \
  $(wx_cxxflags) \

REQUIRED_ARFLAGS = \
  -rus

# Prefer to invoke GNU 'ld' through the compiler frontends 'gcc' and
# 'g++' because that takes care of linking the required libraries for
# each language. Accordingly, pass GNU 'ld' options with '-Wl,'.

# Two subdirectories of /usr/local
#   /usr/local/lib
#   /usr/local/bin
# are placed on the link path in order to accommodate msw dlls, for
# which no canonical location is clearly specified by FHS, because
# they're both binaries and libraries in a sense. These two
# subdirectories seem to be the most popular choices; wx regards its
# dll as a binary, while mpatrol regards its as a library. For msw,
# it seems crucial to list these two subdirectories in exactly the
# order given; if they're specified in reverse order, then mpatrol
# won't work, perhaps because gnu 'ld' finds its dll first and then
# doesn't bother looking for its library.

REQUIRED_LDFLAGS = \
  -L . \
  -L $(system_root)/usr/local/lib \
  -L $(system_root)/usr/local/bin \
  $(REQUIRED_LIBS) \
  $(MPATROL_LIBS) \

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

# Reconsider writing $(lmi_dllflag) in $(REQUIRED_CPPFLAGS), in order
# to use dll import and export attributes.
#
# Such attributes are no longer needed for gcc-3.something. Omitting
# them makes building slightly more efficient--see
#   http://sourceforge.net/mailarchive/message.php?msg_id=10584510
# The most important benefit of omitting them is that static and
# shared libraries can then be built from the same object files.
#
# On the other hand, gcc-4.x uses a deliberately similar method for
# ELF symbol visibility, a feature for which significant benefits are
# claimed:
#   http://www.nedprod.com/programs/gccvisibility.html
# And dll attributes would still be necessary for other toolsets,
# which therefore aren't fully supported yet.
#
# However, 'libwx_new.a' continues to use classic dll attributes,
# because there's never a reason to build it as a static library.

lib%.a              : lmi_dllflag :=
lib%$(SHREXT)       : lmi_dllflag := -DLMI_BUILD_DLL
shared_demo$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL

skeleton$(EXEEXT)   : lmi_wx_new_dllflag := -DLMI_WX_NEW_USING_DLL
wx_new$(SHREXT)     : lmi_wx_new_dllflag := -DLMI_WX_NEW_BUILDING_DLL

liblmi.a liblmi$(SHREXT): $(lmi_common_objects)
libantediluvian.a libantediluvian$(SHREXT): $(antediluvian_common_objects)

shared_demo$(EXEEXT): library_demo.o alert_cli.o liblmi$(SHREXT)

static_demo$(EXEEXT): library_demo.o alert_cli.o $(lmi_common_objects)

# TODO ?? 'lmi*' targets can be built either with a shared or a static
# 'lmi' library. Choose one, or support both--possibly expunging the
# 'shared_demo' and 'static_demo' targets and the code that's unique
# to them.

#lmi_cli$(EXEEXT): $(lmi_cli_objects) liblmi$(SHREXT)

lmi_cli$(EXEEXT): $(lmi_cli_objects) $(lmi_common_objects)

#lmi_wx$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries)
#lmi_wx$(EXEEXT): $(lmi_wx_objects) liblmi$(SHREXT) wx_new$(SHREXT)

lmi_wx$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries) -mwindows
lmi_wx$(EXEEXT): $(lmi_wx_objects) $(lmi_common_objects) wx_new$(SHREXT)

# Experimental.

lmi_wx_shared$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL
lmi_wx_shared$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries) -mwindows
lmi_wx_shared$(EXEEXT): $(lmi_wx_objects) liblmi$(SHREXT) wx_new$(SHREXT)

lmi_cli_shared$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL
lmi_cli_shared$(EXEEXT): $(lmi_cli_objects) liblmi$(SHREXT) wx_new$(SHREXT)

antediluvian_cgi$(EXEEXT): $(antediluvian_cgi_objects) libantediluvian.a

antediluvian_cli$(EXEEXT): $(antediluvian_cli_objects) libantediluvian.a

wx_new$(SHREXT): wx_new.o

# TODO ?? This needs a corresponding test target.
lmi_cgi$(EXEEXT): $(antediluvian_cgi_objects) $(lmi_common_objects)

################################################################################

# Install.

# The third paragraph of the rationale for FHS-2.2final section 3.12.2
# seems to require copies of binaries in opt/<package>, even though
# they must be copied to opt/<package>/bin .

# TODO ?? Should static data files reside in cvs so that this makefile
# can install them?

install_dir := /opt/lmi

data_files := \
  $(wildcard $(addprefix $(src_dir)/,*.xrc *.xpm)) \

.PHONY: install
install: $(default_targets)
	+@[ -d $(install_dir) ] || $(MKDIR) --parents $(install_dir)
	@$(CP) --preserve $^ $(install_dir)
	@$(CP) --preserve $^ $(install_dir)/bin
	@$(CP) --preserve $(data_files) $(install_dir)

################################################################################

# Tests.

.PHONY: test
test: $(test_targets)

################################################################################

# Unit tests.

# Add extra warnings for unit tests.

$(unit_test_targets): REQUIRED_CFLAGS   += $(C_EXTRA_WARNINGS)

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
	@$(TOUCH) $@

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
cli_tests: antediluvian_cli$(EXEEXT)
	@$(ECHO) Test command line interface:
	@./antediluvian_cli$(EXEEXT) --accept --selftest > /dev/null
	@./antediluvian_cli$(EXEEXT) --accept --selftest

################################################################################

# Test common gateway interface.

# This lightweight test emulates what a webserver would do.

.PHONY: cgi_tests
cgi_tests: antediluvian_cgi$(EXEEXT)
	@$(ECHO) Test common gateway interface:
	@./antediluvian_cgi$(EXEEXT) --write_content_string > /dev/null
	@<$(src_dir)/expected.cgi.out \
	  $(SED)    -e'/^[0-9. ]*$$/!d' -e'/[0-9]/!d' \
	  > cgi_touchstone
	@./antediluvian_cgi$(EXEEXT) --enable_test <cgi.test.in \
	  | $(SED)  -e'/^[0-9. ]*$$/!d' -e'/[0-9]/!d' \
	  | $(DIFF) -w - cgi_touchstone \
	  | $(WC)   -l \
	  | $(SED)  -e's/^/  /' -e's/$$/ errors/'

################################################################################

# Test account values in production branch.

# Run the test once, and throw away the results, just to get the
# program into the disk cache. Then run it again and report the
# results.

.PHONY: av_tests
av_tests: static_demo$(EXEEXT) ihs_crc_comp$(EXEEXT)
	@$(ECHO) Test account values:
	@./static_demo$(EXEEXT) > /dev/null
	<$(src_dir)/spew_touchstone $(SED) -e'1s/^/0\t0\n/' >spew_touchstone
	<eraseme.crc                $(SED) -e'1s/^/0\t0\n/' >spew_test
	@./ihs_crc_comp \
	  spew_touchstone \
	  spew_test \
	  | $(SED) -e '/Summary/!d' -e"s/^ /$z/"

################################################################################

# Regression test.

# Relative errors less than 1e-14 are ignored. Machine epsilon for an
# IEC 60559 double is 2.2204460492503131E-16 [C99 5.2.4.2.2/13], so
# that tolerance is about forty-five times epsilon. Experience has
# shown that the discrepancies thus ignored are never material, but
# larger discrepancies may be.

# TODO ?? Needs work: avoid imperative programming like 'install' target,
# which copies large files whether or not they're already current.

regression_test_results := results-$(yyyymmddhhmm)

.PHONY: regression_test
regression_test: install
	@$(ECHO) Regression test:
	@ \
	cd $(install_dir)/data; \
	../product_files; \
	cd $(install_dir)/test; \
	../lmi_cli \
	  --ash_nazg --accept --regress \
	  --data_path=$(install_dir)/data \
	  --test_path=$(install_dir)/test; \
	for z in *test; \
	  { $(install_dir)/ihs_crc_comp $$z $(install_dir)/touchstone/$$z \
	    | sed -e '/Summary/!d' -e"s/^ /$$z/" \
	  } > $(regression_test_results); \
	< $(regression_test_results) sed \
	  -e'/rel err.*e-01[5-9]/d' \
	  -e'/abs.*0\.00.*rel/d' \
	  -e'/abs diff: 0 /d'; \
	$(ECHO) ...regression test completed.

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

# Clean.

files_normally_created_by_running_make := \
  $(wildcard lib*.a) \
  $(wildcard *$(EXEEXT)) \
  $(wildcard *$(SHREXT)) \
  $(wildcard *.d) \
  $(wildcard *.map) \
  $(wildcard *.o) \
  cgi.test.in \
  cgi_touchstone \
  mpatrol.log \
  spew_touchstone \

.PHONY: clean distclean mostlyclean
clean distclean mostlyclean:
	@-$(RM) --force $(files_normally_created_by_running_make)

.PHONY: maintainer-clean
maintainer-clean: distclean

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
	@$(ECHO) cgicc_include_dir       = '$(cgicc_include_dir)'
	@$(ECHO) cgicc_source_dir        = '$(cgicc_source_dir)'
	@$(ECHO) libxml2_include_dir     = '$(libxml2_include_dir)'
	@$(ECHO) xmlwrapp_include_dir    = '$(xmlwrapp_include_dir)'
	@$(ECHO) xmlwrapp_source_dir     = '$(xmlwrapp_source_dir)'
	@$(ECHO) all_include_directories = '$(all_include_directories)'
	@$(ECHO) all_source_directories  = '$(all_source_directories)'

