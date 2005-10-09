// Ledger values: common base class.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_base.cpp,v 1.6 2005-10-09 22:47:27 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_base.hpp"

#include "alert.hpp"
#include "crc32.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <cmath> // std::pow()
#include <functional>
#include <numeric>
#include <string>

//============================================================================
LedgerBase::LedgerBase(int a_Length)
    :m_scaling_factor(1.0)
    ,m_scale_unit("")
{
    Initialize(a_Length);
}

//============================================================================
LedgerBase::LedgerBase(LedgerBase const& obj)
    :m_scaling_factor(obj.m_scaling_factor)
    ,m_scale_unit(obj.m_scale_unit)
{
    Initialize(obj.GetLength());
    Copy(obj);
}

//============================================================================
LedgerBase& LedgerBase::operator=(LedgerBase const& obj)
{
    if(this != &obj)
        {
        Initialize(obj.GetLength());
        Copy(obj);
        }
    return *this;
}

//============================================================================
LedgerBase::~LedgerBase()
{
}

//============================================================================
void LedgerBase::Alloc()
{
    ScalableVectors.insert(BegYearVectors   .begin(), BegYearVectors    .end());
    ScalableVectors.insert(EndYearVectors   .begin(), EndYearVectors    .end());
    ScalableVectors.insert(ForborneVectors  .begin(), ForborneVectors   .end());

    AllVectors.insert(BegYearVectors        .begin(), BegYearVectors    .end());
    AllVectors.insert(EndYearVectors        .begin(), EndYearVectors    .end());
    AllVectors.insert(ForborneVectors       .begin(), ForborneVectors   .end());
    AllVectors.insert(OtherVectors          .begin(), OtherVectors      .end());

    AllScalars.insert(ScalableScalars       .begin(), ScalableScalars   .end());
    AllScalars.insert(OtherScalars          .begin(), OtherScalars      .end());
}

//============================================================================
void LedgerBase::Initialize(int a_Length)
{
    for
        (double_vector_map::iterator i = AllVectors.begin()
        ;i != AllVectors.end()
        ;i++
        )
        {
        (*i).second->assign(a_Length, 0.0);
        }

    for
        (scalar_map::iterator i = AllScalars.begin()
        ;i != AllScalars.end()
        ;i++
        )
        {
        *(*i).second = 0.0;
        }
}

//============================================================================
void LedgerBase::Copy(LedgerBase const& obj)
{
    // We do not do this:
    // AllVectors       = obj.AllVectors; // DO NOT DO THIS
    // The reason is that map<> members are structural artifacts of the
    // design of this class, and are not information in and of themselves.
    // Rather, their contents are information that is added in by derived
    // classes.
    //
    // TODO ?? There has to be a way to abstract this.

    double_vector_map::const_iterator obj_svmi = obj.AllVectors.begin();
    for
        (double_vector_map::iterator svmi = AllVectors.begin()
        ;svmi != AllVectors.end()
        ;svmi++, obj_svmi++
        )
        {
        *(*svmi).second = *(*obj_svmi).second;
        }

    scalar_map::const_iterator obj_sci = obj.AllScalars.begin();
    for
        (scalar_map::iterator svmi = AllScalars.begin()
        ;svmi != AllScalars.end()
        ;svmi++, obj_sci++
        )
        {
        *(*svmi).second = *(*obj_sci).second;
        }

    string_map::const_iterator obj_sti = obj.Strings.begin();
    for
        (string_map::iterator svmi = Strings.begin()
        ;svmi != Strings.end()
        ;svmi++, obj_sti++
        )
        {
        *(*svmi).second = *(*obj_sti).second;
        }
}

//============================================================================
std::string LedgerBase::value_str(std::string const& map_key, int index) const
{
    double_vector_map::const_iterator found = AllVectors.find(map_key);
    if(AllVectors.end() != found)
        {
        return value_cast<std::string>((*(*found).second)[index]);
        }

    fatal_error()
        << "Map key '"
        << map_key
        << "' not found."
        << LMI_FLUSH
        ;
    return "";
}

