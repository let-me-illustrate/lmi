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

# $Id: workhorse.make,v 1.45 2005-08-31 17:55:04 chicares Exp $

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
  antediluvian_cgi$(EXEEXT) \
  antediluvian_cli$(EXEEXT) \
  elapsed_time$(EXEEXT) \
  generate_passkey$(EXEEXT) \
  ihs_crc_comp$(EXEEXT) \
  libantediluvian$(SHREXT) \
  liblmi$(SHREXT) \
  lmi_cli_shared$(EXEEXT) \
  lmi_wx_shared$(EXEEXT) \
  product_files$(EXEEXT) \
  wx_new$(SHREXT) \

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

# Path to libraries from www.boost.org : most required boost libraries
# are implemented exclusively in headers. It seems common in the *nix
# world to leave those headers in the subdirectory of /whatever/src/
# to which the boost distribution is extracted, probably because boost
# does not put all its headers in an include/ subdirectory. But that
# seems broken, so instead copy those headers to /usr/local/include .
#
# TODO ?? If the "common" practice above turns out to be universal,
# then conform to it.

# Directory /usr/local/include/ is searched for headers, but only
# after the special directory for third-party libraries, in order to
# make it easier to test or use later library versions that have
# already been installed in the former directory.

all_include_directories := \
  $(src_dir) \
  $(compiler_include_directory) \
  $(system_root)/opt/lmi/third-party/include \
  $(system_root)/usr/local/include \

all_source_directories := \
  $(src_dir) \
  $(system_root)/opt/lmi/third-party/src/boost/libs/filesystem/src \
  $(system_root)/opt/lmi/third-party/src/cgicc \
  $(system_root)/opt/lmi/third-party/src/libxml \

vpath lib%.a          $(CURDIR)
vpath %.o             $(CURDIR)

# Files whose names match 'my_%.cpp' are taken as product data files,
# which are overridden by any customized files found in a special
# directory.

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

# Too many warnings on correct code, e.g. exact comparison to zero:
#  -Wfloat-equal \

# WX !! The wx library triggers many warnings with these flags:

gcc_common_extra_warnings := \
  -Wextra \
  -Wcast-qual \
  -Wredundant-decls \
  -Wundef \

# INELEGANT !! It might be preferable to specify what is wx dependent
# at the object level.

$(lmi_wx_objects): gcc_common_extra_warnings :=

# Boost normally makes '-Wundef' give spurious warnings:
#   http://aspn.activestate.com/ASPN/Mail/Message/boost/1822550
# but defining BOOST_STRICT_CONFIG:
#   http://www.boost.org/libs/config/config.htm#user_settable
# makes '-Wundef' usable, because boost-1.31.0 doesn't seem to need
# any workarounds for gcc-3.3+ . However, it gives a number of
# warnings with wx-2.5.4 (that have been fixed in a later version).

# Too many warnings for various boost libraries:
#  -Wold-style-cast \
#  -Wshadow \

# Too many warnings for libstdc++:
#  -Wunreachable-code \

# Since at least gcc-3.4.2, -Wmissing-prototypes is deprecated as
# being redundant for C++.

C_WARNINGS   = $(gcc_c_warnings)   $(gcc_common_extra_warnings)
CXX_WARNINGS = $(gcc_cxx_warnings) $(gcc_common_extra_warnings)

################################################################################

# Build type governs optimization flags and use of mpatrol and gprof.

# TODO ?? Also consider defining these libstdc++ macros:
#   _GLIBCXX_DEBUG_ASSERT
#   _GLIBCXX_DEBUG_PEDASSERT
#   _GLIBCXX_DEBUG_VERIFY

MPATROL_LIBS :=

test_targets = unit_tests cgi_tests cli_tests

ifeq (mpatrol,$(findstring mpatrol,$(build_type)))
  optimization_flag := -O0
  MPATROL_LIBS := -lmpatrol -lbfd -liberty $(platform_mpatrol_libraries)
else
  ifeq (gprof,$(findstring gprof,$(build_type)))
    optimization_flag := -O0
    gprof_flag := -pg
  else
    optimization_flag := -O2
  endif
endif

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
  $(platform_boost_libraries) \
  -lxmlwrapp \
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

