// Ledger data.
//
// Copyright (C) 1998, 2001, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#ifndef ledger_hpp
#define ledger_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"
#include "so_attributes.hpp"
#include "xml_lmi.hpp"

#include <boost/shared_ptr.hpp>

#include <iosfwd>
#include <string>
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
/// the right thing. However, the "right thing" may be somewhat
/// surprising due to the shared_ptr members; see the comment on
///   https://savannah.nongnu.org/bugs/?13599
/// in the implementation file.

class LedgerInvariant;
class LedgerVariant;
class ledger_map_holder;

class LMI_SO Ledger
{
  public:
    explicit Ledger
        (int                length
        ,mcenum_ledger_type ledger_type
        ,bool               nonillustrated
        ,bool               no_can_issue
        ,bool               is_composite
        );
    virtual ~Ledger();

    void ZeroInforceAfterLapse();
    Ledger& PlusEq(Ledger const& a_Addend);

    void SetLedgerInvariant(LedgerInvariant const&);
    void SetOneLedgerVariant(mcenum_run_basis, LedgerVariant const&);

    void SetGuarPremium(double);

    void AutoScale();

    ledger_map_holder const&             GetLedgerMap       () const;
    LedgerInvariant const&               GetLedgerInvariant () const;
    LedgerVariant const&                 GetCurrFull        () const;
    LedgerVariant const&                 GetGuarFull        () const;
    LedgerVariant const&                 GetMdptFull        () const;
    LedgerVariant const&                 GetCurrZero        () const;
    LedgerVariant const&                 GetGuarZero        () const;
    LedgerVariant const&                 GetCurrHalf        () const;
    LedgerVariant const&                 GetGuarHalf        () const;

    int                                  GetMaxLength       () const;
    std::vector<mcenum_run_basis> const& GetRunBases        () const;

    mcenum_ledger_type                   ledger_type        () const;
    bool                                 nonillustrated     () const;
    bool                                 no_can_issue       () const;
    bool                                 is_composite       () const;

    unsigned int CalculateCRC() const;
    void Spew(std::ostream& os) const;

    void read (xml::element const&);
    void write(xml::element&) const;
    int                class_version() const;
    std::string const& xml_root_name() const;

    void write       (std::ostream& os) const;
    void write_xsl_fo(std::ostream& os) const;

  private:
    LedgerVariant const& GetOneVariantLedger(mcenum_run_basis) const;
    void SetRunBases(int length);

    // These members store ctor arguments whose values cannot be set
    // otherwise. The rationale for storing them here (rather than in
    // class LedgerInvariant, e.g.) is that they are not data from
    // which reports are generated--rather, they govern how reports
    // are generated, and which reports are permitted.
    //
    // Naming: "is_composite_" because "composite" could be taken as a
    // noun, suggesting a composite ledger; "nonillustrated_" without
    // "is_" because it's unambiguously adjectival.
    mcenum_ledger_type ledger_type_   ;
    bool               nonillustrated_;
    bool               no_can_issue_  ;
    bool               is_composite_  ;

    // TODO ?? This is either badly named or badly implemented. Every
    // instance of this class, even an instance for a single cell, has
    // this variable. It seems to be used only for composites. It
    // seems to cache the latest lapse year on any basis, but only for
    // composites, yet it is not evident why that is desirable for
    // composites but not for all cells.
    double composite_lapse_year_;

    boost::shared_ptr<ledger_map_holder> ledger_map_;
    boost::shared_ptr<LedgerInvariant>   ledger_invariant_;

    // It is convenient to have a vector that holds just the run bases,
    // i.e. just the key_type members of ledger_map_. We can use this for
    // iterating across all bases without exposing the map's data_type,
    // from which we want to shield other classes where possible.
    std::vector<mcenum_run_basis> run_bases_;
};

std::vector<double> numeric_vector
    (Ledger const&      ledger
    ,std::string const& compound_name
    );

bool LMI_SO is_interdicted      (Ledger const&);
void LMI_SO throw_if_interdicted(Ledger const&);

#endif // ledger_hpp

