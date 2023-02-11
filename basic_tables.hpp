// Basic tables.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef basic_tables_hpp
#define basic_tables_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <vector>

class LMI_SO_FWD_DECL product_data;
class LMI_SO_FWD_DECL product_database;

LMI_SO std::vector<double> irc_7702_q
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    );

LMI_SO std::vector<double> irc_7702A_7pp
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    );

LMI_SO std::vector<double> irc_7702_nsp
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    );

LMI_SO std::vector<double> cvat_corridor_factors
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    );

LMI_SO std::vector<double> target_premium_rates
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    );

#endif // basic_tables_hpp
