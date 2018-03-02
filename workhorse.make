# Main lmi makefile, invoked by 'GNUmakefile'.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

$(srcdir)/workhorse.make:: ;

# Configuration.

include $(srcdir)/configuration.make
$(srcdir)/configuration.make:: ;

# Automatic dependencies.

include $(srcdir)/autodependency.make
$(srcdir)/autodependency.make:: ;

# Objects.

include $(srcdir)/objects.make
$(srcdir)/objects.make:: ;

################################################################################

# Effective default target (described above under "Default target").

default_targets := \
  antediluvian_cgi$(EXEEXT) \
  antediluvian_cli$(EXEEXT) \
  libantediluvian$(SHREXT) \
  liblmi$(SHREXT) \
  lmi_cli_shared$(EXEEXT) \

# For targets that depend on wx, build type 'safestdlib' requires a
# compatible wx build, which is not yet available.

ifneq (safestdlib,$(findstring safestdlib,$(build_type)))
  default_targets += \
    lmi_wx_shared$(EXEEXT) \
    skeleton$(SHREXT) \
    wx_new$(SHREXT) \
    wx_test$(EXEEXT) \

endif

# The product_files target doesn't build with shared-library
# 'attributes'.
#
# TODO ?? The gpt server, however, is important; it needs work anyway.

ifeq (,$(USE_SO_ATTRIBUTES))
  default_targets += \
    bcc_ar$(EXEEXT) \
    bcc_cc$(EXEEXT) \
    bcc_ld$(EXEEXT) \
    bcc_rc$(EXEEXT) \
    elapsed_time$(EXEEXT) \
    generate_passkey$(EXEEXT) \
    ihs_crc_comp$(EXEEXT) \
    rate_table_tool$(EXEEXT) \

  ifneq (so_test,$(findstring so_test,$(build_type)))
    default_targets += \
      product_files$(EXEEXT) \

  endif
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
else ifeq (4.9.1,$(gnu_cpp_version))
else ifeq (4.9.2,$(gnu_cpp_version))
else ifeq (6.3.0,$(gnu_cpp_version))
else ifeq (7.2-win32,$(gnu_cpp_version))
else
  $(warning Untested $(GNU_CPP) version '$(gnu_cpp_version)')
endif

ifeq      (3.4.4,$(gnu_cxx_version))
else ifeq (3.4.5,$(gnu_cxx_version))
else ifeq (4.9.1,$(gnu_cxx_version))
else ifeq (4.9.2,$(gnu_cxx_version))
else ifeq (6.3.0,$(gnu_cxx_version))
else ifeq (7.2-win32,$(gnu_cxx_version))
else
  $(warning Untested $(GNU_CXX) version '$(gnu_cxx_version)')
endif

################################################################################

# wx settings.

wx_dir := /opt/lmi/local/bin

wx_config_script := wx-config

# The conventional autotools usage...
ifeq (gcc,$(toolset))
  wx_config_cxxflags := $(shell $(wx_config_script) --cxxflags)
  wx_config_libs     := $(shell $(wx_config_script) --libs)
  wx_config_basename := $(shell $(wx_config_script) --basename)
  wx_config_version  := $(shell $(wx_config_script) --version)
  # [variables used to construct the names of wxCode libraries]
  wxcode_basename := $(shell echo $(wx_config_basename) | sed 's/^wx/wxcode/')
  wxcode_version  := $(shell echo $(wx_config_version) | sed 's/\.[0-9]*$$//')
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
# cgicc: './configure && make' failed in the MSYS environment (though
# MSYS is no longer supported).
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
  $(srcdir) \
  $(srcdir)/tools/pete-2.1.1 \
  $(overriding_include_directories) \
  $(compiler_include_directory) \
  $(wx_include_paths) \
  /opt/lmi/third_party/include \
  /opt/lmi/third_party/src \
  /opt/lmi/local/include \
  /opt/lmi/local/include/libxml2 \

all_source_directories := \
  $(srcdir) \
  /opt/lmi/third_party/src/boost/libs/filesystem/src \
  /opt/lmi/third_party/src/boost/libs/regex/src \
  /opt/lmi/third_party/src/cgicc \