//============================================================================
std::string LedgerBase::value_str(std::string const& map_key) const
{
    string_map::const_iterator found_string = Strings.find(map_key);
    if(Strings.end() != found_string)
        {
        return *(*found_string).second;
        }

    scalar_map::const_iterator found_scalar = AllScalars.find(map_key);
    if(AllScalars.end() != found_scalar)
        {
        return value_cast<std::string>(*(*found_scalar).second);
        }

    fatal_error()
        << "Map key '"
        << map_key
        << "' not found."
        << LMI_FLUSH
        ;
    return "";
}

namespace
{
// Special non-general helper function.
// The sole use of this function is to multiply y, a vector of values in
// a ledger, by z, a vector of inforce factors. For this sole intended use,
// we know that z is nonzero and nondecreasing; therefore, if it ever
// becomes zero, it remains zero. We can safely break at that point in the
// interest of speed because adding y times zero to z is a NOP. Note that
// the inforce factor becomes zero upon lapse.
    static void x_plus_eq_y_times_z
        (std::vector<double>& x
        ,std::vector<double> const& y
        ,std::vector<double> const& z
        )
    {
        std::vector<double>::iterator ix = x.begin();
        std::vector<double>::const_iterator iy = y.begin();
        std::vector<double>::const_iterator iz = z.begin();
        LMI_ASSERT(y.size() <= x.size());
        LMI_ASSERT(y.size() <= z.size());
        while(iy != y.end())
            {
            LMI_ASSERT(ix != x.end());
            LMI_ASSERT(iz != z.end());
            double mult = *iz++;
            if(0.0 == mult)
                {
                break;
                }
            // Don't waste time multiplying by one
            if(1.0 == mult)
                {
                *ix++ += *iy++;
                }
            else
                {
                *ix++ += *iy++ * mult;
                }
            }
    }
} // Unnamed namespace.

//============================================================================
LedgerBase& LedgerBase::PlusEq
    (LedgerBase const&         a_Addend
    ,std::vector<double> const& a_Inforce
    )
{
    LMI_ASSERT(0.0 != m_scaling_factor);
    if(m_scaling_factor != a_Addend.m_scaling_factor)
        {
        fatal_error() << "Cannot add differently scaled ledgers." << LMI_FLUSH;
        }

    double_vector_map::const_iterator a_Addend_svmi;

    a_Addend_svmi = a_Addend.BegYearVectors.begin();
    std::vector<double> const BegYearInforce = a_Inforce;
    for
        (double_vector_map::iterator svmi = BegYearVectors.begin()
        ;svmi != BegYearVectors.end()
        ;svmi++, a_Addend_svmi++
        )
        {
        x_plus_eq_y_times_z
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            ,BegYearInforce
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.BegYearVectors.end());

    std::vector<double>::const_iterator eyi = a_Inforce.begin();
    eyi++;
    std::vector<double> const EndYearInforce(eyi, a_Inforce.end());
    a_Addend_svmi = a_Addend.EndYearVectors.begin();
    for
        (double_vector_map::iterator svmi = EndYearVectors.begin()
        ;svmi != EndYearVectors.end()
        ;svmi++, a_Addend_svmi++
        )
        {
        x_plus_eq_y_times_z
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            ,EndYearInforce
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.EndYearVectors.end());

    std::vector<double> const NumLivesIssued
        (a_Inforce.size()
        ,a_Inforce[0]
        );
    a_Addend_svmi = a_Addend.ForborneVectors.begin();
    for
        (double_vector_map::iterator svmi = ForborneVectors.begin()
        ;svmi != ForborneVectors.end()
        ;svmi++, a_Addend_svmi++
        )
        {
        x_plus_eq_y_times_z
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            ,NumLivesIssued
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.ForborneVectors.end());

    scalar_map::const_iterator a_Addend_ssmi
        = a_Addend.ScalableScalars.begin();
    for
        (scalar_map::iterator ssmi = ScalableScalars.begin()
        ;ssmi != ScalableScalars.end()
        ;ssmi++, a_Addend_ssmi++
        )
        {
        *(*ssmi).second += *(*a_Addend_ssmi).second * a_Inforce[0];
        }
    LMI_ASSERT(a_Addend_ssmi == a_Addend.ScalableScalars.end());

    return *this;
}

