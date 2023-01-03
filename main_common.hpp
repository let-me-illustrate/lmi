// Startup code common to all interfaces.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef main_common_hpp
#define main_common_hpp

#include "config.hpp"

#include <climits>                      // CHAR_BIT
#include <cstdint>
#include <cstdlib>                      // EXIT_SUCCESS, EXIT_FAILURE

static_assert(8 == CHAR_BIT);
static_assert(4 == sizeof(std::int32_t)); // ensure that this exact type exists
static_assert(4 <= sizeof(int));

void initialize_application();
int try_main(int argc, char* argv[]);

#endif // main_common_hpp
