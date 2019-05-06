# Installer for sample databases and miscellaneous libraries.
#
# Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

# Configurable settings ########################################################

prefix    := /opt/lmi

cache_dir := /cache_for_lmi/downloads

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := https://easynews.dl.sourceforge.net/sourceforge
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
sf_mirror := https://downloads.sourceforge.net

# Nonconfigurable settings #####################################################

dest_dir := $(prefix)/third_party

ad_hoc_dir := $(dest_dir)/ad_hoc

third_party_bin_dir     := $(dest_dir)/bin
third_party_include_dir := $(dest_dir)/include
third_party_lib_dir     := $(dest_dir)/lib
third_party_source_dir  := $(dest_dir)/src

# File lists ###################################################################

boost_archive    := boost_1_33_1.tar.bz2
cgicc_archive    := cgicc-3.1.4.tar.bz2
jing_archive     := jing-20091111.zip
md5sum_msw_exe   := md5sum.exe
sample_archive   := lmi-data-20050618T1440Z.tar.bz2
trang_archive    := trang-20091111.zip
xmlwrapp_archive := xmlwrapp-0.9.0.tar.gz

file_list := \
  $(boost_archive) \
  $(cgicc_archive) \
  $(jing_archive) \
  $(md5sum_msw_exe) \
  $(sample_archive) \
  $(trang_archive) \
  $(xmlwrapp_archive) \

boost cgicc xmlwrapp: stem = $(basename $(basename $($@_archive)))
jing trang:           stem =            $(basename $($@_archive))
md5sum_msw:           stem = $(md5sum_msw_exe)
sample:               stem = data

# URLs and archive md5sums #####################################################

$(boost_archive)-url    := $(sf_mirror)/boost/$(boost_archive)
$(cgicc_archive)-url    := ftp://ftp.gnu.org/pub/gnu/cgicc/$(cgicc_archive)
$(jing_archive)-url     := https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/jing-trang/$(jing_archive)
$(md5sum_msw_exe)-url   := https://github.com/vadz/lmi/releases/download/new-cygwin-makefiles/md5sum.exe
$(sample_archive)-url   := https://download.savannah.gnu.org/releases/lmi/$(sample_archive)
$(trang_archive)-url    := https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/jing-trang/$(trang_archive)
$(xmlwrapp_archive)-url := https://github.com/vslavik/xmlwrapp/releases/download/v0.9.0/$(xmlwrapp_archive)

$(boost_archive)-md5    := 2b999b2fb7798e1737d1fff8fac602ef
$(cgicc_archive)-md5    := 6cb5153fc9fa64b4e50c7962aa557bbe
$(jing_archive)-md5     := 13eef193921409a1636377d1efbf9843
$(md5sum_msw_exe)-md5   := eb574b236133e60c989c6f472f07827b
$(sample_archive)-md5   := e7f07133abfc3b9c2252dfa3b61191bc
$(trang_archive)-md5    := 9d31799b948c350850eb9dd14e5b832d
$(xmlwrapp_archive)-md5 := 5e8ac678ab03b7c60ce61ac5424e0849

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

dest_dir_exists = \
  "\nError: Destination directory '$(dest_dir)' already exists." \
  "\nIt is generally unsafe to install one version of a program" \
  "\non top of another. Probably you ought to rename the old" \
  "\nversion in order to preserve it; if not, then remove it." \
  "\n"

ad_hoc_dir_exists = \
  "\nError: Nonce directory '$(ad_hoc_dir)' already exists." \
  "\nProbably it is left over from a previous failure." \
  "\nJust remove it unless you're sure you want whatever files" \
  "\nit might contain." \
  "\n"

# Targets ######################################################################

