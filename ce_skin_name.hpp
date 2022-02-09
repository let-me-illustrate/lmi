// A value-Constrained Enumeration for skin names.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef ce_skin_name_hpp
#define ce_skin_name_hpp

#include "config.hpp"

#include "mc_enum.hpp"

#include <string>
#include <vector>

/// This class encapsulates skin names. As with the related class
/// ce_product_name, its values are available only at run time, so
/// there can be no compile time enum to represent them.
///
/// Valid values are the full (base + extension) names of 'skin*.xrc'
/// skin files found in the (configurable) data directory. As with
/// ce_product_name, the valid values never change during the program
/// lifetime and it needs to be restarted to "notice" any new skins.
/// Unlike ce_product_name, the full file names are presented in the
/// GUI: base names of '.policy' files are designed to be recognizable
/// to end users, but skin names are more esoteric and it is less
/// confusing to show them as file names rather than apparent phrases.

class ce_skin_name
    :public mc_enum_base
{
  public:
    ce_skin_name();
    explicit ce_skin_name(std::string const&);

    ce_skin_name& operator=(std::string const&);

    bool operator==(ce_skin_name const&) const;
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
    // datum_base required implementation.
    std::istream& read (std::istream&) override;
    std::ostream& write(std::ostream&) const override;

    std::string value_;
};

#endif // ce_skin_name_hpp
