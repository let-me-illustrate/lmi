# Copyright (C) 2002, 2003, 2004 Gregory W. Chicares.
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
# http://groups.yahoo.com/group/actuarialsoftware
# email: <chicares@mindspring.com>
# snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

# $Id: GNUmakefile,v 1.1.1.1 2004-05-15 19:58:38 chicares Exp $

MAKEFLAGS = \
  --keep-going \
  --no-builtin-variables \
  --no-builtin-rules \

all: wx_new.dll lmi.exe

# Create dummy libraries
# C:/wx-lmi/lmi[0]$cp --preserve /MinGW/lib/libm.a libregex.a
# C:/wx-lmi/lmi[0]$cp --preserve /MinGW/lib/libm.a libpng.a
# C:/wx-lmi/lmi[0]$cp --preserve /MinGW/lib/libm.a libjpeg.a
# C:/wx-lmi/lmi[0]$cp --preserve /MinGW/lib/libm.a libzlib.a
# C:/wx-lmi/lmi[0]$cp --preserve /MinGW/lib/libm.a libtiff.a

WXDIR = /wxWindows-2.5.1

OBJECTS = \
  docmanager_ex.o \
  docmdichildframe_ex.o \
  html_doc.o \
  html_view.o \
  input.o \
  input_datum.o \
  lmi.o \
  lmi.rc.o \
  mc_enum_types.o \
  previewframe_ex.o \
  text_doc.o \
  text_view.o \
  transferor.o \
  view_ex.o \
  wx_checks.o \
  xml_notebook.o \

system_root   = C:

# Compiling tools.

mingw_dir     = $(system_root)/MinGW/bin
CC            = $(mingw_dir)/gcc
CPP           = $(mingw_dir)/cpp
CXX           = $(mingw_dir)/g++
RC            = $(mingw_dir)/windres

# Other utilities.

gnu_utils_dir = $(system_root)/gnu
BZIP2         = $(gnu_utils_dir)/bzip2
CP            = $(gnu_utils_dir)/cp
DATE          = /cygwin/bin/date
MKDIR         = $(gnu_utils_dir)/mkdir
RM            = $(gnu_utils_dir)/rm
SED           = $(gnu_utils_dir)/sed
TAR           = $(gnu_utils_dir)/tar
TOUCH         = $(gnu_utils_dir)/touch

# Autodependencies.

MAKEDEPEND_0 = \
  -MD -MF $*.d0 \

MAKEDEPEND_1 = \
  < $*.d0 \
  $(SED) \
  -e :a -e '/\\$$/N; s/\\\n//; ta' \
  -e 'h' \
  -e 's/^[^:]*: *//' \
  -e 's/$$/ :/' \
  -e 'G' \
  $(comp_autodependency_kludge) \
  > $*.d; $(RM) $*.d0

-include *.d
*.d: ;

# Settings.

CPPFLAGS = \
  -DWINVER=0x0400 \
  -D__WIN95__ \
  -DSTRICT \
  -DHAVE_W32API_H \
  -D__WXMSW__ \
  -D__WINDOWS__ \
  -D__WXDEBUG__ \
  -DWXUSINGDLL \
  -DDLL_NEW_USING_DLL \
  -DNO_GCC_PRAGMA \
  -DwxUSE_STL \
  -I $(WXDIR)/lib/gcc_dll/mswd \
  -I $(WXDIR)/include \
  -I $(WXDIR)/contrib/include \
  -I /boost/boost_1_31_0 \

gcc_warnings = \
  -pedantic \
  -std=c++98 \
  -Wall \
  -Wcast-align \
  -Wconversion \
  -Wdeprecated \
  -Wimport \
  -Wmissing-prototypes \
  -Wnon-template-friend \
  -Woverloaded-virtual \
  -Wpmf-conversions \
  -Wpointer-arith \
  -Wsign-compare \
  -Wsynth \
  -Wwrite-strings \

# Some boost libraries treat 'long long' as part of the language,
# which it probably soon will be.  
gcc_warnings += -Wno-long-long

# WX!! The wx library triggers many warnings with these flags:
gcc_strong_warnings = \
  $(gcc_warnings) \
  -W \
  -Wcast-qual \

# Boost deems that everyone should turn off this warning flag.
#  -Wundef \

CXXFLAGS = \
  $(gcc_warnings) \
  -ggdb \

RCFLAGS = \
  $(subst -I,--include-dir ,$(CPPFLAGS)) \
  --use-temp-file \

