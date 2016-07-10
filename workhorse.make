# Main lmi makefile, invoked by 'GNUmakefile'.
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

  ifneq (so_test,$(findstring so_test,$(build_type)))
    default_targets += \
      wx_test$(EXEEXT) \

  endif
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
    lmi_md5sum$(EXEEXT) \
    rate_table_tool$(EXEEXT) \
    test_coding_rules$(EXEEXT) \

  ifneq (so_test,$(findstring so_test,$(build_type)))
    default_targets += \
      product_files$(EXEEXT) \

  endif
endif

.PHONY: effective_default_target
effective_default_target: $(default_targets)

################################################################################

# $(subst): workaround for debian, whose MinGW-w64 identifies its
# version 7.x.0 as "7.x-win32".

ifeq (gcc,$(LMI_COMPILER))
  gcc_version   := $(subst -win32,.0,$(shell $(CXX)     -dumpversion))
endif

# These are defined even for toolchains other than gcc.

gnu_cpp_version := $(subst -win32,.0,$(shell $(GNU_CPP) -dumpversion))
gnu_cxx_version := $(subst -win32,.0,$(shell $(GNU_CXX) -dumpversion))

ifeq      (3.4.4,$(gnu_cpp_version))
else ifeq (3.4.5,$(gnu_cpp_version))
else ifeq (4.9.1,$(gnu_cpp_version))
else ifeq (4.9.2,$(gnu_cpp_version))
else ifeq (6.3.0,$(gnu_cpp_version))
else ifeq (7.2.0,$(gnu_cpp_version))
else ifeq (7.3.0,$(gnu_cpp_version))
else ifeq (8,$(gnu_cpp_version))
else ifeq (8.1.0,$(gnu_cpp_version))
else ifeq (8.2.0,$(gnu_cpp_version))
else ifeq (8.3.0,$(gnu_cpp_version))
else ifeq (9,$(gnu_cpp_version))
else ifeq (9.3.0,$(gnu_cpp_version))
else ifeq (10,$(gnu_cpp_version))
else ifeq (10.0,$(gnu_cpp_version))
else
  $(warning Untested $(GNU_CPP) version '$(gnu_cpp_version)')
endif

ifeq      (3.4.4,$(gnu_cxx_version))
else ifeq (3.4.5,$(gnu_cxx_version))
else ifeq (4.9.1,$(gnu_cxx_version))
else ifeq (4.9.2,$(gnu_cxx_version))
else ifeq (6.3.0,$(gnu_cxx_version))
else ifeq (7.2.0,$(gnu_cxx_version))
else ifeq (7.3.0,$(gnu_cxx_version))
else ifeq (8,$(gnu_cxx_version))
else ifeq (8.1.0,$(gnu_cxx_version))
else ifeq (8.2.0,$(gnu_cxx_version))
else ifeq (8.3.0,$(gnu_cxx_version))
else ifeq (9,$(gnu_cxx_version))
else ifeq (9.3.0,$(gnu_cxx_version))
else ifeq (10,$(gnu_cxx_version))
else ifeq (10.0,$(gnu_cxx_version))
else
  $(warning Untested $(GNU_CXX) version '$(gnu_cxx_version)')
endif

################################################################################

# xml library settings.

# Flags for all other xml libraries are provided by *-config scripts,
# but '-lexslt' is a special case--see:
#   https://mail.gnome.org/archives/xslt/2001-October/msg00133.html
#   https://lists.nongnu.org/archive/html/lmi/2020-10/msg00066.html

xml_libraries := \
  $(shell xmlwrapp-config --libs) \
  -lexslt \
  $(shell xslt-config --libs) \
  $(shell xml2-config --libs) \

################################################################################

# wx settings.

wx_dir := $(localbindir)

wx_config_script := wx-config

# The conventional autotools usage...
wx_config_cxxflags := $(shell $(wx_config_script) --cxxflags)
wx_config_libs     := $(shell $(wx_config_script) --libs)
wx_config_basename := $(shell $(wx_config_script) --basename)
wx_config_version  := $(shell $(wx_config_script) --version)
# [variables used to construct the names of wxCode libraries]
wxcode_basename := $(shell echo $(wx_config_basename) | sed 's/^wx/wxcode/')
wxcode_version  := $(shell echo $(wx_config_version) | sed 's/\.[0-9]*$$//')
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
# $(locallibdir) and $(localbindir); see:
#   https://lists.nongnu.org/archive/html/lmi/2006-10/msg00046.html
# for some discussion.

