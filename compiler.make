# Include compiler-specific makefile.
#
# Copyright (C) 2022 Gregory W. Chicares.
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

# Other makefiles included; makefiles not to be remade.

# Don't remake this makefile.

$(srcdir)/compiler.make:: ;

ifeq      (gcc,$(LMI_COMPILER))
  compiler_makefile := compiler_gcc.make
else ifeq (clang,$(LMI_COMPILER))
  compiler_makefile := compiler_clang.make
else
  $(warning Untested compiler '$(LMI_COMPILER)')
endif

include $(srcdir)/$(compiler_makefile)
$(srcdir)/$(compiler_makefile):: ;
