// 7702 (and 7702A) interest rates.
//
// Copyright (C) 2020, 2021, 2022 Gregory W. Chicares.
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

    std::vector<double> const& ic_usual() const {return ic_usual_;}
    std::vector<double> const& ic_glp  () const {return ic_glp_  ;}
    std::vector<double> const& ic_gsp  () const {return ic_gsp_  ;}
    std::vector<double> const& ig_usual() const {return ig_usual_;}
    std::vector<double> const& ig_glp  () const {return ig_glp_  ;}
    std::vector<double> const& ig_gsp  () const {return ig_gsp_  ;}

  private:
    // Private ctor for unit test.
    i7702
        (int                        length
        ,double                     A0
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
        ,std::vector<double> const& use_gen
        ,std::vector<double> const& use_sep
        ,std::vector<double> const& use_flr
        ,std::vector<double> const& use_vlr
        );

    void assert_preconditions();
    void initialize();

    int  const length_;
    bool const trace_;

    // Rates from product database--member names are capitalized to
    // match formulas in documentation, but suffixed to mark them as
    // members. All are annual rates except 'Em_', which is monthly.
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

    // Booleans from product database--stored as doubles because
    // std::vector<bool> is not a container. They are vectors
    // rather than scalars to allow durational enablement--e.g.,
    // variable-rate loans might be forbidden in the issue year.
    std::vector<double> use_gen_;
    std::vector<double> use_sep_;
    std::vector<double> use_flr_;
    std::vector<double> use_vlr_;

    // Derived 7702 interest rates--all monthly.
    std::vector<double> ic_usual_;
    std::vector<double> ic_glp_  ;
    std::vector<double> ic_gsp_  ;
    std::vector<double> ig_usual_;
    std::vector<double> ig_glp_  ;
    std::vector<double> ig_gsp_  ;
};

#endif // i7702_hpp