lmi_include_directories := \
  $(srcdir) \
  $(srcdir)/tools/pete-2.1.1 \
  $(overriding_include_directories) \

sys_include_directories := \
  $(compiler_include_directory) \
  $(wx_include_paths) \
  /opt/lmi/third_party/include \
  $(localincludedir) \
  $(localincludedir)/libxml2 \

all_source_directories := \
  $(srcdir) \
  /opt/lmi/third_party/src/boost/libs/filesystem/src \
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

# Dialect and warning options for gcc.

# The default '-fno-rounding-math' means something like
#   #pragma STDC FENV ACCESS OFF
# which causes harm while bringing no countervailing benefit--see:
#   https://lists.nongnu.org/archive/html/lmi/2017-08/msg00045.html

c_standard   := -fno-ms-extensions -frounding-math -std=c99
cxx_standard := -fno-ms-extensions -frounding-math -std=c++20

# Specify $(gcc_version_specific_warnings) last, in order to override
# other options.

ifeq (3.4.4,$(gcc_version))
  # Suppress spurious gcc-3.4.4 warnings:
  #   http://gcc.gnu.org/bugzilla/show_bug.cgi?id=22207
  gcc_version_specific_warnings := -Wno-uninitialized
  cxx_standard := -std=c++98
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
  #   https://lists.nongnu.org/archive/html/lmi/2015-12/msg00028.html
  #   https://lists.nongnu.org/archive/html/lmi/2015-12/msg00040.html
  gcc_version_specific_warnings := \
    -Wno-conversion \
    -Wno-unused-local-typedefs \
    -Wno-unused-variable \

  cxx_standard := -std=c++11
else ifneq (,$(filter $(gcc_version), 6.3.0))
  gcc_version_specific_warnings := \
    -Wno-conversion \

  cxx_standard := -fno-ms-extensions -frounding-math -std=c++17
else ifneq (,$(filter $(gcc_version), 7.2.0 7.3.0))
  gcc_version_specific_warnings := \

  cxx_standard := -fno-ms-extensions -frounding-math -std=c++17
else ifneq (,$(filter $(gcc_version), 8 8.1.0 8.2.0 8.3.0 9 9.3.0))
  gcc_version_specific_warnings := \

  ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_TRIPLET)))
# See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00026.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00000.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00002.html
    tutelary_flag := -fomit-frame-pointer
  endif

  cxx_standard := -fno-ms-extensions -frounding-math -std=c++2a
else ifneq (,$(filter $(gcc_version), 10 10.0))
  gcc_version_specific_warnings := \

  ifeq (x86_64-w64-mingw32,$(findstring x86_64-w64-mingw32,$(LMI_TRIPLET)))
# See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00026.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00000.html
#   https://lists.nongnu.org/archive/html/lmi/2020-12/msg00002.html
#   https://lists.nongnu.org/archive/html/lmi/2021-03/msg00000.html
#   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=99234
# Fixed in gcc-10.2.1, but this makefile doesn't detect the last
# component of major.minor.patchlevel reliably.
    tutelary_flag := -fomit-frame-pointer
    ifneq (,$(filter $(gcc_version), 10 10.2.1))
      tutelary_flag := -fno-omit-frame-pointer
    endif
  endif

  gcc_cxx_warnings += -Wredundant-tags -Wvolatile

  cxx_standard := -fno-ms-extensions -frounding-math -std=c++20
endif

treat_warnings_as_errors := -pedantic-errors -Werror

# Write '-Wno' options at the end.
#
# Rationale for specific warning options:
# -Wno-parentheses [its diagnostics are beyond pedantic]

