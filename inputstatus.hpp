// Input parameters.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: inputstatus.hpp,v 1.9 2008-07-02 14:49:17 chicares Exp $

#ifndef inputstatus_hpp
#define inputstatus_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "xenumtypes.hpp"
#include "xrangetypes.hpp"

#include <iosfwd>
#include <vector>

// Parameters for a single life that could differ for other lives on
// a multilife policy.

// Implicitly-declared special member functions do the right thing.

class LMI_SO InputStatus
    :virtual private obstruct_slicing<InputStatus>
{
  public:
    InputStatus();
    ~InputStatus();

    bool operator==(InputStatus const&) const;
    bool operator!=(InputStatus const&) const;

    int YearsToRetirement() const;
    bool MakeAgesAndDatesConsistent
        (calendar_date const& EffDate
        ,bool                 UseALB
        ) const;
    bool AreFlatExtrasPresent() const;
    bool IsPolicyRated() const;

// TODO ?? Data should be private.
    r_iss_age            IssueAge;
    r_ret_age            RetAge;
    e_gender             Gender;
    e_smoking            Smoking;
    e_class              Class;
    e_yes_or_no          HasWP;
    e_yes_or_no          HasADD;
    e_yes_or_no          HasTerm;
    r_nonnegative_double TermAmt;
    r_nonnegative_double TotalSpecAmt;
    r_proportion         TermProportion;
    e_yes_or_no          TermUseProportion;

    r_date               DOB;            // Date of birth
    e_yes_or_no          UseDOB;
    r_date               DOR;            // Date of retirement
    e_yes_or_no          UseDOR;

    e_table_rating       SubstdTable;

    std::vector<double>  VectorMonthlyFlatExtra;
};

#endif // inputstatus_hpp

