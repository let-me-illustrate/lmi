// Manage floating-point environment.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef fenv_lmi_hpp
#define fenv_lmi_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#if defined LMI_X87
#   include "fenv_lmi_x86.hpp"
#endif // defined LMI_X87

/// These functions manage the floating-point environment.
///
///   void fenv_initialize();
/// Initialize floating-point environment to lmi default settings.
///
///   e_ieee754_rounding fenv_rounding();
///   void fenv_rounding(e_ieee754_rounding);
/// The rounding-mode functions are similar to C99 fe[gs]etround().
/// Because this is C++, not C, no 'get-' and 'set-' lexemes are
/// needed to simulate overloading, and exceptions obviate return
/// codes (although it's not clear how such a function could ever fail
/// with a valid argument, and invalid arguments can be disallowed by
/// C++'s type system).
///
///   e_ieee754_precision fenv_precision();
///   void fenv_precision(e_ieee754_precision);
/// The precision functions similarly resemble WG14 N751/J11 functions
/// fe[gs]etprecision(). At least for now, they are meaningfully
/// implemented for x87 only.
///
///   LMI_SO bool fenv_is_valid()
/// If current floating-point environment matches lmi default, then
/// return 'true'; else return 'false'.
///
///   LMI_SO bool fenv_validate();
/// Make sure current floating-point environment matches lmi default.
/// If it doesn't, then reset it to lmi default, display a message,
/// and return 'false'; else return 'true'.
///
/// Postcondition: floating-point environment matches lmi default.
///
/// Throws: nothing, unless displaying the error message fails, e.g.
/// due to lack of memory. Failure is indicated by a return code
/// rather than an exception, so that this function can be called at
/// program exit, outside any try block, without abending, which is
/// crucial for GUI frameworks that don't handle exceptions
/// gracefully during normal termination.

namespace floating_point_environment {} // doxygen workaround.

// Pixilated enumerators foster rigor, as they are unlikely to work
// by accident.

enum e_ieee754_precision
    {fe_fltprec  = 887
    ,fe_dblprec  = 883
    ,fe_ldblprec = 881
    };

enum e_ieee754_rounding
    {fe_tonearest  = 88811
    ,fe_downward   = 88813
    ,fe_upward     = 88817
    ,fe_towardzero = 88819
    };

enum enum_fenv_indulgence
    {e_fenv_indulge_nothing = 0
    ,e_fenv_indulge_0x027f  = 0x027f
    };

LMI_SO void fenv_initialize();

LMI_SO e_ieee754_precision fenv_precision();
LMI_SO void                fenv_precision(e_ieee754_precision);

LMI_SO e_ieee754_rounding fenv_rounding();
LMI_SO void               fenv_rounding(e_ieee754_rounding);

LMI_SO bool fenv_is_valid();
LMI_SO bool fenv_validate(enum_fenv_indulgence = e_fenv_indulge_nothing);

#endif // fenv_lmi_hpp
