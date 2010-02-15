# Top-level lmi makefile.
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

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

################################################################################

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

################################################################################

# Directives and such.

# The GNU make manual says:
#   "This is almost always what you want make to do, but it is not
#   historical practice; so for compatibility, you must explicitly
#   request it."

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
# commands, which significantly reduces the number of lines emitted by
# 'make -d', making debug output easier to read.
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

# Local options.
#
# See the documentation in 'local_options.sh'. Including this file
# defines $(local_options), which is passed to submakefiles.

-include $(src_dir)/local_options.make
$(src_dir)/local_options.make:: ;

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

# $(MAKEOVERRIDES) is explicitly specified after $(local_options) when
# a submakefile is invoked, in order to allow a variable definition on
# the 'make' command line to override any definition of the same
# variable in $(local_options).

MAKETARGET = \
  $(MAKE) \
    --directory=$@ \
    --file=$(src_dir)/workhorse.make \
    --no-print-directory \
    $(local_options) $(MAKEOVERRIDES) \
                         src_dir='$(src_dir)' \
                         toolset='$(toolset)' \
                      build_type='$(build_type)' \
               platform-makefile='$(platform-makefile)' \
               USE_SO_ATTRIBUTES='$(USE_SO_ATTRIBUTES)' \
                    yyyymmddhhmm='$(yyyymmddhhmm)' \
  $(MAKECMDGOALS)

.PHONY: $(build_directory)
$(build_directory): $(gpl_files)
	+@[ -d $@ ] || $(MKDIR) --parents $@
	+@$(MAKETARGET)

% :: $(build_directory) ; @:

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

# Mark release candidate.

# Update the version-datestamp header before committing any release
# candidate to cvs. Release candidates are named 'lmi-YYYYMMDDTHHMM'.

.PHONY: release_candidate
release_candidate:
	@$(ECHO) -e \
	  $(gpl_notices) \
	  '#ifndef version_hpp\n' \
	  '#define version_hpp\n\n' \
	  '#include "config.hpp"\n\n' \
	  '#define LMI_VERSION "$(yyyymmddhhmm)"\n\n' \
	  '#endif // version_hpp\n' \
	  | $(SED) -e 's/^ *//' \
	  | $(TR) --delete '\r' \
	  > version.hpp
	@$(ECHO) Version is "'$(yyyymmddhhmm)'".
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
	$(SED) --file=$(src_dir)/text_to_strings.sed \
	| $(TR) --delete '\r' \
	>$(src_dir)/$@

quoted_gpl_html: COPYING
	<$(src_dir)/COPYING \
	$(SED) --file=$(src_dir)/text_to_html_strings.sed \
	| $(TR) --delete '\r' \
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
// along with this program; if not, write to the Free Software Foundation,\n\
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA\n\
//\n\
// http://savannah.nongnu.org/projects/lmi\n\
// email: <gchicares@sbcglobal.net>\n\
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
# which should be updated only if lmi's owner changes the license; and
#   version.hpp
# which should be updated by making target 'release_candidate' as
# needed (these required files are all in cvs; no 'clean' rule deletes
# them because they should never be deleted).

expungible_files := $(wildcard *~ *.bak *eraseme*)

.PHONY: source_clean
source_clean:
	@-$(RM) --force $(expungible_files)

.PHONY: clean
clean: source_clean
	@-$(RM) --force --recursive $(build_directory)

.PHONY: distclean mostlyclean maintainer-clean
distclean mostlyclean maintainer-clean: clean

.PHONY: clobber
clobber: source_clean
	@-$(RM) --force --recursive ../build

################################################################################

# Custom tools built from source.

TEST_CODING_RULES := $(build_directory)/test_coding_rules$(EXEEXT)

.PHONY: custom_tools
custom_tools:
	@$(MAKE) test_coding_rules$(EXEEXT)
	@$(CP) --preserve --update $(TEST_CODING_RULES) /opt/lmi/local/bin

################################################################################

# Check conformity to certain formatting rules; count lines and defects.

xml_files := $(wildcard *.cns *.ill *.xml *.xrc *.xsd *.xsl)

