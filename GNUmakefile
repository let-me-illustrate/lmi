# Top-level lmi makefile.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

this_makefile := $(abspath $(lastword $(MAKEFILE_LIST)))

################################################################################

# The general technique used in the 'Multiple build directories'
# section is due to Paul D. Smith:
#   https://make.mad-scientist.net/papers/multi-architecture-builds/#advanced
# with the following significant differences:
#
# For greater clarity, the 'actual' makefile is a separate file,
# instead of a conditional section of this file.
#
# $(MAKEFLAGS) is used instead of '.SUFFIXES': it is more powerful,
# for instance because it can automatically enforce Paul's advice
# "it's best to invoke make with the -r option".
#
# One of several architecture-specific files is included, based on a
# 'config.guess' probe. Separate files encapsulate variations better.
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

# Process this makefile serially, but pass any '--jobs=' parameter to
# submakefiles.

.NOTPARALLEL:

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

# Directories (and toolchain selection on which they depend).

# SOMEDAY !! Follow the GNU Coding Standards
#   https://www.gnu.org/prep/standards/html_node/Directory-Variables.html
# more closely, changing the value of $(datadir), and perhaps using
# some other standard directories that are commented out for now.

# source files (GNU Coding Standards don't suggest any default value)
srcdir          := $(CURDIR)

# $(srcdir) really shouldn't be overridden. A noisy assertion is more
# helpful than a silent 'override' directive.
ifneq ($(srcdir),$(CURDIR))
  $(error Assertion failure: source directory misconfigured)
endif

# "Source" various crucial environment variables that are used in the
# definitions below.

LMI_ENV_FILE := /tmp/lmi_env_$(shell date -u +'%s_%N').eraseme
$(shell $(srcdir)/transume_toolchain.sh > $(LMI_ENV_FILE))
include $(LMI_ENV_FILE)
$(LMI_ENV_FILE):: ;

prefix          := /opt/lmi
# parent directory for machine-specific binaries
exec_prefix     := $(prefix)/$(LMI_COMPILER)_$(LMI_TRIPLET)
# binaries that users can run
bindir          := $(prefix)/bin
# binaries that administrators can run
#sbindir         := $(exec_prefix)/sbin
# binaries run by programs
#libexecdir      := $(exec_prefix)/libexec
# parent directory for read-only architecture-independent data files
datarootdir     := $(prefix)/share
# idiosyncratic read-only architecture-independent data files
# GNU standard default:
# datadir         := $(datarootdir)
# nonstandard value used for now:
datadir         := $(prefix)/data
# read-only data files that pertain to a single machine
#sysconfdir      := $(prefix)/etc
# architecture-independent data files which the programs modify while they run
#sharedstatedir  := $(prefix)/com
# data files which the programs modify while they run
#localstatedir   := $(prefix)/var
# data files which the programs modify while they run, persisting until reboot
#runstatedir     := $(localstatedir)/run
# headers
#includedir      := $(prefix)/include
docdir          := $(datarootdir)/doc/lmi
htmldir         := $(docdir)
#libdir          := $(exec_prefix)/lib

# These directories are outside the scope of the GNU Coding Standards.
# Therefore, their names may contain '_' for distinction and clarity.
localbindir     := $(prefix)/local/$(LMI_COMPILER)_$(LMI_TRIPLET)/bin
locallibdir     := $(prefix)/local/$(LMI_COMPILER)_$(LMI_TRIPLET)/lib
localincludedir := $(prefix)/local/include
test_dir        := $(prefix)/test
touchstone_dir  := $(prefix)/touchstone

################################################################################

# Other makefiles included; makefiles not to be remade.

# Included files that don't need to be remade are given explicit empty
# commands, which significantly reduces the number of lines emitted by
# 'make -d', making debug output easier to read.
#
# Included makefiles are cited by absolute pathname, e.g.
#   include $(srcdir)/included-file
#   $(srcdir)/included-file:: ;
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

# Don't remake this makefile.

GNUmakefile $(srcdir)/GNUmakefile:: ;

# Configuration.

include $(srcdir)/configuration.make
$(srcdir)/configuration.make:: ;

# Local options.
#
# See the documentation in 'local_options.sh'. Including this file
# defines $(local_options), which is passed to submakefiles.

-include $(srcdir)/local_options.make
$(srcdir)/local_options.make:: ;

################################################################################

# Multiple build directories.

# $(build_type) distinguishes optimized default 'ship' builds from
# special-purpose ones that can be specified on the make command line.

