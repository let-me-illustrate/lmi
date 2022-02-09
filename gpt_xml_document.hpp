// xml document for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef gpt_xml_document_hpp
#define gpt_xml_document_hpp

#include "config.hpp"

#include "gpt_input.hpp"
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <iosfwd>
#include <string>

class LMI_SO gpt_xml_document final
{
    friend class gpt_document;
    friend class gpt_view;

  public:
    gpt_xml_document() = default;
    explicit gpt_xml_document(gpt_input const&);
    explicit gpt_xml_document(std::string const& filename);
    ~gpt_xml_document() = default;

    gpt_input const& input_data() const;

    void read(std::istream const&);
    void write(std::ostream&) const;

  private:
    gpt_xml_document(gpt_xml_document const&) = delete;
    gpt_xml_document& operator=(gpt_xml_document const&) = delete;

    void parse(xml_lmi::dom_parser const&);
    std::string const& xml_root_name() const;

    gpt_input input_data_;
};

inline gpt_input const& gpt_xml_document::input_data() const
{
    return input_data_;
}

#endif // gpt_xml_document_hpp
