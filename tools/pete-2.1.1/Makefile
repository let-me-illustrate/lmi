# PETE with std::vector: makefile.
#
# Copyright (C) 2008 Gregory W. Chicares.
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

################################################################################

lmi_build_type := $(shell /usr/share/misc/config.guess)

EXEEXT :=
ifeq (cygwin,$(findstring cygwin,$(lmi_build_type)))
  EXEEXT := .exe
endif

all: et_vector_test$(EXEEXT)

submake_options := \
  --directory=PETE/Tools/ \
  --file=makefile \
  EXEEXT='$(EXEEXT)' \

et_vector_test$(EXEEXT): et_vector_test.cpp
	$(CXX) -I. $^ -o $@

et_vector_test.cpp: et_vector_operators.hpp

et_vector_operators.hpp: et_vector.in PETE/Tools/MakeOperators$(EXEEXT)
	PETE/Tools/MakeOperators$(EXEEXT) \
	  --guard et_vector_operators_hpp \
	  --classes et_vector.in \
	  --assign-ops \
	  --o $@

PETE/Tools/MakeOperators$(EXEEXT):
	$(MAKE) $(submake_options) clean
	$(MAKE) $(submake_options) newpeteops
	$(MAKE) $(submake_options) installheaders

clean:
	rm -f et_vector_test$(EXEEXT)

distclean: clean
	rm -f PETE/Tools/MakeOperators$(EXEEXT)

maintainer-clean: distclean
	rm -f et_vector_operators.hpp
