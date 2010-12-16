// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef single_cell_document_hpp
#define single_cell_document_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "xml_lmi_fwd.hpp"

#include <boost/scoped_ptr.hpp>

#include <iosfwd>
#include <string>

class Input;

class LMI_SO single_cell_document
    :        private lmi::uncopyable <single_cell_document>
    ,virtual private obstruct_slicing<single_cell_document>
{
    friend class IllustrationDocument;
    friend class IllustrationView;

  public:
    single_cell_document();
    single_cell_document(Input const&);
    single_cell_document(std::string const& filename);
    ~single_cell_document();

    Input const& input_data() const;

    void read(std::istream const&);
    void write(std::ostream&);

  private:
    void parse(xml::element const&);
    std::string const& xml_root_name() const;

    boost::scoped_ptr<Input> const input_data_;
};

inline Input const& single_cell_document::input_data() const
{
    return *input_data_;
}

#endif // single_cell_document_hpp

