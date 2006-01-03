# Top-level lmi makefile.
#
# Copyright (C) 2005, 2006 Gregory W. Chicares.
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

# $Id: GNUmakefile,v 1.50 2006-01-03 21:20:50 chicares Exp $

###############################################################################

# The general technique used in the 'Multiple build directories'
# section is due to Paul D. Smith:
#   http://make.paulandlesley.org/multi-arch.html#advanced
# with the following significant differences:
#
# For greater clarity, the 'actual' makefile is a separate file,
# instead of a conditional section of this file.
#
# $(MAKEFLAGS) is used instead of '.SUFFIXES': it is more powerful,
# for instance because it can automatically enforce Paul's advice
# "it's best to invoke make with the -r option".
#
# One of several architecture-specific files is included, based on the
# result of 'uname'. Separate files encapsulate variations better.
#
# The ':' command Paul gives for his do-nothing rule seems no longer
# to be necessary.
#
# List $(MAKECMDGOALS) as the first target in this makefile; and, if
# $(MAKECMDGOALS) is not defined, then give it the value 'all', which
# "should be the default target" according to the GNU make manual.
# Without these refinements, running 'make' with no arguments would
# not work as expected.
#
# Any defect here should not reflect on Paul D. Smith's reputation.

MAKECMDGOALS ?= all

$(MAKECMDGOALS):

###############################################################################

# Directives and such.

# The GNU make manual says:
#   "This is almost always what you want make to do,
#   but it is not historical practice; so for
#   compatibility, you must explicitly request it."

.DELETE_ON_ERROR:

# Suppress default rules. Instead, specify all desired rules
# explicitly.
#
# Instead of using a '.SUFFIXES:' target with no dependencies,
# specify $(MAKEFLAGS) to do everything that would do, and more.

MAKEFLAGS := \
  --keep-going \
  --no-builtin-rules \
  --no-builtin-variables \

################################################################################

# Directories.

src_dir := $(CURDIR)

# Make sure a temporary directory exists.

TMPDIR ?= /tmp
$(TMPDIR):
	+@[ -d $@ ] || $(MKDIR) --parents $@

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

GNUmakefile $(src_dir)/GNUmakefile:: ;

# Included files that don't need to be remade are given explicit empty
# commands, which significantly reduces by about one-third the number
# of lines emitted by 'make -d', making debug output easier to read.
#
# Included makefiles are cited by absolute pathname, e.g.
#   include $(src_dir)/included-file
#   $(src_dir)/included-file:: ;
# An '--include-dir' option could make an absolute path unnecessary
# in the 'include' statement itself, but the empty remake command
# would not respect '--include-dir'.
#
# The double-colon rule elicits warnings like
#   Makefile `foo' might loop; not remaking it.
# with 'make -d'. The "stupidly" comment at line 1745 of GNU make's
# 'main.c' version 1.194 seems to suggest that writing such a rule is
# a poor practice, but empty commands ought to be excused from that
# inline comment, and perhaps even from that diagnostic.

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

################################################################################

# Pass customary flags to submakefiles if they're defined in the
# environment. See these messages:
#   http://lists.gnu.org/archive/html/help-make/2005-07/msg00023.html
#   http://lists.gnu.org/archive/html/help-make/2005-07/msg00025.html

ifeq "$(origin ARFLAGS)" "environment"
  flags_to_pass_from_environment += ARFLAGS='$(ARFLAGS)'
endif

ifeq "$(origin CFLAGS)" "environment"
  flags_to_pass_from_environment += CFLAGS='$(CFLAGS)'
endif

ifeq "$(origin CPPFLAGS)" "environment"
  flags_to_pass_from_environment += CPPFLAGS='$(CPPFLAGS)'
endif

ifeq "$(origin CXXFLAGS)" "environment"
  flags_to_pass_from_environment += CXXFLAGS='$(CXXFLAGS)'
endif

ifeq "$(origin LDFLAGS)" "environment"
  flags_to_pass_from_environment += LDFLAGS='$(LDFLAGS)'
endif

ifeq "$(origin RCFLAGS)" "environment"
  flags_to_pass_from_environment += RCFLAGS='$(RCFLAGS)'
endif

################################################################################

# Multiple build directories.

# $(build_type) distinguishes optimized 'ship' builds from 'mpatrol'
# builds, which may be created by specifying 'build_type=mpatrol' on
# the make command line. Because 'mpatrol' builds use an invasive
# malloc debugger, they may run quite sluggishly, and they work poorly
# with gdb. But 'ship' builds are designed to work with gdb.
#
# Of course, other build types may be defined.

