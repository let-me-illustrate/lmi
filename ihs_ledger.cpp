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

// $Id: ihs_ledger.cpp,v 1.2 2005-01-31 13:12:48 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_ledger.hpp"

#include "alert.hpp"
#include "crc32.hpp"
#include "ihs_ldginvar.hpp"
#include "ihs_ldgvar.hpp"
#include "ihs_proddata.hpp"

#include <algorithm>
#include <ostream>

//============================================================================
TLedger::TLedger
    (e_ledger_type const& a_LedgerType
    ,int                  a_Length
    ,bool                 a_IsComposite
    )
    :IsComposite        (a_IsComposite)
    ,CompositeLapseYear (0.0)
    ,LedgerType         (a_LedgerType)
{
    Alloc();
    SetRunBases(a_Length);
}

//============================================================================
TLedger::TLedger(TLedger const& obj)
    :streamable()
{
    Alloc();
    Copy(obj);
}

//============================================================================
TLedger& TLedger::operator=(TLedger const& obj)
{
    if(this != &obj)
        {
        Destroy();
        Alloc();
        Copy(obj);
        }
    return *this;
}

//============================================================================
TLedger::~TLedger()
{
    Destroy();
}

//============================================================================
void TLedger::Alloc()
{
    LedgerMap = new LMap;
    LedgerInvariant = new TLedgerInvariant;
}

//============================================================================
void TLedger::Copy(TLedger const& obj)
{
    *LedgerMap          = *obj.LedgerMap;
    *LedgerInvariant    = *obj.LedgerInvariant;
    IsComposite         = obj.IsComposite;
    LedgerType          = obj.LedgerType;
    CompositeLapseYear  = obj.CompositeLapseYear;
}

//============================================================================
void TLedger::Destroy()
{
    delete LedgerInvariant;
    delete LedgerMap;
}

//============================================================================
void TLedger::SetRunBases(int a_Length)
{
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    switch(LedgerType)
        {
        case e_ill_reg:     // {current, midpoint, guaranteed}
        case e_ill_reg_private_placement:
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_mdpt_basis)]         = TLedgerVariant(a_Length);
            }
            break;
        case e_group_private_placement:          // [format not yet final]
        case e_offshore_private_placement:       // [format not yet final]
        case e_individual_private_placement:     // [format not yet final]
                            // fall through: same as NASD for now
        case e_nasd:        // {current, 0% int} X {guar charges, curr charges}
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_zero)] = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_zero)] = TLedgerVariant(a_Length);
            }
            break;
        case e_prospectus:  // {current, 0% int, 1/2 int%} X {guar, curr}
            {
            l_map_rep[e_run_basis(e_run_curr_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis)]         = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_zero)] = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_zero)] = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_curr_basis_sa_half)] = TLedgerVariant(a_Length);
            l_map_rep[e_run_basis(e_run_guar_basis_sa_half)] = TLedgerVariant(a_Length);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << LedgerType
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    for
        (ledger_map::iterator p = l_map_rep.begin()
        ;p != l_map_rep.end()
        ;p++
        )
        {
        ledger_map::key_type const& key = (*p).first;
        ledger_map::mapped_type& data = (*p).second;

        RunBases.push_back(key);

        e_basis exp_and_ga_basis;
        e_sep_acct_basis sa_basis;
        set_separate_bases_from_run_basis(key, exp_and_ga_basis, sa_basis);

        data.SetExpAndGABasis(exp_and_ga_basis);
        data.SetSABasis(sa_basis);

        if(IsComposite)
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
void TLedger::ZeroInforceAfterLapse()
{
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
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
        this_i++;
        }
    std::vector<double>::iterator b =
            LedgerInvariant->InforceLives.begin()
        +   1
        +   lapse_year
        ;
    std::vector<double>::iterator e = LedgerInvariant->InforceLives.end();
    if(b < e)
        {
        std::fill(b, e, 0.0);
        }
}

//============================================================================
TLedger& TLedger::PlusEq(TLedger const& a_Addend)
{
    // TODO ?? We should look at other things like Smoker and handle
    // them in some appropriate manner if they differ across
    // lives in a composite.
    if(LedgerType != a_Addend.GetLedgerType())
        {
        fatal_error()
            << "Cannot add ledgers for products with different"
            << " formatting requirements."
            << LMI_FLUSH
            ;
        }

    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    ledger_map::iterator this_i = l_map_rep.begin();

    ledger_map const& lm_addend = a_Addend.GetLedgerMap().LedgerMapRep;
    ledger_map::const_iterator addend_i = lm_addend.begin();

    LedgerInvariant->PlusEq(*a_Addend.LedgerInvariant);

    LMI_ASSERT(GetIsComposite());
    LMI_ASSERT(!a_Addend.GetIsComposite());

    while(this_i != l_map_rep.end() || addend_i != lm_addend.end())
        {
        LMI_ASSERT((*this_i).first == (*addend_i).first);
        (*this_i).second.PlusEq
            ((*addend_i).second
            ,a_Addend.LedgerInvariant->GetInforceLives()
            );
        CompositeLapseYear = std::max
            (CompositeLapseYear
            ,(*addend_i).second.LapseYear
            );
        (*this_i).second.LapseYear = std::max
            ((*this_i).second.LapseYear
            ,(*addend_i).second.LapseYear
            );
        this_i++, addend_i++;
        }

    LMI_ASSERT(this_i == l_map_rep.end() && addend_i == lm_addend.end());

    return *this;
}

