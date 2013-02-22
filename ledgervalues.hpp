// Run an individual illustration, producing a ledger.
//
// Copyright (C) 1998, 2001, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

// $Id$

#ifndef ledgervalues_hpp
#define ledgervalues_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/shared_ptr.hpp>

#include <string>

class Input;
class Ledger;

/// Run an individual illustration, producing a ledger.
///
/// This class encapsulates a frequently-used series of operations.

class IllusVal
    :        private lmi::uncopyable <IllusVal>
    ,virtual private obstruct_slicing<IllusVal>
{
  public:
    explicit IllusVal(std::string const& filename);
    ~IllusVal();

    double run(Input const&);

    boost::shared_ptr<Ledger const> ledger() const;

  private:
    std::string filename_;
    boost::shared_ptr<Ledger const> ledger_;
};

#endif // ledgervalues_hpp