build_type ?= ship
build_dir := $(exec_prefix)/build/$(build_type)

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
    --file=$(srcdir)/workhorse.make \
    --no-print-directory \
    $(local_options) \
    $(MAKEOVERRIDES) \
                          prefix='$(prefix)' \
                     exec_prefix='$(exec_prefix)' \
                          bindir='$(bindir)' \
                     datarootdir='$(datarootdir)' \
                         datadir='$(datadir)' \
                          docdir='$(docdir)' \
                         htmldir='$(htmldir)' \
                          srcdir='$(srcdir)' \
                     localbindir='$(localbindir)' \
                     locallibdir='$(locallibdir)' \
                 localincludedir='$(localincludedir)' \
                        test_dir='$(test_dir)' \
                  touchstone_dir='$(touchstone_dir)' \
                    LMI_COMPILER='$(LMI_COMPILER)' \
                     LMI_TRIPLET='$(LMI_TRIPLET)' \
                      build_type='$(build_type)' \
               USE_SO_ATTRIBUTES='$(USE_SO_ATTRIBUTES)' \
                    yyyymmddhhmm='$(yyyymmddhhmm)' \
  $(MAKECMDGOALS)

.PHONY: $(build_dir)
$(build_dir): $(gpl_files)
	+@[ -d $@                 ] || $(MKDIR) --parents $@
	+@[ -d $(localbindir)     ] || $(MKDIR) --parents $(localbindir)
	+@[ -d $(locallibdir)     ] || $(MKDIR) --parents $(locallibdir)
	+@[ -d $(localincludedir) ] || $(MKDIR) --parents $(localincludedir)
	+@for z in $(compiler_runtime_files); do \
	    $(INSTALL) -c -m 0775 -c $$z $(localbindir) ; \
	  done;
	+@$(MAKETARGET)

% :: $(build_dir) ; @:

################################################################################

# Display selected variables.

.PHONY: show_env
show_env:
	@printf 'Internal make variables particular to lmi:\n\n'
	@printf 'LMI_COMPILER    = "%s"\n' "$(LMI_COMPILER)"
	@printf 'LMI_TRIPLET     = "%s"\n' "$(LMI_TRIPLET)"
	@printf 'coefficiency    = "%s"\n' "$(coefficiency)"
	@printf 'PATH            = "%s"\n' "$(PATH)"
	@printf 'WINEPATH        = "%s"\n' "$(WINEPATH)"
	@printf 'PERFORM         = "%s"\n' "$(PERFORM)"
	@printf 'prefix          = "%s"\n' "$(prefix)"
	@printf 'exec_prefix     = "%s"\n' "$(exec_prefix)"
	@printf 'localbindir     = "%s"\n' "$(localbindir)"
	@printf 'locallibdir     = "%s"\n' "$(locallibdir)"
	@printf 'localincludedir = "%s"\n' "$(localincludedir)"
	@printf 'test_dir        = "%s"\n' "$(test_dir)"
	@printf 'EXEEXT          = "%s"\n' "$(EXEEXT)"
	@printf 'SHREXT          = "%s"\n' "$(SHREXT)"

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

# Designate a release candidate.

# A release candidate is a revision with an updated version-datestamp
# header: it is merely an sha1sum pointing to a designated commit.
# The code actually released into production each month is the last
# release candidate designated that month. No repository tag would be
# useful because releases can easily be found with this command:
#   git log --follow version.hpp

define version_hpp_text
$(gpl_notices)

#ifndef version_hpp
#define version_hpp

#include "config.hpp"

#define LMI_VERSION "$(yyyymmddhhmm)"

#endif // version_hpp
endef
export version_hpp_text

.PHONY: release_candidate
release_candidate:
	@$(ECHO) "$$version_hpp_text" \
	  | $(SED) -e 's/^ *//' \
	  | $(TR) --delete '\r' \
	  > version.hpp
	@$(ECHO) "  To designate a release candidate, push a change such as:"
	@$(ECHO) \
	  "git commit version.hpp -m'Designate release candidate $(yyyymmddhhmm)'"

################################################################################

# License notices.

# These targets insert the GPL, its required notices, and datestamps
# into files that it is best to generate automatically.

COPYING:
	$(error Cannot build because file $@ is missing)

quoted_gpl: COPYING
	<$(srcdir)/COPYING \
	$(SED) --file=$(srcdir)/text_to_strings.sed \
	| $(TR) --delete '\r' \
	>$(srcdir)/$@