.PHONY: all
all: boost cgicc jing md5sum_msw sample trang xmlwrapp

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
#   patch --strip=1 --dry-run --directory=/any/where </tmp/foo.patch
#
# For example, to update the cgicc patch with additional changes:
#   pushd /tmp
#   mkdir original
#   mkdir modified
#   cd /tmp/original && tar -xjvf /cache_for_lmi/downloads/cgicc-3.1.4.tar.bz2
#   cd /tmp/modified && tar -xjvf /cache_for_lmi/downloads/cgicc-3.1.4.tar.bz2
# Patch first with '--dry-run' to be sure, then for real:
#   patch --strip=1 --dry-run </opt/lmi/src/lmi/cgicc-3.1.4.patch
#   patch --strip=1 </opt/lmi/src/lmi/cgicc-3.1.4.patch
# Modify files in the "modified" directory, then:
#   cd /tmp
#   diff --recursive --unified=3 original modified >cgicc-3.1.4.patch
#   patch --strip=1 --dry-run --directory=original </tmp/cgicc-3.1.4.patch

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
	-[ -e $(stem).patch ] && $(PATCH) --directory=$(ad_hoc_dir) --strip=1 < $(stem).patch
	$(MKDIR) $(third_party_include_dir)/boost/
	$(CP) --force --preserve --recursive $(ad_hoc_dir)/$(stem)/boost/* $(third_party_include_dir)/boost/
	$(MKDIR) $(third_party_source_dir)/boost/
	$(MV)                                $(ad_hoc_dir)/$(stem)/*       $(third_party_source_dir)/boost/

.PHONY: cgicc
cgicc: $(file_list)
	$(PATCH) --directory=$(ad_hoc_dir) --strip=1 < $(stem).patch
	$(MKDIR) $(third_party_include_dir)/cgicc
	$(MV) $(ad_hoc_dir)/$(stem)/cgicc/*.h   $(third_party_include_dir)/cgicc/
	$(MKDIR) $(third_party_source_dir)/cgicc
	$(MV) $(ad_hoc_dir)/$(stem)/cgicc/*.cpp $(third_party_source_dir)/cgicc/
	cd $(dest_dir) && $(MD5SUM) --binary include/cgicc/* src/cgicc/* >$(stem).md5sums
	cd $(dest_dir) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	$(SORT) --key=2 --output=$(stem).X             $(stem).md5sums
	$(SORT) --key=2 --output=$(stem).Y $(dest_dir)/$(stem).md5sums
	$(DIFF) --unified $(stem).X $(stem).Y && $(RM) $(dest_dir)/$(stem).md5sums $(stem).X $(stem).Y

.PHONY: jing
jing: $(file_list)
	$(MKDIR) --parents $(dest_dir)/rng
	$(MV) $(ad_hoc_dir)/$(stem)/bin/$@.jar         $(dest_dir)/rng
	$(MV) $(ad_hoc_dir)/$(stem)/bin/xercesImpl.jar $(dest_dir)/rng

# The 'md5sum_msw' binary is redistributed to msw end users for
# authentication, so the 'fardel' target requires it. On other
# platforms, it cannot be executed directly, but it is needed for
# creating a cross 'fardel' and for running cross unit tests.
#
# It is placed in lmi's 'third_party/bin/' subdirectory--imperatively
# not in lmi's 'local/bin/' subdirectory, which is added to $PATH.
# For cygwin builds, the expressly downloaded 'md5sum.exe' is kept off
# $PATH to prevent it from shadowing cygwin's own version. However,
# for cross builds, it cannot shadow the native 'md5sum', yet some
# cross-built unit tests require an msw binary, so add its directory
# to $WINEPATH to make those tests work (incidentally, 'wine' doesn't
# find it if it's simply symlinked).
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
	-$(MKDIR) --parents $(prefix)/data
	$(MV) $(ad_hoc_dir)/$(stem)/* $(prefix)/data

.PHONY: trang
trang: $(file_list)
	$(MKDIR) --parents $(dest_dir)/rng
	$(MV) $(ad_hoc_dir)/$(stem)/$@.jar $(dest_dir)/rng

.PHONY: xmlwrapp
xmlwrapp: $(file_list)
	-[ -e $(stem).patch ] && $(PATCH) --directory=$(ad_hoc_dir) --strip=1 < $(stem).patch
	$(MKDIR) $(third_party_include_dir)/xmlwrapp/
	$(MV) $(ad_hoc_dir)/$(stem)/include/xmlwrapp/*.h $(third_party_include_dir)/xmlwrapp/
	$(MKDIR) $(third_party_include_dir)/xsltwrapp/
	$(MV) $(ad_hoc_dir)/$(stem)/include/xsltwrapp/*.h $(third_party_include_dir)/xsltwrapp/
	$(MKDIR) $(third_party_source_dir)/libxml/
	$(MV) $(ad_hoc_dir)/$(stem)/src/libxml/* $(third_party_source_dir)/libxml/
	$(MKDIR) $(third_party_source_dir)/libxslt/
	$(MV) $(ad_hoc_dir)/$(stem)/src/libxslt/* $(third_party_source_dir)/libxslt/
	cd $(dest_dir) && $(MD5SUM) --binary include/xmlwrapp/* include/xsltwrapp/* src/libxml/* src/libxslt/* >$(stem).md5sums
	cd $(dest_dir) && $(MD5SUM) --check $(CURDIR)/$(stem).md5sums
	$(SORT) --key=2 --output=$(stem).X             $(stem).md5sums
	$(SORT) --key=2 --output=$(stem).Y $(dest_dir)/$(stem).md5sums
	$(DIFF) --unified $(stem).X $(stem).Y && $(RM) $(dest_dir)/$(stem).md5sums $(stem).X $(stem).Y

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	@[ ! -e $(dest_dir)   ] || { printf '%b' $(dest_dir_exists)   && false; }
	@[ ! -e $(ad_hoc_dir) ] || { printf '%b' $(ad_hoc_dir_exists) && false; }
	$(MKDIR) --parents $(cache_dir)
	$(MKDIR) --parents $(dest_dir)
	$(MKDIR) --parents $(ad_hoc_dir)
	$(MKDIR) $(third_party_bin_dir)
	$(MKDIR) $(third_party_include_dir)
	$(MKDIR) $(third_party_lib_dir)
	$(MKDIR) $(third_party_source_dir)

TARFLAGS := --keep-old-files
%.tar.bz2: TARFLAGS += --bzip2
%.tar.gz:  TARFLAGS += --gzip

UNZIPFLAGS := -q

WGETFLAGS := --no-check-certificate --no-verbose

.PHONY: %.tar.bz2 %.tar.gz
%.tar.bz2 %.tar.gz:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=$(ad_hoc_dir) --file=$(cache_dir)/$@

.PHONY: %.exe
%.exe:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	$(CHMOD) 750 $(cache_dir)/$@

.PHONY: %.zip
%.zip:
	cd $(cache_dir) && [ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$(cache_dir)/$@" | $(MD5SUM) --check
	-$(UNZIP) $(UNZIPFLAGS) $(cache_dir)/$@ -d $(ad_hoc_dir)

# Maintenance ##################################################################

# To test, make the 'clobber' and 'all' targets in that order.

.PHONY: clobber
clobber:
	$(RM) --force --recursive $(dest_dir)
	$(RM) --force --recursive $(ad_hoc_dir)