build_type ?= ship
toolset ?= gcc
build_directory := \
  ../build/$(notdir $(src_dir))/$(uname)/$(toolset)/$(build_type)

gpl_files := \
  COPYING \
  quoted_gpl \
  quoted_gpl_html \

MAKETARGET = \
  $(MAKE) \
    -C $@ \
    -f $(src_dir)/workhorse.make \
    --no-print-directory \
    $(flags_to_pass_from_environment) \
                         src_dir='$(src_dir)' \
                      build_type='$(build_type)' \
               platform-makefile='$(platform-makefile)' \
               USE_SO_ATTRIBUTES='$(USE_SO_ATTRIBUTES)' \
                    yyyymmddhhmm='$(yyyymmddhhmm)' \
  $(MAKECMDGOALS)

.PHONY: $(build_directory)
$(build_directory): $(gpl_files) date_last_made
	+@[ -d $@ ] || $(MKDIR) --parents $@
	+@$(MAKETARGET)

% :: $(build_directory) ;

################################################################################

# Datestamps. These are all UTC. Only the first has an explicit 'Z'
# suffix, to avoid any ambiguity.

yyyymmddhhmm := $(shell $(DATE) -u +'%Y%m%dT%H%MZ')
yyyymmdd     := $(shell $(DATE) -u +'%Y%m%d')
yyyymm       := $(shell $(DATE) -u +'%Y%m')
yyyy         := $(shell $(DATE) -u +'%Y')

.PHONY: date
date:
	@$(ECHO) $(yyyymmddhhmm)

################################################################################

# Lists of files in the source directory.

# Some files indisputably are source files ('*.cpp', e.g.) and some
# indisputably are not (the 'CVS' subdirectory, e.g.). Others might be
# source files for some purposes but not others: makefiles, e.g.,
# should contain copyright and license notices (which can be confirmed
# automatically), and arguably might be included in SLOC, but binary
# targets don't depend on them.

# MSYS !! The definition of $(subdirectories) does not work in MSYS's
# bash if a non-MSYS sed is used. Even this trivial expression to
# remove lines containing a slash
#   $ /c/usr/bin/sed -e'/\//!d'
# fails:
#   c:\usr\bin\sed.exe: -e expression #1, char 3: Unknown command: `/'
# apparently because MSYS so aggressively translates strings that look
# like path fragments. Here is a dramatic example using a non-MSYS
# build of gnu echo from sh-utils 1.12:
#   $ /c/usr/bin/echo '/[/]/!d'
# actually echoes
#   C:/msys/1.0/[/]/!d

# Files that can't be source in any useful sense:

# Graphics files whose format doesn't permit embedding copyright and
# license notices:

binary_graphics  := $(wildcard *.ico *.png)

expungible_files := $(wildcard *~ *.bak *eraseme*)

md5sum_files     := $(wildcard *md5sums)

patch_files      := $(wildcard *patch)

subdirectories   := $(shell $(LS) --classify | $(SED) -e'/\//!d' -e's/\/$$//')

testing_files    := expected.cgi.out $(wildcard *touchstone*)

never_source_files := \
  $(binary_graphics) \
  $(expungible_files) \
  $(md5sum_files) \
  $(patch_files) \
  $(subdirectories) \
  $(testing_files) \
  date_last_made \

# Files that are source in some restrictive sense only:

documentation_files := \
  $(wildcard ChangeLog* README*) \
  INSTALL \

makefiles := \
  $(wildcard *.make) \
  $(wildcard *GNUmakefile) \
  Makefile.am \

scripts := $(wildcard *.sed *.sh)

# TODO ?? The '.xsl' files need to be cleaned up.
# xml_files := $(wildcard *.xrc *.xsl)
xml_files := $(wildcard *.xrc)

xpm_files := $(wildcard *.xpm)

non_source_files := \
  $(never_source_files) \
  $(documentation_files) \
  $(gpl_files) \
  $(makefiles) \
  $(scripts) \
  $(xpm_files) \

# Here's a restrictive definition that comprises all files that are
# prerequisites of any binary target and that are not generated by
# any makefile rule. This list is appropriate for two uses: as the
# prerequisite for datestamp targets; and for counting SLOC, which
# should ignore files that are trivially generated.

prerequisite_files := $(filter-out $(non_source_files),$(wildcard *))

# Don't use a double-colon rule here, because it would cause make to
# mark the target files as having been updated.

