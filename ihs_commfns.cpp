// Ordinary- and universal-life commutation functions.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ihs_commfns.cpp,v 1.29 2009-10-06 03:09:39 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_commfns.hpp"

#include "assert_lmi.hpp"

#include <algorithm>
#include <cmath>     // std::pow()
#include <numeric>

// Calculate C D M N given vector q and vector i.
// We could gain some speed by optionally allowing scalar i and
// treating it as a special case, but this will always work.

//============================================================================
OLCommFns::OLCommFns
    (std::vector<double> const& a_q
    ,std::vector<double> const& a_i
    )
    :q(a_q)
    ,i(a_i)
{
    Length = q.size();
    LMI_ASSERT(i.size() == q.size());

    c.assign(1 + Length, 1.0);
    d.assign(1 + Length, 1.0);
    m.assign(1 + Length, 1.0);
    n.assign(1 + Length, 1.0);

    std::vector<double> v(1 + Length, 1.0);
    std::vector<double> p(1 + Length, 1.0);

    d[0] = 1.0;
    for(int j = 0; j < Length; j++)
        {
        LMI_ASSERT(-1.0 != i[j]);
        v[j] = 1.0 / (1.0 + i[j]);
        p[j] = 1.0 - q[j];
        c[j] = d[j] * v[j] * q[j];
        d[1 + j] = d[j] * v[j] * p[j];
        }
// ignore these commented lines
//  c[Length] = v[Length] * d[Length];  // assumes 1 == p[Length]
//  c[Length] = d[Length];  // but there's no i[Length]

    m[-1 + Length] = c[-1 + Length];
    n[-1 + Length] = d[-1 + Length];
    for(int j = -1 + Length; j; j--)
        {
        m[-1 + j] = m[j] + c[-1 + j];
        n[-1 + j] = n[j] + d[-1 + j];
        }
}

//============================================================================
OLCommFns::~OLCommFns()
{
}

//============================================================================
ULCommFns::ULCommFns
    (std::vector<double> const& a_qc
    ,std::vector<double> const& a_ic
    ,std::vector<double> const& a_ig
    ,mcenum_dbopt               dbo
    ,mcenum_mode                mode
    )
    :qc    (a_qc)
    ,ic    (a_ic)
    ,ig    (a_ig)
    ,dbo_  (dbo)
    ,mode_ (mode)
{
    Length = qc.size();
    LMI_ASSERT(ic.size() == qc.size());
    LMI_ASSERT(ig.size() == qc.size());

//  q.assign(1 + Length, 1.0);
//  i.assign(1 + Length, 1.0);

    ad.resize(1 + Length);
    kd.resize(Length);
    kc.resize(Length);

// erase    std::vector<double> q_prime(1 + Length, 1.0);
//  std::vector<double> v(1 + Length, 1.0);
//  std::vector<double> p(1 + Length, 1.0);
//  std::vector<double> a(1 + Length, 1.0);

    int periods_per_year = mode_;
    int months_between_deductions = 12 / periods_per_year;

    ad[0] = 1.0;
    for(int j = 0; j < Length; j++)
        {
        // Eckley equations (7) and (8).
        double f = qc[j] * (1.0 + ic[j]) / (1.0 + ig[j]);
        double g = 1.0 / (1.0 + f);
        // Eckley equation (11).
        double i = (ic[j] + ig[j] * f) * g;
        // Eckley equation (12).
        double q = f * g;
        // Eckley equation (19).
        if(mce_option2 == dbo_)
            {
            i = i - q;
            }
        double v = 1.0 / (1.0 + i);
        double p = 1.0 - q;
        // Present value of $1 one month hence.
        double vp = v * p;
        LMI_ASSERT(1.0 != vp);
        // Present value of $1 twelve months hence.
        double vp12 = std::pow(vp, 12);
        double vpn  = std::pow(vp, periods_per_year);
        // Twelve times a'' upper 12 (Eckley equations 28 and 31),
        // determined analytically using the geometric series theorem.
//      double aa = 1.0;
//      // Eckley equation (31).
//      double sa = (1.0 - vp12) / (1.0 - std::pow(vp, 6));
//      double qa = (1.0 - vp12) / (1.0 - std::pow(vp, 3));
//      // Eckley equation (28).
//      double ma = (1.0 - vp12) / (1.0 - vp);
        // The prefix k indicates the processing mode, which is
        // an input parameter.
        double ka =
                (1.0 - vp12)
            /   (1.0 - std::pow(vp, months_between_deductions))
            ;

        kd[j] = ka * ad[j];
        kc[j] = ka * ad[j] * v * q;
        ad[1 + j] = ad[j] * vpn;
        }
    an = ad;
    // Don't want last element here.
    an.pop_back();
    std::reverse(an.begin(), an.end());
    std::partial_sum(an.begin(), an.end(), an.begin());
    std::reverse(an.begin(), an.end());

    km = kc;
    std::reverse(km.begin(), km.end());
    std::partial_sum(km.begin(), km.end(), km.begin());
    std::reverse(km.begin(), km.end());

/*
    m[-1 + Length] = c[-1 + Length];
    n[-1 + Length] = d[-1 + Length];
    for(int j = -1 + Length; j; j--)
        {
        m[-1 + j] = m[j] + c[-1 + j];
        n[-1 + j] = n[j] + d[-1 + j];
        }
*/
}

//============================================================================
ULCommFns::~ULCommFns()
{
}

/*
//============================================================================
// The algorithm can be expressed so concisely in APL that I tried
// an STL approach; but the balkiness of the notation makes it
// harder to read than the C approach. This is untested.
void OLCommFns::OLCommFns()
{
    std::vector<double>         c_;
    std::vector<double>         m_;
    std::vector<double>         n_;
    std::vector<double>         p_;

//  v gets recip(1+i)

    std::vector<double> v_(i);
    std::transform(v_.begin(), v_.end(), v_.begin()
        ,compose1
            (bind1st(divides<double>(), 1.0)
            ,bind1st(lesser_of<double>(), MinI)
        );

//  d gets prod cat (1,v*p)
//  std::vector<double> d_(Length, 1.0);
    std::vector<double> d_(q);
    std::transform(d_.begin(), d_.end(), d_.begin(),
          bind1st(minus<double>(), 1.0)
          );
    rotate(d_.begin(), d_.end() - 1, d_.end());
    d[0] = 1.0;
    std::transform(d_.begin(), d_.end(), v_.begin(), d_.begin(),
          multiplies<double>()
          );
    std::partial_sum(d_.begin(), d_.end(), multiplies<double>()
        );

//  c gets d * cat(v*q, 1)
    std::vector<double> c_(q);
    std::transform(c_.begin(), c_.end(), v_.begin(), c_.begin(),
        multiplies<double>()
        );
    std::partial_sum(c_.begin(), c_.end(),
        multiplies<double>()
        );

//  n gets backsum d
    std::vector<double> n_(d_);
    std::reverse(n_.begin(), n_.end());
    std::partial_sum(n_.begin(), n_.end(), n_.begin());
    std::reverse(n_.begin(), n_.end());

//  m gets backsum c
    std::vector<double> m_(c_);
    std::reverse(m_.begin(), m_.end());
    std::partial_sum(m_.begin(), m_.end(), m_.begin());
    std::reverse(m_.begin(), m_.end());
}
*/