vpath lib%.a          $(CURDIR)
vpath %.o             $(CURDIR)

# Files whose names match 'my_%.cpp' are taken as product data files,
# which are overridden by any customized files found in a special
# directory.

vpath my_%.cpp        $(srcdir)/../products/src

vpath %.c             $(all_source_directories)
vpath %.cpp           $(all_source_directories)
vpath %.h             $(all_source_directories)
vpath %.hpp           $(all_source_directories)
vpath %.tpp           $(all_source_directories)
vpath %.xpp           $(all_source_directories)

vpath %.rc            $(all_source_directories)
vpath %.ico           $(srcdir)

vpath quoted_gpl      $(srcdir)
vpath quoted_gpl_html $(srcdir)

################################################################################

# Headers and template-instantiation files to test for physical closure.

# Only files in the source directory are tested. Files that reside
# elsewhere (e.g., headers accompanying libraries) are not tested.

# Exclude headers named 'config_*.hpp' or 'pchlist*.hpp': they are
# designed to signal errors if they are used separately. $(sort) is
# used here to remove duplicates, which are harmless but inefficient.

physical_closure_files := \
  $(sort \
    $(addsuffix .physical_closure,\
      $(filter-out config_%.hpp pchlist%.hpp,\
        $(notdir \
          $(wildcard \
            $(addprefix $(srcdir)/,*.h *.hpp *.tpp *.xpp \
            ) \
          ) \
        ) \
      ) \
    ) \
  ) \

################################################################################

# Overriding options--simply expanded, and empty by default.

tutelary_flag :=

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
else ifneq (,$(filter $(gcc_version), 4.9.1 4.9.2))
  # See:
  #   http://lists.nongnu.org/archive/html/lmi/2015-12/msg00028.html
  #   http://lists.nongnu.org/archive/html/lmi/2015-12/msg00040.html
  gcc_version_specific_warnings := \
    -Wno-conversion \
    -Wno-parentheses \
    -Wno-unused-local-typedefs \
    -Wno-unused-variable \

  cxx_standard := -std=c++11
else ifneq (,$(filter $(gcc_version), 6.3.0))
  # See:
  #   http://lists.nongnu.org/archive/html/lmi/2015-12/msg00028.html
  #   http://lists.nongnu.org/archive/html/lmi/2015-12/msg00040.html
  gcc_version_specific_warnings := \
    -Wno-conversion \
    -Wno-parentheses \
    -Wno-unused-local-typedefs \
    -Wno-unused-variable \

  cxx_standard := -std=c++17

# The default '-fno-rounding-math' means something like
  #   #pragma STDC FENV ACCESS OFF
  # which causes harm while bringing no countervailing benefit--see:
  #   http://lists.nongnu.org/archive/html/lmi/2017-08/msg00045.html
  c_standard   += -frounding-math
  cxx_standard += -frounding-math
endif

treat_warnings_as_errors := -pedantic-errors -Werror

gcc_common_warnings := \
  $(treat_warnings_as_errors) \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wextra \
  -Wimport \
  -Wmultichar \
  -Wpacked \
  -Wpointer-arith \
  -Wredundant-decls \
  -Wshadow \
  -Wsign-compare \
  -Wundef \
  -Wunreachable-code \
  -Wwrite-strings \

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
  -Wsynth \

# Too many warnings on correct code, e.g. exact comparison to zero:
#  -Wfloat-equal \

# WX !! The wx library triggers many diagnostics with the following
# 'extra' flags. This makefile used to inhibit these flags for source
# files that seemed to depend on wx according to a casual heuristic,
# but now they're inhibited by a #pragma in the PCH file that all wx-
# dependent TUs must include.

gcc_common_extra_warnings := \
  -Wcast-qual \

ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
  ifeq (3.4.5,$(gcc_version))
    expression_template_0_test.o: gcc_common_extra_warnings += -Wno-unused-parameter
  endif
endif

# Boost didn't remove an unused parameter in this file, which also
# seems to contain a "maybe-uninitialized" variable--see:
#   http://lists.nongnu.org/archive/html/lmi/2016-12/msg00080.html

