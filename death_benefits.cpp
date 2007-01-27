// Death benefits.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: death_benefits.cpp,v 1.4 2007-01-27 00:00:51 wboutin Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "death_benefits.hpp"

#include "alert.hpp"
#include "basic_values.hpp"
#include "inputs.hpp"

#include <algorithm> // std::min()

//============================================================================
death_benefits::death_benefits(BasicValues const& values)
    :length_(values.GetLength())
{
    // In the antediluvian branch, the vector in the input class
    // is padded to a greater length.
    LMI_ASSERT(length_ <= static_cast<int>(values.Input_->DBOpt  .size()));
    LMI_ASSERT(length_ <= static_cast<int>(values.Input_->SpecAmt.size()));

    // Can't use std::copy() because types differ, length issues aside.
    dbopt_  .resize(length_);
    specamt_.resize(length_);
    for(int j = 0; j < length_; ++j)
        {
        dbopt_  [j] = values.Input_->DBOpt  [j]        ;
        specamt_[j] = values.Input_->SpecAmt[j].value();
        }
}

//============================================================================
death_benefits::~death_benefits()
{
}

//============================================================================
void death_benefits::set_specamt(double amount, int begin_year, int end_year)
{
    // Can't use a standard algorithm because types differ.
    for(int j = begin_year; j < std::min(length_, end_year); j++)
        {
        specamt_[j] = amount;
        }
}

