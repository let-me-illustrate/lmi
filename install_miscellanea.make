# Installer for sample databases and miscellaneous libraries.
#
# Copyright (C) 2007 Gregory W. Chicares.
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

# $Id: install_miscellanea.make,v 1.7 2007-12-03 11:08:30 chicares Exp $

# Configurable settings ########################################################

prefix    := /opt/lmi

cache_dir := /tmp/lmi_cache

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := http://easynews.dl.sourceforge.net/sourceforge
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
sf_mirror := http://downloads.sourceforge.net

# Nonconfigurable settings #####################################################

destination := $(prefix)/third_party

third_party_bin_dir     := $(destination)/bin
third_party_include_dir := $(destination)/include
third_party_lib_dir     := $(destination)/lib
third_party_source_dir  := $(destination)/src

# File lists ###################################################################

boost_archive    := boost_1_33_1.tar.bz2
cgicc_archive    := cgicc-3.1.4.tar.bz2
fop_archive      := fop-0.20.5-bin.tar.gz
sample_archive   := lmi-data-20050618T1440Z.tar.bz2
xmlwrapp_archive := xmlwrapp-0.5.0.tar.gz

file_list := \
  $(boost_archive) \
  $(cgicc_archive) \
  $(fop_archive) \
  $(sample_archive) \
  $(xmlwrapp_archive) \

boost cgicc xmlwrapp: stem =               $(basename $(basename $($@_archive)))
fop:                  stem = $(subst -bin,,$(basename $(basename $($@_archive))))
sample:               stem = data

# URLs and archive md5sums #####################################################

$(boost_archive)-url    := $(sf_mirror)/boost/$(boost_archive)
$(cgicc_archive)-url    := ftp://ftp.gnu.org/pub/gnu/cgicc/$(cgicc_archive)
$(fop_archive)-url      := http://archive.apache.org/dist/xmlgraphics/fop/binaries/$(fop_archive)
$(sample_archive)-url   := http://download.savannah.gnu.org/releases/lmi/$(sample_archive)
$(xmlwrapp_archive)-url := ftp://ftp.freebsd.org/pub/FreeBSD/distfiles/$(xmlwrapp_archive)

$(boost_archive)-md5    := 2b999b2fb7798e1737d1fff8fac602ef
$(cgicc_archive)-md5    := 6cb5153fc9fa64b4e50c7962aa557bbe
$(fop_archive)-md5      := d6b43e3eddf9378536ad8127bc057d41
$(sample_archive)-md5   := e7f07133abfc3b9c2252dfa3b61191bc
$(xmlwrapp_archive)-md5 := b8a07e77f8f8af9ca96bccab7d9dd310

# Utilities ####################################################################

CP     := cp
DIFF   := diff
ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
PATCH  := patch
RM     := rm
TAR    := tar
TOUCH  := touch
WGET   := wget

# Error messages ###############################################################

destination_exists = \
  "\nError: Destination directory '$(destination)' already exists." \
  "\nIt is generally unsafe to install one version of a program" \
  "\non top of another. Probably you ought to rename the old" \
  "\nversion in order to preserve it; if not, then remove it." \
  "\n"

scratch_exists = \
  "\nError: Scratch directory 'scratch' already exists." \
  "\nProbably it is left over from a previous failure." \
  "\nJust remove it unless you're sure you want whatever files" \
  "\nit might contain." \
  "\n"

# Targets ######################################################################

.PHONY: all
all: boost cgicc fop sample xmlwrapp

# For some targets,
#  - saved md5sums are checked, then
#  - fresh md5sums are generated and compared against the saved ones.
# This may seem redundant. The second step is not removed because it
# makes both sets of md5sums available for comparison in case they
# differ, and also because it guides regeneration of the saved md5sums
# in case a library is updated. The first step is not removed because
# it is idiomatic, and its absence would be remarkable.