.PHONY: check_concinnity
check_concinnity: source_clean custom_tools
	@$(TOUCH) --date=$(yyyy)0101 BOY
	@$(TOUCH) --date=$(yyyymm)01 BOM
	@$(TOUCH) --date=$(yyyymmdd) TODAY
	@$(TOUCH) --date=$(yyyymm)22 CANDIDATE
	@[ TODAY -nt CANDIDATE ] && [ version.hpp -ot BOM ] \
	  && $(ECHO) "Is it time to 'make release_candidate'?" || true
	@[ license.cpp -ot BOY ] \
	  && $(ECHO) "Make the 'happy_new_year' target." || true
	@$(RM) --force CANDIDATE
	@$(RM) --force TODAY
	@$(RM) --force BOM
	@$(RM) --force BOY
	@for z in $(build_directory)/*.d; do [ -s $$z ]         || echo $$z; done;
	@for z in $(build_directory)/*.o; do [ -f $${z%%.o}.d ] || echo $$z; done;
	@$(ECHO) "  Problems detected by xmllint:"
	@for z in $(xml_files); \
	  do \
	    $(XMLLINT) --encode ASCII $$z \
	    | $(TR) --delete '\r' \
	    | $(DIFF) \
	      --ignore-blank-lines \
	      --ignore-matching-lines='<\?xml' \
	      --unified=0 \
	      $$z - \
	      || $(ECHO) "... in file $$z"; \
	  done;
	@$(ECHO) "  Miscellaneous problems:"
	@-$(TEST_CODING_RULES) *

################################################################################

# Prepare to commit to cvs.

.PHONY: cvs_ready
cvs_ready: source_clean
	-$(MAKE) check_concinnity
	-$(MAKE) check_physical_closure
	-$(MAKE) all test
	-$(MAKE) all test build_type=mpatrol
	-$(MAKE) test build_type=safestdlib
	-$(MAKE) lmi_wx_shared$(EXEEXT) build_type=so_test USE_SO_ATTRIBUTES=1

################################################################################

# Update copyright notices.

# Be sure to update these hardcoded dates. This target might be made
# either in the last days of the old year or the first days of the
# new, so they can't be derived dynamically from the current date.

old_year := 2009
new_year := 2010

backup_directory := saved_$(old_year)

unutterable := Copyright

.PHONY: happy_new_year
happy_new_year: source_clean
	$(TOUCH) --date=$(old_year)0101 BOY
	$(MKDIR) --parents $(backup_directory)
	for z in *; \
	  do \
	       [ $$z -nt BOY ] \
	    && [ ! -d $$z ] \
	    && $(GREP) --quiet --files-with-matches $(unutterable) $$z \
	    && $(SED) \
	      --in-place='$(backup_directory)/*' \
	      -e'/$(unutterable)/s/$(old_year)/$(old_year), $(new_year)/' \
	      $$z; \
	  done;
	$(RM) --force BOY
	$(ECHO) "Check these potential issues:"
	@$(GREP) '$(old_year)[, ]*$(old_year)' * || true
	@$(GREP) '$(new_year)[, ]*$(old_year)' * || true
	@$(GREP) '$(new_year)[, ]*$(new_year)' * || true
	@$(GREP) $(unutterable) * \
	  | $(SED) \
	    -e '/$(unutterable).*$(new_year) Gregory W. Chicares/d' \
	    -e '/$(unutterable).*$(new_year) Vadim Zeitlin/d' \
	    -e '/unutterable := $(unutterable)/d' \
	    -e '/$(unutterable) (C) .(yyyy) Gregory W. Chicares/d' \
	    -e '/$(unutterable) (C) 1989, 1991 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1987, 1989 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1987, 1989, 1992 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1995, 1996 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) 1995, 1996, 2000 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) <year>  <name of author>/d' \
	    -e '/Gnomovision version 69, $(unutterable) (C) year name of author/d' \
	    -e '/$(unutterable) (C) &lt;year&gt;  &lt;name of author&gt;/d' \
	    -e '/GNU cgicc $(unutterable) (C) 1996, 1997, 1998, 1999, 2000 Stephen F. Booth/d' \
	    -e '/$(unutterable) and license notices for graphics files/d' \
	    -e '/$(unutterable) (C) 1997-2002 Graeme S. Roy <graeme.roy@analog.com>/d' \
	    -e '/(C) $(unutterable) Beman Dawes 1995-2001. Permission to copy, use, modify, sell/d' \
	    -e '/(C) $(unutterable) Beman Dawes 2001. Permission to copy, use, modify, sell/d' \
	    -e '/(C) $(unutterable) Beman Dawes 2000. Permission to copy, use, modify, sell/d' \
	    -e '/$(unutterable) Jens Maurer 2000/d' \
	    -e '/$(unutterable) Kevlin Henney, 2000, 2001. All rights reserved./d' \
	    -e '/$(unutterable) Kevlin Henney, 2000-2003. All rights reserved./d' \
	    -e '/$(unutterable) Terje Sletteb.* and Kevlin Henney, 2005./d' \
	    -e '/Portions marked.*$(unutterable).*Gregory W. Chicares/d' \
	    -e '/oss << "$(unutterable).*" << 1900 + t1->tm_year;/d' \
	    -e '/$(unutterable) (C) 1994$$/d' \
	    -e '/$(unutterable) (C) 1996-1998$$/d' \
	    -e '/"$(unutterable)". That may become too strict in the future/d' \
	    -e '/:..$(unutterable).d$$/d' \
	    -e '/:good_copyright=/d' \
	    -e '/:$(unutterable) (C)$$/d' \
	    -e '/$(unutterable) (C) 1900/d' \
	  || true

################################################################################

# Routine nychthemeral test. Others might call this a 'nightly build'
# or 'daily smoke test'. What's important is not whether it's run in
# the daytime or in the nighttime, but rather that it's run something
# like once every twenty-four hours.

# TODO ?? This is an evolving experiment. Possible enhancements include:
#   - Add other tests, particularly system tests.
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

CVS_RSH := ssh
CVSROOT := :pserver:anonymous@cvs.sv.nongnu.org:/sources/lmi

nychthemeral_directory := $(TMPDIR)/lmi-nychthemeral-$(yyyymmddhhmm)
nychthemeral_log := log-lmi-nychthemeral-$(yyyymmddhhmm)

.PHONY: checkout
checkout:
	$(MKDIR) --parents $(nychthemeral_directory); \
	cd $(nychthemeral_directory); \
	export CVS_RSH='$(CVS_RSH)'; \
	export CVSROOT='$(CVSROOT)'; \
	cvs -z3 co skeleton lmi; \

.PHONY: nychthemeral_test
nychthemeral_test: checkout
	-$(MAKE) --directory=$(nychthemeral_directory)/lmi cvs_ready \
	  >../$(nychthemeral_log)

################################################################################

# Archive a snapshot.

archive_name := lmi-src-$(yyyymmddhhmm)

.PHONY: archive
archive: source_clean
	$(MKDIR) ../$(archive_name)
	-$(CP) --force --preserve --recursive * ../$(archive_name)
	$(TAR) --create --directory=.. --file=$(archive_name).tar $(archive_name)
	$(RM) --force --recursive ../$(archive_name)
	$(BZIP2) --verbose --verbose $(archive_name).tar