%.o: %.cpp
	$(CXX) $(MAKEDEPEND_0) -c $(CPPFLAGS) $(CXXFLAGS) $< -o$@
	$(MAKEDEPEND_1)

%.rc.o: %.rc
	$(RC) $(RCFLAGS) -i $< -o $@

# Suppress the next line to prevent using mpatrol.
# Beware: with mpatrol linked, gdb signals errors on application shutdown.
MPATROL_LIBS = -L$(system_root)/local/lib -lmpatrol -lbfd -liberty -limagehlp

LIBS = \
  $(MPATROL_LIBS) \
  -L $(WXDIR)/lib/gcc_dll \
  -lwx_new \
  -lwxmsw25d_xrc \
  -lwxmsw25d \
  -lstdc++ -lgcc -lodbc32 -lwsock32 -lwinspool -lwinmm -lshell32 \
  -lcomctl32 -lctl3d32 -lodbc32 -ladvapi32 -lodbc32 -lwsock32 \
  -lopengl32 -lglu32 -lole32 -loleaut32 -luuid \

LDFLAGS = -L. -L$(WXDIR)/lib -mwindows -Wl,-Map,map

lmi.exe: $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

wx_new.dll: CPPFLAGS = -DDLL_NEW_BUILDING_DLL
wx_new.dll: wx_new.o
	$(CXX) -shared -Wl,--out-implib,lib$(basename $@).a $^ -o$@

# Unit tests.

unit_test_targets = \
  any_member_test.exe \
  mc_enum_test.exe \
  wx_new_test.exe \

.PHONY: unit_tests
unit_tests: CXXFLAGS = $(gcc_strong_warnings) -ggdb
unit_tests: $(unit_test_targets)
	for z in $(unit_test_targets); { ./$$z; }

any_member_test.exe: any_member_test.o
	$(CXX) -o $@ $^ $(MPATROL_LIBS)

mc_enum_test.exe: mc_enum_test.o mc_enum_test_aux.o
	$(CXX) -o $@ $^ $(MPATROL_LIBS)

wx_new_test.exe: wx_new_test.o
	$(CXX) -o $@ $^ $(MPATROL_LIBS)

# Test header idempotence.

headers = $(wildcard *.hpp)
templates = $(wildcard *.tpp)

.PHONY: check_idempotence
check_idempotence: $(headers:.hpp=.idh) $(templates:.tpp=.idt)

%.idh: %.hpp
	@$(CXX) \
	  -DLMI_IGNORE_PCH \
	  $(CPPFLAGS) $(CXXFLAGS) \
	  -x c++ -w -O0 -fsyntax-only $<
	@$(TOUCH) $@

%.idt: %.tpp
	@$(CXX) \
	  -DLMI_IGNORE_PCH \
	  $(CPPFLAGS) $(CXXFLAGS) \
	  -x c++ -w -O0 -fsyntax-only $<
	@$(TOUCH) $@

clean:
	-$(RM) --force '*.bak'
	-$(RM) --force '*.d'
	-$(RM) --force '*.d0'
	-$(RM) --force '*.ii'
	-$(RM) --force '*.mbt'
	-$(RM) --force '*.mrt'
	-$(RM) --force '*.o'
	-$(RM) --force '*.obj'
	-$(RM) --force '*.ps'
	-$(RM) --force '*.res'
	-$(RM) --force '*.rsc'
	-$(RM) --force '*.tds'
	-$(RM) --force '*.ti'
	-$(RM) --force core
	-$(RM) --force map

mostlyclean: clean
	-$(RM) --force '*.exe'
	-$(RM) --force 'mpatrol.log*'
	-$(RM) --force libwx_new.a
	-$(RM) --force wx_new.dll

# If there are no idempotence problems, then each '.id[ht]' file is
# zero bytes long--so there's no particular need to erase them, while
# they do bear significant information and would take time to rebuild.
distclean: mostlyclean
	-$(RM) --force '*.idh' '*.idt'

yyyymmddhhmm := $(shell $(DATE) -u +'%Y%m%dT%H%MZ')
archname = wx-lmi-$(yyyymmddhhmm)

archive: distclean
	$(MKDIR) ../$(archname)
	-$(CP) --force --preserve --recursive * ../$(archname)
	$(TAR) --create --directory=.. --file=$(archname).tar $(archname)
	$(RM) --force --recursive ../$(archname)
	$(BZIP2) --verbose --verbose $(archname).tar