$(prerequisite_files): ;

# Here's a liberal definition that comprises all files that should
# contain RCS Ids and copyright and license statements--requirements
# that are tested automatically by another target. License files
# themselves are excluded.

unlicensed_files := \
  $(gpl_files) \
  $(never_source_files) \
  $(xpm_files) \

licensed_files := $(filter-out $(unlicensed_files),$(wildcard *))

################################################################################

# Update datestamp files.

# Update the build-datestamp file whenever any other source file has
# changed. Don't use it in any other way: making any file depend on it
# would force costly, unnecessary relinking.

date_last_made: $(filter-out $@,$(prerequisite_files))
	@$(ECHO) These files are more recent than '$@': $?
	@$(TOUCH) $@
	@$(ECHO) Built $(yyyymmddhhmm).

# Update the version-datestamp header before committing any release
# candidate to cvs. Release candidates are named 'lmi-YYYYMMDDTHHMM'.

.PHONY: release_candidate
release_candidate:
	@$(ECHO) -e $(gpl_notices) '#define LMI_VERSION "$(yyyymmddhhmm)"' \
	  | $(SED) -e 's/^ *//' \
	  | $(TR) -d '\r' \
	  > version.hpp
	@$(ECHO) Version is $(yyyymmddhhmm) .
	@$(ECHO) "  Tag a release this way:"
	@$(ECHO) "cvs commit -m\"Mark release candidate\" version.hpp ChangeLog"
	@$(ECHO) "cvs rtag lmi-$(yyyymmddhhmm) lmi"

################################################################################

# License notices.

# These targets insert the GPL, its required notices, and datestamps
# into files that it is best to generate automatically.

COPYING:
	$(error Cannot build because file $@ is missing)

quoted_gpl: COPYING
	<$(src_dir)/COPYING \
	$(SED) -f $(src_dir)/text_to_strings.sed \
	| $(TR) -d '\r' \
	>$(src_dir)/$@

quoted_gpl_html: COPYING
	<$(src_dir)/COPYING \
	$(SED) -f $(src_dir)/text_to_html_strings.sed \
	| $(TR) -d '\r' \
	>$(src_dir)/$@

gpl_notices := \
"\
// Copyright (C) $(yyyy) Gregory W. Chicares.\n\
//\n\
// This program is free software; you can redistribute it and/or modify\n\
// it under the terms of the GNU General Public License version 2 as\n\
// published by the Free Software Foundation.\n\
//\n\
// This program is distributed in the hope that it will be useful,\n\
// but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
// GNU General Public License for more details.\n\
//\n\
// You should have received a copy of the GNU General Public License\n\
// along with this program; if not, write to the Free Software\n\
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n\
//\n\
// http://savannah.nongnu.org/projects/lmi\n\
// email: <chicares@cox.net>\n\
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA\n\
\n\
// $$\rId: $$\n\
\n\
"

################################################################################

# Clean.

# Almost all targets are built in a build directory, so the 'clean'
# target is run there: see 'workhorse.make'.
#
# This makefile has rules to build a few files in the source
# directory, viz.
#   quoted_gpl
#   quoted_gpl_html
# which should be updated only if lmi's owner changes the license;
#   version.hpp
# which should be updated by making target 'release_candidate' as
# needed (these required files are all in cvs; no 'clean' rule deletes
# them because they should never be deleted); and
#   date_last_made
# which is updated (or created if it didn't already exist) whenever
# make is run after any source file has changed.

.PHONY: source_clean
source_clean:
	@-$(RM) --force $(expungible_files)

.PHONY: clobber
clobber: source_clean
	@-$(RM) --force --recursive ../build

################################################################################

# Check conformity to certain formatting rules; count lines and defects.

# Arbitrarily but reasonably, files like scripts and makefiles aren't
# counted in SLOC, but marked defects in them are counted. This has
# some generally nugatory effect on a measure like defects per KLOC.

# In testing files modified this year for a current copyright notice,
# it is assumed that the year appears on the same line as the word
# "Copyright". That may become too strict in the future if more than
# one line is required to show all the years, but it is useful to
# avoid false positives that would arise when the current year appears
# in an RCS Id but not in the copyright notice.

expected_source_files := $(wildcard *.ac *.?pp *.c *.h *.rc *.xrc *.xsl)

# Invoke a supplemental makefile, if it exists, to test things that
# don't belong in the standard sources. For example, it might report
# occurrences of proprietary names.

supplemental_test_makefile = ../forbidden.make

