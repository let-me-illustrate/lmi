// Ledger data.
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

// $Id: ledger.cpp,v 1.4 2005-04-14 01:26:47 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger.hpp"

#include "alert.hpp"
#include "crc32.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"

#include <algorithm>
#include <ostream>

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
    ledger_map& l_map_rep = ledger_map_->held_;
    switch(ledger_type_)
        {
        case e_ill_reg:     // {current, midpoint, guaranteed}
        case e_ill_reg_private_placement:
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_mdpt_basis)]         = LedgerVariant(a_Length);
            }
            break;
        case e_group_private_placement:          // [format not yet final]
        case e_offshore_private_placement:       // [format not yet final]
        case e_individual_private_placement:     // [format not yet final]
                            // fall through: same as NASD for now
        case e_nasd:        // {current, 0% int} X {guar charges, curr charges}
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_zero)] = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_zero)] = LedgerVariant(a_Length);
            }
            break;
        case e_prospectus:  // {current, 0% int, 1/2 int%} X {guar, curr}
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_zero)] = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_zero)] = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_half)] = LedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_half)] = LedgerVariant(a_Length);
            }
            break;
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
        (ledger_map::iterator p = l_map_rep.begin()
        ;p != l_map_rep.end()
        ;++p
        )
        {
        ledger_map::key_type const& key = (*p).first;
        ledger_map::mapped_type& data = (*p).second;

        run_bases_.push_back(key);

        e_basis exp_and_ga_basis;
        e_sep_acct_basis sa_basis;
        set_separate_bases_from_run_basis(key, exp_and_ga_basis, sa_basis);

        data.SetExpAndGABasis(exp_and_ga_basis);
        data.SetSABasis(sa_basis);

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
void Ledger::ZeroInforceAfterLapse()
{
    ledger_map const& l_map_rep = ledger_map_->held();
    ledger_map::const_iterator this_i = l_map_rep.begin();

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
    if(ledger_type_ != a_Addend.GetLedgerType())
        {
        fatal_error()
            << "Cannot add ledgers for products with different"
            << " formatting requirements."
            << LMI_FLUSH
            ;
        }

    ledger_map& l_map_rep = ledger_map_->held_;
    ledger_map::iterator this_i = l_map_rep.begin();

    ledger_map const& lm_addend = a_Addend.GetLedgerMap().held();
    ledger_map::const_iterator addend_i = lm_addend.begin();

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
    ledger_map& l_map_rep = ledger_map_->held_;
    if(l_map_rep.count(a_Basis))
        {
        l_map_rep[a_Basis] = a_Variant;
        }
    else
        {
        hobsons_choice()
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

    // For all ledgers in the map:
    //   find the longest duration we need to print (until the last one lapses)
    ledger_map const& l_map_rep = ledger_map_->held();
    double max_length = 0.0;

    for
        (ledger_map::const_iterator lmci = l_map_rep.begin()
        ;lmci != l_map_rep.end()
        ;++lmci
        )
        {
        max_length = std::max(max_length, (*lmci).second.LapseYear);
        }
    return static_cast<int>(max_length);
}

//============================================================================
void Ledger::AutoScale()
{
    // For the invariant ledger and all variant ledgers in the map:

    // First find the largest number in any ledger
    double mult = ledger_invariant_->DetermineScaleFactor();

    ledger_map& l_map_rep = ledger_map_->held_;
    ledger_map::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); ++lmci)
        {
        mult = std::min(mult, (*lmci).second.DetermineScaleFactor());
        }

    // Now that we have the scale factor, apply it to all:
    // scale all according to the biggest number in any
    ledger_invariant_->ApplyScaleFactor(mult);

    ledger_map::iterator lmi = l_map_rep.begin();
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
    ledger_map const& l_map_rep = ledger_map_->held();
    ledger_map::const_iterator lmci = l_map_rep.begin();
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
    ledger_map const& l_map_rep = ledger_map_->held();
    ledger_map::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); ++lmci)
        {
        (*lmci).second.Spew(os);
        }
}

//============================================================================
LedgerVariant const& Ledger::GetCurrFull() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_curr_basis))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetGuarFull() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_guar_basis))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetMdptFull() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_mdpt_basis))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetCurrZero() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_curr_basis_sa_zero))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetGuarZero() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_guar_basis_sa_zero))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetCurrHalf() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_curr_basis_sa_half))).second;
}

//============================================================================
LedgerVariant const& Ledger::GetGuarHalf() const
{
    return (*GetLedgerMap().held().find(e_run_basis(e_run_guar_basis_sa_half))).second;
}

#if 0
#   if !defined BC_BEFORE_5_5 && !defined GCC_BEFORE_2_96
#      include "ledger_xml_io.cpp"
#   else // Using obsolete tools that can't cope with standard C++.
    int Ledger::class_version() const {return 0;}
    void Ledger::read(xml::node&) {}
    void Ledger::write(xml::node& x) const {}
    std::string Ledger::xml_root_name() const {return "";}
#   endif // Using obsolete tools that can't cope with standard C++.
#endif // 0

