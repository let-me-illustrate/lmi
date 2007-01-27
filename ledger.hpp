// Ledger data.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ledger.hpp,v 1.20 2007-01-27 00:00:51 wboutin Exp $

#ifndef ledger_hpp
#define ledger_hpp

#include "config.hpp"

#include "ledger_excerpt.hpp" // enum_xml_version
#include "so_attributes.hpp"
#include "streamable.hpp"
#include "xenumtypes.hpp"

#include <boost/shared_ptr.hpp>

#include <iosfwd>
#include <vector>

/// Design notes for class Ledger.
///
/// This class holds all the data needed to print an illustration.
/// Class AccountValue generates the data held here, but also stores a
/// great deal of input and intermediate data that can be discarded to
/// save space. This class's data could be saved to permit deferred
/// generation of originally-unforeseen reports without repeating
/// monthiversary processing; that might be a useful enhancement
/// someday, but the calculations are fast enough that no user has
/// asked for it.
///
/// Some values vary by calculation basis (current, guaranteed, etc.)
/// and are stored in a map whose key_type represents that basis. Other
/// values do not vary by basis, and accordingly are stored in a single
/// data structure.
///
/// The implicitly-defined copy ctor and copy assignment operator do
/// the right thing. TODO ?? However, the "right thing" may be somewhat
/// surprising if the shared_ptr members are misunderstood.

class LedgerInvariant;
class LedgerVariant;
class ledger_map_holder;

class LMI_SO Ledger
    :virtual public streamable
{
  public:
    Ledger(); // TODO ?? Remove after expunging 'calculate.hpp'.

    // TODO ?? It's pretty weak to use 100 as a default max length.
    // But when running a composite, life by life, we don't know the max
    // length until we've initialized all lives. We could get the min age
    // by inspecting the input parms for all lives, but to get the age at
    // endt requires initializing each life and doing a database lookup
    // for endt age. Yet 100 won't work for issue age 0 if coverage
    // beyond age 100 is to be shown.
    explicit Ledger
        (e_ledger_type const& a_LedgerType
        ,int                  a_Length        = 100
        ,bool                 a_IsComposite   = false
        );
    virtual ~Ledger();

    void ZeroInforceAfterLapse();
    Ledger& PlusEq(Ledger const& a_Addend);

    void SetLedgerInvariant(LedgerInvariant const&);
    void SetOneLedgerVariant(e_run_basis const&, LedgerVariant const&);

    void SetGuarPremium(double);

    void AutoScale();

    ledger_map_holder const&        GetLedgerMap()       const;
    LedgerInvariant const&          GetLedgerInvariant() const;
    LedgerVariant const&            GetCurrFull()        const;
    LedgerVariant const&            GetGuarFull()        const;
    LedgerVariant const&            GetMdptFull()        const;
    LedgerVariant const&            GetCurrZero()        const;
    LedgerVariant const&            GetGuarZero()        const;
    LedgerVariant const&            GetCurrHalf()        const;
    LedgerVariant const&            GetGuarHalf()        const;

    e_ledger_type const&            GetLedgerType()      const;
    int                             GetMaxLength()       const;
    std::vector<e_run_basis> const& GetRunBases()        const;
    bool                            GetIsComposite()     const;

    unsigned int CalculateCRC() const;
    void Spew(std::ostream& os) const;

    // Class 'streamable' required implementation.
    virtual void read (xml::element const&);
    virtual void write(xml::element&) const;
    virtual int class_version() const;
    virtual std::string xml_root_name() const;

    void write(std::ostream& os) const;

    void write_excerpt(xml::element&, enum_xml_version) const;

    void writeXXX(xml::element&) const;
    void writeXXX(std::ostream& os) const;

  private:
    LedgerVariant const& GetOneVariantLedger(enum_run_basis) const;
    void SetRunBases(int a_Length);

    bool is_composite_;

    // TODO ?? This is either badly named or badly implemented. Every
    // instance of this class, even an instance for a single cell, has
    // this variable. It seems to be used only for composites. It
    // seems to cache the latest lapse year on any basis, but only for
    // composites, yet it is not evident why that is desirable for
    // composites but not for all cells.
    double composite_lapse_year_;

    e_ledger_type ledger_type_;

    boost::shared_ptr<ledger_map_holder> ledger_map_;
    boost::shared_ptr<LedgerInvariant>   ledger_invariant_;

    // It is convenient to have a vector that holds just the run bases,
    // i.e. just the key_type members of ledger_map_. We can use this for
    // iterating across all bases without exposing the map's data_type,
    // from which we want to shield other classes where possible.
    std::vector<e_run_basis> run_bases_;
};

inline ledger_map_holder const& Ledger::GetLedgerMap() const
{
    return *ledger_map_;
}

inline LedgerInvariant const& Ledger::GetLedgerInvariant() const
{
    return *ledger_invariant_;
}

inline e_ledger_type const& Ledger::GetLedgerType() const
{
    return ledger_type_;
}

inline std::vector<e_run_basis> const& Ledger::GetRunBases() const
{
    return run_bases_;
}

inline bool Ledger::GetIsComposite() const
{
    return is_composite_;
}

#endif // ledger_hpp

