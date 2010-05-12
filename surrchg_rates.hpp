// Life insurance illustrations: surrender charge rates.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

#ifndef surrchg_rates_hpp
#define surrchg_rates_hpp

#include "config.hpp"

#include <vector>

class product_database;

class SurrChgRates
{
  public:
    SurrChgRates(product_database const&);

    std::vector<double> const& RatePerDollarOfPremium() const;
    std::vector<double> const& RatePerDollarOfAcctval() const;
    std::vector<double> const& RatePerDollarOfSpecamt() const;
    std::vector<double> const& AcctvalRateDurationalFactor() const;
    std::vector<double> const& SpecamtRateDurationalFactor() const;

    // Ultimately, we'll want to handle tabular surrender charges too.

  private:
    SurrChgRates();

    // Ultimately, we'll want a pointer to a mortality-rate object
    // too, that we can do SNFL calculations.
    void Initialize(product_database const&);

    std::vector<double> RatePerDollarOfPremium_;
    std::vector<double> RatePerDollarOfAcctval_;
    std::vector<double> RatePerDollarOfSpecamt_;

    // These member supports surrender-charge structures that are an
    // attained-age multiple of acctval or specamt times a factor that
    // depends on duration only, e.g.
    //   specamt(t) * rate(x+t) * factor[t]
    // For instance, if the initial surrender charge grades off by ten
    // percent of its original amount each year, use 1.0, 0.9,...0.0 .
    // It is contemplated that these factors be accessed by duration
    // since the addition of each surrender charge layer.
    std::vector<double> AcctvalRateDurationalFactor_;
    std::vector<double> SpecamtRateDurationalFactor_;
};

inline std::vector<double> const&
SurrChgRates::RatePerDollarOfPremium() const
{
    return RatePerDollarOfPremium_;
}

inline std::vector<double> const&
SurrChgRates::RatePerDollarOfAcctval() const
{
    return RatePerDollarOfAcctval_;
}

inline std::vector<double> const&
SurrChgRates::RatePerDollarOfSpecamt() const
{
    return RatePerDollarOfSpecamt_;
}

inline std::vector<double> const&
SurrChgRates::AcctvalRateDurationalFactor() const
{
    return AcctvalRateDurationalFactor_;
}

inline std::vector<double> const&
SurrChgRates::SpecamtRateDurationalFactor() const
{
    return SpecamtRateDurationalFactor_;
}

#endif // surrchg_rates_hpp