.PHONY: check_conformity
check_conformity: source_clean
	@-[ ! -e $(supplemental_test_makefile) ] \
	  || $(MAKE) --no-print-directory -f $(supplemental_test_makefile)
	@$(ECHO) "  Unexpected or oddly-named source files:"
	@$(ECHO) $(filter-out $(expected_source_files),$(prerequisite_files))
	@$(ECHO) "  Files with irregular defect markers:"
	@$(GREP) --line-number '[A-Za-z]!!' $(licensed_files)          || true
	@$(GREP) --line-number '[A-Za-z] !![A-Za-z]' $(licensed_files) || true
	@$(GREP) --line-number \?\? $(licensed_files) | $(SED) -e '/TODO \?\?/d'
	@$(GREP) --line-number \?\?'[A-Za-z]' $(licensed_files)        || true
	@$(GREP) --line-number '?\{3,\}' $(licensed_files)             || true
	@$(ECHO) "  Files with lowercase 'c' in copyright symbol:"
	@$(GREP) --files-with-match '(c) *[12]' $(licensed_files)      || true
	@$(TOUCH) BOY --date=$(yyyy)0101
	@$(ECHO) "  Files lacking current copyright year:"
	@for z in $(licensed_files); \
	  do \
	    if [[ $$z -nt BOY ]] \
	      $(GREP) --files-without-match "Copyright.*$(yyyy)" $$z; \
	  done;
	@$(RM) --force BOY
	@$(ECHO) "  Files that don't point to savannah:"
	@$(GREP) --files-without-match savannah $(licensed_files)      || true
	@$(ECHO) "  Files that lack an RCS Id:"
	@$(GREP) --files-without-match '$$Id.*$$' $(licensed_files)    || true
	@$(ECHO) "  Files with malformed RCS Ids:"
	@$(GREP) --files-with-match '$$Id[^$$]*$$' $(licensed_files)   || true
	@$(ECHO) "  Files that contain non-empty blank lines:"
	@$(GREP) --line-number '^ \+$$' $(licensed_files)              || true
	@$(ECHO) "  Files with blanks at end of line:"
	@$(GREP) --line-number ' $$' $(licensed_files)                 || true
	@$(ECHO) "  Files that improperly contain physical tabs:"
	@$(GREP) -l '	' $(filter-out $(makefiles),$(licensed_files)) || true
	@$(ECHO) "  Files that contain carriage returns:"
	@for z in $(licensed_files) $(xpm_files); \
	  do \
	    $(ECHO) -n $$z; \
	    <$$z $(TR) '\r' '\a' | $(SED) -e'/\a/!d' | $(WC) -l; \
	  done | $(SED) -e'/ 0$$/d';
	@$(ECHO) "  Headers that should include \"config.hpp\" first but don't:"
	@for z in \
	  $(filter-out config.hpp $(wildcard config_*.hpp),$(wildcard *.hpp)); \
	  do \
	    $(SED) \
	    -e'/^#include "config.hpp"/,$$d' \
	    -e'/#.*include/!d' \
	    -e'0,1!d' \
	    -e"s/^.*$$/$$z/" $$z \
	    ; \
	  done;
	@$(ECHO) "  Files that use reserved identifiers:"
	@# The sed commands are sorted alphabetically by group:
	@#   {standard, platform-specific, compiler-specific, regrettable}
	@# TODO ?? Use '[^_A-Za-z0-9]_\|__' to find more reserved
	@# identifiers--then filter them with care.
	@$(GREP) \
	    '__' \
	    $(filter-out GNUmakefile,$(expected_source_files)) \
	  | $(SED) \
	    -e's/"[^"]*"//g' \
	    -e'/__FILE__/d' \
	    -e'/__LINE__/d' \
	    -e'/__STDC__/d' \
	    -e'/__STDC_IEC_559__/d' \
	    -e'/__cplusplus/d' \
	    -e'/__WIN32__/d' \
	    -e'/__X__/d' \
	    -e'/__arg[cv]/d' \
	    -e'/__i386/d' \
	    -e'/__unix__/d' \
	    -e'/__BIG_ENDIAN/d' \
	    -e'/__BORLANDC__/d' \
	    -e'/__BYTE_ORDER/d' \
	    -e'/__COMO__/d' \
	    -e'/__CYGWIN__/d' \
	    -e'/__GLIBCPP__/d' \
	    -e'/__GNUC__/d' \
	    -e'/__MINGW_H/d' \
	    -e'/__MINGW32__/d' \
	    -e'/__MINGW32_MAJOR_VERSION/d' \
	    -e'/__MINGW32_MINOR_VERSION/d' \
	    -e'/__MINGW32_VERSION/d' \
	    -e'/__STRICT_ANSI__/d' \
	    -e'/__asm__/d' \
	    -e'/__attribute__/d' \
	    -e'/__declspec/d' \
	    -e'/__emit__/d' \
	    -e'/__int64/d' \
	    -e'/__stdcall/d' \
	    -e'/__volatile__/d' \
	    -e'/__WXMSW__/d' \
	    -e'/__init_aux/d' \
	    -e'/__pow/d' \
	    -e'/____/d' \
	    -e'/__/!d'
	@$(ECHO) "  Problems detected by xmllint:"
	@for z in $(xml_files); \
	  do \
	    $(XMLLINT) $$z \
	    | $(TR) -d '\r' \
	    | $(DIFF) \
	      --ignore-blank-lines \
	      --unified=0 \
	      $$z - \
	      || $(ECHO) "... in file $$z"; \
	  done;
	@$(ECHO) "Total lines of code:"
	@$(WC) -l $(prerequisite_files) | $(SED) -e'/[Tt]otal/!d' -e's/[^0-9]//'
	@$(ECHO) "Number of source files:"
	@$(WC) -l $(prerequisite_files) | $(SED) -e'/[Tt]otal/d' | $(WC) -l
	@$(ECHO) "Number of marked defects:"
	@$(GREP) \?\? $(licensed_files) | $(WC) -l
	@$(TOUCH) --date=$(yyyymmdd) TODAY
	@$(TOUCH) --date=$(yyyymm)00 BOM
	@$(TOUCH) --date=$(yyyymm)22 CANDIDATE
	@if [[ TODAY -nt CANDIDATE ]] && [[ version.hpp -ot BOM ]] \
	  $(ECHO) "Is it time to 'make release_candidate'?"
	@$(RM) --force CANDIDATE
	@$(RM) --force BOM
	@$(RM) --force TODAY

