// 7PP and CVAT corridor from first principles.
//
// Copyright (C) 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef irc7702_tables_hpp
#define irc7702_tables_hpp

#include "config.hpp"

#include "mc_enum_types.hpp"
#include "oecumenic_enumerations.hpp"
#include "so_attributes.hpp"

#include <vector>

class LMI_SO irc7702_tables final
{
    friend void Test_Corridor_and_7PP();

  public:
    irc7702_tables
        (mcenum_cso_era
        ,oenum_autopisty
        ,oenum_alb_or_anb
        ,mcenum_gender
        ,mcenum_smoking
        ,std::vector<double> const& operative_i
        ,double                     max_coi_rate
        // Potentially defaultable.
        ,int                        min_age
        ,int                        max_age
        );
    std::vector<double> const& ul_corr() {return ul_corr_;}
    std::vector<double> const& ul_7pp () {return ul_7pp_ ;}
    std::vector<double> const& ol_corr() {return ol_corr_;}
    std::vector<double> const& ol_7pp () {return ol_7pp_ ;}

  private:
    std::vector<double> const q_       {};
    std::vector<double> const i_       {};
    int                 const length_  {};
    std::vector<double>       ul_corr_ {};
    std::vector<double>       ul_7pp_  {};
    std::vector<double>       ol_corr_ {};
    std::vector<double>       ol_7pp_  {};
};

#endif // irc7702_tables_hpp
