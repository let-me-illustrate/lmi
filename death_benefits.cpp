// Death benefits.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "death_benefits.hpp"

#include "assert_lmi.hpp"
#include "yare_input.hpp"

#include <algorithm>                    // min()

//============================================================================
death_benefits::death_benefits
    (int                     length
    ,yare_input       const& yi
    ,round_to<double> const& round_specamt
    )
    :length_        {length}
    ,round_specamt_ {round_specamt}
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
        dbopt_   [j] =                yi.DeathBenefitOption[j];
        specamt_ [j] = round_specamt_(yi.SpecifiedAmount   [j]);
        supplamt_[j] = round_specamt_(yi.SupplementalAmount[j]);
        }
}

//============================================================================
void death_benefits::set_specamt(double z, int from_year, int to_year)
{
#if 0
    // Something like this would seem preferable, but it gives
    //   Assertion 'to_year < length_' failed.
    // with 'make cli_selftest' due to the issue noted in the ctor.
    LMI_ASSERT(0 <= from_year);
    LMI_ASSERT(     from_year <= to_year);
    LMI_ASSERT(                  to_year < length_);
    std::fill_n(specamt_.begin() + from_year, to_year - from_year, z);
#endif // 0
    z = round_specamt_(z);
    for(int j = from_year; j < std::min(length_, to_year); ++j)
        {
        specamt_[j] = z;
        }
}

//============================================================================
void death_benefits::set_supplamt(double z, int from_year, int to_year)
{
#if 0
    // Something like this would seem preferable, but it gives
    //   Assertion 'to_year < length_' failed.
    // with 'make cli_selftest' due to the issue noted in the ctor.
    LMI_ASSERT(0 <= from_year);
    LMI_ASSERT(     from_year <= to_year);
    LMI_ASSERT(                  to_year < length_);
    std::fill_n(supplamt_.begin() + from_year, to_year - from_year, z);
#endif // 0
    z = round_specamt_(z);
    for(int j = from_year; j < std::min(length_, to_year); ++j)
        {
        supplamt_[j] = z;
        }
}