operations_posix_windows.o: gcc_common_extra_warnings += -Wno-unused-parameter
operations_posix_windows.o: gcc_common_extra_warnings += -Wno-maybe-uninitialized

# The boost regex library is incompatible with '-Wshadow'.

$(boost_regex_objects): gcc_common_extra_warnings += -Wno-shadow

# The boost regex library improperly defines "NOMINMAX":
#   http://lists.boost.org/Archives/boost/2006/03/102189.php
# at least in version 1.33.1, and there seems to be no easy workaround
# except to blow away all warning options and let a warning appear.
# This problem seems not to occur with gcc-4.x .

ifeq (3.4.5,$(gcc_version))
  static_mutex.o: gcc_common_extra_warnings :=
  static_mutex.o:          gcc_cxx_warnings :=
endif

# Boost normally makes '-Wundef' give spurious warnings:
#   http://aspn.activestate.com/ASPN/Mail/Message/boost/1822550
# but defining BOOST_STRICT_CONFIG:
#   http://www.boost.org/libs/config/config.htm#user_settable
# makes '-Wundef' usable, because boost-1.31.0 doesn't seem to need
# any workarounds for gcc-3.3+ . However, it gives a number of
# warnings with wx-2.5.4 (that have been fixed in a later version).

# Too many warnings for wx and various boost libraries:
#  -Wold-style-cast \

C_WARNINGS = \
  $(gcc_c_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_warnings) \

CXX_WARNINGS = \
  $(gcc_cxx_warnings) \
  $(gcc_common_extra_warnings) \
  $(gcc_version_specific_warnings) \

################################################################################

# Flags for tuning gcc.

# As this is written in 2012, lmi is often built on machines with less
# RAM per core than gcc wants. Experiments show that these flags cut
# gcc's RAM appetite by fifty percent, in return for a ten-percent
# speed penalty that can be overcome by increasing parallelism. There
# seems to be no need for them with gcc-4.x, which uses less RAM.

ifeq (gcc,$(toolset))
  ifeq (3.4.5,$(gcc_version))
    ggc_flags := --param ggc-min-expand=25 --param ggc-min-heapsize=32768
  endif
endif

################################################################################

# Build type governs
#  - optimization flags
#  - gprof
#  - libstdc++ debugging macros

# libstdc++ debugging macros

every_libstdcxx_warning_macro := \
  -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC \

test_targets := unit_tests cgi_tests cli_tests

# Since gcc version 4.6, '-fomit-frame-pointer' has apparently been
# the default. Don't use that because it makes debugging difficult.
# See:
#   https://lists.gnu.org/archive/html/lmi/2016-06/msg00091.html

ifeq (gprof,$(findstring gprof,$(build_type)))
  optimization_flag := -O0 -fno-omit-frame-pointer
  gprof_flag := -pg
else
  ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
    optimization_flag := -O0 -fno-omit-frame-pointer
    libstdcxx_warning_macros := $(every_libstdcxx_warning_macro)
  else
    optimization_flag := -O2 -fno-omit-frame-pointer
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
#
# The same problem was observed with 'my_db.cpp'. For good measure,
# all similarly-coded 'my_*.cpp' files are treated the same way.

my_unoptimizable_files := my_db.o my_fund.o my_prod.o my_rnd.o my_tier.o

$(my_unoptimizable_files): optimization_flag := -O0 -fno-omit-frame-pointer

# Blocking optimization in default $(CXXFLAGS) isn't enough, because
# it is too easily overridden by specifying $(CXXFLAGS) on the command
# line. This flag overrides such overrides:

$(my_unoptimizable_files): tutelary_flag += -O0 -fno-omit-frame-pointer

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
  $(wx_library_paths) $(wx_libraries) \
  $(platform_gui_ldflags) \

wx_pdfdoc_ldflags := \
  -l$(wxcode_basename)_pdfdoc-$(wxcode_version)

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

CFLAGS = \
  $(ggc_flags) $(debug_flag) $(optimization_flag) $(gprof_flag) \

