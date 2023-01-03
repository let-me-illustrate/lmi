// A value-Constrained Enumeration for product names.
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

#ifndef ce_product_name_hpp
#define ce_product_name_hpp

#include "config.hpp"

#include "mc_enum.hpp"
#include "so_attributes.hpp"

#include <iosfwd>
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
/// Valid values are the base names of '.policy' product files found
/// in the (configurable) data directory. They are read only once and
/// cached, not because of any measured improvement in run time, but
/// in order to keep the type from mutating with unknown consequences.
///
/// SOMEDAY !! Reconsider this. It would be nice to recognize changes
/// to the data directory made by advanced users, but is that easily
/// possible without adjusting the base class's 'allowed_' vector and
/// perhaps changing the way ordinal() works?

class LMI_SO ce_product_name final
    :public mc_enum_base
{
  public:
    ce_product_name();
    explicit ce_product_name(std::string const&);

    ce_product_name& operator=(std::string const&);

    bool operator==(ce_product_name const&) const;
    bool operator==(std::string const&) const;

    static int ordinal(std::string const&);

    // mc_enum_base required implementation.
    std::vector<std::string> const& all_strings() const override;
    int cardinality() const override;
    void enforce_proscription() override;
    int ordinal() const override;
    std::string str(int) const override;

    std::string str() const;
    std::string value() const;

  private:
    void concrete_if_not_pure() override {}

    // datum_base required implementation.
    std::istream& read (std::istream&) override;
    std::ostream& write(std::ostream&) const override;

    std::string value_;
};

bool operator==(std::string const&, ce_product_name const&);

#endif // ce_product_name_hpp
