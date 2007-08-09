# Installer for miscellaneous libraries.
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

# $Id: install_miscellanea.make,v 1.1 2007-08-09 15:13:39 chicares Exp $

# Configurable settings ########################################################

# If $(system_root) is empty, then '$(system_root)/foo' means simply
# '/foo', which is reasonable enough for msw; for a posix-emulation
# system like Cygwin, of course, that's not in the msw root directory.

#prefix := $(system_root)/opt/lmi/third_party
# TODO ?? testing
prefix := $(system_root)/trial/third_party

# In the past, it seemed necessary to specify a mirror, e.g.:
#  mirror := http://easynews.dl.sourceforge.net/sourceforge
# but as of about 2006-12 sf.net seems to select one automatically
# when this is passed to wget:
sf_mirror   := http://downloads.sourceforge.net

# Nonconfigurable settings #####################################################

ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
  this_makefile := $(lastword $(MAKEFILE_LIST))
else
  $(error Upgrade to make-3.81)
endif

tools_dir := $(dir $(this_makefile))/tools

third_party_bin_dir     := $(prefix)/bin
third_party_include_dir := $(prefix)/include
third_party_lib_dir     := $(prefix)/lib
third_party_source_dir  := $(prefix)/src

# File lists ###################################################################

# TODO ?? Testing...
# boost_archive := boost_1_33_1.tar.bz2

boost_archive    :=
cgicc_archive    := cgicc-3.1.4.tar.bz2
xmlwrapp_archive := xmlwrapp-0.5.0.tar.gz

file_list := \
  $(boost_archive) \
  $(cgicc_archive) \
  $(xmlwrapp_archive) \

# URLs and archive md5sums #####################################################

$(boost_archive)-url    := $(sf_mirror)/boost/$(boost_archive)
$(cgicc_archive)-url    := ftp://ftp.gnu.org/pub/gnu/cgicc/$(cgicc_archive)
$(xmlwrapp_archive)-url := ftp://ftp.freebsd.org/pub/FreeBSD/distfiles/$(xmlwrapp_archive)

$(boost_archive)-md5    := 2b999b2fb7798e1737d1fff8fac602ef
$(cgicc_archive)-md5    := 6cb5153fc9fa64b4e50c7962aa557bbe
$(xmlwrapp_archive)-md5 := b8a07e77f8f8af9ca96bccab7d9dd310

# Utilities ####################################################################

CP     := cp
ECHO   := echo
GREP   := grep
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
PATCH  := patch
RM     := rm
TAR    := tar
WGET   := wget

# Error messages ###############################################################

wget_missing = \
  "\nError: Unable to find '$(WGET)', which is required for" \
  "\nautomated downloads. Install it on your PATH." \
  "\n"

prefix_exists = \
  "\nError: Prefix directory '$(prefix)' already exists." \
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

# TODO ?? Don't repeat e.g. 'cgicc-3.1.4'.
# TODO ?? And rename the md5 files.

.PHONY: all
all: $(file_list)
#	@$(MV) scratch $(prefix)
# TODO ?? for cgicc:
	$(PATCH) --directory=scratch --strip=0 < $(tools_dir)/cgicc/cgicc_3_1_4_patch
	@$(MKDIR) $(third_party_include_dir)/cgicc
	$(MV) scratch/cgicc-3.1.4/cgicc/*.h   $(third_party_include_dir)/cgicc/
	@$(MKDIR) $(third_party_source_dir)/cgicc
	$(MV) scratch/cgicc-3.1.4/cgicc/*.cpp $(third_party_source_dir)/cgicc/
	cd $(prefix) && $(MD5SUM) --check $(abspath $(tools_dir)/cgicc/cgicc_md5sums)
# TODO ?? for boost:
#	@$(MKDIR) $(third_party_include_dir)/boost/
#	-$(CP) --force --preserve --recursive scratch/$(boost_dir)/boost/* $(third_party_include_dir)/boost/
#	@$(MKDIR) $(third_party_source_dir)/boost/
#	-$(CP) --force --preserve --recursive scratch/$(boost_dir)/*       $(third_party_source_dir)/boost/
# TODO ?? for xmlwrapp:
	$(MKDIR) $(third_party_include_dir)/xmlwrapp/
	$(MV) scratch/xmlwrapp-0.5.0/include/xmlwrapp/*.h $(third_party_include_dir)/xmlwrapp/
	$(MKDIR) $(third_party_include_dir)/xsltwrapp/
	$(MV) scratch/xmlwrapp-0.5.0/include/xsltwrapp/*.h $(third_party_include_dir)/xsltwrapp/
	$(MKDIR) $(third_party_source_dir)/libxml/
	$(MV) scratch/xmlwrapp-0.5.0/src/libxml/* $(third_party_source_dir)/libxml/
	$(MKDIR) $(third_party_source_dir)/libxslt/
	$(MV) scratch/xmlwrapp-0.5.0/src/libxslt/* $(third_party_source_dir)/libxslt/
# TODO ?? Add new file:
#	cd $(prefix) && $(MD5SUM) --check $(abspath $(tools_dir)/xmlwrapp/xmlwrapp_md5sums)
	cd $(prefix) && $(MD5SUM) --check $(abspath $(tools_dir)/xmlwrapp-0_5_0_md5)
# TODO ?? cd /opt/lmi/third_party/src/ ; touch xmlwrapp_config.h

$(file_list): initial_setup

.PHONY: initial_setup
initial_setup:
	@type "$(WGET)" >/dev/null || { $(ECHO) -e $(wget_missing)   && false; }
	@[ ! -e $(prefix) ]        || { $(ECHO) -e $(prefix_exists)  && false; }
	@[ ! -e scratch   ]        || { $(ECHO) -e $(scratch_exists) && false; }
	@$(MKDIR) --parents $(prefix)
	@$(RM) --force --recursive $(prefix)
	@$(MKDIR) --parents $(third_party_bin_dir)
	@$(MKDIR) --parents $(third_party_include_dir)
	@$(MKDIR) --parents $(third_party_lib_dir)
	@$(MKDIR) --parents $(third_party_source_dir)
	@$(MKDIR) --parents scratch

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
	[ -e $@ ] || $(WGET) $(WGETFLAGS) $($@-url)
	$(ECHO) "$($@-md5) *$@" | $(MD5SUM) --check
	-$(TAR) --extract $(TARFLAGS) --directory=scratch --file=$@

# TODO ?? Can we designate a cache directory to hold downloads?

# TODO ?? This safeguards against any older files interfering with new ones
# installed to the same directory.
#	$(RM) --force --recursive $(third_party_include_dir)/boost/

# Test #########################################################################

# TODO ?? Add a test for patching? Not really needed if every
# installed file is validated.

# TODO ?? I hesitate to write rm $(prefix)....

.PHONY: clean
clean:
	$(RM) --force --recursive scratch
	$(RM) --force --recursive $(system_root)/trial/third_party

# TODO ?? This isn't in cvs yet--inline it here?

# /cygdrive/c/opt/lmi/third_party[0]$md5sum include/xmlwrapp/* include/xsltwrapp/* src/libxml/* src/libxslt/* >xmlwrapp-0.5.0_md5

