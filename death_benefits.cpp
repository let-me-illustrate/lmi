// Death benefits.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "death_benefits.hpp"

#include "assert_lmi.hpp"
#include "yare_input.hpp"

#include <algorithm> // std::min()

//============================================================================
death_benefits::death_benefits(int length, yare_input const& yi)
    :length_(length)
{
    // In the antediluvian branch, the vector in the input class
    // is padded to a greater length.
    LMI_ASSERT(length_ <= static_cast<int>(yi.DeathBenefitOption.size()));
    LMI_ASSERT(length_ <= static_cast<int>(yi.SpecifiedAmount   .size()));
    LMI_ASSERT(length_ <= static_cast<int>(yi.SupplementalAmount.size()));

    // SOMEDAY !! Can't use std::copy() because lengths differ?
    dbopt_   .resize(length_);
    specamt_ .resize(length_);
    supplamt_.resize(length_);
    for(int j = 0; j < length_; ++j)
        {
        dbopt_   [j] = yi.DeathBenefitOption[j];
        specamt_ [j] = yi.SpecifiedAmount   [j];
        supplamt_[j] = yi.SupplementalAmount[j];
        }
}

//============================================================================
death_benefits::~death_benefits()
{
}

//============================================================================
void death_benefits::set_specamt(double z, int from_year, int to_year)
{
//    std::fill_n(specamt_.begin() + from_year, to_year - from_year, z);
    // SOMEDAY !! Can't use a standard algorithm?
    for(int j = from_year; j < std::min(length_, to_year); ++j)
        {
        specamt_[j] = z;
        }
}

//============================================================================
void death_benefits::set_supplamt(double z, int from_year, int to_year)
{
//    std::fill_n(supplamt_.begin() + from_year, to_year - from_year, z);
    // SOMEDAY !! Can't use a standard algorithm?
    for(int j = from_year; j < std::min(length_, to_year); ++j)
        {
        supplamt_[j] = z;
        }
}

