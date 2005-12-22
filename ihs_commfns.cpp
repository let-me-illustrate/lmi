// Ordinary- and universal-life commutation functions.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_commfns.cpp,v 1.6 2005-12-22 13:59:49 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_commfns.hpp"

#include "alert.hpp"

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
    (std::vector<double> const& a_q
    ,std::vector<double> const& a_ic
    ,std::vector<double> const& a_ig
    ,e_dbopt const&             a_db_option
    ,e_mode const&              a_asspt_mode
    ,e_mode const&              a_commfn_mode
    ,e_mode const&              a_process_mode
    )
    :qc          (a_q)
    ,ic          (a_ic)
    ,ig          (a_ig)
    ,DBOption    (a_db_option)
    ,AssptMode   (a_asspt_mode)
    ,CommfnMode  (a_commfn_mode)
    ,ProcessMode (a_process_mode)
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

    int months_between_deductions = 12 / ProcessMode.value();

    ad[0] = 1.0;
    for(int j = 0; j < Length; j++)
        {
        // Convert from input mode to monthly.
        // TODO ?? Ideally offer choice of methods.
        if(e_mode(e_monthly) != a_asspt_mode)
            {
            double power = a_asspt_mode / static_cast<double>(e_monthly);
            qc[j] = 1.0     - std::pow(1.0 - qc[j], power);
            ic[j] = -1.0    + std::pow(1.0 + ic[j], power);
            ig[j] = -1.0    + std::pow(1.0 + ig[j], power);
            }
        // Eckley equations (7) and (8)
        double f = qc[j] * (1.0 + ic[j]) / (1.0 + ig[j]);
// TODO ?? What if it exceeds 1.0?
//      LMI_ASSERT(1.0 != f);
        f = std::min(0.99999999, f);
        double g = 1.0 / (1.0 + f);
        // Eckley equation (11)
        double i = (ic[j] + ig[j] * f) * g;
        // Eckley equation (12)
        double q = f * g;
        // Eckley equation (19)
        if(e_dbopt(e_option2) == DBOption)
            {
            i = i - q;
            }
        double v = 1.0 / (1.0 + i);
        double p = 1.0 - q;
        // TODO ?? Present value of $1 one month (?) hence
        double vp = v * p;
        LMI_ASSERT(1.0 != vp);
        // TODO ?? Present value of $1 twelve (?) months (?) hence
        double vp12 = std::pow(vp, 12);
        // Reciprocal of Eckley's a'' upper 12 (eqs 28, 32)
        // analytically determined by geometric series theorem
//      double aa = 1.0;
//      // Eckley equation (32)
//      double sa = (1.0 - vp12) / (1.0 - std::pow(vp, 6));
//      double qa = (1.0 - vp12) / (1.0 - std::pow(vp, 3));
//      // Eckley equation (28)
//      double ma = (1.0 - vp12) / (1.0 - vp);
        // The prefix k indicates the processing mode, which is
        // an input parameter.
        double ka =
                months_between_deductions
            *   (1.0 - vp12)
            /   (1.0 - std::pow(vp, months_between_deductions))
            ;

        kd[j] = ka * ad[j];
        kc[j] = ka * ad[j] * v * q;
        ad[1 + j] = ad[j] * vp12;
        }
// TODO ?? Wrong--don't want last element.
    an = ad;
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

// TODO ?? Factor this out into a unit-test module, if it's even useful.

#include "miscellany.hpp"
#include "timer.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <memory>

//============================================================================
void ULCommFns::SelfTest()
{
    std::ofstream os("ulcf.txt", std::ios_base::out | std::ios_base::trunc);
    static double const COI[30] =   // TSA XXIX, page 32, table 5
        {
        .00018,.00007,.00007,.00006,.00006,.00006,.00006,.00005,.00005,.00005,
        .00005,.00005,.00006,.00007,.00008,.00009,.00010,.00010,.00011,.00011,
        .00011,.00011,.00011,.00011,.00010,.00010,.00010,.00010,.00010,.00010,
        };
/*
[#include "math_functors.hpp"]
    std::vector<double>coi          (COI, COI + lmi_array_size(COI));
    std::vector<double>ic           (coi.size(), i_upper_12_over_12_from_i<double>()(0.10));
    std::vector<double>ig           (coi.size(), i_upper_12_over_12_from_i<double>()(0.04));

    std::auto_ptr<ULCommFns> CF
        (new ULCommFns
            (coi
            ,ic
            ,ig
            ,OptionB
            ,Monthly
            ,Annual
            ,Monthly
            )
        );
*/
    std::vector<double>coi          (COI, COI + lmi_array_size(COI));
    std::vector<double>ic           (coi.size(), 0.10);
    std::vector<double>ig           (coi.size(), 0.04);
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        coi[j] = 1.0 - std::pow(1.0 - coi[j], 12.0);
        }

    std::auto_ptr<Timer> timer(new Timer);

