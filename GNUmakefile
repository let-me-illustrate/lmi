# Top-level lmi makefile.
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

# $Id: GNUmakefile,v 1.4 2005-02-17 23:59:25 chicares Exp $

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

# Multiple build directories.

# $(build_type) distinguishes optimized 'ship' builds from 'mpatrol'
# builds, which may be created by specifying 'build_type=mpatrol' on
# the make command line. Because 'mpatrol' builds use an invasive
# malloc debugger, they may run quite sluggishly, and they work poorly
# with gdb. But 'ship' builds are built to work with gdb.
#
# Of course, other build types may be defined.

build_type ?= ship
build_directory := ../build/$(uname)/$(build_type)

MAKETARGET = \
  $(MAKE) \
    -C $@ \
    -f $(src_dir)/workhorse.make \
    --no-print-directory \
                         src_dir='$(src_dir)' \
                      build_type='$(build_type)' \
               platform-makefile='$(platform-makefile)' \
                idempotent_files='$(idempotent_files)' \
  $(MAKECMDGOALS)


.PHONY: $(build_directory)
$(build_directory): $(datestamp_files)
	+@[ -d $@ ] || $(MKDIR) --parents $@
	+@$(MAKETARGET)

% :: $(build_directory) ;

################################################################################

# Headers and template-instantiation files to test for idempotence.

# The actual idempotence test is performed in the build directory.

# Only files in the current directory are tested. Files that reside
# elsewhere (e.g., headers accompanying libraries) are not tested.

# Exclude headers named 'config_*.hpp': they are designed to signal
# errors if they are used separately.

idempotent_files := $(wildcard *.h *.hpp *.tpp *.xpp)
idempotent_files := $(filter-out config_%.hpp,$(idempotent_files))
idempotent_files := $(addsuffix .idempotent,$(idempotent_files))

################################################################################

# Datestamps.

yyyymmddhhmm := $(shell $(DATE) -u +'%Y%m%dT%H%MZ')
yyyymmdd     := $(shell $(DATE) -u +'%Y%m%dZ')
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

# Files that can't be source in any useful sense:

# Graphics files whose format doesn't permit embedding copyright and
# license notices (TODO ?? can these be replaced by xpm?):

binary_graphics := $(wildcard *.bmp *.ico *.png)

expungible_files := $(wildcard *~ *.bak *eraseme*)

subdirectories := $(shell $(LS) --classify | $(SED) -e'/\//!d' -e's/\/$$//')

testing_files := expected.cgi.out $(wildcard *touchstone*)

never_source_files := \
  $(binary_graphics) \
  $(expungible_files) \
  $(subdirectories) \
  $(testing_files) \
  $(wildcard *-patch-*) \

# Files that are source in some restrictive sense only:

datestamp_files := \
  build.hpp \
  version.hpp \

documentation_files := \
  ChangeLog \
  README \

gpl_files := \
  COPYING \
  quoted_gpl \
  quoted_gpl_html \

makefiles := $(wildcard *make*)

scripts := $(wildcard *.sed)

xpm_files := $(wildcard *.xpm)

non_source_files := \
  $(never_source_files) \
  $(datestamp_files) \
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

licensed_files := $(filter-out $(gpl_files) $(never_source_files),$(wildcard *))

################################################################################

# These headers define datestamp macros.

# Update the version-datestamp header after committing any group of
# files to cvs. Use target 'cvs_ready' to do this reliably.

.PHONY: set_version_datestamp
set_version_datestamp:
	@$(ECHO) $(gpl_notices) '#define LMI_VERSION "$(yyyymmddhhmm)"' \
	  |$(SED) -e's/^ *//' |$(TR) -d '\r' > version.hpp
	@$(ECHO) Version is '$(yyyymmddhhmm)'.

# Update the build-datestamp header whenever any other source file has
# changed. Don't do this every time make is run: that would force
# recompilation of files that depend on this header, but running make
# again immediately after running it successfully should do nothing.
#
# Similarly, update the version-datestamp header, but in a different
# way. Instead of naming a new version whenever any other source file
# has changed, retain the version from the header in cvs and add a
# notation that the code has been modified.

build.hpp: $(filter-out $@,$(prerequisite_files)) version.hpp
	@$(ECHO) These files are more recent than '$@': $?
	@$(ECHO) $(gpl_notices) '#define LMI_BUILD "$(yyyymmddhhmm)"' \
	  |$(SED) -e's/^ *//' |$(TR) -d '\r' > build.hpp
	@$(ECHO) Built '$(yyyymmddhhmm)'.