.PHONY: boost
boost: $(file_list)
	@$(MKDIR) $(third_party_include_dir)/boost/
	$(CP) --force --preserve --recursive scratch/$(stem)/boost/* $(third_party_include_dir)/boost/
	@$(MKDIR) $(third_party_source_dir)/boost/
	$(MV)                                scratch/$(stem)/*       $(third_party_source_dir)/boost/

.PHONY: cgicc
cgicc: $(file_list)
	$(PATCH) --directory=scratch --strip=1 < $(stem).patch
	@$(MKDIR) $(third_party_include_dir)/cgicc
	$(MV) scratch/$(stem)/cgicc/*.h   $(third_party_include_dir)/cgicc/
	@$(MKDIR) $(third_party_source_dir)/cgicc
	$(MV) scratch/$(stem)/cgicc/*.cpp $(third_party_source_dir)/cgicc/
	cd $(destination) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	cd $(destination) && $(MD5SUM) include/cgicc/* src/cgicc/* >$(stem).md5sums
	$(DIFF) $(stem).md5sums $(destination)/$(stem).md5sums && $(RM) $(destination)/$(stem).md5sums

# When the 'fop' tarball is extracted, this message:
#   tar: A lone zero block at 20398
# is expected, and harmless--see:
#   http://issues.apache.org/bugzilla/show_bug.cgi?id=28776

.PHONY: fop
fop: $(file_list)
	@$(MKDIR) $(destination)/$(stem)
	$(MV) scratch/$(stem)/* $(destination)/$(stem)

# The 'clobber' target doesn't remove $(prefix)/data because that
# directory might contain valuable user-customized files; hence, in
# this case, $(MKDIR) must be allowed to fail.

.PHONY: sample
sample: $(file_list)
	@-$(MKDIR) --parents $(prefix)/data
	$(MV) scratch/$(stem)/* $(prefix)/data

.PHONY: xmlwrapp
xmlwrapp: $(file_list)
	$(PATCH) --directory=scratch --strip=1 < $(stem).patch
	@$(MKDIR) $(third_party_include_dir)/xmlwrapp/
	$(MV) scratch/$(stem)/include/xmlwrapp/*.h $(third_party_include_dir)/xmlwrapp/
	@$(MKDIR) $(third_party_include_dir)/xsltwrapp/
	$(MV) scratch/$(stem)/include/xsltwrapp/*.h $(third_party_include_dir)/xsltwrapp/
	@$(MKDIR) $(third_party_source_dir)/libxml/
	$(MV) scratch/$(stem)/src/libxml/* $(third_party_source_dir)/libxml/
	@$(MKDIR) $(third_party_source_dir)/libxslt/
	$(MV) scratch/$(stem)/src/libxslt/* $(third_party_source_dir)/libxslt/
	$(TOUCH) $(destination)/src/xmlwrapp_config.h
	cd $(destination) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	cd $(destination) && $(MD5SUM) src/xmlwrapp_config.h include/xmlwrapp/* include/xsltwrapp/* src/libxml/* src/libxslt/* >$(stem).md5sums
	$(DIFF) $(stem).md5sums $(destination)/$(stem).md5sums && $(RM) $(destination)/$(stem).md5sums

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	@[ ! -e $(destination) ]   || { $(ECHO) -e $(destination_exists) && false; }
	@[ ! -e scratch        ]   || { $(ECHO) -e $(scratch_exists)     && false; }
	@$(MKDIR) --parents $(cache_dir)
	@$(MKDIR) --parents $(destination)
	@$(MKDIR) $(third_party_bin_dir)
	@$(MKDIR) $(third_party_include_dir)
	@$(MKDIR) $(third_party_lib_dir)
	@$(MKDIR) $(third_party_source_dir)
	@$(MKDIR) scratch

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

# New spelling '--no-verbose' has replaced original '--non-verbose':
#   http://sourceware.org/ml/cygwin-apps/2005-10/msg00140.html
# However, don't use
#   WGETFLAGS := '--no-verbose --timestamping'
# because, as this is written in 2007-08, sourceforge's mirror system
# is behaving in anomalous ways that '--no-verbose' would mask.

WGETFLAGS := '--timestamping'

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	cd $(cache_dir) && $(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=scratch --file=$(cache_dir)/$@

# Maintenance ##################################################################

# To test, make the 'clobber' and 'all' targets in that order.

.PHONY: clobber
clobber:
	@$(RM) --force --recursive scratch
	@$(RM) --force --recursive $(destination)