# TODO ?? Is there a better way to handle __WXDEBUG__, such as
# #including some wx configuration header?

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(all_include_directories)) \
  $(lmi_wx_new_dllflag) \
  $(platform_defines) \
  -D__WXDEBUG__ \
  -DBOOST_STRICT_CONFIG \

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
  -L $(system_root)/opt/lmi/third-party/lib \
  -L $(system_root)/opt/lmi/third-party/bin \
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

# Don't use mpatrol when building a shared library to be used by an
# application that uses mpatrol. See my postings to the mpatrol
# mailing list.

lib%$(SHREXT)       : MPATROL_LIBS :=
wx_new$(SHREXT)     : MPATROL_LIBS :=

                      lmi_wx_new_dllflag := -DLMI_WX_NEW_USING_DLL
wx_new$(SHREXT)     : lmi_wx_new_dllflag := -DLMI_WX_NEW_BUILDING_DLL

liblmi.a liblmi$(SHREXT): $(lmi_common_objects)
libantediluvian.a libantediluvian$(SHREXT): $(antediluvian_common_objects)

# TODO ?? 'lmi*' targets can be built either with a shared or a static
# 'lmi' library. Choose one, or support both.

lmi_wx_monolithic$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries) $(platform_gui_ldflags)
lmi_wx_monolithic$(EXEEXT): $(lmi_wx_objects) $(lmi_common_objects) wx_new$(SHREXT)

lmi_wx_shared$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL
lmi_wx_shared$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries) $(platform_gui_ldflags)
lmi_wx_shared$(EXEEXT): $(lmi_wx_objects) liblmi$(SHREXT) wx_new$(SHREXT)

lmi_wx_static$(EXEEXT): REQUIRED_LDFLAGS += $(platform_wx_libraries) $(platform_gui_ldflags)
lmi_wx_static$(EXEEXT): $(lmi_wx_objects) liblmi.a wx_new$(SHREXT)

lmi_cli_monolithic$(EXEEXT): $(cli_objects) $(lmi_common_objects)

lmi_cli_shared$(EXEEXT): lmi_dllflag := -DLMI_USE_DLL
lmi_cli_shared$(EXEEXT): $(cli_objects) liblmi$(SHREXT)

lmi_cli_static$(EXEEXT): $(cli_objects) liblmi.a

antediluvian_cgi$(EXEEXT): $(cgi_objects) libantediluvian$(SHREXT)

antediluvian_cli$(EXEEXT): $(cli_objects) libantediluvian$(SHREXT)

wx_new$(SHREXT): wx_new.o

# TODO ?? This needs a corresponding test target.
lmi_cgi$(EXEEXT): $(cgi_objects) $(lmi_common_objects)

################################################################################

# Install.

# TODO ?? It's confusing to place
#   $(data_files) in $(bin_dir), but other
#   data files in $(data_dir).
# And some of the files in $(data_dir) are found only in the ftp
# area of savannah; should they be in cvs?

# TODO ?? This line
#   $(CP) --preserve --update $^
# isn't quite right: running 'make install build_type=[...]' with
# different build_types and picking the latest version of each
# component can produce a mismatched set.

prefix         := /opt/lmi
exec_prefix    := $(prefix)
bin_dir        := $(exec_prefix)/bin
data_dir       := $(exec_prefix)/data
test_dir       := $(exec_prefix)/test
touchstone_dir := $(exec_prefix)/touchstone

data_files := \
  $(wildcard $(addprefix $(src_dir)/,*.xrc *.xpm)) \

.PHONY: install
install: $(default_targets)
	+@[ -d $(exec_prefix)    ] || $(MKDIR) --parents $(exec_prefix)
	+@[ -d $(bin_dir)        ] || $(MKDIR) --parents $(bin_dir)
	+@[ -d $(data_dir)       ] || $(MKDIR) --parents $(data_dir)
	+@[ -d $(test_dir)       ] || $(MKDIR) --parents $(test_dir)
	+@[ -d $(touchstone_dir) ] || $(MKDIR) --parents $(touchstone_dir)
	@$(CP) --preserve --update $^ $(data_files) $(bin_dir)
	@cd $(data_dir); $(bin_dir)/product_files

################################################################################

# Archive data files. Designed for maintainer use only.

