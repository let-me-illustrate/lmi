// A value-Constrained Enumeration for product names.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ce_product_name.hpp,v 1.4 2006-01-29 13:52:00 chicares Exp $

#ifndef ce_product_name_hpp
#define ce_product_name_hpp

#include "config.hpp"

#include "mc_enum.hpp"

#include <cstddef> // std::size_t
#include <string>
#include <vector>

/// This class encapsulates product names. Being derived from class
/// mc_enum_base, it presents an interface compatible with other
/// enumerative classes. It differs from most enumerative classes in
/// that its valid values are knowable only at run time, so that it
/// cannot associate an enum with each value; and in that all its
/// valid values are always allowable. Otherwise, its implementation
/// is similar to that of template class mc_enum.
///
/// Because there is no enum to map to, there is no 'm' for "Mapped"
/// at the beginning of the class's or file's name.
///
/// Valid values are the base names of policy ('.pol') files found
/// in the (configurable) data directory. They are read only once and
/// cached, not because of any measured improvement in run time, but
/// in order to keep the type from mutating with unknown consequences.
///
/// TODO ?? Reconsider this. It seems desirable to recognize changes
/// to the data directory made by advanced users, but is that easily
/// possible without adjusting the base class's 'allowed_' vector and
/// perhaps changing the way ordinal() works?
///
/// TODO ?? Policy-file names are read the first time this class is
/// instantiated. That's not the best time to do it, because an error
/// is reported if no such file is found. It would be better to test
/// that at startup. A simple singleton might be wanted if this
/// operation should really be performed only once. Alternatively, the
/// program might write the 'sample' product's files if it finds no
/// other product.
///
/// TODO ?? Add unit tests.
///
/// TODO ?? Can't read() and write() be moved to the base class?

class ce_product_name
    :public mc_enum_base
    ,private boost::equality_comparable<ce_product_name, ce_product_name>
    ,private boost::equality_comparable<ce_product_name, std::string>
{
  public:
    ce_product_name();
    explicit ce_product_name(std::string const&);

    ce_product_name& operator=(std::string const&);

    bool operator==(ce_product_name const&) const;
    bool operator==(std::string const&) const;

    // datum_base overrides. Consider moving the implementation into
    // the base class.
    virtual std::istream& read (std::istream& is);
    virtual std::ostream& write(std::ostream& os) const;

    // mc_enum_base overrides.
    virtual std::size_t allowed_ordinal() const;
    virtual std::size_t cardinality() const;
    virtual std::string str(int) const;

    std::string str() const;
    std::string value() const;

  private:
    std::size_t ordinal() const;
    std::size_t ordinal(std::string const&) const;
    std::vector<std::string> const& product_names() const;

    std::string value_;
};

#endif // ce_product_name_hpp

