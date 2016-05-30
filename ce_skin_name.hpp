// A value-Constrained Enumeration for skin names.
//
// Copyright (C) 2016 Gregory W. Chicares.
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

#ifndef ce_skin_name_hpp
#define ce_skin_name_hpp

#include "config.hpp"

#include "mc_enum.hpp"

#include <cstddef>                      // std::size_t
#include <string>
#include <vector>

/// This class encapsulates skin names. It is similar to ce_product_name in
/// that its values are only available at run time, so there can be no compile
/// time enum to represent them.
///
/// Valid values are the base names of 'skin*.xrc' product files found
/// in the (configurable) data directory. As with ce_product_name, the valid
/// values never change during the program lifetime and it needs to be
/// restarted to "notice" the new skins.

class ce_skin_name
    :public mc_enum_base
    ,private boost::equality_comparable<ce_skin_name, ce_skin_name>
    ,private boost::equality_comparable<ce_skin_name, std::string>
{
  public:
    ce_skin_name();
    explicit ce_skin_name(std::string const&);

    ce_skin_name& operator=(std::string const&);

    bool operator==(ce_skin_name const&) const;
    bool operator==(std::string const&) const;

    static std::size_t ordinal(std::string const&);

    // mc_enum_base required implementation.
    virtual std::vector<std::string> const& all_strings() const;
    virtual std::size_t cardinality() const;
    virtual void enforce_proscription();
    virtual std::size_t ordinal() const;
    virtual std::string str(int) const;

    std::string str() const;
    std::string value() const;

  private:
    static std::vector<std::string> const& skin_names();

    // datum_base required implementation.
    // TODO ?? Consider moving the implementation into the base class.
    virtual std::istream& read (std::istream&);
    virtual std::ostream& write(std::ostream&) const;

    std::string value_;
};

#endif // ce_skin_name_hpp

