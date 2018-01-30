// Output mode enum used in PDF generation helpers.
//
// Copyright (C) 2017, 2018 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef output_mode_hpp
#define output_mode_hpp

#include "config.hpp"

/// Convenient enum used with functions that can either actually render
/// something or just pretend doing it in order to compute the space that would
/// be taken by it, in the layout phase.
enum enum_output_mode
    {e_output_normal
    ,e_output_measure_only
    };

#endif // output_mode_hpp