CXXFLAGS = \
  $(ggc_flags) $(debug_flag) $(optimization_flag) $(gprof_flag) \

LDFLAGS = \
  $(gprof_flag) \
  -Wl,-Map,$@.map \

# Explicitly disable the infelicitous auto-import default. See:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/19758
#     [2006-05-18T11:38:01Z from Earnie Boyd]
# Do not disable it for MinGW-w64, which seems to require it.

ifeq (3.4.5,$(gcc_version))
  LDFLAGS += -Wl,--disable-auto-import
endif

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
  -DBOOST_NO_AUTO_PTR \
  -DBOOST_STRICT_CONFIG \

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \

REQUIRED_ARFLAGS = \
  -rus

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
#  - libxml2 and libxslt put their dlls in bin/
# It is crucial to list these two subdirectories in exactly the order
# given. If they were specified in reverse order, then gnu 'ld' would
# find a dll before its import library, which latter would therefore
# be ignored--and that prevented mpatrol from working correctly.

all_library_directories := \
  . \
  $(overriding_library_directories) \
  /opt/lmi/local/lib \
  /opt/lmi/local/bin \

EXTRA_LDFLAGS :=

REQUIRED_LDFLAGS = \
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(REQUIRED_LIBS) \

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
ALL_CXXFLAGS = $(REQUIRED_CXXFLAGS) $(CXXFLAGS) $(tutelary_flag)
ALL_CFLAGS   = $(REQUIRED_CFLAGS)   $(CFLAGS)   $(tutelary_flag)
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

%.o: %.rc
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

# '-DLMI_BUILD_SO' would be incorrect here, even though a shared
# library is being built. The 'LMI_*_SO' macros are used only in
# source files that are unrelated to wx, and that are therefore not
# part of $(skeleton_objects).
skeleton$(SHREXT): lmi_so_attributes := -DLMI_USE_SO
skeleton$(SHREXT): EXTRA_LDFLAGS := $(wx_pdfdoc_ldflags) $(wx_ldflags)
skeleton$(SHREXT): $(skeleton_objects) liblmi$(SHREXT) wx_new$(SHREXT)

lmi_wx_shared$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
lmi_wx_shared$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
lmi_wx_shared$(EXEEXT): $(lmi_wx_objects) skeleton$(SHREXT) liblmi$(SHREXT)

lmi_wx_static$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
lmi_wx_static$(EXEEXT): $(lmi_wx_objects) $(skeleton_objects) liblmi.a wx_new$(SHREXT)

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

wx_test$(EXEEXT): lmi_so_attributes := -DLMI_USE_SO
wx_test$(EXEEXT): EXTRA_LDFLAGS := $(wx_ldflags)
wx_test$(EXEEXT): $(wx_test_objects) skeleton$(SHREXT) liblmi$(SHREXT)

# TODO ?? This needs a corresponding test target.
lmi_cgi$(EXEEXT): $(cgi_objects) $(lmi_common_objects)

lmi_msw_res.o: lmi.ico

################################################################################

# Install.

# TODO ?? This line
#   $(CP) --preserve --update $^
# isn't quite right: running 'make install build_type=[...]' with
# different build_types and picking the latest version of each
# component can produce a mismatched set.

# SOMEDAY !! Follow the GNU Coding Standards
#   https://www.gnu.org/prep/standards/html_node/Directory-Variables.html
# more closely, changing the value of $(datadir), and perhaps using
# some other standard directories that are commented out for now.

prefix         := /opt/lmi
# parent directory for machine-specific binaries
exec_prefix    := $(prefix)
# binaries that users can run
bindir         := $(exec_prefix)/bin
# binaries that administrators can run
#sbindir        := $(exec_prefix)/sbin
# binaries run by programs
#libexecdir     := $(exec_prefix)/libexec
# parent directory for read-only architecture-independent data files
datarootdir    := $(prefix)/share
# idiosyncratic read-only architecture-independent data files
# GNU standard default:
# datadir        := $(datarootdir)
# nonstandard value used for now:
datadir        := $(exec_prefix)/data
# read-only data files that pertain to a single machine
#sysconfdir     := $(prefix)/etc
# architecture-independent data files which the programs modify while they run
#sharedstatedir := $(prefix)/com
# data files which the programs modify while they run
#localstatedir  := $(prefix)/var
# data files which the programs modify while they run, persisting until reboot
#runstatedir    := $(localstatedir)/run
# headers
#includedir     := $(prefix)/include
docdir         := $(datarootdir)/doc/lmi
htmldir        := $(docdir)
#libdir         := $(exec_prefix)/lib
# srcdir: set to $(CURDIR) upstream; no GNU default value

