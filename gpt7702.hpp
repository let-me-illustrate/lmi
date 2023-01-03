// Internal Revenue Code section 7702
//
// Copyright (C) 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef gpt7702_hpp
#define gpt7702_hpp

#include "config.hpp"

#include "currency.hpp"
#include "gpt_commutation_functions.hpp"
#include "mc_enum_type_enums.hpp"       // mcenum_defn_life_ins

#include <vector>

// https://lists.nongnu.org/archive/html/lmi/2014-06/msg00002.html
//
//                ---- triggers ---- | -------------- data ---------------
//                queue  queue queue |                                 cum
//                prems adjust  pos  |     cum              rejected prems
//                paid-  event  pmt  | GLP GLP GSP forceout    pmt    paid
// -----------------------------------------------------------------------
// non-1035 issue    -     -     -   |  -   -   -      -        -       -
// 1035     issue    -     -     t   |  -   -   -      -        -       -
// dbo     change    -     t     -   |  -   -   -      -        -       -
// specamt change    -     t     -   |  -   -   -      -        -       -
// withdrawal        t     t     -   |  -   -   -      -        -       -
// -----------------------------------------------------------------------
// initialization    -     -     -   |  i   i   i      -        -       i
// GPT adjustment    -     -     -   |  u   u   u      -        -       -
// march of time     -     -     -   |  r   u   -      -        -       -
// decr prems paid   -     -     -   |  -   -   -      -        -       u
// forceout          -     -     -   |  -   r   r      w        -       u
// new premium       -     -     -   |  -   r   r      -        w       u

/// Guideline premium test.
///
/// 7702 !! This should be a base class, with distinct derived classes
/// for illustration and admin systems.

class gpt7702
{
    friend class gpt_test;

  public:
    gpt7702
        (std::vector<double> const& qc
        ,std::vector<double> const& glp_ic
        ,std::vector<double> const& glp_ig
        ,std::vector<double> const& gsp_ic
        ,std::vector<double> const& gsp_ig
        ,gpt_vector_parms    const& charges
        );

    void initialize_gpt
        (mcenum_defn_life_ins    defn_life_ins
        ,double                  fractional_duration
        ,double                  inforce_glp
        ,double                  inforce_cum_glp
        ,double                  inforce_gsp
        ,currency                inforce_cum_f1A
        ,gpt_scalar_parms const& arg_parms
        );

    // return amount forced out
    currency update_gpt
        (gpt_scalar_parms const&
        ,double                  fractional_duration
        ,currency                f2A_value
        );

    // return amount rejected
    currency accept_payment         (currency);

    // queue notifications
    void enqueue_exch_1035          (currency);
    void enqueue_f1A_decrease       (currency);
    void enqueue_adj_event          ();

    // const accessors
    double   raw_glp                () const;
    double   raw_cum_glp            () const;
    double   raw_gsp                () const;
    currency rounded_glp            () const;
    currency rounded_cum_glp        () const;
    currency rounded_gsp            () const;
    currency cum_f1A                () const;

  protected:
    void adjust_guidelines          (gpt_scalar_parms const&);
    void increment_boy              ();

  private:
    void dequeue_exch_1035          ();
    void dequeue_f1A_decrease       ();
    void dequeue_adj_event          (gpt_scalar_parms const&);

    currency force_out(currency f2A_value);

    bool     is_issued_today        () const;
    currency guideline_limit        () const;

    // unchangeable basis of calculations (subsumes gpt_vector_parms)
    gpt_cf_triad     cf_;

    // changeable policy status (all scalar)
    gpt_scalar_parms s_parms_           {};

    // queued data
    currency queued_exch_1035_amt_      {C0};
    currency queued_f1A_decrement_      {C0};

    // tableau data
    double   glp_                       {0.0};
    double   cum_glp_                   {0.0};
    double   gsp_                       {0.0};
    currency forceout_amount_           {C0};
    currency rejected_pmt_              {C0};
    currency cum_f1A_                   {C0};

    // queued agenda
    bool queued_exch_1035_              {false};
    bool queued_f1A_decrease_           {false};
    bool queued_adj_event_              {false};

    // server state
    mcenum_defn_life_ins defn_life_ins_ {mce_gpt};
    double   fractional_duration_       {0.0};
};

#endif // gpt7702_hpp
