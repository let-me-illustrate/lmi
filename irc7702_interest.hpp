// 7702 (and 7702A) interest rates.
//
// Copyright (C) 2020, 2021 Gregory W. Chicares.
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

#ifndef irc7702_interest_hpp
#define irc7702_interest_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <vector>

class LMI_SO_FWD_DECL product_database;
class LMI_SO_FWD_DECL stratified_charges;

LMI_SO double iglp(); // 7702 !! obsolescent
LMI_SO double igsp(); // 7702 !! obsolescent

class LMI_SO i7702 final
{
  public:
    i7702
        (product_database   const&
        ,stratified_charges const&
        );
    i7702(i7702 const&) = delete;
    i7702& operator=(i7702 const&) = delete;
    ~i7702() = default;

    std::vector<double> const& ig     () const {return ig_;  }
    std::vector<double> const& gross  () const {return gross_;  }
    std::vector<double> const& net_glp() const {return net_glp_;}
    std::vector<double> const& net_gsp() const {return net_gsp_;}
    std::vector<double> const& bogus  () const {return bogus_;  }

    // 7702 !! ephemeral
    std::vector<double> const& spread() const {return spread_;}

  private:
    product_database   const& database_;
    stratified_charges const& stratified_;

    std::vector<double> spread_;

    std::vector<double> ig_     ;
    std::vector<double> gross_  ;
    std::vector<double> net_glp_;
    std::vector<double> net_gsp_;
    // 7702 !! deprecated:
    std::vector<double> bogus_  ;
};

#endif // irc7702_interest_hpp