gcc_common_warnings := \
  $(treat_warnings_as_errors) \
  -Wall \
  -Walloc-zero \
  -Walloca \
  -Wcast-align \
  -Wcast-function-type \
  -Wconversion \
  -Wdangling-else \
  -Wdeprecated-declarations \
  -Wdisabled-optimization \
  -Wdouble-promotion \
  -Wduplicated-branches \
  -Wduplicated-cond \
  -Wextra \
  -Wformat-nonliteral \
  -Wformat-security \
  -Wformat-signedness \
  -Wformat-y2k \
  -Wimport \
  -Winvalid-pch \
  -Wlogical-op \
  -Wmissing-include-dirs \
  -Wmultichar \
  -Wnull-dereference \
  -Wpacked \
  -Wpointer-arith \
  -Wredundant-decls \
  -Wrestrict \
  -Wshadow \
  -Wsign-compare \
  -Wstack-protector \
  -Wswitch-enum \
  -Wtrampolines \
  -Wundef \
  -Wunreachable-code \
  -Wunused-macros \
  -Wvector-operation-performance \
  -Wno-parentheses \

# Warnings that are not generally useful.
#
# -Wdate-time: only for "bit-wise-identical reproducible compilations"
# -Wmissing-declarations: for C++, this flags valid functions that
#   arguably belong in an unnamed namespace
# -Wunsafe-loop-optimizations: incompatible with ranged for-loops

# Consider these later:
postponed_gcc_common_warnings := \

gcc_c_warnings := \
  $(c_standard) \
  $(gcc_common_warnings) \
  -Wbad-function-cast \
  -Wc++-compat \
  -Winit-self \
  -Wjump-misses-init \
  -Wmissing-prototypes \
  -Wnested-externs \
  -Wold-style-definition \
  -Wstrict-prototypes \
  -Wtraditional-conversion \
  -Wunsuffixed-float-constants \
  -Wwrite-strings \

gcc_cxx_warnings := \
  $(cxx_standard) \
  $(gcc_common_warnings) \
  -Wc++11-compat \
  -Wc++14-compat \
  -Wc++1z-compat \
  -Wcatch-value=3 \
  -Wconditionally-supported \
  -Wctor-dtor-privacy \
  -Wdelete-non-virtual-dtor \
  -Wdeprecated \
  -Wextra-semi \
  -Wnoexcept \
  -Wnoexcept-type \
  -Wnon-template-friend \
  -Wnon-virtual-dtor \
  -Wold-style-cast \
  -Woverloaded-virtual \
  -Wplacement-new=2 \
  -Wpmf-conversions \
  -Wregister \
  -Wreorder \
  -Wstrict-null-sentinel \
  -Wsuggest-override \
  -Wsynth \
  -Wuseless-cast \
  -Wzero-as-null-pointer-constant \

# Warnings that are not generally useful.
#
# -Wsign-promo: too many false positives--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00016.html
# -Wsuggest-final-methods, and
# -Wsuggest-final-types: use these only occasionally, like -Weffc++;
#   work with '-Wsuggest-final-types' first, because making a class
#   final may resolve '-Wsuggest-final-methods' suggestions for its
#   members; but expect many false positives
# -Wfloat-equal: too many warnings on correct code, e.g.,
#   exact comparison to zero

# Consider these later:
postponed_gcc_cxx_warnings := \

gcc_common_extra_warnings := \
  -Wcast-qual \

bourn_cast_test.o: gcc_common_extra_warnings += \
  -Wno-double-promotion \

$(cgicc_objects): gcc_common_extra_warnings += \
  -Wno-conversion \
  -Wno-zero-as-null-pointer-constant \

ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
  ifeq (3.4.5,$(gcc_version))
    expression_template_0_test.o: gcc_common_extra_warnings += -Wno-unused-parameter
  endif
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

# XMLWRAPP !! Remove these workarounds after updating xmlwrapp. See:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00018.html
# et seqq.:
$(xmlwrapp_objects): gcc_common_extra_warnings += \
  -Wno-conversion \

# SOMEDAY !! Address some of these '-Wconversion' issues.

wno_conv_objects := \
  rate_table.o \
  round_glibc.o \

$(wno_conv_objects): gcc_common_extra_warnings += -Wno-conversion -Wfloat-conversion

wno_sign_conv_objects := \
  $(boost_regex_objects) \
  $(wx_dependent_objects) \
  crc32.o \
  getopt.o \
  md5.o \
  round_glibc.o \