quoted_gpl_html: COPYING
	<$(srcdir)/COPYING \
	$(SED) --file=$(srcdir)/text_to_html_strings.sed \
	| $(TR) --delete '\r' \
	>$(srcdir)/$@

define gpl_notices :=
// Copyright (C) $(yyyy) Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
endef

################################################################################

# Clean and its kindred.

# This makefile has rules to build a few files in the source
# directory, viz.
#   quoted_gpl
#   quoted_gpl_html
# which should be updated only if lmi's owner changes the license; and
#   version.hpp
# which should be updated by making target 'release_candidate' as
# needed (these required files are all in the repository; no 'clean'
# rule deletes them because they should never be deleted).

expungible_files := $(wildcard *~ *.bak *eraseme*)

.PHONY: source_clean
source_clean:
	@-$(RM) --force $(expungible_files)

# Simple aliases.

.PHONY: distclean mostlyclean maintainer-clean
distclean mostlyclean maintainer-clean: clean

# This precondition permits writing progressively severer 'clean'
# targets more clearly. To use an alternative like
#   rm -rf $(build_dir)/../..
# would be to invite disaster.
ifneq ($(build_dir),$(exec_prefix)/build/$(build_type))
  $(error Assertion failure: build directory misconfigured)
endif

.PHONY: clean
clean: source_clean
	-$(RM) --force --recursive $(exec_prefix)/build/$(build_type)

.PHONY: clobber
clobber: source_clean
	-$(RM) --force --recursive $(exec_prefix)/build

.PHONY: raze
raze: source_clean
	-$(RM) --force --recursive $(prefix)/gcc_x86_64-w64-mingw32
	-$(RM) --force --recursive $(prefix)/gcc_x86_64-pc-linux-gnu

.PHONY: eviscerate
eviscerate: source_clean
	-$(RM) --force --recursive $(prefix)/bin
	-$(RM) --force --recursive $(prefix)/local
	-$(RM) --force --recursive $(prefix)/third_party
	-$(RM) --force --recursive $(prefix)/zzz
	-$(RM) --force --recursive $(prefix)/gcc_x86_64-w64-mingw32
	-$(RM) --force --recursive $(prefix)/gcc_x86_64-pc-linux-gnu

# A simple '$(RM) --force --recursive $(bindir)' would remove the
# directory as well as its contents. However, if $(bindir) is the
# current working directory in another terminal, hilarity ensues:
#   sh: 0: getcwd() failed: No such file or directory
# While '-delete' is not POSIX, it's supported by GNU coreutils and
# {Open,Free}BSD. The command might instead have been written thus:
#   find $(bindir) -type f -print0 | xargs -0 rm -rf
# but '-print0' isn't POSIX either.

.PHONY: uninstall
uninstall:
	-find $(bindir) -type f -delete

################################################################################

# Custom tools built from source.

TEST_CODING_RULES := $(build_dir)/test_coding_rules$(EXEEXT)

.PHONY: custom_tools
custom_tools:
	@[ "$$LMI_TRIPLET" = "x86_64-pc-linux-gnu" ] \
	  || ($(ECHO) "'$@' requires x86_64-pc-linux-gnu." && false)
	@$(MAKE) test_coding_rules$(EXEEXT)
	@$(INSTALL) -c -m 0775 $(TEST_CODING_RULES) $(localbindir)

################################################################################

# Check conformity to certain formatting rules; count lines and defects.
#
# The tests in $(build_dir) identify object ('.o') files with no
# corresponding autodependency ('.d') file, and zero-byte '.d' files.
# Either of these suggests a build failure that may render dependency
# files invalid; 'make clean' should provide symptomatic relief.
#
# The "find ... '*.sed'" test somewhat loosely identifies source files
# whose executable bit is improperly set. It is properly set iff the
# file starts with a hash-bang; to avoid the cost of opening every
# file, a simple heuristic is used, '*.sh *.sed' being the only files
# permitted (though not required) to be executable. No exception need
# be made for msw '.bat' files, which normally should not be run in a
# *nix shell.

prefascicle_dir ?= $(srcdir)
xml_files := $(wildcard $(addprefix $(prefascicle_dir)/,*.cns *.ill *.xml *.xrc *.xsd *.xsl))

