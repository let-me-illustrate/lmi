// Ledger data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "ledger.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "bourn_cast.hpp"
#include "crc32.hpp"
#include "global_settings.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()
#include "miscellany.hpp"               // minmax, scale_power()
#include "oecumenic_enumerations.hpp"   // methuselah

#include <algorithm>
#include <ostream>

// TODO ?? Is it really a good idea to have shared_ptr data members?
// If so, should there be a member function
//   Ledger Ledger::Clone() const
//   {
//       Ledger new_ledger(*this);
//       new_ledger.ledger_map_       = std::shared_ptr<ledger_map_holder>(new ledger_map_holder(*ledger_map_));
//       new_ledger.ledger_invariant_ = std::shared_ptr<LedgerInvariant>(new LedgerInvariant(*ledger_invariant_));
//       return new_ledger;
//   }
// that would make unshared copies? If not, then Clone() would be
// unnecessary--the copy ctor would suffice. This is a problem for
// the (poorly-named) member function AutoScale(), which refuses to
// be applied to the same object twice (though perhaps that's an
// unnecessary restriction). What problem did shared_ptr data members
// solve, and were they the best way to solve it?
//
// Perhaps member function AutoScale() should be replaced by, e.g.,
//   Ledger ScaledLedger() const;
// yet would that double the memory requirements when a container of
// ledgers must be retained?
//
// See this discussion
//   https://savannah.nongnu.org/bugs/?13599
// of a demonstrable problem stemming from this defect.

//============================================================================
Ledger::Ledger
    (int                length
    ,mcenum_ledger_type ledger_type
    ,bool               nonillustrated
    ,bool               no_can_issue
    ,bool               is_composite
    )
    :ledger_type_          {ledger_type}
    ,nonillustrated_       {nonillustrated}
    ,no_can_issue_         {no_can_issue}
    ,is_composite_         {is_composite}
    ,ledger_map_           {new ledger_map_holder}
    ,ledger_invariant_     {new LedgerInvariant(length)}
{
    SetRunBases(length);

    if(is_composite_)
        {
        // By default, issue age is initialized to zero, and maturity
        // age to 100, to accord with the default 'length' of 100.
        // Both are reset to actual values when those become known.
        // For a composite, however, they're updated thus:
        //   Age     = std::min(Age    , a_Addend.Age    );
        //   EndtAge = std::max(EndtAge, a_Addend.EndtAge);
        // as each addend is aggregated, so they must be initialized
        // beforehand to values that would not otherwise make sense.
        ledger_invariant_->Age     = methuselah;
        ledger_invariant_->EndtAge = 0;
        }
}

//============================================================================
void Ledger::SetRunBases(int length)
{
    ledger_map_t& l_map_rep = ledger_map_->held_;
    switch(ledger_type_)
        {
        case mce_ill_reg:
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_mdpt_sep_full] = LedgerVariant(length);
            }
            break;
        case mce_group_private_placement:      // Deliberately fall through.
        case mce_individual_private_placement: // Deliberately fall through.
        case mce_finra:
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(length);
            }
            break;
        // Formerly, three-rate illustrations were required for
        // prospectuses. Since this code was written, that requirement
        // has become inoperative, but the code is preserved in case
        // such a format becomes desirable for some other reason.
        // For now, the difference from mce_finra is seen only in
        // regression testing ('emit_test_data' output only).
        case mce_prospectus_abeyed: // {curr, 0% int, 1/2 int%} X {guar, curr}
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_half] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_half] = LedgerVariant(length);
            }
            break;
        case mce_offshore_private_placement_obsolete: // fall through
        case mce_ill_reg_private_placement_obsolete:  // fall through
        case mce_variable_annuity_obsolete:
            {
            alarum() << "Unsupported ledger type '" << ledger_type_ << "'." << LMI_FLUSH;
            }
        }

    for(auto& i : l_map_rep)
        {
        ledger_map_t::key_type const& key = i.first;
        ledger_map_t::mapped_type& data = i.second;

        run_bases_.push_back(key);
        data.set_run_basis(key);

        if(is_composite_)
            {
            // Lapse year is initialized to omega and set to a lower
            // value only upon lapse during account value accumulation.
            // That is inappropriate for a composite, which has no
            // such accumulation process.
            //
            // TODO ?? Perhaps default initial values for some other
            // members are also inappropriate for composites.
            data.LapseYear = 0;
            }
        }
}