# '-Wsign-conversion' is generally unusable with C++ because the STL
# defectively uses unsigned types. It is useful to enable it from
# time to time and filter the output thus:
#   grep 'error:' | sed -e '/size_type/d'

$(wno_sign_conv_objects): gcc_common_extra_warnings += -Wno-sign-conversion

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
else ifeq (safestdlib,$(findstring safestdlib,$(build_type)))
  optimization_flag := -O0 -fno-omit-frame-pointer
  libstdcxx_warning_macros := $(every_libstdcxx_warning_macro)
else
  optimization_flag := -O2 -fno-omit-frame-pointer
endif

# An overriding version of 'my_prod.cpp', which is used to create a
# nondistributable binary, contains so many large strings that, after
# consuming more than one CPU minute and 1 MiB of RAM, MinGW gcc-3.4.5
# produced a diagnostic such as
#   warning: NULL pointer checks disabled:
#   39933 basic blocks and 167330 registers
# which was historically prevented by specifying '-O0'. In 2019-02,
# however, using gcc-7.3, with many more products in 'my_*.cpp' files,
# building with '-O0' caused a stack overflow at run time with 32-bit
# msw, which can be prevented by specifying any optimization option
# except '-O0'. That stack overflow seemed to be caused by adding
# twenty-one 2017 CSO products to 'my_db.cpp' in particular; however,
# after the same new products were added to 'my_prod.cpp' and that
# file was compiled with '-O0', the stack overflow occurred no matter
# what flags 'my_db.cpp' was compiled with--presumably because all
# linked objects share the same stack and it's their total size that
# matters.
#
# The '-fno-var-tracking-assignments' flag avoids a specific compiler
# diagnostic in 'my_prod.cpp', which contains a very large number of
# lengthy strings. That flag is specified explicitly here to ensure
# that it's still used even if the '-O' flag is someday changed, even
# though gcc currently documents that any optimization flag other than
# '-O0' implies it.

# For simplicity and robustness, the same options are used for all
# 'my_*.cpp' files.

product_file_sources := my_db.o my_fund.o my_prod.o my_rnd.o my_tier.o

product_file_flags := -Os -fno-var-tracking-assignments -fno-omit-frame-pointer

$(product_file_sources): optimization_flag += $(product_file_flags)

# $(optimization_flag) is part of the default $(CXXFLAGS), but a
# target-specific assignment there isn't enough, because it is too
# easily overridden by specifying $(CXXFLAGS) on the command line.
# This flag overrides any such override:

$(product_file_sources): tutelary_flag += $(product_file_flags)

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
  $(xml_libraries) \

wx_ldflags = \
  $(wx_library_paths) $(wx_libraries) \
  $(platform_gui_ldflags) \

wx_pdfdoc_ldflags := \
  -l$(wxcode_basename)_pdfdoc-$(wxcode_version)

################################################################################

# Flags.

# Define uppercase FLAGS recursively for greater flexibility: e.g., so
# that they reflect downstream conditional changes to the lowercase
# (and often immediately-expanded) variables they're composed from.

debug_flag := -ggdb

# MinGW gcc-3.4.2 writes dwarf2 debug records if '-ggdb' is specified,
# but the version of gdb packaged with it expects stabs format.
#
ifeq (3.4.2,$(gcc_version))
  debug_flag := -g
  gcc_version_specific_warnings := -Wno-uninitialized
endif

# 'c_l_flags' are to be used in both compiler and linker commands.
# The gprof '-pg' flag is one example. Another is '-fPIC', which
# pc-linux-gnu requires for '-shared':
#   https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html#DOCF1
# Yet another is 'debug_flag'.

c_l_flags := $(debug_flag) $(gprof_flag)

ifeq (x86_64-pc-linux-gnu,$(LMI_TRIPLET))
  c_l_flags += -fPIC
endif

# As this is written in 2012, lmi is often built on machines with less
# RAM per core than gcc wants. Experiments show that these flags cut
# gcc's RAM appetite by fifty percent, in return for a ten-percent
# speed penalty that can be overcome by increasing parallelism. There
# seems to be no need for them with gcc-4.x, which uses less RAM.