//============================================================================
void TLedger::SetLedgerInvariant(TLedgerInvariant const& a_Invariant)
{
    *LedgerInvariant = a_Invariant;
}

//============================================================================
void TLedger::SetGuarPremium(double a_GuarPrem)
{
    LedgerInvariant->GuarPrem = a_GuarPrem;
}

//============================================================================
void TLedger::SetOneLedgerVariant
    (e_run_basis const& a_Basis
    ,TLedgerVariant const& a_Variant
    )
{
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    if(l_map_rep.count(a_Basis))
        {
        l_map_rep[a_Basis] = a_Variant;
        }
    else
        {
        hobsons_choice() << "Setting ledger for unused basis." << LMI_FLUSH;
        }
}

//============================================================================
int TLedger::GetMaxLength() const
{
    if(IsComposite)
        {
        // TODO ?? The rationale for this special case is not evident.
        // This variable is initialized to zero in the ctor. If it has
        // acquired a value that's actually meaningful, then it looks
        // like the variables used in the general case would have, too,
        // which would reduce this special case to mere caching, which
        // seems like a premature optimization and a needless
        // complication.
        return static_cast<int>(CompositeLapseYear);
        }

    // For all ledgers in the map:
    //   find the longest duration we need to print (until the last one lapses)
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    double max_length = 0.0;

    for
        (ledger_map::const_iterator lmci = l_map_rep.begin()
        ;lmci != l_map_rep.end()
        ;lmci++
        )
        {
        max_length = std::max(max_length, (*lmci).second.LapseYear);
        }
    return static_cast<int>(max_length);
}

//============================================================================
void TLedger::AutoScale()
{
    // For the invariant ledger and all variant ledgers in the map:

    // First find the largest number in any ledger
    double mult = LedgerInvariant->DetermineScaleFactor();

    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    ledger_map::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); lmci++)
        {
        mult = std::min(mult, (*lmci).second.DetermineScaleFactor());
        }

    // Now that we have the scale factor, apply it to all:
    // scale all according to the biggest number in any
    LedgerInvariant->ApplyScaleFactor(mult);

    ledger_map::iterator lmi = l_map_rep.begin();
    for(;lmi != l_map_rep.end(); lmi++)
        {
        (*lmi).second.ApplyScaleFactor(mult);
        }
}

//============================================================================
unsigned int TLedger::CalculateCRC() const
{
    CRC crc;
    LedgerInvariant->UpdateCRC(crc);
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    ledger_map::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); lmci++)
        {
        (*lmci).second.UpdateCRC(crc);
        }

    return crc.value();
}

//============================================================================
void TLedger::Spew(std::ostream& os) const
{
    LedgerInvariant->Spew(os);
    ledger_map& l_map_rep = LedgerMap->LedgerMapRep;
    ledger_map::const_iterator lmci = l_map_rep.begin();
    for(;lmci != l_map_rep.end(); lmci++)
        {
        (*lmci).second.Spew(os);
        }
}

//============================================================================
TLedgerVariant const& TLedger::GetCurrFull() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_curr_basis))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetGuarFull() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_guar_basis))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetMdptFull() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_mdpt_basis))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetCurrZero() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_curr_basis_sa_zero))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetGuarZero() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_guar_basis_sa_zero))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetCurrHalf() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_curr_basis_sa_half))).second;
}

//============================================================================
TLedgerVariant const& TLedger::GetGuarHalf() const
{
    return (*GetLedgerMap().LedgerMapRep.find(e_run_basis(e_run_guar_basis_sa_half))).second;
}

#if 0
#   if !defined BC_BEFORE_5_5 && !defined GCC_BEFORE_2_96
#      include "ledger_xml_io.cpp"
#   else // Using obsolete tools that can't cope with standard C++.
    int TLedger::class_version() const {return 0;}
    void TLedger::read(xml::node&) {}
    void TLedger::write(xml::node& x) const {}
    std::string TLedger::xml_root_name() const {return "";}
#   endif // Using obsolete tools that can't cope with standard C++.
#endif // 0