/// Set inforce lives to zero after lapse.
///
/// Accomplish this by (1) shortening the inforce-lives vector to the
/// chosen lapse year, increased by one because values are always
/// illustrated to the end of that year--iff that is shorter--and then
/// (2) restoring it to its original length.
///
/// Pick the highest lapse year of any basis (i.e. any LedgerVariant).
/// This is extremely likely to mean the lapse year on the current
/// basis; but if it's the lapse year on some other basis, we don't
/// want to truncate values on that other basis, even if it means
/// that the vector of inforce lives does not correspond to the
/// current values.

void Ledger::ZeroInforceAfterLapse()
{
    int original_length = lmi::ssize(ledger_invariant_->InforceLives);
    int unlapsed_length = 1 + greatest_lapse_dur();
    if(unlapsed_length < original_length)
        {
        ledger_invariant_->InforceLives.resize(unlapsed_length);
        ledger_invariant_->InforceLives.resize(original_length);
        }
}

//============================================================================
Ledger& Ledger::PlusEq(Ledger const& a_Addend)
{
    // TODO ?? We should look at other things like Smoker and handle
    // them in some appropriate manner if they differ across
    // lives in a composite.
    //
    // For vectors, the {BOY, EOY, forborne,...} distinction works.
    // For scalars, the situation is less satisfactory: the "addition"
    // method is hardcoded for many, and many are ignored. Probably a
    // larger set of possibilities is wanted: e.g., Smoker might use
    // a method like "blank unless identical across all cells", while
    // various ages and durations might use {...minimum, maximum,...}.
    // Perhaps these distinctions should be expressed not as named
    // subcollections of containers but rather as enumerators.

    if(ledger_type_ != a_Addend.ledger_type())
        {
        alarum()
            << "Cannot add ledgers for products with different"
            << " formatting requirements."
            << LMI_FLUSH
            ;
        }

    nonillustrated_ = nonillustrated_ || a_Addend.nonillustrated();
    no_can_issue_   = no_can_issue_   || a_Addend.no_can_issue  ();

    LMI_ASSERT(is_composite());
    LMI_ASSERT(!a_Addend.is_composite());

    ledger_map_t& l_map_rep = ledger_map_->held_;
    ledger_map_t::iterator this_i = l_map_rep.begin();

    ledger_map_t const& lm_addend = a_Addend.GetLedgerMap().held();
    ledger_map_t::const_iterator addend_i = lm_addend.begin();

    ledger_invariant_->PlusEq(*a_Addend.ledger_invariant_);

    while(this_i != l_map_rep.end() || addend_i != lm_addend.end())
        {
        LMI_ASSERT((*this_i).first == (*addend_i).first);
        (*this_i).second.PlusEq
            ((*addend_i).second
            ,a_Addend.ledger_invariant_->GetInforceLives()
            );
        (*this_i).second.LapseYear = std::max
            ((*this_i).second.LapseYear
            ,(*addend_i).second.LapseYear
            );
        ++this_i, ++addend_i;
        }

    LMI_ASSERT(this_i == l_map_rep.end() && addend_i == lm_addend.end());

    return *this;
}

//============================================================================
void Ledger::SetLedgerInvariant(LedgerInvariant const& a_Invariant)
{
    *ledger_invariant_ = a_Invariant;
}

//============================================================================
void Ledger::SetGuarPremium(double a_GuarPrem)
{
    ledger_invariant_->GuarPrem = a_GuarPrem;
}

//============================================================================
void Ledger::SetOneLedgerVariant
    (mcenum_run_basis     a_Basis
    ,LedgerVariant const& a_Variant
    )
{
    ledger_map_t& l_map_rep = ledger_map_->held_;
    if(l_map_rep.count(a_Basis))
        {
        l_map_rep[a_Basis] = a_Variant;
        }
    else
        {
        alarum()
            << "Failed attempt to set ledger for unused basis '"
            << mc_str(a_Basis)
            << "'."
            << LMI_FLUSH
            ;
        }
}

/// Number of rows on an illustration: duration when last basis lapses.

