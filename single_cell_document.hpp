// xml document for single-cell illustration.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: single_cell_document.hpp,v 1.3 2005-03-02 03:33:22 chicares Exp $

#ifndef single_cell_document_hpp
#define single_cell_document_hpp

#include "config.hpp"

#include "expimp.hpp"

#include "obstruct_slicing.hpp"

#include <boost/scoped_ptr.hpp>
#include <boost/utility.hpp>

#include <istream>
#include <memory>
#include <ostream>
#include <string>

class IllusInputParms;

// XMLWRAPP !! Recommend an xmlwrapp_fwd header to Peter. It might
// contain all of the following (we don't use them all here):
namespace xml
{
    class node;
    class event_parser;
    class tree_parser;
    class init;
}

class LMI_EXPIMP single_cell_document
    :private boost::noncopyable
    ,virtual private obstruct_slicing<single_cell_document>
{
// TODO ?? Too many long-distance friendships.
    friend class IllustrationDocument;
    friend class IllustrationView;
    friend class CensusView;

  public:
    single_cell_document();
    single_cell_document(IllusInputParms const&);
    single_cell_document(std::string const& filename);
    ~single_cell_document();

    IllusInputParms const& input_data() const;

    void read(std::istream& is);
    void write(std::ostream& os);

  private:
    // XMLWRAPP !! We'd like the argument to be const, but that
    // doesn't work in xmlwrapp-0.2.0 because get_root_node()
    // isn't const. Perhaps it should be, since it is logically
    // although not physically const--discuss with Peter.
    void parse(xml::tree_parser&);
    std::string xml_root_name() const;

    boost::scoped_ptr<IllusInputParms> const input_data_;
};

inline IllusInputParms const& single_cell_document::input_data() const
{
    return *input_data_;
}

inline std::istream& operator>>
    (std::istream& is
    ,single_cell_document& doc
    )
{
    doc.read(is);
    return is;
}

inline std::ostream& operator<<
    (std::ostream& os
    ,single_cell_document& doc
    )
{
    doc.write(os);
    return os;
}

#endif // single_cell_document_hpp