# no GNU standard for 'test' or 'touchstone': directory names therefore
# contain '_' for distinction and clarity
test_dir       := $(exec_prefix)/test
touchstone_dir := $(exec_prefix)/touchstone

data_files := \
  $(wildcard $(addprefix $(srcdir)/,*.ico *.mst *.png *.xml *.xrc *.xsd *.xsl)) \

help_files := \
  $(wildcard $(addprefix $(srcdir)/,*.html)) \

.PHONY: preinstall
preinstall:
	@[ -z "$(compiler_runtime_files)" ] \
	  || $(CP) --preserve --update $(compiler_runtime_files) /opt/lmi/local/bin

.PHONY: install
install: preinstall $(default_targets)
	+@[ -d $(exec_prefix)    ] || $(MKDIR) --parents $(exec_prefix)
	+@[ -d $(bindir)         ] || $(MKDIR) --parents $(bindir)
	+@[ -d $(datadir)        ] || $(MKDIR) --parents $(datadir)
	+@[ -d $(test_dir)       ] || $(MKDIR) --parents $(test_dir)
	+@[ -d $(touchstone_dir) ] || $(MKDIR) --parents $(touchstone_dir)
	@$(CP) --preserve --update $(default_targets) $(bindir)
	@$(CP) --preserve --update $(data_files) $(datadir)
	@$(CP) --preserve --update $(help_files) $(datadir)
ifeq (,$(USE_SO_ATTRIBUTES))
	@cd $(datadir); $(PERFORM) $(bindir)/product_files$(EXEEXT)
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
  sample.database \
  sample.funds \
  sample.ndx \
  sample.policy \
  sample.rounding \
  sample.strata \

.PHONY: archive_shared_data_files
archive_shared_data_files:
	cd $(datadir)/..; \
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

# The obvious y2038 problem is ignored because any breakage it causes
# will be, well, obvious.

fardel_date_script := \
  d0=`$(DATE) +%Y-%m-01`; \
  d1=`$(DATE) --utc --date="$$d0 + 1 month " +%s`; \
  d2=`$(DATE) --utc --date="$$d0 + 2 months" +%s`; \
  j1=`expr 2440588 + $$d1 / 86400`; \
  j2=`expr 2440588 + $$d2 / 86400`; \
  printf "$$j1 $$j2" >expiry; \

# Several shared libraries are required by lmi, but there seems to be
# no straightforward way to discover their individual names because
# libtool virtualizes them. However, this is an issue only on msw,
# where, if lmi was installed by the script provided, lmi's own
# 'local/' directory should contain all required shared libraries and
# no others.

