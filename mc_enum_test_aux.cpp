// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "mc_enum.tpp"
#include "mc_enum_test_aux.hpp"

// The mc_enum types used for unit testing are explicitly instantiated
// here, and not in the unit-test driver. This deliberately follows
// the recommended practice used throughout lmi, and is necessary for
// proving that explicit instantiation works as intended.

extern enum_holiday const holiday_enums[] = {h_Theophany, h_Easter, h_Pentecost};
extern char const*const holiday_strings[] = {"Theophany", "Easter", "Pentecost"};
template<> struct mc_enum_key<enum_holiday>
  :public mc_enum_data<enum_holiday, 3, holiday_enums, holiday_strings> {};
template class mc_enum<enum_holiday>;

// Ni'ihau is deliberately (mis)spelled with an underbar instead of an
// apostrophe, in order to test provide_for_backward_compatibility().

extern enum_island const  island_enums[] = {i_Easter, i_Pago_Pago, i_Ni_ihau};
extern char const*const island_strings[] = {"Easter", "Pago Pago", "Ni_ihau"};
template<> struct mc_enum_key<enum_island>
  :public mc_enum_data<enum_island, 3, island_enums, island_strings> {};
template class mc_enum<enum_island>;
