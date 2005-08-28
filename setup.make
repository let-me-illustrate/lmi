# Set up build environment.
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

# $Id: setup.make,v 1.4 2005-08-28 23:09:34 wboutin Exp $

.PHONY: all
all: setup

src_dir := $(CURDIR)

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

setup.make:: ;

# Configuration.

include $(src_dir)/configuration.make
$(src_dir)/configuration.make:: ;

# TODO ?? Use these if we build libraries.

# Automatic dependencies.

include $(src_dir)/autodependency.make
$(src_dir)/autodependency.make:: ;

# Objects.

include $(src_dir)/objects.make
$(src_dir)/objects.make:: ;

###############################################################################

# TODO ?? Move these definitions elsewhere.

third_party_bin_dir     := $(system_root)/opt/lmi/third-party/bin
third_party_include_dir := $(system_root)/opt/lmi/third-party/include
third_party_lib_dir     := $(system_root)/opt/lmi/third-party/lib

# Greg--using "source" seems inconsistent with using "src" everywhere
# else; it also causes compilation problems, so I opted to keep all
# names consistent.

third_party_src_dir  := $(system_root)/opt/lmi/third-party/src

.PHONY: setup
setup: \
  dummy_libraries \
  frozen_cgicc \
  frozen_xmlwrapp \
  frozen_boost \
  frozen_libxml2 \

