// Guard floating-point environment.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

// $Id$

#ifndef fenv_guard_hpp
#define fenv_guard_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

/// Guard class for critical floating-point calculations.
///
/// Invariant: the floating-point control word has the desired value.
///
/// ctor: establish the invariant.
///
/// dtor: display an error message if the invariant wasn't maintained.
///
/// Intended use: instantiate on the stack at the beginning of any
/// floating-point calculations that presume the invariant.

class LMI_SO fenv_guard
    :        private lmi::uncopyable <fenv_guard>
    ,virtual private obstruct_slicing<fenv_guard>
{
  public:
    fenv_guard();
    ~fenv_guard();

    static int instance_count();

  private:
    static int instance_count_;
};

#endif // fenv_guard_hpp

