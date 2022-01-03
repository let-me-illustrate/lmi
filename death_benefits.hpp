// Death benefits.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef death_benefits_hpp
#define death_benefits_hpp

#include "config.hpp"

#include "currency.hpp"
#include "mc_enum_type_enums.hpp"
#include "round_to.hpp"

#include <vector>

class yare_input;

class death_benefits final
{
  public:
    explicit death_benefits
        (int                     length
        ,yare_input       const&
        ,round_to<double> const& round_specamt
        );
    ~death_benefits() = default;

    void set_specamt (currency z, int from_year, int to_year);
    void set_supplamt(currency z, int from_year, int to_year);

    std::vector<mcenum_dbopt> const& dbopt   () const;
    std::vector<currency>     const& specamt () const;
    std::vector<currency>     const& supplamt() const;

  private:
    death_benefits(death_benefits const&) = delete;
    death_benefits& operator=(death_benefits const&) = delete;

    int length_;

    round_to<double> round_specamt_;

    std::vector<mcenum_dbopt> dbopt_   ;
    std::vector<currency>     specamt_ ;
    std::vector<currency>     supplamt_;
};

inline std::vector<mcenum_dbopt> const& death_benefits::dbopt() const
{
    return dbopt_;
}

inline std::vector<currency> const& death_benefits::specamt() const
{
    return specamt_;
}

inline std::vector<currency> const& death_benefits::supplamt() const
{
    return supplamt_;
}

#endif // death_benefits_hpp
