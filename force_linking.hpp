// Force linking despite overly-aggressive linker "optimization".
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
//
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// This is a derived work based on wxWidgets file
//   include/wx/link.h (C) 2005 Vaclav Slavik
// which is covered by the wxWidgets license.
//
// The original was trivially modified by GWC in 2014 to make all
// identifiers specific to lmi; and in any later years given in the
// copyright notice above.

#ifndef force_linking_hpp
#define force_linking_hpp

#include "config.hpp"

/// Immunize the present TU against an "optimizing" linker.
///
/// To immunize 'foo.o', when 'main.o' is sure to be linked, write:
///   LMI_FORCE_LINKING_IN_SITU(foo) // in 'foo.cpp'
///   LMI_FORCE_LINKING_EX_SITU(foo) // in 'main.cpp'

#define LMI_FORCE_LINKING_IN_SITU(translation_unit_name)       \
    extern void lmi_link_dummy_func_##translation_unit_name(); \
    void lmi_link_dummy_func_##translation_unit_name() {}

/// Immunize a different TU against an "optimizing" linker.
///
/// To immunize 'foo.o', when 'main.o' is sure to be linked, write:
///   LMI_FORCE_LINKING_IN_SITU(foo) // in 'foo.cpp'
///   LMI_FORCE_LINKING_EX_SITU(foo) // in 'main.cpp'

#define LMI_FORCE_LINKING_EX_SITU(translation_unit_name)       \
    extern void lmi_link_dummy_func_##translation_unit_name(); \
    static struct lmi_force_link##translation_unit_name        \
    {                                                          \
        lmi_force_link##translation_unit_name()                \
        {                                                      \
            lmi_link_dummy_func_##translation_unit_name();     \
        }                                                      \
    } lmi_link_dummy_var_##translation_unit_name;

#endif // force_linking_hpp