.PHONY: check_concinnity
check_concinnity: source_clean custom_tools
	@$(TOUCH) --date=$(yyyy)0101 BOY
	@$(TOUCH) --date=$(yyyymm)01 BOM
	@$(TOUCH) --date=$(yyyymmdd) TODAY
	@$(TOUCH) --date=$(yyyymm)22 CANDIDATE
	@[ TODAY -nt CANDIDATE ] && [ version.hpp -ot BOM ] \
	  && $(ECHO) "Is it time to 'make release_candidate'?" || true
	@[ -f license.cpp ] && [ license.cpp -ot BOY ] \
	  && $(ECHO) "Make the 'happy_new_year' target." || true
	@$(RM) --force CANDIDATE
	@$(RM) --force TODAY
	@$(RM) --force BOM
	@$(RM) --force BOY
	@cd $(prefascicle_dir) && [ -f md5sums ] \
	  && <md5sums $(SED) -e'/\.test$$\|\.test0$$\|\.test1$$\|\.tsv$$\|\.xml$$/d' \
	  | $(MD5SUM) --check --quiet || true
	@for z in $(build_dir)/*.d; do [ -s $$z ]         || echo $$z; done;
	@for z in $(build_dir)/*.o; do [ -f $${z%%.o}.d ] || echo $$z; done;
	@find $(prefascicle_dir) -maxdepth 1 -executable -type f \
	  -not -name '*.sh' -not -name '*.sed' \
	  -not -name 'commit-msg' \
	  -not -name 'pre-commit' \
	  -not -name 'post-checkout' \
	  | $(SED) -e's/^/Improperly executable: /'
	@find $(prefascicle_dir) \
	  -not \( -path $(prefascicle_dir)/third_party -prune \) \
	  -not \( -path $(prefascicle_dir)/.git/modules -prune \) \
	  -executable -type f -print0 \
	  | xargs --null --max-args=1 --max-procs="$(shell nproc)" ./check_script.sh
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
	@-cd $(prefascicle_dir) && $(PERFORM) $(TEST_CODING_RULES) *

################################################################################

# Update copyright notices.

# Be sure to update these hardcoded dates. This target might be made
# either in the last days of the old year or the first days of the
# new, so they can't be derived dynamically from the current date.
#
# For rate tables etc., see 'gwc/develop2.txt'.

old_year := 2021
new_year := 2022

backup_directory := ../saved_$(old_year)

unutterable := Copyright

# The first two commands in the recipe use 'mkdir' without '-p',
# deliberately: if they fail, something is amiss (probably the
# result of trying to update copyrights twice in the same year).

.PHONY: happy_new_year
happy_new_year: source_clean
	$(MKDIR) $(backup_directory)
	$(MKDIR) $(backup_directory)/hooks
	$(TOUCH) --date=$(old_year)0101 BOY
	for z in * hooks/*; \
	  do \
	       [ $$z -nt BOY ] \
	    && [ ! -d $$z ] \
	    && $(GREP) --quiet --files-with-matches $(unutterable) $$z \
	    && $(SED) \
	      --in-place='$(backup_directory)/*' \
	      -e'/$(unutterable)/s/$(old_year)/$(old_year), $(new_year)/' \
	      $$z \
	    || true; \
	  done;
	$(RM) --force BOY
	@$(ECHO) "Check these potential issues:"
	$(GREP) --directories=skip '$(old_year)[, ]*$(old_year)' * || true
	$(GREP) --directories=skip '$(new_year)[, ]*$(old_year)' * || true
	$(GREP) --directories=skip '$(new_year)[, ]*$(new_year)' * || true
	@$(ECHO) "...end first list of potential issues."
	[ -z '$(wildcard *.?pp)' ] || $(GREP) '$(old_year)' *.?pp \
	  | $(SED) \
	    -e '/$(old_year)[, ]*$(new_year)/d' \
	    -e'/https*:\/\/lists.nongnu.org\/archive\/html\/lmi\/$(old_year)/d' \
	    -e'/\(VERSION\|version\).*$(old_year)[0-9]\{4\}T[0-9]\{4\}Z/d' \
	  || true
	@$(ECHO) "...end second list of potential issues."
	$(GREP) --directories=skip $(unutterable) * \
	  | $(SED) \
	    -e '/$(unutterable).*$(new_year) Gregory W. Chicares/d' \
	    -e '/$(unutterable).*$(new_year) Vadim Zeitlin/d' \
	    -e '/unutterable := $(unutterable)/d' \
	    -e '/$(unutterable) (C) .(yyyy) Gregory W. Chicares/d' \
	    -e '/$(unutterable) \((C)\|&copy;\) 1989, 1991 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1997-2016 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1987, 1989 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1987, 1989, 1992 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) 1995, 1996 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) 1995, 1996, 2000 Free Software Foundation, Inc./d' \
	    -e '/$(unutterable) (C) <year>  <name of author>/d' \
	    -e '/Gnomovision version 69, $(unutterable) (C) year name of author/d' \
	    -e '/$(unutterable) (C) <var>yyyy<\/var>  *<var>name of author<\/var>/d' \
	    -e '/$(unutterable) (C) &lt;year&gt;  &lt;name of author&gt;/d' \
	    -e '/GNU cgicc $(unutterable) (C) 1996, 1997, 1998, 1999, 2000 Stephen F. Booth/d' \
	    -e '/$(unutterable) and license notices for graphics files/d' \
	    -e '/(C) $(unutterable) Beman Dawes 1995-2001. Permission to copy, use, modify, sell/d' \
	    -e '/(C) $(unutterable) Beman Dawes 2001. Permission to copy, use, modify, sell/d' \
	    -e '/(C) $(unutterable) Beman Dawes 2000. Permission to copy, use, modify, sell/d' \
	    -e '/$(unutterable) Kevlin Henney, 2000, 2001. All rights reserved./d' \
	    -e '/$(unutterable) Kevlin Henney, 2000-2003. All rights reserved./d' \
	    -e '/$(unutterable) Terje Sletteb/d' \
	    -e '/$(unutterable) (C) 1993 by Sun Microsystems, Inc./d' \
	    -e '/$(unutterable) (C) 1994$$/d' \
	    -e '/$(unutterable) (C) 1996-1998$$/d' \
	    -e '/:good_copyright=/d' \
	    -e '/:$(unutterable) (C)$$/d' \
	    -e '/$(unutterable).*[$$](date -u +.%Y.)/d' \
	    -e '/http:\/\/www.gnu.org\/prep\/maintain\/maintain.html#$(unutterable)-Notices/d' \
	    -e '/year appears on the same line as the word "$(unutterable)"/d' \
	    -e '/document.add_comment("$(unutterable) (C) " + y + " Gregory W. Chicares.");/d' \
	    -e '/oss << R*"$(unutterable) .*" << year;/d' \
	  || true
	@$(ECHO) "...end third list of potential issues."
	@$(ECHO) "Done."

################################################################################

# Routine nychthemeral test. Others might call this a 'nightly build'
# or 'daily smoke test'. What's important is not whether it's run in
# the daytime or in the nighttime, but rather that it's run something
# like once every twenty-four hours.

# TODO ?? This is an evolving experiment. Possible enhancements include:
#   - Add other tests, particularly system tests.
#       Consider using $(srcdir)/../products/src if it exists.
#       Also consider using the repository only, with testdecks stored
#         either there or in ftp.
#   - Test skeleton branch, too.
#   - Gather statistics, e.g., elapsed time and total size of binaries.
#   - Filter logs e.g. as 'fancy.make' does.
#   - Upload outcome (e.g. log and statistics) to savannah.
#       Can a simple binary measure {success, failure} be devised?
#   - Set this up as a 'cron' job (or equivalent on other platforms).
#   - Devise a simple way to test changes to this target without
#       requiring a large download and an hour of crunching.

nychthemeral_directory := /opt/lmi/lmi-nychthemeral-$(yyyymmddhhmm)
nychthemeral_log := log-lmi-nychthemeral-$(yyyymmddhhmm)

.PHONY: checkout
checkout:
	$(MKDIR) --parents $(nychthemeral_directory) \
	&& cd $(nychthemeral_directory) \
	&& git clone git://git.savannah.nongnu.org/lmi.git \

# SOMEDAY !! Add build types for gcov (-fprofile-arcs -ftest-coverage)
# and for gprof. See:
#   https://lists.nongnu.org/archive/html/lmi/2014-10/msg00115.html

.PHONY: test_various_build_types
test_various_build_types: source_clean
	-$(MAKE) test build_type=safestdlib
	-$(MAKE) all cgi_tests cli_tests build_type=so_test
	-$(MAKE) check_concinnity
	-$(MAKE) check_physical_closure
	-$(MAKE) all test

.PHONY: nychthemeral_test
nychthemeral_test: checkout
	-$(MAKE) --directory=$(nychthemeral_directory)/lmi test_various_build_types \
	  >../$(nychthemeral_log) 2>&1

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
