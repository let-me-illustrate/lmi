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

#ifndef i7702_hpp
#define i7702_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <vector>

class LMI_SO_FWD_DECL product_database;
class LMI_SO_FWD_DECL stratified_charges;

class LMI_SO i7702 final
{
    friend struct i7702_test;

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

  private:
    // Private ctor for unit test.
    i7702
        (double                     A0
        ,double                     A1
        ,std::vector<double> const& Bgen
        ,std::vector<double> const& Bsep
        ,std::vector<double> const& Bflr
        ,std::vector<double> const& Bvlr
        ,std::vector<double> const& Cgen
        ,std::vector<double> const& Csep
        ,std::vector<double> const& Cflr
        ,std::vector<double> const& Cvlr
        ,std::vector<double> const& Dgen
        ,std::vector<double> const& Dsep
        ,std::vector<double> const& Dflr
        ,std::vector<double> const& Dvlr
        ,std::vector<double> const& Em
        );

    void initialize();

    // Parameters from product database--member names are
    // capitalized to match formulas in documentation, but
    // suffixed to mark them as members.
    double              A0_   ;
    double              A1_   ;
    std::vector<double> Bgen_ ;
    std::vector<double> Bsep_ ;
    std::vector<double> Bflr_ ;
    std::vector<double> Bvlr_ ;
    std::vector<double> Cgen_ ;
    std::vector<double> Csep_ ;
    std::vector<double> Cflr_ ;
    std::vector<double> Cvlr_ ;
    std::vector<double> Dgen_ ;
    std::vector<double> Dsep_ ;
    std::vector<double> Dflr_ ;
    std::vector<double> Dvlr_ ;
    std::vector<double> Em_   ;

    // Derived 7702 interest rates.
    std::vector<double> ig_     ;
    std::vector<double> gross_  ;
    std::vector<double> net_glp_;
    std::vector<double> net_gsp_;
};

#endif // i7702_hpp
