# MinGW installation.
#
# This is public domain. It's not 'Copyright 2006', but including that
# quoted string keeps our cvs style-conformity script from complaining.

# http://savannah.nongnu.org/projects/lmi

# $Id: mingw_setup.make,v 1.12 2006-02-01 12:57:37 wboutin Exp $

# REVIEW: Here, say exactly what this makefile does, and what its
# prerequisites are; and write some excruciatingly clear instructions
# for running it.

# This makefile is self-contained, which is intended to make a MinGW
# community review easy. Some overlap now exists with the unified lmi
# makefile system, but that seems justified for the purpose of
# providing a MinGW installation for those not involved with lmi.

###############################################################################

# Configurable variables.

# These variables can be configured at the command-line. This example shows how
# MSYS users might benefit from configuring these variables:
#   make -f mingw_setup.make mingw_current system_root="/c/"
# Users who prefer multiple versions of MinGW would like this configuration:
#   make -f mingw_setup.make mingw_current mingw_dir="/Mingw-344"

src_dir := $(CURDIR)

system_root := c:

# Path to MinGW install directory.
mingw_dir := $(system_root)/MinGW

sf_mirror := http://easynews.dl.sourceforge.net/sourceforge

TMPDIR  ?= $(system_root)/tmp
$(TMPDIR):
	+@[ -d $@ ] || $(MKDIR) --parents $@

# Consider moving other configurable variables here, i.e., 'mingw_requirements'
# and 'mingw_extras'.

###############################################################################

# Standard utilities.

# REVIEW: First, in our own local toolset, let's copy 'wget.exe' to
# /usr/bin/ and, I guess, /gnu/ too. Then we don't need
# $(msys_utils_dir), right? At least we don't need it here, so let's
# remove it hence. Then this file would look for all utilities in
# /gnu/ ; let's use /usr/bin/ instead, because it's standard and
# /gnu/ is just plain weird. (The same applies to all lmi makefiles
# I'd guess.) [But see below for this one particular makefile only.]
#
# Then, consider how someone on the mailing list might use this.
# Would they have MSYS installed already? Probably--or else they
# could easily install it. That's the likeliest environment for
# them to have, so let's default to that: that means setting the
# root directory to '/c'. It also means that they already have almost
# all the utilities we require, already on $(PATH), so we can just
# assume that they're there--except for 'wget'...are there any other
# exceptions? (Testing should reveal any.)

# REVIEW: What happens if a required utility is unavailable?
# Would it make sense to test each, e.g.,
#   $(CP) --version >/dev/null 2>&1
# ? In light of the preceding paragraph, that should be done only for
# 'wget' and anything else that MSYS doesn't automatically provide.

BZIP2  := bzip2
CP     := cp
ECHO   := echo
GZIP   := gzip
MD5SUM := md5sum
MKDIR  := mkdir
MV     := mv
RM     := rm
TAR    := tar
WGET   := $(system_root)/msys/1.0/local/bin/wget

###############################################################################

# Install MinGW.

# The required packages contain some duplicate files, so the order of
# extraction is important. It follows these instructions:
#   http://groups.yahoo.com/group/mingw32/message/1145
# It is apparently fortuitous that the order is alphabetical.

# Minimal installation requirements.

mingw_requirements = \
  binutils-2.16.91-20050827-1.tar.gz \
  gcc-core-3.4.4-20050522-1.tar.gz \
  gcc-g++-3.4.4-20050522-1.tar.gz \
  mingw-runtime-3.8.tar.gz \
  w32api-3.3.tar.gz \

# These aren't necessary for compiling a C++ program with <windows.h>.
# Although, they're included here in an attempt to reproduce a C++-only
# version of MinGW-5.0.0 .

mingw_extras = \
  mingw32-make-3.80.0-3.tar.gz \
  mingw-utils-0.3.tar.gz \

# Download archives if they're out of date, then extract them.

%.tar.bz2:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(MD5SUM) --check $(src_dir)/$@.md5
	$(CP) --force --preserve $@ $(mingw_dir)
	$(BZIP2) --decompress --keep --force $@
	$(TAR) --extract --file=$*.tar

%.tar.gz:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(MD5SUM) --check $(src_dir)/$@.md5
	$(CP) --force --preserve $@ $(mingw_dir)
	$(GZIP) --decompress --force $@
	$(TAR) --extract --file=$*.tar

