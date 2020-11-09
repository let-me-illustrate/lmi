// Text to be dropped into report templates.
//
// Copyright (C) 2020 Gregory W. Chicares.
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

#ifndef lingo_hpp
#define lingo_hpp

#include "config.hpp"

#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <string>

/// Company-specific lingo.

class LMI_SO lingo final
{
  public:
    explicit lingo(std::string const& filename);

    // Legacy functions to support creating product files programmatically.
    static void write_lingo_files();
    static void write_proprietary_lingo_files();

  private:
    // This class does not derive from xml_serializable, but it
    // implements these three functions that are akin to virtuals
    // of class xml_serializable.
    static int class_version();
    static std::string const& xml_root_name();
    static void write_proem
        (xml_lmi::xml_document& document
        ,std::string const&     file_leaf_name
        );
};

#endif // lingo_hpp
