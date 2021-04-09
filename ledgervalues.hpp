// Run an individual illustration, producing a ledger.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef ledgervalues_hpp
#define ledgervalues_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <memory>                       // shared_ptr
#include <string>

class Input;
class Ledger;

/// Run an individual illustration, producing a ledger.
///
/// This class encapsulates a frequently-used series of operations.

class IllusVal final
{
  public:
    explicit IllusVal(std::string const& filename);
    ~IllusVal() = default;

    void run(Input const&);

    std::shared_ptr<Ledger const> ledger() const;

  private:
    IllusVal(IllusVal const&) = delete;
    IllusVal& operator=(IllusVal const&) = delete;

    std::string filename_;
    std::shared_ptr<Ledger const> ledger_;
};

#endif // ledgervalues_hpp
