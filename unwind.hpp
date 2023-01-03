// C++ exception unwinder for pc-linux-gnu.
//
// Copyright (C) 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef unwind_hpp
#define unwind_hpp

#include "config.hpp"

extern bool g_unwind;

class scoped_unwind_toggler
{
  public:
    scoped_unwind_toggler(bool z = false)
        :original_g_unwind {g_unwind}
        {
        g_unwind = z;
        }
    ~scoped_unwind_toggler()
        {
        g_unwind = original_g_unwind;
        }

  private:
    bool const original_g_unwind = g_unwind;
};

#endif // unwind_hpp