# This target must be kept synchronized with the latest packages
# available from http://www.mingw.org/download.shtml .
#
# REVIEW: "latest": does that mean "current" or "candidate"?

# REVIEW: How do 'mingw_current' and 'mingw_20050827' differ? Would
# it be better to have a series of targets like
#   mingw_[some later date]
#   mingw_20050827
#   mingw_[some earlier date]
#   mingw_[some even earlier date]
# and make 'mingw_current' an alias for the latest? Could the
# differences be factored out so that there are multiple dated
# versions of $(mingw_requirements) but only one target?

.PHONY: mingw_current
mingw_current:
	$(MKDIR) --parents $(TMPDIR)/$@
	-@[ -e $(mingw_dir) ]
	@$(ECHO) "Preserving your existing MinGW installation is\
	  strongly recommended."
	$(MKDIR) $(mingw_dir)
	$(MAKE) \
	  --directory=$(TMPDIR)/$@ \
	  --file=$(src_dir)/mingw_setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_current_from_tmp_dir

.PHONY: install_mingw_current_from_tmp_dir
install_mingw_current_from_tmp_dir: $(mingw_requirements) $(mingw_extras)
	-$(RM) --recursive *.tar
	$(MV) --force * $(mingw_dir)

.PHONY: mingw_20050827
mingw_20050827:
	$(MKDIR) --parents $(TMPDIR)/$@
	-@[ -e $(mingw_dir) ]
	@$(ECHO) "Preserving your existing MinGW installation is\
	strongly recommended."
	$(MKDIR) $(mingw_dir)
	$(MAKE) \
	  --directory=$(TMPDIR)/$@ \
	  --file=$(src_dir)/mingw_setup.make \
	    mingw_dir='$(mingw_dir)' \
	      src_dir='$(src_dir)' \
	  install_mingw_20050827_from_tmp_dir

.PHONY: install_mingw_20050827_from_tmp_dir
install_mingw_20050827_from_tmp_dir: $(mingw_requirements)
	-$(RM) --recursive *.tar
	$(MV) --force * $(mingw_dir)

###############################################################################

# Installation programs that require human interaction.

human_interactive_tools = \
  gdb-5.2.1-1.exe \
  MSYS-1.0.10.exe \

# REVIEW: In what directory would the downloaded file be saved?

%.exe:
	[ -e $@ ] || $(WGET) --non-verbose $(sf_mirror)/mingw/$@
	$(MD5SUM) --check $(src_dir)/$@.md5
	./$@

.PHONY: human_interactive_setup
human_interactive_setup:
	$(MAKE) \
	  --directory=$(TMPDIR) \
	  --file=$(src_dir)/mingw_setup.make \
	    src_dir='$(src_dir)' \
	  install_human_interactive_tools_from_tmp_dir

.PHONY: install_human_interactive_tools_from_tmp_dir
install_human_interactive_tools_from_tmp_dir: $(human_interactive_tools)

###############################################################################

# Upgrade wget-1.9.1-mingwPORT.tar.bz2 .

# This target uses wget to install wget. That may seem silly on the
# face of it, but it's actually useful for upgrading to a later
# version of wget.

wget_mingwport = wget-1.9.1

.PHONY: wget_mingwport
wget_mingwport:
	$(MAKE) \
	  --directory=$(TMPDIR) \
	  --file=$(src_dir)/mingw_setup.make \
	    src_dir='$(src_dir)' \
	  install_wget_mingwport_from_tmp_dir

.PHONY: install_wget_mingwport_from_tmp_dir
install_wget_mingwport_from_tmp_dir:
	[ -e $(wget_mingwport)-mingwPORT.tar.bz2 ] || $(WGET) --non-verbose \
	  $(sf_mirror)/mingw/$(wget_mingwport)-mingwPORT.tar.bz2
	$(MD5SUM) --check $(src_dir)/$(wget_mingwport)-mingwport.tar.bz2.md5
	$(BZIP2) --decompress --force --keep \
	  $(wget_mingwport)-mingwPORT.tar.bz2
	$(TAR) --extract --file $(wget_mingwport)-mingwPORT.tar
	$(CP) --preserve \
	  $(wget_mingwport)/mingwPORT/wget.exe /msys/1.0/local/bin/

# REVIEW: How have you tested this makefile? How do you know that it's
# correct? How could you tell that the installation has succeeded? Can
# things like this be tested automatically? How do you know that no
# existing file is downloaded unnecessarily?