//============================================================================
// Multiplier to keep max < one billion units.
//
// TODO ?? It would be nicer to factor out
//   1000000000.0 (max width)
//   and 1.0E-18 (highest number we translate to words)
// and make them variables.
double LedgerBase::DetermineScaleFactor() const
{
    double min_val = 0.0;
    double max_val = 0.0;

    for
        (double_vector_map::const_iterator svmi = ScalableVectors.begin()
        ;svmi != ScalableVectors.end()
        ;svmi++
        )
        {
        std::vector<double> const& v = *(*svmi).second;
        min_val = std::min(min_val, *std::min_element(v.begin(), v.end()));
        max_val = std::max(max_val, *std::max_element(v.begin(), v.end()));
        }

    // If minimum value is negative, it needs an extra character to
    // display the minus sign. So it needs as many characters as
    // ten times its absolute value.
    double widest = std::max
        (max_val
        ,min_val * -10
        );

    if(widest < 1000000000.0 || widest == 0)
        {
        return 1.0;
        }
    double d = std::log10(widest);
    d = 3.0 * std::floor(d / 3.0);
    d = std::pow(10.0, 6.0 - d);

    LMI_ASSERT(1.0E-18 <= d);
    LMI_ASSERT(d <= 1.0);

    return d;
}

namespace
{
    static std::string look_up_scale_unit(double a_ScalingFactor)
        {
        if(0.0 == a_ScalingFactor)
            {
            hobsons_choice()
                << "Scaling factor is zero. Report will be invalid."
                << LMI_FLUSH
                ;
            return "ZERO";
            }

        double power = -std::log10(a_ScalingFactor);
        // Assert absolute equality of two floating-point quantities, because
        // they must both have integral values.
        LMI_ASSERT(power == std::floor(power));
        int z = static_cast<int>(power);

        // US names are used; UK names are different.
        // Assume that numbers over 999 quintillion (US) will not be needed.
        switch(z)
            {
            case 0:
                {
                return "";
                }
                //  break;
            case 3:
                {
                return "thousand";
                }
                //  break;
            case 6:
                {
                return "million";
                }
                //  break;
            case 9:
                {
                return "billion";
                }
                //  break;
            case 12:
                {
                return "trillion";
                }
                //  break;
            case 15:
                {
                return "quadrillion";
                }
                //  break;
            case 18:
                {
                return "quintillion";
                }
                //  break;
            default:
                {
                fatal_error()
                    << "Case '"
                    << z
                    << "' not found."
                    << LMI_FLUSH
                    ;
                return 0; // Bogus return--actually unreachable.
                }
            }
        }
} // Unnamed namespace.

//============================================================================
// Multiplies all scalable vectors by the factor from DetermineScaleFactor().
// Only columns are scaled, so we operate here only on vectors. A header
// that shows e.g. face amount should show the true face amount, unscaled.
void LedgerBase::ApplyScaleFactor(double a_Mult)
{
    LMI_ASSERT(0.0 != a_Mult);
    LMI_ASSERT(0.0 != m_scaling_factor);
    if(1.0 != m_scaling_factor)
        {
        hobsons_choice() << "Cannot scale the same ledger twice." << LMI_FLUSH;
        }

    m_scaling_factor = a_Mult;
    if(1.0 == m_scaling_factor)
        {
        // Don't waste time multiplying all these vectors by one
        return;
        }
    m_scale_unit = look_up_scale_unit(m_scaling_factor);

    // TODO ?? Would be clearer with bind1st.
    std::vector<double>M(GetLength(), m_scaling_factor);
    for
        (double_vector_map::iterator svmi = ScalableVectors.begin()
        ;svmi != ScalableVectors.end()
        ;svmi++
        )
        {
        // ET !! *(*svmi).second *= M;
        std::vector<double>& v = *(*svmi).second;
        std::transform
            (v.begin()
            ,v.end()
            ,M.begin()
            ,v.begin()
            ,std::multiplies<double>()
            );
        }
}

//============================================================================
std::string const& LedgerBase::ScaleUnit() const
{
    return m_scale_unit;
}

//============================================================================
double LedgerBase::ScaleFactor() const
{
    return m_scaling_factor;
}