ifeq (gcc,$(LMI_COMPILER))
  ifeq (3.4.5,$(gcc_version))
    ggc_flags := --param ggc-min-expand=25 --param ggc-min-heapsize=32768
  endif
endif

CFLAGS = \
  $(ggc_flags) $(optimization_flag) $(c_l_flags) \

CXXFLAGS = \
  $(ggc_flags) $(optimization_flag) $(c_l_flags) \

LDFLAGS = $(c_l_flags) -Wl,-Map,$@.map \

# Explicitly disable the infelicitous auto-import default. See:
#   http://article.gmane.org/gmane.comp.gnu.mingw.user/19758
#     [2006-05-18T11:38:01Z from Earnie Boyd]
# Do not disable it for MinGW-w64, which seems to require it.

ifeq (3.4.5,$(gcc_version))
  LDFLAGS += -Wl,--disable-auto-import
endif

ifneq (,$(USE_SO_ATTRIBUTES))
  ifeq (mingw32,$(findstring mingw32,$(LMI_TRIPLET)))
    LDFLAGS += -Wl,--disable-auto-import -static-libstdc++
  endif
  actually_used_lmi_so_attributes = -DLMI_USE_SO_ATTRIBUTES $(lmi_so_attributes)
endif

# wx sets _FILE_OFFSET_BITS to 64; this definition is explicitly
# repeated below to make assurance doubly sure--see:
#   https://lists.nongnu.org/archive/html/lmi/2019-03/msg00039.html
# et seq.
#
# The BOOST_STATIC_ASSERT definition seems to belong in CPPFLAGS with
# the other macro definitions. However, writing it there elicits:
#   warning: ISO C99 requires whitespace after the macro name
# which may simply be a gnu CPP defect--the documentation:
#   https://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html
# says "-D'name(args...)=definition' works", and adding a blank either
# before or after '=' is an error.

REQUIRED_CPPFLAGS = \
  $(addprefix -I , $(lmi_include_directories)) \
  $(addprefix -isystem , $(sys_include_directories)) \
  $(lmi_wx_new_so_attributes) \
  $(actually_used_lmi_so_attributes) \
  $(platform_defines) \
  $(libstdcxx_warning_macros) \
  $(wx_predefinitions) \
  -D_FILE_OFFSET_BITS=64 \
  -DBOOST_NO_AUTO_PTR \
  -DBOOST_NO_STD_ALLOCATOR \
  -DBOOST_STRICT_CONFIG \
  -DBOOST_STATIC_ASSERT_HPP \

REQUIRED_CFLAGS = \
  $(C_WARNINGS) \

REQUIRED_CXXFLAGS = \
  $(CXX_WARNINGS) \
  -D'BOOST_STATIC_ASSERT(A)=static_assert((A))' \

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

# Architecture-specific directories $(locallibdir) and $(localbindir)
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
  $(locallibdir) \
  $(localbindir) \

EXTRA_LDFLAGS :=

REQUIRED_LDFLAGS = \
  $(addprefix -L , $(all_library_directories)) \
  $(EXTRA_LDFLAGS) \
  $(REQUIRED_LIBS) \
  $(EXTRA_LIBS) \

# The '--use-temp-file' windres option seems to be often helpful and
# never harmful.
#
# As seems customary, $(ALL_CPPFLAGS) is passed to 'windres', which
# doesn't recognize '-isystem'--hence the $(subst) workaround.

REQUIRED_RCFLAGS = \
  $(subst -isystem,--include-dir,$(ALL_CPPFLAGS)) \
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
# And dll attributes would still be necessary for other toolchains,
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
#
# Architecture-independent files are installed with '-c'.
# Architecture-dependent files are installed without '-c'.
# Of course, '-c' is ignored; it flags situations where '-C'
# might be useful.

data_files := \
  $(wildcard $(addprefix $(srcdir)/,*.ico *.png *.xml *.xrc *.xsd *.xsl)) \

help_files := \
  $(wildcard $(addprefix $(srcdir)/,*.html)) \

