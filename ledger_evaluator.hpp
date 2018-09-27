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

#include <string>
#include <unordered_map>
#include <vector>

/// Class allowing to retrieve the string representation of any scalar or
/// vector stored in a ledger.

class LMI_SO ledger_evaluator
{
    friend class Ledger;

    template<typename K, typename T> using umap = std::unordered_map<K,T>;
    using scalar_map_t = umap<std::string,            std::string >;
    using vector_map_t = umap<std::string,std::vector<std::string>>;

  public:
    std::string operator()(std::string const& scalar) const;
    std::string operator()(std::string const& vector, int index) const;

  private:
    // Constructible only by friends: see Ledger::make_evaluator().
    ledger_evaluator(scalar_map_t&& scalars, vector_map_t&& vectors)
        :scalars_ {scalars}
        ,vectors_ {vectors}
    {
    }

    scalar_map_t const scalars_;
    vector_map_t const vectors_;
};

#endif // ledger_evaluator_hpp