################################################################################

# Prepare to commit to cvs.

.PHONY: cvs_ready
cvs_ready: source_clean
	-$(MAKE) check_conformity
	-$(MAKE) check_idempotence
	-$(MAKE) all test
	-$(MAKE) all test build_type=mpatrol
	-$(MAKE) lmi_wx_shared$(EXEEXT) build_type=so_test USE_SO_ATTRIBUTES=1

################################################################################

# Routine circadian test. Others might call this the 'nightly build'
# or 'daily smoke test'. What's important is not whether it's run in
# the daytime or in the nighttime, but rather that it's run something
# like once every twenty-four hours.

# TODO ?? This is an evolving experiment. Possible enhancements include:
#   - Add other tests, particularly regression tests.
#       Consider using ../products/src if it exists.
#       Also consider using cvs only, and using testdecks from cvs or ftp.
#   - Test skeleton branch, too.
#   - Gather statistics, e.g., elapsed time and total size of binaries.
#   - Filter logs e.g. as 'fancy.make' does.
#   - Upload outcome (e.g. log and statistics) to savannah.
#       Can a simple binary measure {success, failure} be devised?
#   - Set this up as a 'cron' job (or equivalent on other platforms).
#   - Devise a simple way to test changes to this target without
#       requiring 5 Mb of downloading and 10 minutes of crunching.

circadian_directory := $(TMPDIR)/lmi-circadian-$(yyyymmddhhmm)
circadian_log := log-lmi-circadian-$(yyyymmddhhmm)

.PHONY: checkout
checkout:
	$(MKDIR) --parents $(circadian_directory); \
	cd $(circadian_directory); \
	export CVS_RSH="ssh"; \
	cvs -z3 -d:ext:anoncvs@savannah.gnu.org:/cvsroot/lmi co skeleton lmi; \

.PHONY: circadian_test
circadian_test: checkout
	-$(MAKE) -C $(circadian_directory)/lmi cvs_ready >../$(circadian_log)

################################################################################

# Archive a snapshot.

archname := lmi-$(yyyymmddhhmm)

.PHONY: archive
archive: source_clean
	$(MKDIR) ../$(archname)
	-$(CP) --force --preserve --recursive * ../$(archname)
	$(TAR) --create --directory=.. --file=$(archname).tar $(archname)
	$(RM) --force --recursive ../$(archname)
	$(BZIP2) --verbose --verbose $(archname).tar

