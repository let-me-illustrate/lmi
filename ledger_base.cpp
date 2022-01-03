// Ledger values: common base class.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "ledger_base.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "crc32.hpp"
#include "et_vector.hpp"
#include "stl_extensions.hpp"           // nonstd::power()
#include "value_cast.hpp"

#include <algorithm>                    // max(), min()
#include <stdexcept>                    // logic_error

//============================================================================
LedgerBase::LedgerBase(int a_Length)
    :scale_power_ {0}
    ,scale_unit_  {""}
{
    Initialize(a_Length);
}

//============================================================================
LedgerBase::LedgerBase(LedgerBase const& obj)
    :scale_power_ {obj.scale_power_}
    ,scale_unit_  {obj.scale_unit_}
{
    Initialize(obj.GetLength());
    Copy(obj);
}

//============================================================================
LedgerBase& LedgerBase::operator=(LedgerBase const& obj)
{
    if(this != &obj)
        {
        scale_power_ = obj.scale_power_;
        scale_unit_  = obj.scale_unit_;
        Initialize(obj.GetLength());
        Copy(obj);
        }
    return *this;
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
    for(auto& i : AllVectors)
        {
        i.second->assign(a_Length, 0.0);
        }

    for(auto& i : AllScalars)
        {
        *i.second = 0.0;
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
    // scale_power_ and scale_unit_ aren't copied here because they're
    // copied explicitly by the caller.
    //
    // TODO ?? There has to be a way to abstract this.

    double_vector_map::const_iterator obj_svmi = obj.AllVectors.begin();
    for
        (double_vector_map::iterator svmi = AllVectors.begin()
        ;svmi != AllVectors.end()
        ;++svmi, ++obj_svmi
        )
        {
        *(*svmi).second = *(*obj_svmi).second;
        }

    scalar_map::const_iterator obj_sci = obj.AllScalars.begin();
    for
        (scalar_map::iterator svmi = AllScalars.begin()
        ;svmi != AllScalars.end()
        ;++svmi, ++obj_sci
        )
        {
        *(*svmi).second = *(*obj_sci).second;
        }

    string_map::const_iterator obj_sti = obj.Strings.begin();
    for
        (string_map::iterator svmi = Strings.begin()
        ;svmi != Strings.end()
        ;++svmi, ++obj_sti
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

    alarum() << "Map key '" << map_key << "' not found." << LMI_FLUSH;
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

    alarum() << "Map key '" << map_key << "' not found." << LMI_FLUSH;
    return "";
}

//============================================================================
double_vector_map const& LedgerBase::all_vectors() const
{
    return AllVectors;
}

namespace
{
/// Special non-general helper function.
///
/// Multiplies y, a vector of ledger values, by z, a vector of inforce
/// factors; then adds the result into x, a vector of composite-ledger
/// values, up to the length of y (which is less than or equal to the
/// length of x).
///
/// In this sole use case, z must be nonincreasing and nonnegative,
/// because it is a survivorship function. Once it becomes zero (due
/// to maturity or lapse), it remains zero thenceforth; therefore, it
/// is appropriate and safe to break the loop at that point.

    static void x_plus_eq_y_times_z
        (std::vector<double>      & x
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

/// Special non-general helper function.
///
/// Adds y, a vector of ledger values, into x, a vector of composite-
/// ledger values, up to the length of y (which is less than or equal
/// to the length of x).
///
/// Equivalent to:
///   // ET !! This is of the form 'x[iota rho y] gets y'.
///   for(int j = 0; j < y.size(); ++j) {x[j] = y[j];}

    static void x_sub_iota_rho_y_gets_y
        (std::vector<double>      & x
        ,std::vector<double> const& y
        )
    {
        std::vector<double>::iterator ix = x.begin();
        std::vector<double>::const_iterator iy = y.begin();
        LMI_ASSERT(y.size() <= x.size());
        while(iy != y.end())
            {
            LMI_ASSERT(ix != x.end());
            *ix++ = *iy++;
            }
    }
} // Unnamed namespace.

//============================================================================
// TODO ?? Adds cells by policy duration, not calendar duration: when
// cell issue dates differ, the result is valid only in that probably-
// unexpected sense.
LedgerBase& LedgerBase::PlusEq
    (LedgerBase          const& a_Addend
    ,std::vector<double> const& a_Inforce
    )
{
    if(scale_power_ != a_Addend.scale_power_)
        {
        alarum() << "Cannot add differently scaled ledgers." << LMI_FLUSH;
        }

    double_vector_map::const_iterator a_Addend_svmi;

    a_Addend_svmi = a_Addend.BegYearVectors.begin();
    std::vector<double> const BegYearInforce = a_Inforce;
    for
        (double_vector_map::iterator svmi = BegYearVectors.begin()
        ;svmi != BegYearVectors.end()
        ;++svmi, ++a_Addend_svmi
        )
        {
        x_plus_eq_y_times_z
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            ,BegYearInforce
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.BegYearVectors.end());

    std::vector<double> const EndYearInforce
        (a_Inforce.begin() + 1
        ,a_Inforce.end()
        );
    a_Addend_svmi = a_Addend.EndYearVectors.begin();
    for
        (double_vector_map::iterator svmi = EndYearVectors.begin()
        ;svmi != EndYearVectors.end()
        ;++svmi, ++a_Addend_svmi
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
        ;++svmi, ++a_Addend_svmi
        )
        {
        x_plus_eq_y_times_z
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            ,NumLivesIssued
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.ForborneVectors.end());

    a_Addend_svmi = a_Addend.OtherVectors.begin();
    for
        (double_vector_map::iterator svmi = OtherVectors.begin()
        ;svmi != OtherVectors.end()
        ;++svmi, ++a_Addend_svmi
        )
        {
        x_sub_iota_rho_y_gets_y
            (*(*svmi).second
            ,*(*a_Addend_svmi).second
            );
        }
    LMI_ASSERT(a_Addend_svmi == a_Addend.OtherVectors.end());

    scalar_map::const_iterator a_Addend_ssmi = a_Addend.ScalableScalars.begin();
    for
        (scalar_map::iterator ssmi = ScalableScalars.begin()
        ;ssmi != ScalableScalars.end()
        ;++ssmi, ++a_Addend_ssmi
        )
        {
        *(*ssmi).second += *(*a_Addend_ssmi).second * a_Inforce[0];
        }
    LMI_ASSERT(a_Addend_ssmi == a_Addend.ScalableScalars.end());

    string_map::const_iterator a_Addend_smi = a_Addend.Strings.begin();
    for
        (string_map::iterator smi = Strings.begin()
        ;smi != Strings.end()
        ;++smi, ++a_Addend_smi
        )
        {
        *(*smi).second = *(*a_Addend_smi).second;
        }
    LMI_ASSERT(a_Addend_smi == a_Addend.Strings.end());

    return *this;
}

/// Return highest and lowest scalable values.

minmax<double> LedgerBase::scalable_extrema() const
{
    minmax<double> extrema;

    for(auto const& i : ScalableVectors)
        {
        extrema.subsume(minmax<double>(*i.second));
        }

    return extrema;
}

namespace
{
    // US names are used; obsolescent UK names are different.
    // Assume that values over US$ 999 quintillion will not arise.
    std::string look_up_scale_unit(int decimal_power)
        {
        return
             ( 0 == decimal_power) ? ""
            :( 3 == decimal_power) ? "thousand"
            :( 6 == decimal_power) ? "million"
            :( 9 == decimal_power) ? "billion"
            :(12 == decimal_power) ? "trillion"
            :(15 == decimal_power) ? "quadrillion"
            :(18 == decimal_power) ? "quintillion"
            : throw std::logic_error("Unnamed scaling unit.")
            ;
        }
} // Unnamed namespace.

/// Scale all scalable vectors by a decimal power.
///
/// Scale only designated columns (vectors). Interest-rate columns,
/// e.g., are not scaled because they aren't denominated in dollars.
///
/// Scalars are never scaled: e.g., a $1,000,000,000 specified amount
/// is shown as such in a header (using a scalar variable representing
/// its initial value) even if a column representing the same quantity
/// (using a vector variable) depicts it as $1,000,000 thousands.

void LedgerBase::apply_scale_factor(int decimal_power)
{
    if(0 != scale_power_)
        {
        alarum() << "Cannot scale the same ledger twice." << LMI_FLUSH;
        }

    scale_power_ = decimal_power;
    scale_unit_ = look_up_scale_unit(scale_power_);

    if(0 == scale_power_)
        {
        // Don't waste time multiplying all these vectors by one
        return;
        }

    double const scale_factor = 1.0 / nonstd::power(10.0, scale_power_);
    for(auto& i : ScalableVectors)
        {
        *i.second *= scale_factor;
        }
}

//============================================================================
std::string const& LedgerBase::scale_unit() const
{
    return scale_unit_;
}

//============================================================================
void LedgerBase::UpdateCRC(CRC& crc) const
{
    for(auto const& i : AllVectors)
        {
        crc += *i.second;
        }

    for(auto const& i : AllScalars)
        {
        crc += *i.second;
        }

    for(auto const& i : Strings)
        {
        crc += *i.second;
        }
}

//============================================================================
void LedgerBase::Spew(std::ostream& os) const
{
    for(auto const& i : AllVectors)
        {
        SpewVector(os, i.first, *i.second);
        }

    for(auto const& i : AllScalars)
        {
        os
            << i.first
            << "=="
            << std::setprecision(DECIMAL_DIG) << *i.second
            << '\n'
            ;
        }

    for(auto const& i : Strings)
        {
        os
            << i.first
            << "=="
            << *i.second
            << '\n'
            ;
        }
}
