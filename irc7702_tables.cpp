// 7PP and CVAT corridor from first principles.
//
// Copyright (C) 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "irc7702_tables.hpp"

#include "assert_lmi.hpp"
#include "commutation_functions.hpp"
#include "cso_table.hpp"
#include "et_vector.hpp"
#include "math_functions.hpp"
#include "ssize_lmi.hpp"

irc7702_tables::irc7702_tables
    (mcenum_cso_era             cso_era
    ,oenum_autopisty            autopisty
    ,oenum_alb_or_anb           alb_or_anb
    ,mcenum_gender              gender
    ,mcenum_smoking             smoking
    ,std::vector<double> const& operative_i
    ,double                     max_coi_rate
    ,int                        min_age
    ,int                        max_age
    )
    :q_       {cso_table(cso_era, autopisty, alb_or_anb, gender, smoking, min_age, max_age)}
    ,i_       {operative_i}
    ,length_  {lmi::ssize(q_)}
    // Initialize to proper length to support PETE usage below:
    ,ul_corr_ (length_)
    ,ul_7pp_  (length_)
    ,ol_corr_ (length_)
    ,ol_7pp_  (length_)
{
    std::vector<double> q12(length_);
    q12 += apply_binary(coi_rate_from_q<double>(), q_, max_coi_rate);

    std::vector<double> i12(length_);
    i12 += apply_unary(i_upper_12_over_12_from_i<double>(), i_);

    ULCommFns const ulcf(q12, i12, i12, mce_option1_for_7702, mce_monthly);

    ul_corr_ += ulcf.aD() / (ulcf.aDomega() + ulcf.kM());

    // 'E' is the shift operator, so E(7) f(x) = f(x+7).
    std::vector<double> E7aN(ulcf.aN());
    E7aN.insert(E7aN.end(), 7, 0.0);
    E7aN.erase(E7aN.begin(), 7 + E7aN.begin());
    ul_7pp_ += (ulcf.aDomega() + ulcf.kM()) / (ulcf.aN() - E7aN);

    std::vector<double> i_over_delta(length_);
    i_over_delta += i_ / log(1 + i_); // PETE's log(), not std::log()
    OLCommFns const olcf(q_, i_);

    // Alternative calculations that may be useful someday are given
    // in comments.

    // reciprocal of (Mx - Momega-1) / Dx
    ol_corr_ +=
          olcf.D()
//      / ((olcf.M() - olcf.M().back()) * i_over_delta + olcf.Domega())
        / ((olcf.M()                  ) * i_over_delta + olcf.Domega())
        ;

    // (Mx - Momega-1 + Domega-1) / (Nx - Nx+7)
    std::vector<double> E7N(olcf.N());
    E7N.insert(E7N.end(), 7, 0.0);
    E7N.erase(E7N.begin(), 7 + E7N.begin());
    ol_7pp_ +=
//        ((olcf.M() - olcf.M().back()) * i_over_delta + olcf.Domega())
//        ((olcf.M() - olcf.M().back())                + olcf.Domega())
          ((olcf.M()                  ) * i_over_delta + olcf.Domega())
        / (olcf.N() - E7N)
        ;
}
