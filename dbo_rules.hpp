// Death benefit option (DBO) rules.
//
// Copyright (C) 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef dbo_rules_hpp
#define dbo_rules_hpp

#include "config.hpp"

#include "mc_enum_types.hpp"

#include <vector>

bool dbo_at_issue_is_allowed(mce_dbopt);
bool dbo_transition_is_allowed(mce_dbopt from, mce_dbopt to);
bool dbo_sequence_is_allowed(std::vector<mce_dbopt> const&);

#endif // dbo_rules_hpp
