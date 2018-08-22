// Ledger evaluator returning values of all ledger fields.
//
// Copyright (C) 2017, 2018 Gregory W. Chicares.
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

#ifndef ledger_evaluator_hpp
#define ledger_evaluator_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <cstddef>                      // size_t
#include <string>
#include <unordered_map>
#include <vector>

/// Class allowing to retrieve the string representation of any scalar or
/// vector stored in a ledger.

class LMI_SO ledger_evaluator
{
  public:
    std::string operator()(std::string const& scalar) const;
    std::string operator()(std::string const& vector, std::size_t index) const;

  private:
    using all_scalars = std::unordered_map<std::string,            std::string >;
    using all_vectors = std::unordered_map<std::string,std::vector<std::string>>;

    // Objects of this class can only be created by Ledger::make_evaluator().
    ledger_evaluator(all_scalars&& scalars, all_vectors&& vectors)
        :scalars_{scalars}
        ,vectors_{vectors}
    {
    }

    all_scalars const scalars_;
    all_vectors const vectors_;

    friend class Ledger;
};

#endif // ledger_evaluator_hpp