version.hpp: $(filter-out $@,$(prerequisite_files))
	@$(CP) version.hpp eraseme
	@$(SED) <eraseme -e's/Z"$$/Z (modified)"/' >version.hpp
	@$(RM) eraseme

################################################################################

# License notices.

# These targets insert the GPL, its required notices, and datestamps
# into files that it is best to generate automatically.

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

# Almost all targets are built in a build directory. This makefile
# has rules to build a few files in the source directory, viz.
#   quoted_gpl
#   quoted_gpl_html
# which should be updated only if the owner changes the license;
#   version.hpp
# which should be updated when anything is committed to cvs; and
#   build.hpp
# which is updated automatically whenever make is run after any source
# file has changed. These required files are all in cvs; no 'clean'
# rule deletes them because they should never be deleted.

.PHONY: clean distclean mostlyclean
clean distclean mostlyclean:
	@-$(RM) --force $(expungible_files)

.PHONY: maintainer-clean
maintainer-clean: distclean

.PHONY: clobber
clobber: maintainer-clean
	@-$(RM) --force --recursive ../build

################################################################################

# Check conformity to certain formatting rules; count lines and defects.

# Arbitrarily but reasonably, files like scripts and makefiles aren't
# counted in SLOC, but marked defects in them are counted. This has
# some generally nugatory effect on a measure like defects per KLOC.

eat := pwd > /dev/null # Gobble unwanted grep exit codes.

.PHONY: check_conformity
check_conformity: mostlyclean
	@$(ECHO) "  Unexpected or oddly-named source files:"
	@$(ECHO) $(filter-out $(wildcard *.?pp *.c *.h),$(prerequisite_files))
	@$(ECHO) "  Files that don't point to savannah:"
	@-$(GREP) --files-without-match savannah $(licensed_files)     || $(eat)
	@$(ECHO) "  Files that lack an RCS Id:"
	@-$(GREP) --files-without-match '$$Id.*$$' $(licensed_files)   || $(eat)
	@$(ECHO) "  Files that contain non-empty blank lines:"
	@-$(GREP) --line-number '^ \+$$' $(licensed_files)             || $(eat)
	@$(ECHO) "  Files that improperly contain physical tabs:"
	@-$(GREP) -l '	' $(filter-out $(makefiles),$(licensed_files)) || $(eat)
	@$(ECHO) "  Files that contain carriage returns:"
	@for z in $(licensed_files); \
	  do \
	    $(ECHO) -n $$z; \
	    <$$z $(TR) '\r' '\a' | $(SED) -e'/\a/!d' | $(WC) -l; \
	  done | $(SED) -e'/ 0$$/d';
	@$(ECHO) "  Headers that should include \"config.hpp\" first but don't:"
	@for z in $(filter-out $(wildcard config*.hpp),$(wildcard *.hpp)); \
	  do \
	    $(SED) \
	    -e'/^#include "config.hpp"/,$$d' \
	    -e'/#.*include/!d' \
	    -e'0,1!d' \
	    -e"s/^.*$$/$$z/" $$z \
	    ; \
	  done;
	@$(ECHO) "Total lines of code:"
	@$(WC) -l $(prerequisite_files) | $(SED) -e'/[Tt]otal/!d' -e's/[^0-9]//'
	@$(ECHO) "Number of source files:"
	@$(WC) -l $(prerequisite_files) | $(SED) -e'/[Tt]otal/d' | $(WC) -l
	@$(ECHO) "Number of marked defects:"
	@$(GREP) \?\? $(licensed_files) | $(WC) -l

################################################################################

# Prepare to commit to cvs.

.PHONY: cvs_ready
cvs_ready: distclean set_version_datestamp
	-$(MAKE) check_conformity 2>&1 |$(SED) -e'/ermission.denied/d'
	-$(MAKE) check_idempotence
	-$(MAKE) all test
	-$(MAKE) all test build_type=mpatrol

################################################################################

# Archive a snapshot.

archname := lmi-$(yyyymmddhhmm)

archive: distclean set_version_datestamp
	$(MKDIR) ../$(archname)
	-$(CP) --force --preserve --recursive * ../$(archname)
	$(TAR) --create --directory=.. --file=$(archname).tar $(archname)
	$(RM) --force --recursive ../$(archname)
	$(BZIP2) --verbose --verbose $(archname).tar

