// Ledger data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#include "ledger.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "crc32.hpp"
#include "global_settings.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "map_lookup.hpp"
#include "mc_enum_types_aux.hpp"        // mc_str()

#include <algorithm>
#include <ostream>

// TODO ?? Is it really a good idea to have shared_ptr data members?
// If so, should there be a member function
//   Ledger Ledger::Clone() const
//   {
//       Ledger new_ledger(*this);
//       new_ledger.ledger_map_       = boost::shared_ptr<ledger_map_holder>(new ledger_map_holder(*ledger_map_));
//       new_ledger.ledger_invariant_ = boost::shared_ptr<LedgerInvariant>(new LedgerInvariant(*ledger_invariant_));
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
    :ledger_type_          (ledger_type)
    ,nonillustrated_       (nonillustrated)
    ,no_can_issue_         (no_can_issue)
    ,is_composite_         (is_composite)
    ,composite_lapse_year_ (0.0)
    ,ledger_map_           (new ledger_map_holder)
    ,ledger_invariant_     (new LedgerInvariant(length))
{
    SetRunBases(length);
}

//============================================================================
Ledger::~Ledger()
{
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
        case mce_variable_annuity:             // Deliberately fall through.
        case mce_nasd:
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(length);
            }
            break;
#if 0
        // Formerly, three-rate illustrations were required for
        // prospectuses. Since this code was written, that requirement
        // has become inoperative, but the code is preserved in case
        // such a format becomes desirable for some other reason.
        //
        case nonexistent:  // {current, 0% int, 1/2 int%} X {guar, curr}
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(length);
            l_map_rep[mce_run_gen_curr_sep_half] = LedgerVariant(length);
            l_map_rep[mce_run_gen_guar_sep_half] = LedgerVariant(length);
            }
            break;
#endif
        default:
            {
            fatal_error()
                << "Case '"
                << ledger_type_
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    for
        (ledger_map_t::iterator p = l_map_rep.begin()
        ;p != l_map_rep.end()
        ;++p
        )
        {
        ledger_map_t::key_type const& key = (*p).first;
        ledger_map_t::mapped_type& data = (*p).second;

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

//============================================================================
// TODO ?? This seems to be a bad idea: it zeroes what should already
// have been zero.
void Ledger::ZeroInforceAfterLapse()
{
    ledger_map_t const& l_map_rep = ledger_map_->held();
    ledger_map_t::const_iterator this_i = l_map_rep.begin();

    // Pick the highest lapse year of any basis (i.e. any LedgerVariant).
    // Set inforce lives to zero at the end of that year and thereafter.
    // This is extremely likely to mean the lapse year on the current
    // basis; but if it's the lapse year on some other basis, we don't
    // want to truncate values on that other basis, even if it means
    // that the vector of inforce lives does not correspond to the
    // current values.
    int lapse_year = 0;
    while(this_i != l_map_rep.end())
        {
        lapse_year = std::max
            (lapse_year
            ,static_cast<int>((*this_i).second.LapseYear)
            );
        ++this_i;
        }
    std::vector<double>::iterator b =
            ledger_invariant_->InforceLives.begin()
        +   1
        +   lapse_year
        ;
    std::vector<double>::iterator e = ledger_invariant_->InforceLives.end();
    if(b < e)
        {
        std::fill(b, e, 0.0);
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
        fatal_error()
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
        composite_lapse_year_ = std::max
            (composite_lapse_year_
            ,(*addend_i).second.LapseYear
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
        fatal_error()
            << "Failed attempt to set ledger for unused basis '"
            << mc_str(a_Basis)
            << "'."
            << LMI_FLUSH
            ;
        }
}

//============================================================================
int Ledger::GetMaxLength() const
{
    if(is_composite_)
        {
        // TODO ?? The rationale for this special case is not evident.
        // This variable is initialized to zero in the ctor. If it has
        // acquired a value that's actually meaningful, then it looks
        // like the variables used in the general case would have, too,
        // which would reduce this special case to mere caching, which
        // seems like a premature optimization and a needless
        // complication.
        return static_cast<int>(composite_lapse_year_);
        }

    // For all ledgers in the map, find the longest duration that must
    // be printed (until the last one lapses).
    ledger_map_t const& l_map_rep = ledger_map_->held();
    double max_length = 0.0;

    for
        (ledger_map_t::const_iterator lmci = l_map_rep.begin()
        ;lmci != l_map_rep.end()
        ;++lmci
        )
        {
        max_length = std::max(max_length, (*lmci).second.LapseYear);
        }
    return static_cast<int>(max_length);
}

//============================================================================
// Scale all numbers in every column of every subledger according to the
// largest absolute value of any number in any column of every subledger.
void Ledger::AutoScale()
{
    double mult = ledger_invariant_->DetermineScaleFactor();

    ledger_map_t& l_map_rep = ledger_map_->held_;
    ledger_map_t::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); ++lmci)
        {
        mult = std::min(mult, (*lmci).second.DetermineScaleFactor());
        }

    ledger_invariant_->ApplyScaleFactor(mult);

    ledger_map_t::iterator lmi = l_map_rep.begin();
    for(;lmi != l_map_rep.end(); ++lmi)
        {
        (*lmi).second.ApplyScaleFactor(mult);
        }
}

//============================================================================
unsigned int Ledger::CalculateCRC() const
{
    CRC crc;
    ledger_invariant_->UpdateCRC(crc);
    ledger_map_t const& l_map_rep = ledger_map_->held();
    ledger_map_t::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); ++lmci)
        {
        (*lmci).second.UpdateCRC(crc);
        }

    return crc.value();
}

//============================================================================
void Ledger::Spew(std::ostream& os) const
{
    ledger_invariant_->Spew(os);
    ledger_map_t const& l_map_rep = ledger_map_->held();
    ledger_map_t::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); ++lmci)
        {
        (*lmci).second.Spew(os);
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
        fatal_error() << s << LMI_FLUSH;
        }
}