.PHONY: $(third_party_bin_dir)
$(third_party_bin_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

.PHONY: $(third_party_lib_dir)
$(third_party_lib_dir):
	+@[ -d $@ ] || $(MKDIR) --parents $@

###############################################################################

# Create dummy libraries.

# At least with gnu tools, a zero-byte file may be linked with no
# diagnostic (and no effect, which is the intention).

# Create dummy libraries for boost and xmlwrapp: see discussion of
# 'third-party' libraries in other makefiles. For boost, derive
# library names from the platform-dependent linker option, because
# the names of boost libraries incorporate a platform suffix.

# Prevent license issues by providing dummy versions of problematic
# libraries that wx builds by default.
#
# TODO ?? Apparently libregex.a is no longer a problem since wx-2.5.0,
# and it's not clear why zlib was ever on this list. Consider removing
# those libraries from this list after carefully reauditing the
# licenses of all libraries wx uses.

dummy_library_names := \
  $(patsubst -l%,lib%.a,$(platform_boost_libraries)) \
  libjpeg.a \
  libpng.a \
  libregex.a \
  libtiff.a \
  libxmlwrapp.a \
  libzlib.a \

.PHONY: dummy_libraries
dummy_libraries: $(third_party_bin_dir) $(third_party_lib_dir)
	@$(foreach z, $(dummy_library_names), \
	  $(TOUCH) --date=20000101 $(third_party_lib_dir)/$(z); \
	  )

###############################################################################

# Install and patch cgicc-3.1.4 .

# TODO ?? Prefer to define $(WGET) elsewhere and use the definition here.

# TODO ?? Prefer to define $(TMPDIR) elsewhere and use the definition here.
# Can the definition in 'GNUmakefile' be shared?

.PHONY: frozen_cgicc
frozen_cgicc: $(third_party_bin_dir) $(third_party_lib_dir)
	$(MAKE) \
	-C /tmp \
	-f $(src_dir)/setup.make \
	                src_dir='$(src_dir)' \
        third_party_include_dir='$(third_party_include_dir)' \
            third_party_src_dir='$(third_party_srcdir)' \
	install_frozen_cgicc_from_tmp_dir

#	  --no-print-directory \

# TODO ?? Avoid using '/msys/1.0/bin/patch': instead, fix definition of $(PATCH).

# TODO ?? Make this target abend if it's not run in /tmp/ ?

.PHONY: install_frozen_cgicc_from_tmp_dir
install_frozen_cgicc_from_tmp_dir:
	[ -e cgicc-3.1.4.tar.bz2 ] || wget --non-verbose ftp://ftp.gnu.org/pub/gnu/cgicc/cgicc-3.1.4.tar.bz2
	$(ECHO) "6cb5153fc9fa64b4e50c7962aa557bbe  cgicc-3.1.4.tar.bz2" |$(MD5SUM) --check
	[ -e cgicc-3.1.4.tar ] || $(BZIP2) --decompress --keep cgicc-3.1.4.tar.bz2
	$(TAR) --extract --file=cgicc-3.1.4.tar
	$(PATCH) --strip=0 < $(src_dir)/cgicc-patch-gwc-20050217
	$(MKDIR) --parents $(third_party_include_dir)/cgicc/
	$(MKDIR) --parents $(third_party_src_dir)/cgicc/
	$(CP) --preserve cgicc-3.1.4/cgicc/*.h $(third_party_include_dir)/cgicc/
	$(CP) --preserve cgicc-3.1.4/cgicc/*.cpp $(third_party_src_dir)/cgicc/

###############################################################################

# Install and patch xmlwrapp-0.2.0

# TODO ?? Prefer to define $(GZIP) elsewhere and use the definition here.

.PHONY: frozen_xmlwrapp
frozen_xmlwrapp: $(third_party_include_dir) $(third_party_src_dir)
	$(MAKE) \
	-C /tmp \
	-f $(src_dir)/setup.make \
	                src_dir='$(src_dir)' \
	third_party_include_dir='$(third_party_include_dir)' \
	    third_party_src_dir='$(third_party_src_dir)' \
	install_frozen_xmlwrapp_from_tmp_dir

.PHONY: install_frozen_xmlwrapp_from_tmp_dir
install_frozen_xmlwrapp_from_tmp_dir:
# This command won't work because there's no longer a valid URL for the
# version currently used in production:
#	|| wget --non-verbose http://pmade.org/software/xmlwrapp/download/xmlwrapp-0.2.0.tar.gz

# The following assumes 'xmlwrapp-0.2.0.tar.gz' exists in '/tmp/' already.
	[ -e xmlwrapp-0.2.0.tar.gz ]
	$(ECHO) "f142e8bc349597ecbaebb4a8e246b65a  xmlwrapp-0.2.0.tar.gz" |$(MD5SUM) --check
	[ -e xmlwrapp-0.2.0.tar ] || gzip -d xmlwrapp-0.2.0.tar.gz
	$(TAR) --extract --verbose --file=xmlwrapp-0.2.0.tar
	$(PATCH) --strip=0 < $(src_dir)/xmlwrapp-patch-gwc-20050217
	$(MKDIR) --parents $(third_party_include_dir)/xmlwrapp/
	$(MKDIR) --parents $(third_party_src_dir)/libxml/
	$(CP) --preserve xmlwrapp-0.2.0/include/xmlwrapp/*.h $(third_party_include_dir)/xmlwrapp/
	$(CP) --preserve xmlwrapp-0.2.0/src/libxml/* $(third_party_src_dir)/libxml/

###############################################################################

# Install boost-1.31.0

.PHONY: frozen_boost
frozen_boost: $(third_party_include_dir) $(third_party_src_dir)
	$(MAKE) \
	-C /tmp \
	-f $(src_dir)/setup.make \
	                src_dir='$(src_dir)' \
	third_party_include_dir='$(third_party_include_dir)' \
	    third_party_src_dir='$(third_party_src_dir)' \
	install_frozen_boost_from_tmp_dir

.PHONY: install_frozen_boost_from_tmp_dir
install_frozen_boost_from_tmp_dir:
	[ -e boost_1_31_0.tar.bz2 ] || wget --non-verbose http://umn.dl.sourceforge.net/sourceforge/boost/boost_1_31_0.tar.bz2
	$(ECHO) "8cc183538eaa5cfc53d88d0e94bd2fd4  boost_1_31_0.tar.bz2" |$(MD5SUM) --check
	[ -e boost_1_31_0.tar ] || $(BZIP2) --decompress --keep boost_1_31_0.tar.bz2
	$(TAR) --extract --file=boost_1_31_0.tar
	$(MKDIR) --parents $(third_party_include_dir)/boost/
	$(MKDIR) --parents $(third_party_src_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_31_0/boost/* $(third_party_include_dir)/boost/
	-$(CP) --force --preserve --recursive boost_1_31_0/* $(third_party_src_dir)/boost/

###############################################################################
# This version has not been formally tested and released for production with
# lmi. The version actually used can't be easily rebuilt, so this target is
# intended to provide a stop-gap library until it can be formally tested.

# Install libxml2-2.6.19

.PHONY: frozen_libxml2
frozen_libxml2: \
	$(third_party_bin_dir) \
	$(third_party_include_dir) \
	$(third_party_src_dir)
	$(MAKE) \
	-C /tmp \
	-f $(src_dir)/setup.make \
	                src_dir='$(src_dir)' \
	    third_party_bin_dir='$(third_party_bin_dir)' \
	third_party_include_dir='$(third_party_include_dir)' \
	 third_party_src_dir='$(third_party_src_dir)' \
	install_frozen_libxml2_from_tmp_dir

.PHONY: install_frozen_libxml2_from_tmp_dir
install_frozen_libxml2_from_tmp_dir:
	[ -e libxml2-2.6.19.tar.bz2 ] || wget --non-verbose http://ftp.gnome.org/pub/GNOME/sources/libxml2/2.6/libxml2-2.6.19.tar.bz2
	$(ECHO) "ed581732d586f86324ec46e572526ede  libxml2-2.6.19.tar.bz2" |$(MD5SUM) --check
	[ -e libxml2-2.6.19.tar ] || $(BZIP2) --decompress --keep libxml2-2.6.19.tar.bz2
	$(TAR) --extract --file=libxml2-2.6.19.tar

# The following two commands don't do what's needed; I'm stumped here.
#	$(MAKE) -C libxml2-2.6.19 \
#        configure && make
	$(MKDIR) --parents $(third_party_include_dir)/libxml/
	-$(CP) --force --preserve --recursive libxml2-2.6.19/include/libxml/* $(third_party_include_dir)/libxml/ 2>/dev/null
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2-2.dll $(third_party_bin_dir)
	$(CP) --force --preserve libxml2-2.6.19/.libs/libxml2.dll.a $(third_party_lib_dir)