int Ledger::greatest_lapse_dur() const
{
    ledger_map_t const& l_map_rep = ledger_map_->held();
    double lapse_dur = 0.0;
    for(auto const& i : l_map_rep)
        {
        lapse_dur = std::max(lapse_dur, i.second.LapseYear);
        }
    return bourn_cast<int>(lapse_dur);
}

//============================================================================
// Scale all numbers in every column of every subledger according to the
// largest absolute value of any number in any column of every subledger.
void Ledger::AutoScale()
{
    minmax<double> extrema = ledger_invariant_->scalable_extrema();

    ledger_map_t& l_map_rep = ledger_map_->held_;
    for(auto const& i : l_map_rep)
        {
        extrema.subsume(i.second.scalable_extrema());
        }

    int const max_power = 9;
    int const k = scale_power(max_power, extrema.minimum(), extrema.maximum());

    ledger_invariant_->apply_scale_factor(k);

    for(auto& i : l_map_rep)
        {
        i.second.apply_scale_factor(k);
        }
}

//============================================================================
unsigned int Ledger::CalculateCRC() const
{
    CRC crc;
    ledger_invariant_->UpdateCRC(crc);
    ledger_map_t const& l_map_rep = ledger_map_->held();
    for(auto const& i : l_map_rep)
        {
        i.second.UpdateCRC(crc);
        }

    return crc.value();
}

//============================================================================
void Ledger::Spew(std::ostream& os) const
{
    ledger_invariant_->Spew(os);
    ledger_map_t const& l_map_rep = ledger_map_->held();
    for(auto const& i : l_map_rep)
        {
        i.second.Spew(os);
        }
}

//============================================================================
ledger_map_holder const& Ledger::GetLedgerMap() const
{
    LMI_ASSERT(ledger_map_.get());
    return *ledger_map_;
}

//============================================================================
LedgerInvariant const& Ledger::GetLedgerInvariant() const
{
    LMI_ASSERT(ledger_invariant_.get());
    return *ledger_invariant_;
}

//============================================================================
std::vector<mcenum_run_basis> const& Ledger::GetRunBases() const
{
    return run_bases_;
}

//============================================================================
mcenum_ledger_type Ledger::ledger_type() const
{
    return ledger_type_;
}

//============================================================================
bool Ledger::nonillustrated() const
{
    return nonillustrated_;
}

//============================================================================
bool Ledger::no_can_issue() const
{
    return no_can_issue_;
}

//============================================================================
bool Ledger::is_composite() const
{
    return is_composite_;
}

//============================================================================
LedgerVariant const& Ledger::GetOneVariantLedger(mcenum_run_basis b) const
{
    return map_lookup(GetLedgerMap().held(), b);
}

//============================================================================
LedgerVariant const& Ledger::GetCurrFull() const
{
    return GetOneVariantLedger(mce_run_gen_curr_sep_full);
}

//============================================================================
LedgerVariant const& Ledger::GetGuarFull() const
{
    return GetOneVariantLedger(mce_run_gen_guar_sep_full);
}

//============================================================================
LedgerVariant const& Ledger::GetMdptFull() const
{
    return GetOneVariantLedger(mce_run_gen_mdpt_sep_full);
}

//============================================================================
LedgerVariant const& Ledger::GetCurrZero() const
{
    return GetOneVariantLedger(mce_run_gen_curr_sep_zero);
}

//============================================================================
LedgerVariant const& Ledger::GetGuarZero() const
{
    return GetOneVariantLedger(mce_run_gen_guar_sep_zero);
}

//============================================================================
LedgerVariant const& Ledger::GetCurrHalf() const
{
    return GetOneVariantLedger(mce_run_gen_curr_sep_half);
}

//============================================================================
LedgerVariant const& Ledger::GetGuarHalf() const
{
    return GetOneVariantLedger(mce_run_gen_guar_sep_half);
}