installable_binaries := \
  $(default_targets) \
  $(wildcard $(localbindir)/*$(SHREXT)) \
  $(wildcard $(locallibdir)/*$(SHREXT)) \

.PHONY: install
install: $(default_targets)
	+@[ -d $(exec_prefix)    ] || $(MKDIR) --parents $(exec_prefix)
	+@[ -d $(bindir)         ] || $(MKDIR) --parents $(bindir)
	+@[ -d $(datadir)        ] || $(MKDIR) --parents $(datadir)
	+@[ -d $(test_dir)       ] || $(MKDIR) --parents $(test_dir)
	+@[ -d $(touchstone_dir) ] || $(MKDIR) --parents $(touchstone_dir)
	@$(INSTALL) -m 0775 $(installable_binaries) $(bindir)
	@$(INSTALL) -c -m 0664 $(data_files) $(datadir)
	@$(INSTALL) -c -m 0664 $(help_files) $(datadir)
	@datadir=$(datadir) srcdir=$(srcdir) $(srcdir)/mst_to_xst.sh
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
  sample*.database \
  sample*.funds \
  sample*.lingo \
  sample.ndx \
  sample*.policy \
  sample*.rounding \
  sample*.strata \

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
# Planned improvements:
#  - include a source archive automatically
#  - copy 'configurable_settings.xml' from a configurable location
#  - optionally build a tarball containing no subdirectory
#  - optionally remove the dependency on 'install'
#      (or keep it from running 'product_files' imperatively)
#  - separate $(shared_data_files) out of $(fardel_files) below,
#      making it independently overridable
#  - consider making $(fardel_dir) local to 'fardel:', because
#      it probably can't be overridden

# To create a customized package, override one or more of:
#  - fardel_name
#  - extra_fardel_binaries
#  - extra_fardel_files
#  - extra_fardel_checksummed_files
# thus:
#   make fardel_name=ThisWayIsGood fardel
# and not thus:
#   fardel_name=DoNotDoThisForItWillNotWork make fardel

fardel_name := lmi-$(yyyymmddhhmm)
fardel_root := $(prefix)/fardels
fardel_dir  := $(fardel_root)/$(fardel_name)

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
  $(wildcard $(localbindir)/*$(SHREXT)) \
  $(wildcard $(locallibdir)/*$(SHREXT)) \
  $(wildcard $(bindir)/lmi_md5sum$(EXEEXT)) \
  $(wildcard $(bindir)/product_files$(EXEEXT)) \
  $(extra_fardel_binaries) \

fardel_files := \
  $(addprefix $(datadir)/,$(shared_data_files)) \
  $(addprefix $(datadir)/,*.xst) \
  $(data_files) \
  $(help_files) \
  $(extra_fardel_files) \

# Sensitive files are authenticated at run time.
#
# MD5 !! A native 'lmi_md5sum$(EXEEXT)' is provided because lmi once
# used it for run-time authentication, and still uses it temporarily
# for experimental timings. Once that experiment concludes, remove it
# from $(fardel_checksummed_files) and $(fardel_binaries).
#
# Binary files other than 'lmi_md5sum$(EXEEXT)' are not authenticated
# because they aren't easily forged but are sizable enough to make
# authentication too slow. An incorrect version of any such file might
# be distributed by accident, but that problem would not be caught by
# generating an md5sum for the incorrect file. 'lmi_md5sum$(EXEEXT)'
# is however authenticated because replacing it with a program that
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
  *.dat *.database *.funds *.lingo *.ndx *.policy *.rounding *.strata *.xst \
  expiry \
  lmi_md5sum$(EXEEXT) \

.PHONY: fardel
fardel: install
	+@[ -d $(fardel_dir) ] || $(MKDIR) --parents $(fardel_dir)
	@$(MAKE) --file=$(this_makefile) --directory=$(fardel_dir) wrap_fardel
	@$(ECHO) "Created '$(fardel_name)' archive in '$(fardel_root)'."

.PHONY: wrap_fardel
wrap_fardel:
	@$(INSTALL) -m 0664 $(datadir)/configurable_settings.xml .
	@$(INSTALL) -m 0664 $(datadir)/company_logo.png .
	@$(INSTALL) -m 0664 $(datadir)/group_quote_banner.png .
	@$(INSTALL) -m 0775 $(fardel_binaries) .
	@$(INSTALL) -m 0664 $(fardel_files) .
	@$(fardel_date_script)
	@$(MD5SUM) --binary $(fardel_checksummed_files) >validated.md5
	@$(PERFORM) $(bindir)/generate_passkey > passkey
	@$(TAR) \
	  --bzip2 \
	  --create \
	  --directory=$(fardel_root) \
	  --file=$(fardel_root)/$(fardel_name).tar.bz2 \
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
	@$(INSTALL) -c -m 0664 $< .

sample.ill: $(srcdir)/sample.ill
	@$(INSTALL) -c -m 0664 $< .

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
	@$(INSTALL) -c -m 0664 $(datadir)/$@ .

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
	@printf '\n%s\n' "Running $*:"
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

.PHONY: cli_timing
cli_timing: lmi_cli_shared$(EXEEXT)
	@$(PERFORM) ./lmi_cli_shared$(EXEEXT) $(self_test_options) \
	  >$(srcdir)/Speed_$(LMI_COMPILER)_$(LMI_TRIPLET)

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
# historical versions in its own way). An undatestamped copy of the
# analysis, stripped of descriptive text and sorted in descending
# order, is saved as 'regressions.tsv'.

system_test_analysis    := $(test_dir)/analysis-$(yyyymmddhhmm)
system_test_regressions := $(test_dir)/regressions.tsv
system_test_diffs       := $(test_dir)/diffs-$(yyyymmddhhmm)
system_test_md5sums     := $(test_dir)/md5sums-$(yyyymmddhhmm)
system_test_md5sums2    := $(test_dir)/md5sums

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
	@$(INSTALL) -c -m 0664 $(system_test_md5sums) $(system_test_md5sums2)
	@-< $(system_test_analysis) $(SED) \
	  -e '/rel err.*e-0*1[5-9]/d' \
	  -e '/abs.*0\.00.*rel/d' \
	  -e '/abs diff: 0 /d'
	@-< $(system_test_analysis) $(SED) \
	  -e 's/   Summary: max abs diff: /\t/' \
	  -e 's/ max rel err:  /\t/' \
	  | $(SORT) --key=2gr --key=3gr \
	  > $(system_test_regressions)
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

# Report gcc version.
#
# Intended for use in shell scripts thus:
#   gcc_version=$(make show_gcc_version)
# so no newline is printed.

.PHONY: show_gcc_version
show_gcc_version:
	@printf "$(gcc_version)"

# Display selected variables.

.PHONY: show_flags
show_flags:
	@printf 'gcc_version             = "%s"\n' "$(gcc_version)"
	@printf 'gnu_cpp_version         = "%s"\n' "$(gnu_cpp_version)"
	@printf 'gnu_cxx_version         = "%s"\n' "$(gnu_cxx_version)"
	@printf 'ALL_CPPFLAGS            = "%s"\n' "$(ALL_CPPFLAGS)"
	@printf 'ALL_CFLAGS              = "%s"\n' "$(ALL_CFLAGS)"
	@printf 'ALL_CXXFLAGS            = "%s"\n' "$(ALL_CXXFLAGS)"
	@printf 'ALL_ARFLAGS             = "%s"\n' "$(ALL_ARFLAGS)"
	@printf 'ALL_LDFLAGS             = "%s"\n' "$(ALL_LDFLAGS)"
	@printf 'ALL_RCFLAGS             = "%s"\n' "$(ALL_RCFLAGS)"
	@printf 'srcdir                  = "%s"\n' "$(srcdir)"
	@printf 'lmi_include_directories = "%s"\n' "$(lmi_include_directories)"
	@printf 'sys_include_directories = "%s"\n' "$(sys_include_directories)"
	@printf 'all_source_directories  = "%s"\n' "$(all_source_directories)"
	@printf 'wx_include_paths        = "%s"\n' "$(wx_include_paths)"
	@printf 'wx_libraries            = "%s"\n' "$(wx_libraries)"
	@printf 'wx_library_paths        = "%s"\n' "$(wx_library_paths)"
	@printf 'wx_predefinitions       = "%s"\n' "$(wx_predefinitions)"

.PHONY: show_disabled_g++_warnings
show_disabled_g++_warnings:
	$(CXX) $(ALL_CXXFLAGS) -Q --help=warning | $(GREP) '[[]disabled[]]'
