// Miscellaneous mathematical operations as function objects.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: math_functors.hpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifndef math_functors_hpp
#define math_functors_hpp

#include "config.hpp"

#include <algorithm>
#include <cmath>
#include <functional>

template<typename T>
struct greater_of
    :public std::binary_function<T,T,T>
{
    T operator()(T const& x, T const& y) const
        {
        return std::max(x, y);
        }
};

template<typename T>
struct lesser_of
    :public std::binary_function<T,T,T>
{
    T operator()(T const& x, T const& y) const
        {
        return std::min(x, y);
        }
};

template<typename T>
struct mean
    :public std::binary_function<T, T, T>
{
    T operator()(T const& x, T const& y) const
        {
        return 0.5 * (x + y);
        }
};

template<typename T>
struct i_upper_12_over_12_from_i
    :public std::unary_function<T, T>
{
    T operator()(T const& i) const
        {
        return -1.0L + std::pow((1.0L + i), 1.0L / 12.0L);
        }
};

template<typename T>
struct i_from_i_upper_12_over_12
    :public std::unary_function<T, T>
{
    T operator()(T const& i) const
        {return -1.0L + std::pow((1.0L + i), 12.0L);}
};

template<typename T>
struct d_upper_12_from_i
    :public std::unary_function<T, T>
{
    T operator()(T const& i) const
        {return 12.0L * (1.0L - std::pow(1.0L + i, -1.0L / 12.0L));}
};

// Convert q to a monthly COI rate. The COI charge is assessed against
// all insureds who are alive at the beginning of the month. Assuming
// that deaths occur at the end of the month, the monthly-equivalent
// q should be divided by one minus itself to obtain the COI rate.
//
// The value of 'q' might exceed unity, for example if guaranteed COI
// rates for simplified issue are 120% of 1980 CSO, so that case is
// accommodated. A value of zero might arise from a partial-mortality
// multiplier that equals zero for some or all durations, and that
// case arises often enough to merit a special optimization. Negative
// values of the arguments are not plausible and are not tested.

template<typename T>
struct coi_rate_from_q
    :public std::binary_function<T,T,T>
{
    T operator()(T const& q, T const& max_coi) const
        {
        if(0.0 == q)
            {
            return 0.0;
            }
        else if(1.0 <= q)
            {
            return max_coi;
            }
        else
            {
            long double monthly_q = 1.0L - std::pow(1.0L - q, 1.0L / 12.0L);
            return std::min
                (max_coi
                ,static_cast<T>(monthly_q / (1.0L - monthly_q))
                );
            }
        }
};

#endif  // math_functors_hpp

