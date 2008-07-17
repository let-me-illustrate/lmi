// Ledger data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger.cpp,v 1.24 2008-07-17 19:04:29 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "crc32.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "mc_enum_types_aux.hpp" // porting_cast()

#include <algorithm>
#include <ostream>

// TODO ?? Doubts and shortcomings:
//
// Is it really a good idea to have shared_ptr data members? If so,
// should there be a member function
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
// Composites typically have zeros in all columns for numerous years
// at the end, which is a defect for printing at least. Perhaps
//   ApplyScaleFactor()
// (implemented elsewhere) should be augmented (and renamed) to
// serve the more general purpose of preparing a ledger for printing;
// then, it might truncate every column to GetMaxLength() (which has
// documented problems of its own). Or perhaps this truncation should
// be a separate function, which could then be applied even for the
// composite output used for regression testing. Is it only the
// composite which stands in need of truncation?

//============================================================================
Ledger::Ledger
    (e_ledger_type const& a_LedgerType
    ,int                  a_Length
    ,bool                 a_IsComposite
    )
    :is_composite_         (a_IsComposite)
    ,composite_lapse_year_ (0.0)
    ,ledger_type_          (a_LedgerType)
    ,ledger_map_           (new ledger_map_holder)
    ,ledger_invariant_     (new LedgerInvariant)
{
    SetRunBases(a_Length);
}

//============================================================================
Ledger::~Ledger()
{
}

//============================================================================
void Ledger::SetRunBases(int a_Length)
{
    ledger_map_t& l_map_rep = ledger_map_->held_;
    switch(ledger_type_)
        {
        case mce_ill_reg:
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_mdpt_sep_full] = LedgerVariant(a_Length);
            }
            break;
        case mce_group_private_placement:      // Deliberately fall through.
        case mce_offshore_private_placement:   // Deliberately fall through.
        case mce_individual_private_placement: // Deliberately fall through.
        case mce_variable_annuity:             // Deliberately fall through.
        case mce_nasd:
            {
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(a_Length);
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
            l_map_rep[mce_run_gen_curr_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_full] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_curr_sep_zero] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_zero] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_curr_sep_half] = LedgerVariant(a_Length);
            l_map_rep[mce_run_gen_guar_sep_half] = LedgerVariant(a_Length);
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
#if 0
        e_basis exp_and_ga_basis;
        e_sep_acct_basis sa_basis;
        set_separate_bases_from_run_basis(e_run_basis(porting_cast<enum_run_basis>(key)), exp_and_ga_basis, sa_basis);

        data.SetExpAndGABasis(exp_and_ga_basis);
        data.SetSABasis(sa_basis);
#endif // 0
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

    if(ledger_type_ != a_Addend.GetLedgerType())
        {
        fatal_error()
            << "Cannot add ledgers for products with different"
            << " formatting requirements."
            << LMI_FLUSH
            ;
        }

    ledger_map_t& l_map_rep = ledger_map_->held_;
    ledger_map_t::iterator this_i = l_map_rep.begin();

    ledger_map_t const& lm_addend = a_Addend.GetLedgerMap().held();
    ledger_map_t::const_iterator addend_i = lm_addend.begin();

    ledger_invariant_->PlusEq(*a_Addend.ledger_invariant_);

    LMI_ASSERT(GetIsComposite());
    LMI_ASSERT(!a_Addend.GetIsComposite());

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
    (e_run_basis const& a_Basis
    ,LedgerVariant const& a_Variant
    )
{
    mcenum_run_basis run_basis = porting_cast<mcenum_run_basis>(a_Basis.value());
    ledger_map_t& l_map_rep = ledger_map_->held_;
    if(l_map_rep.count(run_basis))
        {
        l_map_rep[run_basis] = a_Variant;
        }
    else
        {
        fatal_error()
            << "Failed attempt to set ledger for unused basis '"
            << a_Basis
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
LedgerVariant const& Ledger::GetOneVariantLedger(mcenum_run_basis b) const
{
    ledger_map_t::const_iterator i(GetLedgerMap().held().find(b));
    if(i == GetLedgerMap().held().end())
        {
        // DEPRECATED Should display string name.
        fatal_error() << "No values for basis '" << b << "'" << LMI_FLUSH;
        }
    return i->second;
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