fardel_binaries := \
  $(bindir)/liblmi$(SHREXT) \
  $(bindir)/lmi_cli_shared$(EXEEXT) \
  $(bindir)/lmi_wx_shared$(EXEEXT) \
  $(bindir)/skeleton$(SHREXT) \
  $(bindir)/wx_new$(SHREXT) \
  $(bindir)/wx_test$(EXEEXT) \
  $(wildcard $(prefix)/local/bin/*$(SHREXT)) \
  $(wildcard $(prefix)/local/lib/*$(SHREXT)) \
  $(wildcard $(bindir)/product_files$(EXEEXT)) \
  $(extra_fardel_binaries) \

fardel_files := \
  $(addprefix $(datadir)/,$(shared_data_files)) \
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

# PDF !! Add "*.mst" below, only when the MST implementation is released.

fardel_checksummed_files = \
  $(extra_fardel_checksummed_files) \
  *.dat *.database *.funds *.ndx *.policy *.rounding *.strata \
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
	@$(CP) $(datadir)/configurable_settings.xml .
	@$(CP) --preserve $(fardel_binaries) $(fardel_files) .
	@$(fardel_date_script)
	@$(MD5SUM) --binary $(fardel_checksummed_files) >validated.md5
	@$(PERFORM) $(bindir)/generate_passkey > passkey
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

# Some 'antediluvian' test targets require a '.policy' to exist even
# though they don't actually read its contents.

$(test_targets): eraseme.policy

eraseme.policy:
	@$(TOUCH) $@

################################################################################

# Test data.

sample.cns: $(srcdir)/sample.cns
	$(CP) --preserve --update $< .

sample.ill: $(srcdir)/sample.ill
	$(CP) --preserve --update $< .

test_data := \
  sample.cns \
  sample.ill \

################################################################################

# Configurable settings.

# If this file:
#   /etc/opt/lmi/configurable_settings.xml
# exists, it overrides files with the same name in other directories.
# Developers may have reason to prefer that it not exist; therefore,
# local copies are provided for as needed.

configurable_settings.xml:
	@$(CP) --preserve --update $(datadir)/$@ .

################################################################################

# Unit tests.

# Use '--output-sync=recurse' so that the output of each test is
# written as a separate block even though the tests are run in
# parallel. Each block is written when its test ends, so the order
# of the blocks is indeterminate. When tests are run in parallel,
# their internal timings may be less accurate because of competition
# for resources other than CPUs. Tests can of course be run in series
# simply by not specifying '--jobs=' on the command line; that is
# advisable if output is written to logs which are to be compared,
# e.g., to determine whether timings have changed.

.PHONY: unit_tests
unit_tests: $(test_data)
	@-$(MAKE) --file=$(this_makefile) build_unit_tests
	@ $(MAKE) --file=$(this_makefile) --output-sync=recurse run_unit_tests

.PHONY: build_unit_tests
build_unit_tests: configurable_settings.xml $(unit_test_targets)

.PHONY: unit_tests_not_built
unit_tests_not_built:
	@$(ECHO) "List of unit-test targets that did not build successfully:"
	@$(ECHO) $(filter-out $(shell $(LS) -1 *$(EXEEXT)),$(unit_test_targets))
	@$(ECHO) "List ends."

.PHONY: run_unit_tests
run_unit_tests: unit_tests_not_built $(addsuffix -run,$(unit_test_targets))

.PHONY: %$(EXEEXT)-run
%$(EXEEXT)-run:
	@printf "\nRunning $*:\n"
	@-$(PERFORM) ./$* --accept

################################################################################

# Test command-line interface.

cli_subtargets := cli_tests_init cli_selftest $(addprefix cli_test-,$(test_data))

$(cli_subtargets): $(datadir)/configurable_settings.xml

# Use '--jobs=1' to force tests to run in series: running them in
# parallel would scramble their output. Using '--output-sync=recurse'
# would prevent scrambling, but would not preserve the order of the
# tests; that alternative is not chosen because a uniform order makes
# visual inspection a little easier, while the tests are so fast that
# parallelism affords no palpable advantage.
#
# Ignore the "disabling jobserver mode" warning.

.PHONY: cli_tests
cli_tests: $(test_data) antediluvian_cli$(EXEEXT) lmi_cli_shared$(EXEEXT)
	@$(MAKE) --file=$(this_makefile) --jobs=1 $(cli_subtargets)

.PHONY: cli_tests_init
cli_tests_init:
	@$(ECHO) Test command line interface:

# Run the self test once, discarding the results, just to get the
# program into the disk cache. Then run it again and report results.

self_test_options := --accept --data_path=$(datadir) --selftest

.PHONY: cli_selftest
cli_selftest:
	@$(PERFORM) ./antediluvian_cli$(EXEEXT) $(self_test_options) > /dev/null
	@$(PERFORM) ./antediluvian_cli$(EXEEXT) $(self_test_options)
	@$(PERFORM) ./lmi_cli_shared$(EXEEXT) $(self_test_options) > /dev/null
	@$(PERFORM) ./lmi_cli_shared$(EXEEXT) $(self_test_options)

cli_test-sample.ill: special_emission :=
cli_test-sample.cns: special_emission := emit_composite_only

.PHONY: cli_test-%
cli_test-%:
	@$(ECHO) Test $*:
	@$(PERFORM) ./lmi_cli_shared$(EXEEXT) \
	  --accept \
	  --data_path=$(datadir) \
	  --emit=$(special_emission),emit_text_stream,emit_quietly,emit_timings \
	  --file=$* \
	  | $(SED) -e '/milliseconds/!d'
	@$(PERFORM) ./lmi_cli_shared$(EXEEXT) \
	  --accept \
	  --data_path=$(datadir) \
	  --emit=$(special_emission),emit_text_stream,emit_quietly \
	  --file=$* \
	  >$*.touchstone
	@<$*.touchstone \
	  $(DIFF) \
	      --ignore-matching-lines='Prepared on' \
	      - $(srcdir)/$*.touchstone \
	  | $(WC)   -l \
	  | $(SED)  -e 's/^/  /' -e 's/$$/ errors/'

################################################################################

# Test common gateway interface.

# This lightweight test emulates what a webserver would do.

.PHONY: cgi_tests
cgi_tests: $(test_data) configurable_settings.xml antediluvian_cgi$(EXEEXT)
	@$(ECHO) Test common gateway interface:
	@$(PERFORM) ./antediluvian_cgi$(EXEEXT) --write_content_string > /dev/null
	@$(PERFORM) ./antediluvian_cgi$(EXEEXT) --enable_test <cgi.test.in >cgi.touchstone
	@<cgi.touchstone \
	  $(DIFF) \
	      --ignore-matching-lines='Prepared on' \
	      --ignore-matching-lines='Compiled at' \
	      --ignore-matching-lines=':[ 0-9]*milliseconds' \
	      - $(srcdir)/cgi.touchstone \
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

touchstone_md5sums := $(touchstone_dir)/md5sums

touchstone_exclusions := $(touchstone_md5sums) $(touchstone_dir)/ChangeLog

touchstone_files := \
  $(filter-out $(touchstone_exclusions),$(wildcard $(touchstone_dir)/*))

# Don't call md5sum with an empty $(touchstone_files) list, which would
# cause md5sum to wait indefinitely for input; instead, create an empty
# $(touchstone_md5sums) iff none exists, because this is a prerequisite
# of the 'system_test' target.

$(touchstone_md5sums): $(touchstone_files)
	@-[ -f "$@" ] || $(TOUCH) $@
	@-[ -n "$^" ] && cd $(touchstone_dir) && $(MD5SUM) --binary $(notdir $^) > $@
	@$(SORT) --key=2 --output=$@ $@

testdeck_suffixes    := cns ill ini inix mec gpt
test_result_suffixes := test test0 test1 monthly_trace.* mec.tsv mec.xml gpt.tsv gpt.xml

# These files summarize system-test results and their differences from
# results saved in $(touchstone_dir). Datestamps are embedded in their
# names to accumulate history in $(test_dir), which permits comparison
# across several versions. An undatestamped copy of the md5sums is
# made for storage in a version-control system (which naturally keeps
# historical versions in its own way).

system_test_analysis := $(test_dir)/analysis-$(yyyymmddhhmm)
system_test_diffs    := $(test_dir)/diffs-$(yyyymmddhhmm)
system_test_md5sums  := $(test_dir)/md5sums-$(yyyymmddhhmm)
system_test_md5sums2 := $(test_dir)/md5sums

%.cns:  test_emission := emit_quietly,emit_test_data
%.ill:  test_emission := emit_quietly,emit_test_data
%.ini:  test_emission := emit_quietly,emit_custom_0
%.inix: test_emission := emit_quietly,emit_custom_1
%.mec:  test_emission := emit_quietly,emit_test_data
%.gpt:  test_emission := emit_quietly,emit_test_data

dot_test_files =
%.cns: dot_test_files = $(basename $(notdir $@)).*test
%.ill: dot_test_files = $(basename $(notdir $@)).*test

# This must be a 'make' variable so that the targets it contains can
# be made PHONY.
#
# Use $(wildcard) here because its convenient 'nullglob' semantics are
# not portably available in the bourne shell.
#
# In the 'system_test' target, sort its contents iff $(LS) supports
# '--sort=size': parallel runs are slightly faster when the biggest
# jobs are started first.

testdecks := $(wildcard $(addprefix $(test_dir)/*., $(testdeck_suffixes)))

# Naming the output files would be more natural, but that's infeasible
# because $(test_emission) can be overridden implicitly in ways that a
# makefile cannot readily discern.

.PHONY: $(testdecks)
$(testdecks):
	@-$(PERFORM) $(bindir)/lmi_cli_shared$(EXEEXT) \
	  --accept \
	  --ash_nazg \
	  --data_path=$(datadir) \
	  --emit=$(test_emission) \
	  --pyx=system_testing \
	  --file=$@
	@$(MD5SUM) --binary $(basename $(notdir $@)).* >> $(system_test_md5sums)
	@for z in $(dot_test_files); \
	  do \
	    $(PERFORM) $(bindir)/ihs_crc_comp$(EXEEXT) $$z $(touchstone_dir)/$$z \
	    | $(SED) -e '/Summary.*max rel err/!d' -e "s/^ /$$z/" \
	    >> $(system_test_analysis); \
	  done

.PHONY: system_test
system_test: $(datadir)/configurable_settings.xml $(touchstone_md5sums) install
	@$(ECHO) System test:
	@$(RM) --force $(addprefix $(test_dir)/*., $(test_result_suffixes))
	@[ "$(strip $(testdecks))" != "" ] || ( $(ECHO) No testdecks. && false )
	@testdecks=`$(LS) --sort=size $(testdecks) || $(ECHO) $(testdecks)` \
	  && $(MAKE) --file=$(this_makefile) --directory=$(test_dir) $$testdecks
	@$(SORT) --output=$(system_test_analysis) $(system_test_analysis)
	@$(SORT) --key=2  --output=$(system_test_md5sums) $(system_test_md5sums)
	@$(CP) --preserve --update $(system_test_md5sums) $(system_test_md5sums2)
	@-< $(system_test_analysis) $(SED) \
	  -e '/rel err.*e-0*1[5-9]/d' \
	  -e '/abs.*0\.00.*rel/d' \
	  -e '/abs diff: 0 /d'
	@$(DIFF) --brief $(system_test_md5sums) $(touchstone_md5sums) \
	  && $(ECHO) "All `<$(touchstone_md5sums) $(WC) -l` files match." \
	  || $(MAKE) --file=$(this_makefile) system_test_discrepancies

.PHONY: system_test_discrepancies
system_test_discrepancies:
	@$(ECHO) "*** System test failed ***"
	@-$(DIFF) \
	    --brief \
	    --report-identical-files \
	    $(test_dir) \
	    $(touchstone_dir) \
	    > $(system_test_diffs) \
	  || true
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e '/^Only in/d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^\(.*\)$$/  \1 system-test files compared/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e '/^Files.*are identical$$/!d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^\(.*\)$$/  \1 system-test files match/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e '/^Files.*are identical$$/d' \
	    -e '/^Only in/d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^\(.*\)$$/  \1 system-test files differ/'
	@-<$(system_test_diffs) \
	  $(SED) \
	    -e '/^Only in.*touchstone:/!d' \
	    -e '/md5sums$$/d' \
	  | $(WC) -l \
	  | $(SED) -e 's/^\(.*\)$$/  \1 system-test files missing/'
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
	@$(ECHO) srcdir                  = '$(srcdir)'
	@$(ECHO) all_include_directories = '$(all_include_directories)'
	@$(ECHO) all_source_directories  = '$(all_source_directories)'
	@$(ECHO) wx_include_paths        = '$(wx_include_paths)'
	@$(ECHO) wx_libraries            = '$(wx_libraries)'
	@$(ECHO) wx_library_paths        = '$(wx_library_paths)'
	@$(ECHO) wx_predefinitions       = '$(wx_predefinitions)'

