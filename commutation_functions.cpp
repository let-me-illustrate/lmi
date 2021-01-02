// Ordinary- and universal-life commutation functions.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "commutation_functions.hpp"

#include "assert_lmi.hpp"
#include "et_vector.hpp"                // [VECTORIZE]
#include "ssize_lmi.hpp"

#include <algorithm>                    // rotate_copy() [VECTORIZE]
#include <cmath>                        // pow()
#include <functional>                   // multiplies    [VECTORIZE]
#include <numeric>                      // partial_sum()

/// Interest- and mortality-rate vectors --> commutation functions.
///
/// In the general case, interest rates may vary by year. Most often,
/// they are the same for all years; but optimizing for that common
/// special case at the cost of code complexity would probably be
/// a mistake.
///
/// SOMEDAY !! Revisit the 'VECTORIZE' alternative with gcc-4.x .
/// With gcc-3.4.5, it's twenty-five percent slower as measured by the
/// unit test's mete_olcf().

OLCommFns::OLCommFns
    (std::vector<double> const& a_q
    ,std::vector<double> const& a_i
    )
    :q {a_q}
    ,i {a_i}
{
    Length = lmi::ssize(q);
    LMI_ASSERT(lmi::ssize(i) == lmi::ssize(q));

#if defined VECTORIZE
    ed.resize(Length);
    d .resize(Length);
    c .resize(Length);
    n .resize(Length);
    m .resize(Length);

    std::vector<double> v(Length);
    v += 1.0 / (1.0 + i);

    ed += v * (1.0 - q);
    std::partial_sum(ed.begin(), ed.end(), ed.begin(), std::multiplies<double>());

    std::rotate_copy(ed.begin(), -1 + ed.end(), ed.end(), d.begin());
    d[0] = 1.0;

    c += d * v * q;
#else  // !defined VECTORIZE
    d.resize(1 + Length);
    c.resize(    Length);
    n.resize(    Length);
    m.resize(    Length);

    d[0] = 1.0;
    for(int j = 0; j < Length; ++j)
        {
        LMI_ASSERT(-1.0 != i[j]);
        double v = 1.0 / (1.0 + i[j]);
        double p = 1.0 - q[j];
        c[j] = d[j] * v * q[j];
        d[1 + j] = d[j] * v * p;
        }

    ed = d;
    ed.erase(ed.begin());
    d.pop_back();
#endif // !defined VECTORIZE

    std::partial_sum(d.rbegin(), d.rend(), n.rbegin());
    std::partial_sum(c.rbegin(), c.rend(), m.rbegin());
}

/// Interest- and mortality-rate vectors --> commutation functions.
///
/// Constructor arguments:
///   a_qc  Eckley's Q:  mortality rates
///   a_ic  Eckley's ic: "current"    interest rates
///   a_ig  Eckley's ig: "guaranteed" interest rates
///   dbo   death benefit option
///   mode  n-iversary mode
///
/// Numeric arguments--mortality and interest rates--must be on
/// the mode for which commutation functions are wanted. If monthly
/// functions are to be obtained from annual rates, convert the
/// rates to monthly before passing them as arguments. There's more
/// than one way to perform a modal conversion, and it's not this
/// class's responsibility to choose.
///
/// The mode argument specifies the frequency of UL n-iversary
/// processing. This is most often monthly, but need not be.

ULCommFns::ULCommFns
    (std::vector<double> const& a_qc
    ,std::vector<double> const& a_ic
    ,std::vector<double> const& a_ig
    ,mcenum_dbopt_7702          dbo
    ,mcenum_mode                mode
    )
    :qc    {a_qc}
    ,ic    {a_ic}
    ,ig    {a_ig}
    ,dbo_  {dbo}
    ,mode_ {mode}
{
    Length = lmi::ssize(qc);
    LMI_ASSERT(lmi::ssize(ic) == lmi::ssize(qc));
    LMI_ASSERT(lmi::ssize(ig) == lmi::ssize(qc));

    ad.resize(1 + Length);
    kd.resize(    Length);
    kc.resize(    Length);
    an.resize(    Length);
    km.resize(    Length);

    int periods_per_year = mode_;
    int months_per_period = 12 / periods_per_year;

    ad[0] = 1.0;
    for(int j = 0; j < Length; ++j)
        {
        LMI_ASSERT( 0.0 <= qc[j] && qc[j] <= 1.0);
        LMI_ASSERT(-1.0 <  ic[j]);
        LMI_ASSERT( 0.0 <= ig[j]);
        // Eckley equations (7) and (8).
        double f = qc[j] * (1.0 + ic[j]) / (1.0 + ig[j]);
        // f cannot be negative, so division by 1+f is safe.
        double g = 1.0 / (1.0 + f);
        // Eckley equation (11).
        double i = (ic[j] + ig[j] * f) * g;
        // Eckley equation (12).
        double q = f * g;
        // Eckley equation (19).
        if(mce_option2_for_7702 == dbo_)
            {
            i = i - q;
            }
        LMI_ASSERT(-1.0 != i);
        double v = 1.0 / (1.0 + i);
        double p = 1.0 - q;
        // Present value of $1 one month hence.
        double vp = v * p;
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
        double ka = 1.0;
        if(1.0 != vp)
            {
            ka = (1.0 - vp12) / (1.0 - std::pow(vp, months_per_period));
            }
        kd[j] = ka * ad[j];
        kc[j] = ka * ad[j] * v * q;
        ad[1 + j] = ad[j] * vpn;
        }

    ead = ad;
    ead.erase(ead.begin());
    ad.pop_back();

    std::partial_sum(ad.rbegin(), ad.rend(), an.rbegin());
    std::partial_sum(kc.rbegin(), kc.rend(), km.rbegin());
}
