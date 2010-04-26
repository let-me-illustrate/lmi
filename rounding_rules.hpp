// Rounding rules.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
#include "obstruct_slicing.hpp"
#include "round_to.hpp"
#include "so_attributes.hpp"
#include "xml_serializable.hpp"

#include <boost/utility.hpp>

#include <string>

/// Product rounding rules.
///
/// The principle is to round transactional changes to account value,
/// rather than rounding account value itself after each transaction.
/// This is the way admin systems are most likely to work, and
/// gratuitous differences between systems should be avoided. Thus,
/// rounding is a property of the transaction, and not of the variable
/// it ultimately affects.
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
    :        private boost::noncopyable
    ,virtual private obstruct_slicing  <rounding_rules>
    ,        public  xml_serializable  <rounding_rules>
    ,        public  MemberSymbolTable <rounding_rules>
{
    friend class RoundingDocument;

  public:
    explicit rounding_rules(std::string const& filename);
    ~rounding_rules();

    round_to<double> const& datum(std::string const& name);

    // Legacy functions to support creating product files programmatically.
    static void write_rounding_files();
    static void write_proprietary_rounding_files();

  private:
    rounding_rules();

    void ascribe_members();

    // xml_serializable required implementation.
    virtual int         class_version() const;
    virtual std::string xml_root_name() const;

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

    round_to<double> round_specamt_           ;
    round_to<double> round_death_benefit_     ;
    round_to<double> round_naar_              ;
    round_to<double> round_coi_rate_          ;
    round_to<double> round_coi_charge_        ;
    round_to<double> round_gross_premium_     ;
    round_to<double> round_net_premium_       ;
    round_to<double> round_interest_rate_     ;
    round_to<double> round_interest_credit_   ;
    round_to<double> round_withdrawal_        ;
    round_to<double> round_loan_              ;
    round_to<double> round_corridor_factor_   ;
    round_to<double> round_surrender_charge_  ;
    round_to<double> round_irr_               ;
    round_to<double> round_min_specamt_       ;
    round_to<double> round_max_specamt_       ;
    round_to<double> round_min_premium_       ;
    round_to<double> round_max_premium_       ;
    round_to<double> round_interest_rate_7702_;
};

#endif // rounding_rules_hpp

