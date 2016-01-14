// Rounding rules.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#ifndef rounding_rules_hpp
#define rounding_rules_hpp

#include "config.hpp"

#include "any_member.hpp"
#include "mc_enum.hpp"
#include "mc_enum_types.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "xml_serializable.hpp"

#include <string>

/// Parameters of a rounding rule.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO rounding_parameters
    :virtual private obstruct_slicing<rounding_parameters>
{
    friend class rounding_rules;

  public:
    rounding_parameters
        (int                decimals
        ,rounding_style     style
        ,std::string const& gloss = std::string()
        );
    ~rounding_parameters();

    bool operator==(rounding_parameters const&) const;

    int                       decimals() const;
    mce_rounding_style const& style   () const;
    std::string const&        gloss   () const;

    rounding_style            raw_style() const;

  private:
    rounding_parameters();

    int                decimals_;
    mce_rounding_style style_   ;
    std::string        gloss_   ;
};

/// Product rounding rules.
///
/// The principle is to round transactional changes to account value,
/// rather than rounding account value itself after each transaction.
/// This is the way admin systems are most likely to work, and
/// gratuitous differences between systems should be avoided. Thus,
/// rounding is a property of the transaction, and not of the variable
/// it ultimately affects.
///
/// Implicitly-declared special member functions do the right thing.
///
/// Notes on particular rounding functors.
///
/// IRR must be rounded down in order to avoid overstating it, but its
/// precision is an arbitrary parameter.
///
/// Premium and specamt must be rounded in the right direction for
/// 7702 and 7702A calculations. In rounding functors' names:
///   min- means we want the minimum amount that satisfies some
///     condition, so we have to round it up to be sure; and
///   max- means the opposite.
/// The 7702 and 7702A interest rate must be rounded up, if at all.

class LMI_SO rounding_rules
    :        private lmi::uncopyable   <rounding_rules>
    ,virtual private obstruct_slicing  <rounding_rules>
    ,        public  xml_serializable  <rounding_rules>
    ,        public  MemberSymbolTable <rounding_rules>
{
    friend class RoundingDocument;

  public:
    explicit rounding_rules(std::string const& filename);
    ~rounding_rules();

    rounding_parameters const& datum(std::string const& name) const;

    // Legacy functions to support creating product files programmatically.
    static void write_rounding_files();
    static void write_proprietary_rounding_files();

  private:
    rounding_rules();

    void ascribe_members();

    // xml_serializable required implementation.
    virtual int                class_version() const;
    virtual std::string const& xml_root_name() const;

    // xml_serializable overrides.
    virtual void read_element
        (xml::element const& e
        ,std::string const&  name
        ,int                 file_version
        );
    virtual void write_element
        (xml::element&       parent
        ,std::string const&  name
        ) const;
    virtual void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_leaf_name
        ) const;

    rounding_parameters round_specamt_           ;
    rounding_parameters round_death_benefit_     ;
    rounding_parameters round_naar_              ;
    rounding_parameters round_coi_rate_          ;
    rounding_parameters round_coi_charge_        ;
    rounding_parameters round_gross_premium_     ;
    rounding_parameters round_net_premium_       ;
    rounding_parameters round_interest_rate_     ;
    rounding_parameters round_interest_credit_   ;
    rounding_parameters round_withdrawal_        ;
    rounding_parameters round_loan_              ;
    rounding_parameters round_interest_rate_7702_;
    rounding_parameters round_corridor_factor_   ;
    rounding_parameters round_nsp_rate_7702_     ;
    rounding_parameters round_seven_pay_rate_    ;
    rounding_parameters round_surrender_charge_  ;
    rounding_parameters round_irr_               ;
    rounding_parameters round_min_specamt_       ;
    rounding_parameters round_max_specamt_       ;
    rounding_parameters round_min_premium_       ;
    rounding_parameters round_max_premium_       ;
    rounding_parameters round_min_init_premium_  ;
};

void LMI_SO load(rounding_rules      &, fs::path const&);
void LMI_SO save(rounding_rules const&, fs::path const&);

#endif // rounding_rules_hpp

