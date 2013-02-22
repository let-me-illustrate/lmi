# Installer for sample databases and miscellaneous libraries.
#
# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

# Configurable settings ########################################################

prefix    := /opt/lmi

cache_dir := /cache_for_lmi/downloads

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

boost_archive    := boost_1_38_0.tar.bz2
cgicc_archive    := cgicc-3.1.4.tar.bz2
fop_archive      := fop-0.20.5-bin.tar.gz
jing_archive     := jing-20091111.zip
md5sum_msw_exe   := md5sum.exe
sample_archive   := lmi-data-20050618T1440Z.tar.bz2
trang_archive    := trang-20091111.zip
xmlwrapp_archive := xmlwrapp-0.6.0.tar.gz

file_list := \
  $(boost_archive) \
  $(cgicc_archive) \
  $(fop_archive) \
  $(jing_archive) \
  $(md5sum_msw_exe) \
  $(sample_archive) \
  $(trang_archive) \
  $(xmlwrapp_archive) \

boost cgicc xmlwrapp: stem =               $(basename $(basename $($@_archive)))
fop:                  stem = $(subst -bin,,$(basename $(basename $($@_archive))))
jing trang:           stem =                          $(basename $($@_archive))
md5sum_msw:           stem = $(md5sum_msw_exe)
sample:               stem = data

# URLs and archive md5sums #####################################################

$(boost_archive)-url    := $(sf_mirror)/boost/$(boost_archive)
$(cgicc_archive)-url    := ftp://ftp.gnu.org/pub/gnu/cgicc/$(cgicc_archive)
$(fop_archive)-url      := http://archive.apache.org/dist/xmlgraphics/fop/binaries/$(fop_archive)
$(jing_archive)-url     := http://jing-trang.googlecode.com/files/$(jing_archive)
$(md5sum_msw_exe)-url   := http://etree.org/cgi-bin/counter.cgi/software/md5sum.exe#!md5!eb574b236133e60c989c6f472f07827b
$(sample_archive)-url   := http://download.savannah.gnu.org/releases/lmi/$(sample_archive)
$(trang_archive)-url    := http://jing-trang.googlecode.com/files/$(trang_archive)
$(xmlwrapp_archive)-url := $(sf_mirror)/xmlwrapp/$(xmlwrapp_archive)

$(boost_archive)-md5    := 5eca2116d39d61382b8f8235915cb267
$(cgicc_archive)-md5    := 6cb5153fc9fa64b4e50c7962aa557bbe
$(fop_archive)-md5      := d6b43e3eddf9378536ad8127bc057d41
$(jing_archive)-md5     := 13eef193921409a1636377d1efbf9843
$(md5sum_msw_exe)-md5   := eb574b236133e60c989c6f472f07827b
$(sample_archive)-md5   := e7f07133abfc3b9c2252dfa3b61191bc
$(trang_archive)-md5    := 9d31799b948c350850eb9dd14e5b832d
$(xmlwrapp_archive)-md5 := 331369a1b0e0539b1ce95a67e4c2bec4

# Utilities ####################################################################

CHMOD  := chmod
CP     := cp
DIFF   := diff
ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
PATCH  := patch
RM     := rm
SORT   := sort
TAR    := tar
TOUCH  := touch
UNZIP  := unzip
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
all: boost cgicc fop jing md5sum_msw sample trang xmlwrapp

# Patches were generated according to this advice:
#
# http://www.linuxfromscratch.org/patches/submit.html
# "When creating the patch, you should be in a directory just above
# the package directory so that the resulting patch can be applied
# with patch -p1"
#
# because they generally span multiple subdirectories. Use commands
# like these if they ever need to be regenerated:
#
#   cd /tmp
#   diff --recursive --unified=3 original modified >foo.patch
#   patch --strip=1 --dry-run --directory=/wherever/scratch </tmp/foo.patch

