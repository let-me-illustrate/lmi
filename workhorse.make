# Main lmi makefile, invoked by 'GNUmakefile'.
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

# This is not a top-level makefile.

ifeq (0,$(MAKELEVEL))
  $(error Don't use this as a top-level makefile)
endif

################################################################################

# Directives and such.

# The GNU make manual says:
#   "This is almost always what you want make to do,
#   but it is not historical practice; so for
#   compatibility, you must explicitly request it."

.DELETE_ON_ERROR:

################################################################################

# Default target.

# The default target is the first target by definition. But the
# dependencies desired here in turn depend on $(EXEEXT), which is
# defined in a configuration makefile that's included below, and that
# makefile has a target to prevent attempting to build it. To resolve
# that circularity, this target depends on another target that's
# defined after other makefiles are included.

.PHONY: all
all: effective_default_target

################################################################################

# Other makefiles included; makefiles not to be remade.

# Don't remake this file.

$(src_dir)/workhorse.make:: ;

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

# Effective default target (described above under "Default target").

default_targets := \
  antediluvian_cgi$(EXEEXT) \
  antediluvian_cli$(EXEEXT) \
  libantediluvian$(SHREXT) \
  liblmi$(SHREXT) \
  lmi_cli_shared$(EXEEXT) \
  lmi_wx_shared$(EXEEXT) \
  wx_new$(SHREXT) \

# The product_files target doesn't build with shared-library
# 'attributes'. That matters little because that target is deprecated.
#
# TODO ?? The gpt server, however, is important; it needs work anyway.
# The other binaries should be reconsidered. The antediluvian $(EXEEXT)
# targets wouldn't build with $(USE_SO_ATTRIBUTES) because of known
# problems with 'calculate.hpp', which however has been expunged, so
# perhaps they can now be removed from this list.

ifeq (,$(USE_SO_ATTRIBUTES))
  default_targets += \
    bcc_ar$(EXEEXT) \
    bcc_cc$(EXEEXT) \
    bcc_ld$(EXEEXT) \
    bcc_rc$(EXEEXT) \
    elapsed_time$(EXEEXT) \
    generate_passkey$(EXEEXT) \
    gpt_server$(EXEEXT) \
    gpt_so_test$(EXEEXT) \
    ihs_crc_comp$(EXEEXT) \
    product_files$(EXEEXT) \

endif

.PHONY: effective_default_target
effective_default_target: $(default_targets)

################################################################################

ifeq (gcc,$(toolset))
  gcc_version := $(shell $(CXX) -dumpversion)
endif

# These are defined even for toolsets other than gcc.

gnu_cpp_version := $(shell $(GNU_CPP) -dumpversion)
gnu_cxx_version := $(shell $(GNU_CXX) -dumpversion)

ifeq      (3.4.4,$(gnu_cpp_version))
else ifeq (3.4.5,$(gnu_cpp_version))
else
  $(error Untested $(GNU_CPP) version '$(gnu_cpp_version)')
endif

ifeq      (3.4.4,$(gnu_cxx_version))
else ifeq (3.4.5,$(gnu_cxx_version))
else
  $(error Untested $(GNU_CXX) version '$(gnu_cxx_version)')
endif

################################################################################

# wx settings.

wx_dir := /opt/lmi/local/bin

# Use our '-portable' script if it exists; else fall back on the
# script wx provides.

wx_config_script := \
  $(firstword \
    $(wildcard \
      $(addprefix $(wx_dir)/,wx-config-portable wx-config \
      ) \
    ) \
  )

# The conventional autotools usage...
ifeq (gcc,$(toolset))
  wx_config_cxxflags := $(shell $(wx_config_script) --cxxflags)
  wx_config_libs     := $(shell $(wx_config_script) --libs)
endif
# ...combines options that we prefer to keep separate.

wx_include_paths := \
  $(shell \
    $(ECHO) $(wx_config_cxxflags) \
    | $(SED) \
      -e 's/^/ /' \
      -e 's/ -[^I][^ ]*//g' \
      -e 's/ -I/ /g' \
  )

wx_predefinitions := \
  $(shell \
    $(ECHO) $(wx_config_cxxflags) \
    | $(SED) \
      -e 's/^/ /' \
      -e 's/ -[^DU][^ ]*//g' \
  )

wx_library_paths := \
  $(shell \
    $(ECHO) $(wx_config_libs) \
    | $(SED) \
      -e 's/^/ /' \
      -e 's/ -[^L][^ ]*//g' \
      -e 's/ -L/ -L /g' \
  )

wx_libraries := \
  $(shell \
    $(ECHO) $(wx_config_libs) \
    | $(SED) \
      -e 's/^/ /' \
      -e 's/ -[^l][^ ]*//g' \
  )

platform_wx_libraries := $(wx_library_paths) $(wx_libraries)

# Target 'wx_config_check', and the variables that it alone uses,
# are experimental and may disappear in a future release.

wx_cxxflag_check := $(wx_include_paths) $(wx_predefinitions)
wx_cxxflag_check := $(subst -I ,-I,$(wx_cxxflag_check))
wx_cxxflag_check := $(subst -D ,-D,$(wx_cxxflag_check))
wx_cxxflag_check := $(subst -U ,-U,$(wx_cxxflag_check))
wx_libs_check    := $(wx_library_paths) $(wx_libraries)
wx_libs_check    := $(subst -L ,-L,$(wx_libs_check))
wx_libs_check    := $(subst -l ,-l,$(wx_libs_check))

.PHONY: wx_config_check
wx_config_check:
	@$(ECHO) wx_dir is $(wx_dir)
	@$(ECHO) wx_config_script is $(wx_config_script)
	@$(ECHO) Omitted from 'wx-config --cxxflags':
	@$(ECHO) $(filter-out $(wx_cxxflag_check),$(wx_config_cxxflags))
	@$(ECHO) Omitted from 'wx-config --libs':
	@$(ECHO) $(filter-out $(wx_libs_check),$(wx_config_libs))
	@$(ECHO) Result of 'wx-config --cxxflags':
	@$(ECHO) "  " $(wx_config_cxxflags)
	@$(ECHO) Result of 'wx-config --libs':
	@$(ECHO) "  " $(wx_config_libs)
	@$(ECHO) wx_include_paths is $(wx_include_paths)
	@$(ECHO) wx_predefinitions is $(wx_predefinitions)
	@$(ECHO) wx_library_paths is $(wx_library_paths)
	@$(ECHO) wx_libraries is $(wx_libraries)

################################################################################

# Location of include files and prerequisites.

# Prefer the 'vpath' directive to the $(VPATH) variable because
#  - it permits finer control; and
#  - $(VPATH) requires a platform-dependent path separator, which
#    makes it harder to write a cross-platform makefile.

# Directories set in $(overriding_include_directories) are searched
# before any others except the primary source directory. There seems
# to be no conventional name for such a variable: automake has
# deprecated $(INCLUDES) and recommends using $(CPPFLAGS) or a
# prefixed variant, but $(CPPFLAGS) can't do the right thing for
#   -D overrides, which must come at the end of a command, and
#   -I overrides, which must come at the beginning
# simultaneously, so distinct variables are necessary.

# Treat certain external libraries as collections of source files to
# be compiled and linked explicitly here, instead of building them
# separately and linking them as normal libraries. Rationale:
#
# boost: the build system provided is outlandish.
#
# cgicc: './configure && make' fails in the MSYS environment.
#
# xmlwrapp: the autotoolized build system doesn't support Comeau C++
# (or any other compiler but gcc) on msw.
#
# For msw at least, these libraries are somewhat arbitrarily placed in
#   /opt/lmi/third_party/
# while properly autotoolized libraries are installed in
#   /opt/lmi/local/
# ; see:
#   http://lists.gnu.org/archive/html/lmi/2006-10/msg00046.html
# for some discussion.

# A default installation places gnome xml-library headers here:
#  libxml2:  /usr/local/include/libxml2/libxml
#  libxslt:  /usr/local/include/libxslt
# Both those libraries provide '*-config' scripts that don't respect
# an overriding $(prefix): they apparently hardcode the paths above,
# so there's no point in calling them.

all_include_directories := \
  . \
  $(src_dir) \
  $(src_dir)/tools/pete-2.1.1 \
  $(overriding_include_directories) \
  $(compiler_include_directory) \
  $(wx_include_paths) \
  /opt/lmi/third_party/include \
  /opt/lmi/third_party/src \
  /opt/lmi/local/include \
  /opt/lmi/local/include/libxml2 \

all_source_directories := \
  $(src_dir) \
  /opt/lmi/third_party/src/boost/libs/filesystem/src \
  /opt/lmi/third_party/src/boost/libs/regex/src \
  /opt/lmi/third_party/src/boost/libs/system/src \
  /opt/lmi/third_party/src/cgicc \

vpath lib%.a          $(CURDIR)
vpath %.o             $(CURDIR)

# Files whose names match 'my_%.cpp' are taken as product data files,
# which are overridden by any customized files found in a special
# directory.

vpath my_%.cpp        $(src_dir)/../products/src

vpath %.c             $(all_source_directories)
vpath %.cpp           $(all_source_directories)
vpath %.h             $(all_source_directories)
vpath %.hpp           $(all_source_directories)
vpath %.tpp           $(all_source_directories)
vpath %.xpp           $(all_source_directories)

vpath %.rc            $(all_source_directories)
vpath %.ico           $(src_dir)

vpath quoted_gpl      $(src_dir)
vpath quoted_gpl_html $(src_dir)

################################################################################

# Headers and template-instantiation files to test for physical closure.

# Only files in the source directory are tested. Files that reside
# elsewhere (e.g., headers accompanying libraries) are not tested.

# Exclude headers named 'config_*.hpp': they are designed to signal
# errors if they are used separately.

physical_closure_files := \
  $(addsuffix .physical_closure,\
    $(filter-out config_%.hpp,\
      $(notdir \
        $(wildcard \
          $(addprefix $(src_dir)/,*.h *.hpp *.tpp *.xpp \
          ) \
        ) \
      ) \
    ) \
  )

################################################################################

# Files that depend on wx, which can't use the strictest gcc warnings.

# Files are deemed to depend on wx iff they contain 'include *<wx/'.
# This heuristic isn't foolproof because wx headers might be included
# indirectly. Include an innocuous header like <wx/version.h> in files
# for which it fails.

wx_dependent_objects := \
  $(addsuffix .o,\
    $(basename \
      $(notdir \
        $(shell $(GREP) \
          --files-with-matches \
          'include *<wx/' \
          $(src_dir)/*.?pp \
        ) \
      ) \
    ) \
  )

wx_dependent_physical_closure_files := \
  $(addsuffix .physical_closure,\
    $(notdir \
      $(shell $(GREP) \
        --files-with-matches \
        'include *<wx/' \
        $(wildcard \
          $(addprefix $(src_dir)/,*.h *.hpp *.tpp *.xpp \
          ) \
        ) \
      ) \
    ) \
  )

################################################################################

# Warning options for gcc.

c_standard   := -std=c99
cxx_standard := -std=c++98

# Specify $(gcc_version_specific_warnings) last, in order to override
# other options.

ifeq (3.4.4,$(gcc_version))
  # Suppress spurious gcc-3.4.4 warnings:
  #   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=22207
  gcc_version_specific_warnings := -Wno-uninitialized
else ifeq (3.4.5,$(gcc_version))
  # Suppress spurious gcc-3.4.5 warnings:
  #   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=22207
  gcc_version_specific_warnings := -Wno-uninitialized
  # Fix "hello world":
  #   http://sourceforge.net/tracker/index.php?func=detail&aid=2373234&group_id=2435&atid=102435
  cxx_standard := -std=gnu++98
  # Use a correct snprintf() implementation:
  #   http://article.gmane.org/gmane.comp.gnu.mingw.user/27539
  cxx_standard += -posix
endif

treat_warnings_as_errors := -pedantic-errors -Werror

gcc_common_warnings := \
  $(treat_warnings_as_errors) \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wimport \
  -Wmultichar \
  -Wpacked \
  -Wpointer-arith \
  -Wsign-compare \
  -Wundef \
  -Wwrite-strings \

# Some boost libraries treat 'long long' as part of the language,
# which it probably soon will be, so permit it now.

gcc_common_warnings += -Wno-long-long

gcc_c_warnings := \
  $(c_standard) \
  $(gcc_common_warnings) \
  -Wmissing-prototypes \

gcc_cxx_warnings := \
  $(cxx_standard) \
  $(gcc_common_warnings) \
  -Wctor-dtor-privacy \
  -Wdeprecated \
  -Wnon-template-friend \
  -Woverloaded-virtual \
  -Wpmf-conversions \
  -Winvalid-pch \
  -Wsynth \

# Too many warnings on correct code, e.g. exact comparison to zero:
#  -Wfloat-equal \

# WX !! The wx library triggers many warnings with the following
# 'extra' flags. (Use '-W' for backward compatibility, instead of the
# modern equivalent '-Wextra'.)

gcc_common_extra_warnings := \
  -W \
  -Wcast-qual \
  -Wredundant-decls \

ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
  ifeq (3.4.5,$(gcc_version))
    expression_template_0_test.o: gcc_common_extra_warnings += -Wno-unused-parameter
  endif
endif

$(wx_dependent_objects):                gcc_common_extra_warnings :=
$(wx_dependent_physical_closure_files): gcc_common_extra_warnings :=

# Boost didn't remove an unused parameter in this file:

operations_posix_windows.o: gcc_common_extra_warnings += -Wno-unused-parameter

# The boost regex library improperly defines "NOMINMAX":
#   http://lists.boost.org/Archives/boost/2006/03/102189.php
# at least in version 1.33.1, and there seems to be no easy workaround
# except to blow away all warning options and let a warning appear.
# This library also seems to require
#   -Wl,--allow-multiple-definition
# for reasons unknown, at least with MinGW gcc-3.4.5 .

static_mutex.o: gcc_common_extra_warnings :=
static_mutex.o:          gcc_cxx_warnings :=

# Boost normally makes '-Wundef' give spurious warnings:
#   http://aspn.activestate.com/ASPN/Mail/Message/boost/1822550
# but defining BOOST_STRICT_CONFIG:
#   http://www.boost.org/libs/config/config.htm#user_settable
# makes '-Wundef' usable, because boost-1.31.0 doesn't seem to need
# any workarounds for gcc-3.3+ . However, it gives a number of
# warnings with wx-2.5.4 (that have been fixed in a later version).

# Too many warnings for various boost libraries:
#  -Wold-style-cast \
#  -Wshadow \

# Too many warnings for libstdc++:
#  -Wunreachable-code \

# Since at least gcc-3.4.2, -Wmissing-prototypes is deprecated as
# being redundant for C++.

C_WARNINGS = \
  $(gcc_c_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_warnings) \

CXX_WARNINGS = \
  $(gcc_cxx_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_warnings) \

################################################################################

# Build type governs
#  - optimization flags
#  - mpatrol
#  - gprof
#  - libstdc++ debugging and concept-checking macros

# libstdc++ debugging and concept-checking macros are given in the
# intended '_GLIBCXX' forms first, and also in the '_GLIBCPP' forms
# that apparently were used for gcc-3.3.x .
#
# '-D_GLIBXX_DEBUG_PEDANTIC' is required for gcc-3.4.x and gcc-4.0.0;
# this known defect was later corrected.

every_libstdcxx_warning_macro := \
  -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_CONCEPT_CHECKS \
  -D_GLIBCPP_DEBUG -D_GLIBCPP_DEBUG_PEDANTIC -D_GLIBCPP_CONCEPT_CHECKS \
                   -D_GLIBXX_DEBUG_PEDANTIC \

MPATROL_LDFLAGS :=
MPATROL_LIBS    :=

test_targets := unit_tests cgi_tests cli_tests

ifeq (mpatrol,$(findstring mpatrol,$(build_type)))
  ifeq (3.4.4,$(gcc_version))
    MPATROL_LDFLAGS := -Wl,--allow-multiple-definition
  endif
  optimization_flag := -O0
  MPATROL_LIBS := -lmpatrol -lbfd -liberty $(platform_mpatrol_libraries)
else
  ifeq (gprof,$(findstring gprof,$(build_type)))
    optimization_flag := -O0
    gprof_flag := -pg
  else
    ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
      optimization_flag := -O0
      libstdcxx_warning_macros := $(every_libstdcxx_warning_macro)
    else
      optimization_flag := -O2
    endif
  endif
endif

# An overriding version of 'my_prod.cpp', which is used to create a
# nondistributable binary, contains so many large strings that, after
# consuming more than one CPU minute and 1 MiB of RAM, MinGW gcc-3.4.5
# produces a diagnostic such as
#   warning: NULL pointer checks disabled:
#   39933 basic blocks and 167330 registers
# Adding '-fno-delete-null-pointer-checks' to $(CPPFLAGS) might
# suffice to suppress the diagnostic, but this file actually doesn't
# need any optimization at all.

my_prod.o: optimization_flag := -O0

################################################################################

# Libraries and associated options.
#
# The link command promiscuously mentions libxml2 for all targets.
# Measurements show that this costs one-tenth of a second on
# reasonable hardware, and it saves the trouble of maintaining a list
# of which targets require which libraries.
#
# TODO ?? Consider refining it anyway, because it's unclean: libxml2
# isn't actually required for all targets.
#
# Rationale for freezing particular versions of third-party libraries:
# see topic:
#   "How can the Boost libraries be used successfully for important projects?"
# in this faq:
#   http://boost.org/more/faq.htm

REQUIRED_LIBS := \
  $(platform_boost_libraries) \
  $(platform_xmlwrapp_libraries) \
  $(platform_gnome_xml_libraries) \

wx_ldflags = \
  $(platform_wx_libraries) \
  $(platform_gui_ldflags) \

################################################################################

# Flags.

# Define FLAGS variables recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the variables
# they're composed from.

debug_flag := -ggdb

# MinGW gcc-3.4.2 writes dwarf2 debug records if '-ggdb' is specified,
# but the version of gdb packaged with it expects stabs format.
#
ifeq (3.4.2,$(gcc_version))
  debug_flag := -g
  gcc_version_specific_warnings := -Wno-uninitialized
endif

pch_header := "<pchfile.hpp>"
$(wx_dependent_objects): pch_header := "<pchfile_wx.hpp>"


all:

ifneq (,$(USE_PCH))

.PHONY: pch
pch: pchfile.hpp.gch pchfile_wx.hpp.gch

%.hpp.gch: %.hpp
	$(CXX) -x c++-header -c $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) $< -o$@

# Files are deemed to depend on wx iff they contain 'include *<wx/'.
# This heuristic isn't foolproof because wx headers might be included
# indirectly. Include an innocuous header like <wx/version.h> in files
# for which it fails.

pch_using_objects := \
  $(addsuffix .o,\
    $(basename \
      $(notdir \
        $(shell $(GREP) \
          --files-with-matches \
          'include LMI_PCH_HEADER' \
          $(src_dir)/*.?pp \
        ) \
      ) \
    ) \
  )

$(pch_using_objects): pch

actually_used_pch_flags := -DLMI_COMPILER_USES_PCH

endif # USE_PCH

CFLAGS = \
  $(debug_flag) $(optimization_flag) $(gprof_flag) \

CXXFLAGS = \
  $(debug_flag) $(optimization_flag) $(gprof_flag) \

# Explicitly disable the infelicitous auto-import default. See:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/19758
#     [2006-05-18T11:38:01Z from Earnie Boyd]

LDFLAGS = \
  $(gprof_flag) \
  -Wl,-Map,$@.map \
  -Wl,--disable-auto-import \

ifneq (,$(USE_SO_ATTRIBUTES))
  actually_used_lmi_so_attributes = -DLMI_USE_SO_ATTRIBUTES $(lmi_so_attributes)
endif

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(all_include_directories)) \
  $(lmi_wx_new_so_attributes) \
  $(actually_used_lmi_so_attributes) \
  $(platform_defines) \
  $(libstdcxx_warning_macros) \
  $(wx_predefinitions) \
  -DBOOST_STRICT_CONFIG \
 -DLMI_PCH_HEADER=$(pch_header) \
  $(actually_used_pch_flags) \

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

# TODO ?? Define $(wx_cxxflags) for msw as well as posix.

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \
  $(wx_cxxflags) \

REQUIRED_ARFLAGS = \
  -rus

# Boost filesystem library #includes additional file. Furthermore, there's
# a warning while compiling it.
$(boost_filesystem_objects): REQUIRED_CPPFLAGS += -I/opt/lmi/third_party/src/boost
$(boost_filesystem_objects): REQUIRED_CXXFLAGS += -Wno-error

# Prefer to invoke GNU 'ld' through the compiler frontends 'gcc' and
# 'g++' because that takes care of linking the required libraries for
# each language. Accordingly, pass GNU 'ld' options with '-Wl,'.

# Directories set in $(overriding_library_directories) are searched
# before any others except the current build directory. There seems
# to be no conventional name for such a variable: automake recommends
# $(LDADD) or a prefixed variant for both '-l' and '-L' options, but
# $(LDADD) can't do the right thing in all cases: e.g., to override a
# default mpatrol library with a custom build,
#   -L overrides must come at the beginning of a command, but
#   -l options must come at the end, so that mpatrol is linked last.
# That is, in the typical automake usage
#   $(LINK) $(LDFLAGS) $(OBJECTS) $(LDADD) $(LIBS)
# no single variable can be changed to produce
#   $(LINK) $(LDFLAGS) $(OBJECTS) -L custom_path $(LIBS) -l custom
# for a custom version of a library whose default version is already
# specified in $(LIBS). Thus, a distinct variable is necessary for
# path overrides, so distinct variables are necessary.

# Two subdirectories of /opt/lmi/local/
#   /opt/lmi/local/lib
#   /opt/lmi/local/bin
# are placed on the link path in order to accommodate msw dlls, for
# which no canonical location is clearly specified by FHS, because
# they're both binaries and libraries in a sense. These two
# subdirectories seem to be the most popular choices, and usage
# varies, at least for msw:
#  - wx-2.7.0 built with autotools puts its dll in lib/
#  - libxml2 and mpatrol put their dlls in bin/
# It is crucial to list these two subdirectories in exactly the order
# given. If they were specified in reverse order, then gnu 'ld' would
# find a dll before its import library, which latter would therefore
# be ignored--and that would prevent mpatrol from working correctly.

all_library_directories := \
  . \
  $(overriding_library_directories) \
  /opt/lmi/local/lib \
  /opt/lmi/local/bin \

EXTRA_LDFLAGS :=

# Keep mpatrol at the end of the library list.
REQUIRED_LDFLAGS = \
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(REQUIRED_LIBS) \
  $(MPATROL_LDFLAGS) \
  $(MPATROL_LIBS) \

# The '--use-temp-file' windres option seems to be often helpful and
# never harmful. The $(subst) workaround for '-I' isn't needed with
#   GNU windres 2.15.91 20040904
# and later versions, but is needed with
#   GNU windres 2.13.90 20030111
# and earlier versions. The $(subst) workaround for '-mno-cygwin' is
# needed as long as
#  - that option is included in $(ALL_CPPFLAGS), as it apparently
#      should be because it affects the preprocessor; and
#  - $(ALL_CPPFLAGS) is passed to 'windres', which seems common; and
#  - 'windres' doesn't gracefully ignore that option.

REQUIRED_RCFLAGS = \
  $(subst -mno-cygwin,,$(subst -I,--include-dir ,$(ALL_CPPFLAGS))) \
  --use-temp-file \

# To create msw import libraries, use '-Wl,--out-implib,$@.a'. There
# seems to be no advantage to that anymore as of 2005-01, but keeping
# the ancient incantation in a comment here does no harm.

ALL_ARFLAGS  = $(REQUIRED_ARFLAGS)  $(ARFLAGS)
ALL_CPPFLAGS = $(REQUIRED_CPPFLAGS) $(CPPFLAGS)
ALL_CXXFLAGS = $(REQUIRED_CXXFLAGS) $(CXXFLAGS)
ALL_CFLAGS   = $(REQUIRED_CFLAGS)   $(CFLAGS)
ALL_LDFLAGS  = $(REQUIRED_LDFLAGS)  $(LDFLAGS)
ALL_RCFLAGS  = $(REQUIRED_RCFLAGS)  $(RCFLAGS)

################################################################################

# Pattern rules.

%.o: %.c
	$(CC)  $(MAKEDEPEND_FLAGS) -c $(ALL_CPPFLAGS) $(ALL_CFLAGS)   $< -o$@
	$($(MAKEDEPEND_COMMAND))

%.o: %.cpp
	$(CXX) $(MAKEDEPEND_FLAGS) -c $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) $< -o$@
	$($(MAKEDEPEND_COMMAND))

%.rc.o: %.rc
	$(RC) -o $@ $(ALL_RCFLAGS) -i $<
	$(MAKEDEPEND_NON_GCC_COMMAND)

%$(EXEEXT):
	$(LD) -o $@ $^ $(ALL_LDFLAGS)
	$(POST_LINK_COMMAND)

%$(SHREXT):
	$(LD) -o $@ -shared $^ $(ALL_LDFLAGS)

%.a:
	$(AR) $(ALL_ARFLAGS) $@ $^

# TODO ?? Should target-specific dependencies reside in 'objects.make'
# instead?

# Define USE_SO_ATTRIBUTES to use shared-library 'attributes'.
#
# Such attributes are no longer needed for gcc-3.something. Omitting
# them makes building slightly more efficient--see
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/14751
#     [2005-01-18T01:09:29Z from Greg Chicares]
# The most important benefit of omitting them is that static and
# shared libraries can then be built from the same object files.
#
# On the other hand, gcc-4.x uses a deliberately similar method for
# ELF symbol visibility, a feature for which significant benefits are
# claimed:
#   http://www.nedprod.com/programs/gccvisibility.html
# And dll attributes would still be necessary for other toolsets,
# which therefore aren't fully supported yet.
#
# However, 'libwx_new.a' continues to use classic dll attributes,
# because there's never a reason to build it as a static library.

lib%.a              : lmi_so_attributes :=
lib%$(SHREXT)       : lmi_so_attributes := -DLMI_BUILD_SO

# Don't use mpatrol when building a shared library to be used by an
# application that uses mpatrol. See my postings to the mpatrol
# mailing list.

lib%$(SHREXT)       : MPATROL_LIBS :=
wx_new$(SHREXT)     : MPATROL_LIBS :=

wx_new$(SHREXT)     : EXTRA_LDFLAGS :=

                      lmi_wx_new_so_attributes := -DLMI_WX_NEW_USE_SO
wx_new$(SHREXT)     : lmi_wx_new_so_attributes := -DLMI_WX_NEW_BUILD_SO

liblmi.a liblmi$(SHREXT): EXTRA_LDFLAGS :=
liblmi.a liblmi$(SHREXT): $(lmi_common_objects)
libantediluvian.a libantediluvian$(SHREXT): $(antediluvian_common_objects)

# TODO ?? 'lmi*' targets can be built either with a shared or a static
# 'lmi' library. Choose one, or support both.

lmi_wx_monolithic$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
lmi_wx_monolithic$(EXEEXT): $(lmi_wx_objects) $(lmi_common_objects) wx_new$(SHREXT)

lmi_wx_shared$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
lmi_wx_shared$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
lmi_wx_shared$(EXEEXT): $(lmi_wx_objects) liblmi$(SHREXT) wx_new$(SHREXT)

lmi_wx_static$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
lmi_wx_static$(EXEEXT): $(lmi_wx_objects) liblmi.a wx_new$(SHREXT)

lmi_cli_monolithic$(EXEEXT): $(cli_objects) $(lmi_common_objects)

lmi_cli_shared$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
lmi_cli_shared$(EXEEXT): $(cli_objects) liblmi$(SHREXT)

lmi_cli_static$(EXEEXT): $(cli_objects) liblmi.a

antediluvian_cgi$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
antediluvian_cgi$(EXEEXT): $(cgi_objects) libantediluvian$(SHREXT)

antediluvian_cli$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
antediluvian_cli$(EXEEXT): $(cli_objects) libantediluvian$(SHREXT)

antediluvian_cli_monolithic$(EXEEXT): $(cli_objects) $(antediluvian_common_objects)

wx_new$(SHREXT): wx_new.o

# TODO ?? This needs a corresponding test target.
lmi_cgi$(EXEEXT): $(cgi_objects) $(lmi_common_objects)

lmi.rc.o: lmi.ico

################################################################################

# Install.

# TODO ?? This line
#   $(CP) --preserve --update $^
# isn't quite right: running 'make install build_type=[...]' with
# different build_types and picking the latest version of each
# component can produce a mismatched set.

# SOMEDAY !! Follow the GNU Coding Standards more closely, writing
#   bindir datadir docdir srcdir
# all with no '_', and changing the value of $(data_dir). These are
# intended by the GCS to represent destinations, but here $(htmldir)
# is actually a source at present: it's the directory to which the
# savannah 'web' cvs has been downloaded.

prefix         := /opt/lmi
exec_prefix    := $(prefix)
bin_dir        := $(exec_prefix)/bin
data_dir       := $(exec_prefix)/data
datarootdir    := $(prefix)/share
docdir         := $(datarootdir)/doc/lmi
htmldir        := $(docdir)
test_dir       := $(exec_prefix)/test
touchstone_dir := $(exec_prefix)/touchstone

data_files := \
  $(wildcard $(addprefix $(src_dir)/,*.ico *.png *.xml *.xrc *.xsd *.xsl)) \

help_files := \
  $(wildcard $(addprefix $(htmldir)/,*.html)) \

.PHONY: install
install: $(default_targets)
	+@[ -d $(exec_prefix)    ] || $(MKDIR) --parents $(exec_prefix)
	+@[ -d $(bin_dir)        ] || $(MKDIR) --parents $(bin_dir)
	+@[ -d $(data_dir)       ] || $(MKDIR) --parents $(data_dir)
	+@[ -d $(test_dir)       ] || $(MKDIR) --parents $(test_dir)
	+@[ -d $(touchstone_dir) ] || $(MKDIR) --parents $(touchstone_dir)
	@$(CP) --preserve --update $^ $(bin_dir)
	@$(CP) --preserve --update $(data_files) $(data_dir)
	@$(CP) --preserve --update $(help_files) $(data_dir)
ifeq (,$(USE_SO_ATTRIBUTES))
	@cd $(data_dir); $(bin_dir)/product_files$(EXEEXT)
else
	@$(ECHO) "Can't build product_files$(EXEEXT) with USE_SO_ATTRIBUTES."
endif

################################################################################

# Archive data files. Designed for maintainer use only.

data_archive_name := lmi-data-$(yyyymmddhhmm).tar

shared_data_files = \
  qx_ann.dat \
  qx_ann.ndx \
  qx_cso.dat \
  qx_cso.ndx \
  qx_ins.dat \
  qx_ins.ndx \
  sample.dat \
  sample.db4 \
  sample.funds \
  sample.ndx \
  sample.policy \
  sample.rounding \
  sample.strata \

.PHONY: archive_shared_data_files
archive_shared_data_files:
	cd $(data_dir)/..; \
	$(TAR) \
	  --create \
	  --file=$(data_archive_name) \
	  --verbose \
	  $(addprefix data/,$(shared_data_files)); \
	$(BZIP2) $(data_archive_name); \

################################################################################

# End-user package for msw. No such thing is needed for posix.
#
# Eventually a source archive will be included automatically.

# To create a customized package, override:
#  - fardel_name
#  - fardel_dir
#  - fardel_date_script
#  - extra_fardel_binaries
#  - extra_fardel_files
#  - extra_fardel_checksummed_files

fardel_name := lmi-$(yyyymmddhhmm)
fardel_dir  := $(prefix)/$(fardel_name)

# The first value assigned to 'd1' shows one possibility for the
# beginning date. It's deliberately overridden with another that sets
# the beginning date to the current date. The obvious y2038 problem is
# ignored because any breakage it causes will be obvious.

fardel_date_script := \
  d0=`$(DATE) +%Y-%m-01`; \
  d1=`$(DATE) --utc --date="$$d0 + 1 month         " +%s`; \
  d1=`$(DATE) --utc +%s`; \
  d2=`$(DATE) --utc --date="$$d0 + 2 months - 1 day" +%s`; \
  j1=`expr 2440587 + $$d1 / 86400`; \
  j2=`expr 2440587 + $$d2 / 86400`; \
  echo -n "$$j1 $$j2" >expiry; \

# Several shared libraries are required by lmi, but there seems to be
# no straightforward way to discover their individual names because
# libtool virtualizes them. However, this is an issue only on msw,
# where, if lmi was installed by the script provided, lmi's own
# 'local/' directory should contain all required shared libraries and
# no others.

fardel_binaries := \
  $(bin_dir)/liblmi$(SHREXT) \
  $(bin_dir)/lmi_cli_shared$(EXEEXT) \
  $(bin_dir)/lmi_wx_shared$(EXEEXT) \
  $(bin_dir)/wx_new$(SHREXT) \
  $(wildcard $(prefix)/local/bin/*$(SHREXT)) \
  $(wildcard $(prefix)/local/lib/*$(SHREXT)) \
  $(wildcard $(bin_dir)/product_files$(EXEEXT)) \
  $(extra_fardel_binaries) \

fardel_files := \
  $(addprefix $(data_dir)/,$(shared_data_files)) \
  $(data_files) \
  $(help_files) \
  $(extra_fardel_files) \

# Sensitive files are authenticated at run time.
#
# Binary files other than 'md5sum$(EXEEXT)' are not authenticated
# because they aren't easily forged but are sizable enough to make
# authentication too slow. An incorrect version of any such file might
# be distributed by accident, but that problem would not be caught by
# generating an md5sum for the incorrect file. 'md5sum$(EXEEXT)' is
# however authenticated because replacing it with a program that
# always reports success would circumvent authentication.
#
# 'passkey' is derived from the md5sums of other files; computing its
# md5sum would therefore be pointless.
#
# Write wildcards verbatim here. The $(wildcard) function doesn't find
# files created by a target's own rules. Each of these wildcards must
# expand to something, because $(shared_data_files) contains at least
# one file of each given type.

extra_fardel_checksummed_files = \
  configurable_settings.xml \

fardel_checksummed_files = \
  $(extra_fardel_checksummed_files) \
  *.dat *.db4 *.funds *.ndx *.policy *.rounding *.strata \
  expiry \
  md5sum$(EXEEXT) \

.PHONY: fardel
fardel: install
	+@[ -d $(fardel_dir) ] || $(MKDIR) --parents $(fardel_dir)
	@$(MAKE) --file=$(this_makefile) --directory=$(fardel_dir) wrap_fardel
	@$(ECHO) "Created '$(fardel_name)' archive in '$(prefix)'."

# A native 'md5sum$(EXEEXT)' must be provided because lmi uses it for
# run-time authentication.
#
# $(CP) is used without '--update' so that custom extra files can
# replace defaults regardless of their datestamps.

.PHONY: wrap_fardel
wrap_fardel:
	@$(CP) $(prefix)/third_party/bin/md5sum$(EXEEXT) .
	@$(CP) $(bin_dir)/configurable_settings.xml .
	@$(CP) --preserve $(fardel_binaries) $(fardel_files) .
	@$(fardel_date_script)
	@$(MD5SUM) $(fardel_checksummed_files) >validated.md5
	@$(bin_dir)/generate_passkey > passkey
	@$(TAR) \
	  --bzip2 \
	  --create \
	  --directory=$(prefix) \
	  --file=$(prefix)/$(fardel_name).tar.bz2 \
	  $(fardel_name)

################################################################################

# Tests.

.PHONY: test
test: $(test_targets)

# Some test targets require 'sample.policy' to exist even though they
# don't actually read its contents.

$(test_targets): sample.policy

sample.policy:
	@$(TOUCH) $@

################################################################################

# Test data.

sample.cns: $(addprefix $(src_dir)/,sample.cns)
sample.ill: $(addprefix $(src_dir)/,sample.ill)

test_data := \
  sample.cns \
  sample.ill \

$(test_data):
	@$(CP) --preserve --update $(addprefix $(src_dir)/,$@) .

################################################################################

# Unit tests.

.PHONY: unit_tests
unit_tests: $(test_data) $(unit_test_targets) run_unit_tests

.PHONY: unit_tests_not_built
unit_tests_not_built:
	@$(ECHO) "List of unit-test targets that did not build successfully:"
	@$(ECHO) $(filter-out $(shell $(LS) -1 *$(EXEEXT)),$(unit_test_targets))
	@$(ECHO) "List ends."

.PHONY: run_unit_tests
run_unit_tests: unit_tests_not_built $(addsuffix -run,$(unit_test_targets))

# Create 'mpatrol.log' if it doesn't already exist, so that it can be
# parsed for diagnostics unconditionally when unit tests are run.

mpatrol.log:
	@$(TOUCH) $@

# MSYS !! The initial ';' in the first $(SED) command works around a
# problem caused by MSYS.

.PHONY: %$(EXEEXT)-run
%$(EXEEXT)-run: mpatrol.log
	@[ -z $(compiler_runtime_files) ] \
	  || $(CP) --preserve --update $(compiler_runtime_files) .
	@$(ECHO) -e "\nRunning $*:"
	@-./$* --accept
	@[ -f mpatrol.log ] \
	  && <mpatrol.log $(SED) \
	    -e ';/^total warnings\|^total errors/!d' \
	    -e 's/^\(.*$$\)/  mpatrol: \1/' \

################################################################################

# Test command-line interface.

.PHONY: cli_tests
cli_tests: cli_tests_init cli_selftest $(addprefix cli_test-,$(test_data))

.PHONY: cli_tests_init
cli_tests_init:
	@$(ECHO) Test command line interface:

# Run the self test once, discarding the results, just to get the
# program into the disk cache. Then run it again and report results.

self_test_options := --accept --data_path=$(data_dir) --selftest

.PHONY: cli_selftest
cli_selftest: antediluvian_cli$(EXEEXT) lmi_cli_shared$(EXEEXT)
	@./antediluvian_cli$(EXEEXT) $(self_test_options) > /dev/null
	@./antediluvian_cli$(EXEEXT) $(self_test_options)
	@./lmi_cli_shared$(EXEEXT) $(self_test_options) > /dev/null
	@./lmi_cli_shared$(EXEEXT) $(self_test_options)

cli_test-sample.ill: file_option := --illfile
cli_test-sample.cns: file_option := --cnsfile

cli_test-sample.ill: special_emission :=
cli_test-sample.cns: special_emission := emit_composite_only

.PHONY: cli_test-%
cli_test-%: $(test_data) lmi_cli_shared$(EXEEXT)
	@$(ECHO) Test $*:
	@./lmi_cli_shared$(EXEEXT) \
	  --accept \
	  --data_path=$(data_dir) \
	  --emit=$(special_emission),emit_text_stream,emit_quietly,emit_timings \
	  $(file_option)=$* \
	  | $(SED) -e '/milliseconds/!d'
	@./lmi_cli_shared$(EXEEXT) \
	  --accept \
	  --data_path=$(data_dir) \
	  --emit=$(special_emission),emit_text_stream,emit_quietly \
	  $(file_option)=$* \
	  >$*.touchstone
	@<$*.touchstone \
	  $(DIFF) \
	      --ignore-all-space \
	      --ignore-matching-lines='Prepared on' \
	      - $(src_dir)/$*.touchstone \
	  | $(WC)   -l \
	  | $(SED)  -e 's/^/  /' -e 's/$$/ errors/'

################################################################################

# Test common gateway interface.

# This lightweight test emulates what a webserver would do.

# MSYS !! The initial ';' in several $(SED) commands works around a
# problem caused by MSYS.

.PHONY: cgi_tests
cgi_tests: $(test_data) antediluvian_cgi$(EXEEXT)
	@$(ECHO) Test common gateway interface:
	@./antediluvian_cgi$(EXEEXT) --write_content_string > /dev/null
	@./antediluvian_cgi$(EXEEXT) --enable_test <cgi.test.in >cgi.touchstone
	@<cgi.touchstone \
	  $(DIFF) \
	      --ignore-all-space \
	      --ignore-matching-lines='Prepared on' \
	      --ignore-matching-lines='Compiled at' \
	      --ignore-matching-lines=':[ 0-9]*milliseconds' \
	      - $(src_dir)/cgi.touchstone \
	  | $(WC)   -l \
	  | $(SED)  -e 's/^/  /' -e 's/$$/ errors/'

################################################################################

# System test.

# Output is compared with $(DIFF), which reports all textual
# discrepancies without regard to relevance; and also with
# 'ihs_crc_comp', which interprets '.test' files and calculates
# maximum relative and absolute errors for each file.
#
# Relative errors less than 1e-14 are ignored. Machine epsilon for an
# IEC 60559 double is 2.2204460492503131E-16 [C99 5.2.4.2.2/13], so
# that tolerance is about forty-five times epsilon. Experience has
# shown that the discrepancies thus ignored are never material, but
# larger discrepancies may be.

test_result_suffixes     := test test0 monthly_trace.* mec.xml

system_test_analysis := $(test_dir)/analysis-$(yyyymmddhhmm)
system_test_diffs    := $(test_dir)/diffs-$(yyyymmddhhmm)
system_test_md5sums  := $(test_dir)/md5sums-$(yyyymmddhhmm)

.PHONY: system_test
system_test: install
	@$(ECHO) System test:
	@-cd $(test_dir); \
	  $(foreach z, $(addprefix *., $(test_result_suffixes)), $(RM) --force $z;)
	@cd $(test_dir); \
	  $(bin_dir)/lmi_cli_shared$(EXEEXT) \
	    --ash_nazg --accept --regress \
	    --data_path=$(data_dir) \
	    --test_path=$(test_dir); \
	  $(MD5SUM) \
	    $(addprefix *.,$(test_result_suffixes)) \
	    >$(system_test_md5sums); \
	  for z in *.test; \
	    do \
	      $(bin_dir)/ihs_crc_comp$(EXEEXT) $$z $(touchstone_dir)/$$z \
	      | $(SED) -e ';/Summary/!d' -e "s/^ /$$z/"; \
	    done > $(system_test_analysis);
	@-< $(system_test_analysis) $(SED) \
	  -e ';/rel err.*e-0*1[5-9]/d' \
	  -e ';/abs.*0\.00.*rel/d' \
	  -e ';/abs diff: 0 /d'
	@-$(DIFF) \
	    --brief \
	    --report-identical-files \
	    $(test_dir) \
	    $(touchstone_dir) \
	    > $(system_test_diffs) \
	  || true
	@$(ECHO) Summarizing test results
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e ';/^Only in/d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ system-test files compared/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e ';/^Files.*are identical$$/!d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ system-test files match/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e ';/^Files.*are identical$$/d' \
	    -e ';/^Only in /d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ system-test nonmatching files/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e ';/^Only in.*test\/touchstone/!d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^/  /' \
	  | $(SED) -e 's/$$/ system-test missing files/'
	@$(ECHO) ...system test completed.

################################################################################

# Test headers and template-instantiation files for physical closure
# (see section 3.4 of Lakos, _Large-Scale C++ Software Design_).

# When a file passes the closure test, create an empty file with
# suffix '.physical_closure' in the build directory to record that
# success, and to prevent subsequent updates of this target from
# testing that file again unless it changes.

# Treat '.h' files as C++. Some C++ headers use that suffix, though
# this project's do not. By default, g++ reports closure defects with
# this simple technique, but gcc does not, probably because it strives
# for compatibility with pre-standard C. See
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/20528
#     [2006-08-12T15:14:21Z from Greg Chicares]
# though, and
#   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=28528
# which explains why that technique doesn't work for '.h' files with
# gcc versions in [3.4, 4.1].

physical_closure_files := \
    $(filter-out %.h.physical_closure,$(physical_closure_files))

# Changing one header may cause another that had the closure property
# to lose it, but the present test has no power to discover that. An
# autodependency mechanism could be created to fill that gap, but that
# would be gilding the lily: a nychthemeral build from scratch
# suffices to find closure problems before they can cause actual harm.

.PHONY: check_physical_closure
check_physical_closure: $(physical_closure_files)

%.physical_closure: %
	@$(GNU_CXX) \
	  -DLMI_IGNORE_PCH \
	  $(ALL_CPPFLAGS) $(ALL_CXXFLAGS) \
	  -x c++ -w -O0 -fsyntax-only $<
	@$(TOUCH) $@

################################################################################

# Remove EDG prelinker files. This is experimental and seems no panacea.

edg_prelinker_files := \
  $(wildcard *.ii *.ti) \

.PHONY: clean_edg
clean_edg:
	@-$(RM) --force $(edg_prelinker_files)

################################################################################

# Display selected variables.

.PHONY: show_flags
show_flags:
	@$(ECHO) ALL_CPPFLAGS            = '$(ALL_CPPFLAGS)'
	@$(ECHO) ALL_CFLAGS              = '$(ALL_CFLAGS)'
	@$(ECHO) ALL_CXXFLAGS            = '$(ALL_CXXFLAGS)'
	@$(ECHO) ALL_ARFLAGS             = '$(ALL_ARFLAGS)'
	@$(ECHO) ALL_LDFLAGS             = '$(ALL_LDFLAGS)'
	@$(ECHO) ALL_RCFLAGS             = '$(ALL_RCFLAGS)'
	@$(ECHO) src_dir                 = '$(src_dir)'
	@$(ECHO) all_include_directories = '$(all_include_directories)'
	@$(ECHO) all_source_directories  = '$(all_source_directories)'
	@$(ECHO) wx_include_paths        = '$(wx_include_paths)'
	@$(ECHO) wx_libraries            = '$(wx_libraries)'
	@$(ECHO) wx_library_paths        = '$(wx_library_paths)'
	@$(ECHO) wx_predefinitions       = '$(wx_predefinitions)'

