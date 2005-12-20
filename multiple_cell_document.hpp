// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2003, 2005 Gregory W. Chicares.
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

// $Id: multiple_cell_document.hpp,v 1.5 2005-12-20 00:46:41 chicares Exp $

#ifndef multiple_cell_document_hpp
#define multiple_cell_document_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <istream>
#include <ostream>
#include <string>
#include <vector>

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

class LMI_EXPIMP multiple_cell_document
    :private boost::noncopyable
    ,virtual private obstruct_slicing<multiple_cell_document>
{
// TODO ?? Avoid long-distance friendship.
    friend class CensusDocument;

  public:
    multiple_cell_document();
    multiple_cell_document(std::string const& filename);
    ~multiple_cell_document();

    std::vector<IllusInputParms> const& case_parms() const;
    std::vector<IllusInputParms> const& class_parms() const;
    std::vector<IllusInputParms> const& cell_parms() const;

    void read(std::istream& is);
    void write(std::ostream& os) const;

  private:
    // XMLWRAPP !! We'd like the argument to be const, but that
    // doesn't work in xmlwrapp-0.2.0 because get_root_node()
    // isn't const. Perhaps it should be, since it is logically
    // although not physically const--discuss with Peter.
    void parse(xml::tree_parser&);
    std::string xml_root_name() const;

    // Default parameters for the whole case, stored as a vector for
    // parallelism with class_parms_ and cell_parms_. Naturally, this
    // vector must have exactly one element.
    std::vector<IllusInputParms> case_parms_;

    // Default parameters for each employee class.
    std::vector<IllusInputParms> class_parms_;

    // Parameters for each cell.
    std::vector<IllusInputParms> cell_parms_;
};

inline std::istream& operator>>
    (std::istream& is
    ,multiple_cell_document& doc
    )
{
    doc.read(is);
    return is;
}

inline std::ostream& operator<<
    (std::ostream& os
    ,multiple_cell_document& doc
    )
{
    doc.write(os);
    return os;
}

inline std::vector<IllusInputParms> const& multiple_cell_document::case_parms() const
{
    return case_parms_;
}

inline std::vector<IllusInputParms> const& multiple_cell_document::class_parms() const
{
    return class_parms_;
}

inline std::vector<IllusInputParms> const& multiple_cell_document::cell_parms() const
{
    return cell_parms_;
}

#endif // multiple_cell_document_hpp