# For some targets,
#  - fresh md5sums are generated, then
#  - saved md5sums are checked, then
#  - the fresh and saved md5sums are compared against each other
#    (after sorting, because 'md5sum' output order is unspecified).
# This may seem redundant. The first step is not removed because it
# makes both sets of md5sums available for comparison in case they
# differ, and also because it guides regeneration of the saved md5sums
# in case a library is updated. The second step is not removed because
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
	cd $(destination) && $(MD5SUM) include/cgicc/* src/cgicc/* >$(stem).md5sums
	cd $(destination) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	$(SORT) --key=2 --output=$(stem).X                $(stem).md5sums
	$(SORT) --key=2 --output=$(stem).Y $(destination)/$(stem).md5sums
	$(DIFF) --unified $(stem).X $(stem).Y && $(RM) $(destination)/$(stem).md5sums $(stem).X $(stem).Y

# When the 'fop' tarball is extracted, this message:
#   tar: A lone zero block at 20398
# is expected, and harmless--see:
#   http://issues.apache.org/bugzilla/show_bug.cgi?id=28776

.PHONY: fop
fop: $(file_list)
	@$(MKDIR) $(destination)/$(stem)
	$(MV) scratch/$(stem)/* $(destination)/$(stem)

.PHONY: jing
jing: $(file_list)
	@$(MKDIR) --parents $(destination)/rng
	$(MV) scratch/$(stem)/bin/$@.jar         $(destination)/rng
	$(MV) scratch/$(stem)/bin/xercesImpl.jar $(destination)/rng

# The 'md5sum_msw' binary is required only by the msw-specific
# 'fardel' target. On other platforms, it can't be executed, but it
# could be used to create a cross 'fardel'.
#
# It is placed in lmi's 'third_party/bin/' subdirectory--imperatively
# not in lmi's 'local/bin/' subdirectory, which is added to $PATH.
#
# Should the given URL ever become invalid, see:
#   http://www.openoffice.org/dev_docs/using_md5sums.html#links
# to find another.

.PHONY: md5sum_msw
md5sum_msw: $(file_list)
	$(CP) --preserve $(cache_dir)/$(stem) $(third_party_bin_dir)

# The 'clobber' target doesn't remove $(prefix)/data because that
# directory might contain valuable user-customized files; hence, in
# this case, $(MKDIR) must be allowed to fail.

.PHONY: sample
sample: $(file_list)
	@-$(MKDIR) --parents $(prefix)/data
	$(MV) scratch/$(stem)/* $(prefix)/data

.PHONY: trang
trang: $(file_list)
	@$(MKDIR) --parents $(destination)/rng
	$(MV) scratch/$(stem)/$@.jar $(destination)/rng

.PHONY: xmlwrapp
xmlwrapp: $(file_list)
	-[ -e $(stem).patch ] && $(PATCH) --directory=scratch --strip=1 < $(stem).patch
	@$(MKDIR) $(third_party_include_dir)/xmlwrapp/
	$(MV) scratch/$(stem)/include/xmlwrapp/*.h $(third_party_include_dir)/xmlwrapp/
	@$(MKDIR) $(third_party_include_dir)/xsltwrapp/
	$(MV) scratch/$(stem)/include/xsltwrapp/*.h $(third_party_include_dir)/xsltwrapp/
	@$(MKDIR) $(third_party_source_dir)/libxml/
	$(MV) scratch/$(stem)/src/libxml/* $(third_party_source_dir)/libxml/
	@$(MKDIR) $(third_party_source_dir)/libxslt/
	$(MV) scratch/$(stem)/src/libxslt/* $(third_party_source_dir)/libxslt/
	cd $(destination) && $(MD5SUM) include/xmlwrapp/* include/xsltwrapp/* src/libxml/* src/libxslt/* >$(stem).md5sums
	cd $(destination) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	$(SORT) --key=2 --output=$(stem).X                $(stem).md5sums
	$(SORT) --key=2 --output=$(stem).Y $(destination)/$(stem).md5sums
	$(DIFF) --unified $(stem).X $(stem).Y && $(RM) $(destination)/$(stem).md5sums $(stem).X $(stem).Y

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
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=scratch --file=$(cache_dir)/$@

.PHONY: %.exe
%.exe:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	$(CHMOD) 750 $(cache_dir)/$@

.PHONY: %.zip
%.zip:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	-$(UNZIP) $(cache_dir)/$@ -d scratch

# Maintenance ##################################################################

# To test, make the 'clobber' and 'all' targets in that order.

.PHONY: clobber
clobber:
	@$(RM) --force --recursive scratch
	@$(RM) --force --recursive $(destination)

