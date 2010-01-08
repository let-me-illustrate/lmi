// xml document for MEC testing.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

#ifndef mec_xml_document_hpp
#define mec_xml_document_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <iosfwd>
#include <string>

class mec_input;

class LMI_SO mec_xml_document
    :private boost::noncopyable
    ,virtual private obstruct_slicing<mec_xml_document>
{
    friend class mec_document;
    friend class mec_view;

  public:
    mec_xml_document();
    mec_xml_document(mec_input const&);
    ~mec_xml_document();

    mec_input const& input_data() const;

    void read(std::istream const&);
    void write(std::ostream&);

  private:
    void parse(xml::element const&);
    std::string xml_root_name() const;

    boost::scoped_ptr<mec_input> const input_data_;
};

inline mec_input const& mec_xml_document::input_data() const
{
    return *input_data_;
}

#endif // mec_xml_document_hpp

