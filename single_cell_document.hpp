// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef single_cell_document_hpp
#define single_cell_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <iosfwd>
#include <string>

class LMI_SO single_cell_document final
{
    friend class IllustrationDocument;
    friend class IllustrationView;
    friend class input_test; // For mete_ill_xsd().

  public:
    single_cell_document() = default;
    single_cell_document(Input const&);
    single_cell_document(std::string const& filename);
    ~single_cell_document() = default;

    Input const& input_data() const;

    void read(std::istream const&);
    void write(std::ostream&) const;

  private:
    single_cell_document(single_cell_document const&) = delete;
    single_cell_document& operator=(single_cell_document const&) = delete;

    void parse(xml_lmi::dom_parser const&);

    int                class_version() const;
    std::string const& xml_root_name() const;

    bool data_source_is_external(xml::document const&) const;
    void validate_with_xsd_schema
        (xml::document const& xml
        ,std::string const&   xsd
        ) const;
    xslt::stylesheet& cell_sorter() const;
    std::string xsd_schema_name(int version) const;

    Input input_data_;
};

inline Input const& single_cell_document::input_data() const
{
    return input_data_;
}

#endif // single_cell_document_hpp
