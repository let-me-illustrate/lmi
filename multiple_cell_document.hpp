// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifndef multiple_cell_document_hpp
#define multiple_cell_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"
#include "uncopyable_lmi.hpp"
#include "xml_lmi_fwd.hpp"

#include <iosfwd>
#include <string>
#include <vector>

class LMI_SO multiple_cell_document
    :        private lmi::uncopyable <multiple_cell_document>
    ,virtual private obstruct_slicing<multiple_cell_document>
{
// TODO ?? Avoid long-distance friendship...in single-cell class, too.
    friend class CensusDocument;
    friend class CensusView;

  public:
    multiple_cell_document();
    multiple_cell_document(std::string const& filename);
    ~multiple_cell_document();

    std::vector<Input> const& case_parms() const;
    std::vector<Input> const& class_parms() const;
    std::vector<Input> const& cell_parms() const;

    void read(std::istream const&);
    void write(std::ostream&) const;

  private:
    void parse(xml::element const&);
    std::string const& xml_root_name() const;

    // Default parameters for the whole case, stored as a vector for
    // parallelism with class_parms_ and cell_parms_. Naturally, this
    // vector must have exactly one element.
    std::vector<Input> case_parms_;

    // Default parameters for each employee class.
    std::vector<Input> class_parms_;

    // Parameters for each cell.
    std::vector<Input> cell_parms_;
};

inline std::vector<Input> const& multiple_cell_document::case_parms() const
{
    return case_parms_;
}

inline std::vector<Input> const& multiple_cell_document::class_parms() const
{
    return class_parms_;
}

inline std::vector<Input> const& multiple_cell_document::cell_parms() const
{
    return cell_parms_;
}

#endif // multiple_cell_document_hpp

