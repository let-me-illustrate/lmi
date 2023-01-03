// List of headers to precompile for general use.
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

#ifndef pchlist_hpp
#define pchlist_hpp

#if !defined LMI_OKAY_TO_INCLUDE_PCHLIST_HPP
#   error This file is not intended for separate inclusion.
#endif // !defined LMI_OKAY_TO_INCLUDE_PCHLIST_HPP

#if defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

// This list includes the most frequently used standard and boost
// headers and a list of frequently included lmi headers, both based
// on gcc's dependencies output.

#   include "config.hpp"

#   include "account_value.hpp"
#   include "alert.hpp"
#   include "any_entity.hpp"
#   include "any_member.hpp"
#   include "assert_lmi.hpp"
#   include "authenticity.hpp"
#   include "basic_values.hpp"
#   include "calendar_date.hpp"
#   include "ce_product_name.hpp"
#   include "config.hpp"
#   include "configurable_settings.hpp"
#   include "data_directory.hpp"
#   include "database.hpp"
#   include "datum_base.hpp"
#   include "datum_boolean.hpp"
#   include "datum_string.hpp"
#   include "dbindex.hpp"
#   include "dbnames.hpp"
#   include "death_benefits.hpp"
#   include "facets.hpp"
#   include "fenv_lmi.hpp"
#   include "global_settings.hpp"
#   include "handle_exceptions.hpp"
#   include "illustrator.hpp"
#   include "input.hpp"
#   include "input_sequence.hpp"
#   include "interest_rates.hpp"
#   include "ledger.hpp"
#   include "ledger_base.hpp"
#   include "ledger_invariant.hpp"
#   include "ledger_variant.hpp"
#   include "main_common.hpp"
#   include "mc_enum.hpp"
#   include "mc_enum_type_enums.hpp"
#   include "mc_enum_types.hpp"
#   include "mc_enum_types.xpp"
#   include "mc_enum_types_aux.hpp"
#   include "miscellany.hpp"
#   include "mvc_model.hpp"
#   include "null_stream.hpp"
#   include "numeric_io_cast.hpp"
#   include "numeric_io_traits.hpp"
#   include "oecumenic_enumerations.hpp"
#   include "outlay.hpp"
#   include "path.hpp"
#   include "path_utility.hpp"
#   include "platform_dependent.hpp"
#   include "round_to.hpp"
#   include "rounding_rules.hpp"
#   include "rtti_lmi.hpp"
#   include "safely_dereference_as.hpp"
#   include "so_attributes.hpp"
#   include "stratified_charges.hpp"
#   include "stream_cast.hpp"
#   include "system_command.hpp"
#   include "timer.hpp"
#   include "tn_range.hpp"
#   include "tn_range_type_trammels.hpp"
#   include "tn_range_types.hpp"
#   include "value_cast.hpp"
#   include "xml_serializable.hpp"
#   include "yare_input.hpp"
#   include "zero.hpp"

#   include <cstddef>
#   include <cstdlib>
#   include <exception>
#   include <istream>
#   include <iterator>
#   include <stdexcept>
#   include <string>
#   include <type_traits>
#   include <utility>
#   include <vector>

#endif // defined LMI_COMPILER_USES_PCH && !defined LMI_IGNORE_PCH

#endif // pchlist_hpp