data_archname := lmi-data-$(yyyymmddhhmm).tar

shared_data_files = \
  qx_ann.dat \
  qx_ann.ndx \
  qx_cso.dat \
  qx_cso.ndx \
  qx_ins.dat \
  qx_ins.ndx \
  sample.dat \
  sample.db4 \
  sample.fnd \
  sample.ndx \
  sample.pol \
  sample.rnd \
  sample.tir \

.PHONY: archive_shared_data_files
archive_shared_data_files:
	cd $(data_dir)/..; \
	$(TAR) \
	  --create \
	  --file=$(data_archname) \
	  --verbose \
	  $(addprefix data/,$(shared_data_files)); \
	$(BZIP2) $(data_archname); \

################################################################################

# Tests.

.PHONY: test
test: $(test_targets)

################################################################################

# Unit tests.

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

# Regression test.

# Output is compared with $(DIFF), which reports all textual
# discrepancies without regard to relevance; and also with
# 'ihs_crc_comp', which interprets '.test' files and calculates
# maximum relative and absolute errors for each file.
#
# Relative errors less than 1e-14 are ignored. Machine epsilon for an
# IEC 60559 double is 2.2204460492503131E-16 [C99 5.2.4.2.2/13], so
# that tolerance is about forty-five times epsilon. Experience has
# shown that the discrepancies thus ignored are never material, but
# larger discrepancies may be.

test_result_suffixes     := test test0 debug

regression_test_analysis := $(test_dir)/analysis-$(yyyymmddhhmm)
regression_test_diffs    := $(test_dir)/diffs-$(yyyymmddhhmm)
regression_test_md5sums  := $(test_dir)/md5sums-$(yyyymmddhhmm)

.PHONY: regression_test
regression_test: install
	@$(ECHO) Regression test:
	@-cd $(test_dir); \
	  $(foreach z, $(addprefix *., $(test_result_suffixes)), $(RM) --force $z;)
	@cd $(test_dir); \
	  $(bin_dir)/lmi_cli_shared \
	    --ash_nazg --accept --regress \
	    --data_path=$(data_dir) \
	    --test_path=$(test_dir); \
	  $(MD5SUM) \
	    $(addprefix *.,$(test_result_suffixes)) \
	    >$(regression_test_md5sums); \
	  for z in *.test; \
	    { $(bin_dir)/ihs_crc_comp $$z $(touchstone_dir)/$$z \
	      | $(SED) -e '/Summary/!d' -e"s|^ |$$z|" \
	    } > $(regression_test_analysis)
	@-< $(regression_test_analysis) $(SED) \
	  -e'/rel err.*e-01[5-9]/d' \
	  -e'/abs.*0\.00.*rel/d' \
	  -e'/abs diff: 0 /d'
	@-$(DIFF) \
	    --brief \
	    --report-identical-files \
	    $(test_dir) \
	    $(touchstone_dir) \
	    > $(regression_test_diffs) \
	  || true
	@$(ECHO) Summarizing test results
	@-<$(regression_test_diffs) \
	  $(SED) \
	    -e'/^Only in/d' \
	  | $(WC) -l \
	  | $(SED) 's/^/  /' \
	  | $(SED) -e's/$$/ regression-test files compared/'
	@-<$(regression_test_diffs) \
	  $(SED) \
	    -e'/^Files.*are identical$$/!d' \
	  | $(WC) -l \
	  | $(SED) 's/^/  /' \
	  | $(SED) -e's/$$/ regression-test files match/'
	@-<$(regression_test_diffs) \
	  $(SED) \
	    -e'/^Files.*are identical$$/d' \
	    -e'/^Only in /d' \
	  | $(WC) -l \
	  | $(SED) 's/^/  /' \
	  | $(SED) -e's/$$/ regression-test nonmatching files/'
	@-<$(regression_test_diffs) \
	  $(SED) \
	    -e'/^Only in.*test\/touchstone/!d' \
	  | $(WC) -l \
	  | $(SED) 's/^/  /' \
	  | $(SED) -e's/$$/ regression-test missing files/'
	@$(ECHO) ...regression test completed.

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
	@$(ECHO) all_include_directories = '$(all_include_directories)'
	@$(ECHO) all_source_directories  = '$(all_source_directories)'

