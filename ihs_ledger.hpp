// Ledger data.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_ledger.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef ihs_ledger_hpp
#define ihs_ledger_hpp

#ifdef ledger_hpp
#   error Probable lmi/ihs header conflict.
#endif // ledger_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "streamable.hpp"
#include "xenumtypes.hpp"

#include <iosfwd>
#include <vector>

// This class holds all the output values calculated by class AccountValue.
// Values that vary by basis (curr, guar, etc.) are in a map of TLedgerVariant.
// Values that do not vary by basis are in a TLedgerInvariant member.
//
// Motivation: this class holds all the information needed to print an
// illustration. This lets us discard the sizeable overhead of AccountValue
// and keep only the output values after calculations are done for one life.

class TLedgerInvariant;
class TLedgerVariant;
class LMap;

class LMI_EXPIMP TLedger
    :virtual public streamable
{
public:
    // TODO ?? It's pretty weak to use 100 as a default max length.
    // But when running a composite, life by life, we don't know the max
    // length until we've initialized all lives. We could get the min age
    // by inspecting the input parms for all lives, but to get the age at
    // endt requires initializing each life and doing a database lookup
    // for endt age. Yet 100 won't work for issue age 0 if coverage
    // beyond age 100 is to be shown.
    TLedger
        (e_ledger_type const& a_LedgerType
        ,int                  a_Length        = 100
        ,bool                 a_IsComposite   = false
        );
    TLedger(TLedger const&);
    TLedger& operator=(TLedger const&);
    virtual ~TLedger();

    void                ZeroInforceAfterLapse();
    TLedger&            PlusEq(TLedger const& a_Addend);

    void                SetLedgerInvariant
                            (TLedgerInvariant const& a_Invariant
                            );
    void                SetOneLedgerVariant
                            (e_run_basis const& a_Basis
                            ,TLedgerVariant const& a_Variant
                            );
    void                SetGuarPremium(double);

    void                AutoScale();

    LMap const&                     GetLedgerMap()          const;
    TLedgerInvariant const&         GetLedgerInvariant()    const;
    TLedgerVariant const&           GetCurrFull()           const;
    TLedgerVariant const&           GetGuarFull()           const;
    TLedgerVariant const&           GetMdptFull()           const;
    TLedgerVariant const&           GetCurrZero()           const;
    TLedgerVariant const&           GetGuarZero()           const;
    TLedgerVariant const&           GetCurrHalf()           const;
    TLedgerVariant const&           GetGuarHalf()           const;

    e_ledger_type const&            GetLedgerType()         const;
    int                             GetMaxLength()          const;
    std::vector<e_run_basis> const& GetRunBases()           const;
    bool                            GetIsComposite()        const;

    unsigned int        CalculateCRC() const;
    void                Spew(std::ostream& os) const;

    void read(xml::node&);
    void write(xml::node&) const;
    int class_version() const;
    std::string xml_root_name() const;

    void write(std::ostream& os) const;

private:
    void                Alloc();
    void                Copy(TLedger const&);
    void                Destroy();

    void                SetRunBases(int a_Length);

    bool                IsComposite;

    // TODO ?? This is either badly named or badly implemented. Every
    // instance of this class, even an instance for a single cell, has
    // this variable. It seems to be used only for composites. It
    // seems to cache the latest lapse year on any basis, but only for
    // composites, but it is not evident why that is desirable for
    // composites but not for all cells.
    double              CompositeLapseYear;

    // TODO ?? Consider making this a smart ptr?
    // but std::auto_ptr requires that the size be known...
    LMap*               LedgerMap;
    TLedgerInvariant*   LedgerInvariant;
    e_ledger_type       LedgerType;

    // It is convenient to have a vector that holds just the run bases,
    // i.e. just the key_type members of LedgerMap. We can use this for
    // iterating across all bases without exposing the map's data_type,
    // from which we want to shield other classes where possible.
    std::vector<e_run_basis> RunBases;
};

inline LMap const& TLedger::GetLedgerMap() const
{
    return *LedgerMap;
}

inline TLedgerInvariant const& TLedger::GetLedgerInvariant() const
{
    return *LedgerInvariant;
}

inline e_ledger_type const& TLedger::GetLedgerType() const
{
    return LedgerType;
}

inline std::vector<e_run_basis> const& TLedger::GetRunBases() const
{
    return RunBases;
}

inline bool TLedger::GetIsComposite() const
{
    return IsComposite;
}

#endif // ihs_ledger_hpp