/// Access a vector by the name used in xml output.
///
/// The name is generally a compound consisting of a base name
/// indicating the vector's meaning, and a suffix indicating its
/// calculation basis. A null suffix is used for vectors that do not
/// vary by basis.
///
/// This function addresses some special cases, but not all. Some
/// vectors, like 'InforceLives', are special by their nature, and
/// therefore aren't accessible via all_vectors(). Some other
/// vectors are special only because of design flaws: for example,
/// IRRs that vary by basis reside in the invariant ledger. Only
/// special cases represented by vectors offered for use in the
/// calculation summary are addressed here for now--with the
/// temporary exception of 'NetDeathBenefit'.
///
/// A better solution would intrude class template any_member into
/// class Ledger. Then other special cases, such as vectors not of
/// type double, could be handled automatically.
///
/// It seems inefficient to return by value, and that's especially
/// regrettable because all vectors except for a few special cases
/// could be returned by reference. Consideration should be given
/// someday to putting every vector in the ledger object and then
/// returning 'std::vector<double> const&'. However, this function
/// is called only in FormatSelectedValuesAsHtml() as this is written
/// in 2008-10, and measurement shows that function's speed to be
/// the same whether this function returns by value or by reference.

std::vector<double> numeric_vector
    (Ledger const&      ledger
    ,std::string const& compound_name
    )
{
    std::string::size_type pos = compound_name.rfind("_");
    if(std::string::npos == pos)
        {
        pos = compound_name.size();
        }
    std::string const base_name = compound_name.substr(0, pos);
    std::string const suffix    = compound_name.substr(pos);

    LedgerInvariant const& invar = ledger.GetLedgerInvariant();
    LedgerVariant   const& curr  = ledger.GetCurrFull();

    int const length = invar.GetLength();

    std::vector<double> attained_age     (length);
    std::vector<double> policy_year      (length);
    std::vector<double> net_death_benefit(length);
    for(int j = 0; j < length; ++j)
        {
        attained_age     [j] = j + invar.Age;
        policy_year      [j] = j + 1        ;
        net_death_benefit[j] = curr.EOYDeathBft[j] - curr.TotalLoanBalance[j];
        }

    typedef LedgerBase const& B;
    B z =
          "_Current"        == suffix ? static_cast<B>(ledger.GetCurrFull())
        : "_Guaranteed"     == suffix ? static_cast<B>(ledger.GetGuarFull())
        : "_Midpoint"       == suffix ? static_cast<B>(ledger.GetMdptFull())
        : "_CurrentZero"    == suffix ? static_cast<B>(ledger.GetCurrZero())
        : "_GuaranteedZero" == suffix ? static_cast<B>(ledger.GetGuarZero())
        : "_CurrentHalf"    == suffix ? static_cast<B>(ledger.GetCurrHalf())
        : "_GuaranteedHalf" == suffix ? static_cast<B>(ledger.GetGuarHalf())
        :                               static_cast<B>(invar)
        ;

    return
          "AttainedAge"       == compound_name ? attained_age
        : "InforceLives"      == compound_name ? invar.InforceLives
        : "IrrCsv_Current"    == compound_name ? invar.IrrCsvCurrInput
        : "IrrCsv_Guaranteed" == compound_name ? invar.IrrCsvGuarInput
        : "IrrDb_Current"     == compound_name ? invar.IrrDbCurrInput
        : "IrrDb_Guaranteed"  == compound_name ? invar.IrrDbGuarInput
        : "NetDeathBenefit"   == compound_name ? net_death_benefit
        : "PolicyYear"        == compound_name ? policy_year
        : "SupplSpecAmt"      == compound_name ? invar.TermSpecAmt
        : "SupplDeathBft"     == base_name ? *map_lookup(z.all_vectors(), "TermPurchased")
        : *map_lookup(z.all_vectors(), base_name)
        ;
}

namespace
{
std::string reason_to_interdict(Ledger const& z)
{
    if(z.nonillustrated())
        {
        return "Illustrations are forbidden for this policy form.";
        }
    // Don't even think about it, say no go.
    else if(z.no_can_issue() && !global_settings::instance().regression_testing())
        {
        return "New-business illustrations are forbidden for this policy form.";
        }
    else
        {
        return "";
        }
}
} // Unnamed namespace.

bool is_interdicted(Ledger const& z)
{
    return !reason_to_interdict(z).empty();
}

void throw_if_interdicted(Ledger const& z)
{
    std::string s = reason_to_interdict(z);
    if(!s.empty())
        {
        alarum() << s << LMI_FLUSH;
        }
}