//============================================================================
void LedgerBase::UpdateCRC(CRC& crc) const
{
// TODO ?? std::transform() might be cleaner.
    for
        (double_vector_map::const_iterator vmi = AllVectors.begin()
        ;vmi != AllVectors.end()
        ;vmi++
        )
        {
        crc += *(*vmi).second;
        }

    for
        (scalar_map::const_iterator sci = AllScalars.begin()
        ;sci != AllScalars.end()
        ;sci++
        )
        {
        crc += *(*sci).second;
        }

    for
        (string_map::const_iterator sti = Strings.begin()
        ;sti != Strings.end()
        ;sti++
        )
        {
        crc += *(*sti).second;
        }
}

//============================================================================
void LedgerBase::Spew(std::ostream& os) const
{
    static int const prec = max_stream_precision();

    // These changes are intended to facilitate regression testing.
    // Changing only this function, only in this file, causes '.test'
    // output to reflect the following modifications.
    //
    // Copy:
    //   "AcctValLoadAMD"     --> "SepAcctLoad"
    //   "ExpRatRsvCash"      --> "ExperienceReserve"
    //   "Loan"               --> "NewCashLoan"
    //   "PolFee"             --> "PolicyFee"
    //   "ExpRatRsvCash"      --> "KFactor"
    //   "ExpRatRsvCash"      --> "ProjectedCoiCharge"
    //   "PremiumTaxIsTiered" --> "PremiumTaxLoadIsTiered"
    //
    // Then expunge:
    //   "AcctValLoadAMD"
    //   "AcctValLoadBOM"
    //   "AnnPolFee"
    //   "BOYPrefLoan"
    //   "ExpRatRsvCash"
    //   "ExpRatRsvForborne"
    //   "ExpRsvInt"
    //   "Loan"
    //   "MlyPolFee"
    //   "PolFee"
    //   "PremiumTaxIsTiered"
    //   "PolicyShortName"
    //
    // New fields "KFactor" and "ProjectedCoiCharge" are initialized
    // with the value in old field "ExpRatRsvCash". That value is, of
    // course, wrong unless it happens to be zero; but it happens to
    // be correct for almost all present test cases.

    double_vector_map vectors(AllVectors);
    if(vectors.count("AcctValLoadAMD")) vectors["SepAcctLoad"       ] = vectors["AcctValLoadAMD"];
    if(vectors.count("ExpRatRsvCash" )) vectors["ExperienceReserve" ] = vectors["ExpRatRsvCash" ];
    if(vectors.count("Loan"          )) vectors["NewCashLoan"       ] = vectors["Loan"          ];
    if(vectors.count("PolFee"        )) vectors["PolicyFee"         ] = vectors["PolFee"        ];
    if(vectors.count("ExpRatRsvCash" )) vectors["KFactor"           ] = vectors["ExpRatRsvCash" ];
    if(vectors.count("ExpRatRsvCash" )) vectors["ProjectedCoiCharge"] = vectors["ExpRatRsvCash" ];

    if(vectors.count("AcctValLoadAMD"   )) vectors.erase("AcctValLoadAMD"   );
    if(vectors.count("AcctValLoadBOM"   )) vectors.erase("AcctValLoadBOM"   );
    if(vectors.count("AnnPolFee"        )) vectors.erase("AnnPolFee"        );
    if(vectors.count("BOYPrefLoan"      )) vectors.erase("BOYPrefLoan"      );
    if(vectors.count("ExpRatRsvCash"    )) vectors.erase("ExpRatRsvCash"    );
    if(vectors.count("ExpRatRsvForborne")) vectors.erase("ExpRatRsvForborne");
    if(vectors.count("ExpRsvInt"        )) vectors.erase("ExpRsvInt"        );
    if(vectors.count("Loan"             )) vectors.erase("Loan"             );
    if(vectors.count("MlyPolFee"        )) vectors.erase("MlyPolFee"        );
    if(vectors.count("PolFee"           )) vectors.erase("PolFee"           );

    scalar_map scalars(AllScalars);
    if(scalars.count("PremiumTaxIsTiered")) scalars["PremiumTaxLoadIsTiered"] = scalars["PremiumTaxIsTiered"];
    if(scalars.count("PremiumTaxIsTiered")) scalars.erase("PremiumTaxIsTiered");

    string_map strings(Strings);
    if(strings.count("PolicyShortName"   )) strings.erase("PolicyShortName"   );

    for
        (double_vector_map::const_iterator vmi = vectors.begin()
        ;vmi != vectors.end()
        ;vmi++
        )
        {
        SpewVector(os, (*vmi).first, *(*vmi).second);
        }

    for
        (scalar_map::const_iterator sci = scalars.begin()
        ;sci != scalars.end()
        ;sci++
        )
        {
        os
            << (*sci).first
            << "=="
            << std::setprecision(prec) << *(*sci).second
            << '\n'
            ;
        }

    for
        (string_map::const_iterator sti = strings.begin()
        ;sti != strings.end()
        ;sti++
        )
        {
        os
            << (*sti).first
            << "=="
            << *(*sti).second
            << '\n'
            ;
        }
}

