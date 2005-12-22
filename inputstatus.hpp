// Input parameters.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: inputstatus.hpp,v 1.4 2005-12-22 13:59:49 chicares Exp $

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
    // TODO ?? This shouldn't be const. The
    // Date of birth and issue age are two facets of what must be the same
    // quantity. The same holds for date or age of retirement. When the
    // underlying quantity is changed by manipulating either facet, the
    // other facet must be changed automatically to maintain consistency.
    //
    // We need to do this in some cases where we have a const reference to
    // the InputStatus object; using const_cast here seemed preferable to
    // making several member variables mutable, which would let anyone
    // modify them. The ugliness of the conversion to a non-const non-template
    // type should give you pause before you consider using this technique.
    //
    // TODO ?? This damage should be undone. Only the ihs 'valtype' stuff
    // 'needs' to call this through a const reference.
    //
    // Why not look up the ALB/ANB here in the database? Because we don't
    // necessarily know all database initialization parameters yet--or do we?
    void MakeAgesAndDatesConsistent
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

