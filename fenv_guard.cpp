// Guard floating-point environment.
//
// Copyright (C) 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "fenv_guard.hpp"

#include "fenv_lmi.hpp"

fenv_guard::fenv_guard()
{
    instance_count_++;
    fenv_initialize();
}

fenv_guard::~fenv_guard()
{
    try
        {
        instance_count_--;
        fenv_validate(e_fenv_indulge_nothing);
        }
    catch(...)
        {
        }
}

int fenv_guard::instance_count()
{
    return instance_count_;
}

int fenv_guard::instance_count_ = 0;

