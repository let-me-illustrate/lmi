// Input sequences (e.g. 1 3; 7 5;0; --> 1 1 1 7 7 0...): interval
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef input_sequence_interval_hpp
#define input_sequence_interval_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

enum duration_mode
    {e_invalid_mode
    ,e_duration
    ,e_attained_age
    ,e_number_of_years
    ,e_inception
    ,e_inforce
    ,e_retirement
    ,e_maturity
    };

/// A single 'span' in the input-sequence grammar.
///
/// value_{number,keyword} applies throughout the interval
///   [begin_duration, end_duration)

struct ValueInterval
{
    ValueInterval() = default;

    double        value_number     = 0.0;
    std::string   value_keyword;
    bool          value_is_keyword = false;
    int           begin_duration   = 0;
    duration_mode begin_mode       = e_inception;
    int           end_duration     = 0;
    duration_mode end_mode         = e_maturity;
    bool          insane           = false;
};

LMI_SO std::string abridge_diagnostics(char const* what);

#endif // input_sequence_interval_hpp