// 0 extra leaks if #ifdef out remainder of fn
    int const trials = 1000;
    for(int j = 0; j < trials; j++)
        {
        ULCommFns
            (coi
            ,ic
            ,ig
            ,e_dbopt(e_option2)
            ,e_mode(e_annual)
            ,e_mode(e_annual)
            ,e_mode(e_monthly)
            );
        }
//timer->stop();
//timer->elapsed_msec_str();
//string xxx = foo();
// 1 extra leaks if #ifdef out remainder of fn
    os
        << "Commutation function calculation time for "
        << trials
        << " trials: "
        << timer->stop().elapsed_msec_str()
//      << timer->elapsed_msec_str()
///     << xxx
        << "\n\n"
        ;

// 1 extra leak if #ifdef out remainder of fn
    std::auto_ptr<ULCommFns> CF
        (new ULCommFns
            (coi
            ,ic
            ,ig
            ,e_dbopt(e_option2)
            ,e_mode(e_annual)
            ,e_mode(e_annual)
            ,e_mode(e_monthly)
            )
        );

    os << "Universal life commutation functions\n";
    os
        << std::setw( 3) << "yr"
        << std::setw( 6) << "i"
        << std::setw( 9) << "q"
        << std::setw(13) << "c"
        << std::setw(13) << "d"
        << std::setw(13) << "m"
        << std::setw(13) << "n"
        << '\n'
        ;
    for(unsigned int j = 0; j < coi.size(); j++)
        {
        os
            << std::setw(3)  << j
            << std::setiosflags(std::ios_base::fixed)
            << std::setprecision(3)
            << std::setw(6)  << ic[j]
            << std::setprecision(6)
            << std::setw(9)  << coi[j]
            << std::setprecision(9)
            << std::setw(13) << CF->aD()[j]
            << std::setw(13) << CF->kD()[j]
            << std::setw(13) << CF->kC()[j]
            << '\n'
            ;
        }
    os << '\n';
}

//============================================================================
void OLCommFns::SelfTest()
{
    std::ofstream os("olcf.txt", std::ios_base::out | std::ios_base::trunc);

    static double const Q[100] =    // I think this is unisex unismoke ANB 80CSO
        {
        .00354,.00097,.00091,.00089,.00085,.00083,.00079,.00077,.00073,.00072,
        .00071,.00072,.00078,.00087,.00097,.00110,.00121,.00131,.00139,.00144,
        .00148,.00149,.00150,.00149,.00149,.00147,.00147,.00146,.00148,.00151,
        .00154,.00158,.00164,.00170,.00179,.00188,.00200,.00214,.00231,.00251,
        .00272,.00297,.00322,.00349,.00375,.00406,.00436,.00468,.00503,.00541,
        .00583,.00630,.00682,.00742,.00807,.00877,.00950,.01023,.01099,.01181,
        .01271,.01375,.01496,.01639,.01802,.01978,.02164,.02359,.02558,.02773,
        .03016,.03296,.03629,.04020,.04466,.04955,.05480,.06031,.06606,.07223,
        .07907,.08680,.09568,.10581,.11702,.12911,.14191,.15541,.16955,.18445,
        .20023,.21723,.23591,.25743,.28381,.32074,.37793,.47661,.65644,1.0000,
        };

    std::vector<double>q                (Q, Q + lmi_array_size(Q));
    std::vector<double>i                (100, 0.04);

    std::auto_ptr<OLCommFns> CF(new OLCommFns(q, i));

    os << "Ordinary life commutation functions\n";
    os
        << std::setw( 3) << "yr"
        << std::setw( 6) << "i"
        << std::setw( 9) << "q"
        << std::setw(13) << "c"
        << std::setw(13) << "d"
        << std::setw(13) << "m"
        << std::setw(13) << "n"
        << '\n'
        ;
    for(unsigned int j = 0; j < q.size(); j++)
        {
        os
            << std::setw(3)  << j
            << std::setiosflags(std::ios_base::fixed)
            << std::setprecision(3)
            << std::setw(6)  << i[j]
            << std::setprecision(6)
            << std::setw(9)  << q[j]
            << std::setprecision(9)
            << std::setw(13) << CF->C()[j]
            << std::setw(13) << CF->D()[j]
            << std::setw(13) << CF->M()[j]
            << std::setw(13) << CF->N()[j]
            << '\n'
            ;
        }
    os << '\n';
}

