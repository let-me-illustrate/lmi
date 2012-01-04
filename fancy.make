# Experimental makefile for fancy formatted reports.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

# This experimental makefile runs unit tests, filters out most routine
# output, and summarizes results. It's handy, but abstruse. Usually,
# makefiles simply emit the output of the commands they run, and using
# these techniques in the main makefiles might make them harder to
# understand, so they're factored out here, at least until experience
# suggests a better way to use them.

.PHONY: all
all: unit_tests_fancy

src_dir := $(CURDIR)

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

fancy.make:: ;

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

# Unit-test report.

# These special targets are used to print begin and end times for
# other subtargets. If these $(DATE) commands were inlined in such a
# subtarget, then 'make' would evaluate them all when preparing to run
# its commands, making the begin and end times identical.

volatile_time = $(shell $(DATE) -u +'%Y%m%dT%H%M%SZ')
print_time_flags = -f $(src_dir)/GNUmakefile --no-print-directory
print_begin_time = @$(MAKE) -f fancy.make $(print_time_flags) begin_time
print_end_time   = @$(MAKE) -f fancy.make $(print_time_flags) end_time

.PHONY: begin_time
begin_time:
	@$(ECHO) Begun at $(volatile_time)

.PHONY: end_time
end_time:
	@$(ECHO) Ended at $(volatile_time)

# TODO ?? WENDY Would you please provide a definition of $(SLEEP) for
# all platforms? For 'msw_generic.make', you can forward to the MSYS
# implementation this way:
#   C:/usr/bin[0]$cat sleep
#   #! /usr/bin/sh
#   /msys/1.0/bin/sleep "$@"
SLEEP = /usr/bin/sleep

# This target demonstrates the reason for $(print_end_time).
#
.PHONY: test_volatile_time
test_volatile_time:
	$(print_begin_time)
	@$(ECHO) ?Begun at $(volatile_time)
	# Do something that takes a while.
	@$(SLEEP) 10
	# This isn't the right answer, because $$(shell) is expanded when
	# the makefile is read, not when the commands are executed.
	@$(ECHO) ?Ended at $(shell $(DATE) -u +'%Y%m%dT%H%M%SZ')
	# This gives the right answer.
	$(print_end_time)

define REPORT_TIMING
	@$(GREP) Elapsed $(1) \
	  | $(SED) -e 's/Elapsed time//' -e 's/\(.*\):\(.*\)/\2   \1/' \
	  | $(SED) -e 's/^/  /'
endef

define REPORT_DIAGNOSTICS
	@$(SED) -f diagnostics.sed $(1) \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ diagnostics for all $(2) targets/'
endef

define REPORT_ERRORS
	@$(SED) -ferrors.sed $(1) \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ fatal errors for all $(2) targets/'
	@$(ECHO) Fatal errors by file:
	@-$(GREP) '\*\*\*' $(1) \
	  | $(SED) -f errors.sed
	@$(ECHO) Fatal errors end.
endef

unit_test_results = /tmp/lmi_unit_tests

.PHONY: unit_tests_fancy
unit_tests_fancy: #$(unit_test_targets)
	$(print_begin_time)
	@$(RM) --force $(unit_test_results)
	@$(MAKE) -f $(src_dir)/GNUmakefile $(unit_test_targets) > $(unit_test_results) 2>&1
	$(call REPORT_TIMING,$(unit_test_results))
	$(call REPORT_DIAGNOSTICS,$(unit_test_results),"unit test")
	$(call REPORT_ERRORS,$(unit_test_results),"unit test")
	@$(MAKE) -f $(src_dir)/GNUmakefile run_unit_tests >> $(unit_test_results) 2>&1
	@$(ECHO) $(words $(unit_test_targets)) unit tests attempted
	@$(GREP) -c 'no errors detected' $(unit_test_results) \
	  | $(SED) -e 's/$$/ unit tests passed/'
	$(print_end_time)