/*

Analysis and design: a few maps instead of many vectors

We want to assemble a large number of vectors into a collection so that
we can iterate over the collection instead of performing operations (e.g.
initialize, copy, test for equality, find max/min element, multiply) for
each vector separately. The reason is that performing a half dozen
operations for each of a couple dozen vectors requires us to write about
a dozen squared names, getting each one exactly right. Adding a new vector
requires us to write its name in each of a half dozen places. This was the
approach originally taken, and the resulting code was sprawling, hard to
maintain, and difficult to validate.

We note that almost all are vector<double>, have the same size(), and
are initialized to zero. Treating only this majority subset of vector
members substantially fulfills our wishes, so we won't attempt polymorphic
treatment of the other vectors.

A map<> is clearly called for. Certain implementation choices must be made.

Choice 1: Who owns the data? We want only one instance of the data to
exist. If we have a bunch of vectors and a map<key, vector> then we have
two copies; that wastes memory and, worse, the copies can become
unsynchronized.

1.a. Vector members own the data. Then we need a map<key, vector<>*>.

1.b. Map owns the data. Then we want vector& members that refer to map
entries. Reason: we extremely often want to read or write one element
of a vector, and we don't want a map lookup each time. But this approach
appears problematic: for instance, what happens to those references when
we copy the map?

We choose 1.a.

Choice 2. Some vectors represent beginning-of-year data; others,
end-of-year data. Some are "arithmetic"; others are not. Arithmetic, an
adjective, here means that arithmetic can be performed upon them in the
sense of the following example. Payments can be multiplied by a scale
factor (1000, 1000000, etc.) to make them print in a given width, and
can be added together when combining multiple ledgers into a composite,
so they are arithmetic. Interest rates are not.

2.a. Separate maps for BOY and EOY.

2.b. Wrap vector<>* into a class and add a member BOY/EOY/non-arith indicator.

To make this choice, we need to consider wrappers further.

Choice 3. We can use vector<> pointers as such, or wrap them. Since we
chose to use vector<> members that own their own storage, the pointer
would be needed only within this class, for dealing with whole groups of
vectors at a time.

3.a. Use vector<>* . Only map::data_type is a pointer; key_type is not.
STL algorithms like copy() would operate on the pointers themselves,
which is not the behavior we want. We could code such operations by hand:
iterate across the map, and dereference pointers to operate on the
data inside a loop or with transform() e.g.

3.b. Use wrappers. The extra level of indirection is not a real concern
because we would intend to address all elements of a vector when going
through the wrapper.

A wrapper for data_type doesn't need to be very elaborate. According to
Matt Austern's authoritative book, the type requirements of map<> specify
that map::data_type be Assignable. It need not even be EqualityComparable.

The equivalence between map[k] and
    (*((map.insert(value_type(k, data_type()))).first)).second
suggests that data_type must also be DefaultConstructible.

The operations we need for each vector<> are initialization, assignment,
scaling (multiplying by a scalar), adding, and finding the max and min
elements. The last four of these six are arithmetic, so we can regard
the necessary operations as predominantly arithmetic. So the choice
really amounts to
    3.a. iterate across map, dereference vector, perform operation
    3.b. iterate across map, perform operation; push operation into ptr class
The second option would mean defining a min_element operation on the ptr
class, which sounds like a Bad Idea.

A hybrid might be considered: 3.a. for arithmetic operations, and 3.b. for
other operations. This seems to secure the disadvantages of both pure
approaches at the cost of increased complexity.

We choose 3.a., which impels us to choose 2.a.

*/

