// xml document for multiple-cell illustrations.
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#ifndef multiple_cell_document_hpp
#define multiple_cell_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "so_attributes.hpp"
#include "xml_lmi_fwd.hpp"

#include <iosfwd>
#include <string>
#include <vector>

/// A census represented as an xml document.
///
/// The document is composed of three vectors of class Input.
///
/// cell_parms_: parameters for each cell. There are one or more
/// cells. Each represents one illustration. An illustration depicts
/// projected values for a particular life-insurance policy. Usually
/// it corresponds to a concrete person. However, a large, unwieldy
/// census may for simplicity be approximated by a small number of
/// representative cells: e.g., 100 35-year-olds, 250 45-year-olds,
/// and so on, using Input::NumberOfIdenticalLives to specify the
/// number of people in each subgroup. That is why the abstract name
/// "cell" is used here.
///
/// These cells are what the census manager displays. They encompass
/// all the particular illustrations that can be produced from the
/// census. A composite illustration can also be produced: it is the
/// sum of all particular illustrations, weighted by the number of
/// identical lives in each cell.
///
/// case_parms_: default parameters for the whole census. This is
/// somewhat artificially stored as a vector for parallelism with
/// class_parms_ and cell_parms_, but this vector by its nature must
/// always have exactly one element. It serves as a template embodying
/// parameters that are common to all cells. When a new cell is added
/// in the census manager, that new cell is copied from this default.
/// To change one or more parameters across all cells, users modify
/// the default cell and propagate the changes to all cells (and to
/// all class defaults--vide infra).
///
/// class_parms_: default parameters for each employee class. A census
/// may be partitioned into "classes" using Input::EmployeeClass; for
/// example, it may contain two classes: "Managers" whose specified
/// amount is two times salary, and "Workers" who get a flat $50,000.
/// To increase the amount for "Workers" to $75,000, users modify the
/// appropriate class default and propagate the changes to all cells.
/// Because Input::EmployeeClass is simply a string, the number of
/// classes is boundless, and each cell belongs to exactly one class.
/// When a new cell is added in the census manager, it inherits the
/// case-default employee class; users have not asked for a command to
/// add a new cell copied from a selection of class defaults, although
/// that could of course be implemented.

class LMI_SO multiple_cell_document final
{
// TODO ?? Avoid long-distance friendship...in single-cell class, too.
    friend class CensusDocument;
    friend class CensusView;
    friend class input_test;    // For mete_cns_xsd().

  public:
    multiple_cell_document();
    multiple_cell_document(std::string const& filename);
    ~multiple_cell_document() = default;

    std::vector<Input> const& case_parms() const;
    std::vector<Input> const& class_parms() const;
    std::vector<Input> const& cell_parms() const;

    void read(std::istream const&);
    void write(std::ostream&) const;

  private:
    multiple_cell_document(multiple_cell_document const&) = delete;
    multiple_cell_document& operator=(multiple_cell_document const&) = delete;

    void parse   (xml_lmi::dom_parser const&);
    void parse_v0(xml_lmi::dom_parser const&);

    void assert_vector_sizes_are_sane() const;

    int                class_version() const;
    std::string const& xml_root_name() const;

    bool data_source_is_external(xml::document const&) const;
    void validate_with_xsd_schema
        (xml::document const& xml
        ,std::string const&   xsd
        ) const;
    xslt::stylesheet& cell_sorter() const;
    std::string xsd_schema_name(int version) const;

    std::vector<Input> case_parms_;
    std::vector<Input> class_parms_;
    std::vector<Input> cell_parms_;
};

/// Default parameters for the whole case, stored as a vector for
/// parallelism with class_parms_ and cell_parms_. Naturally, this
/// vector must have exactly one element.

inline std::vector<Input> const& multiple_cell_document::case_parms() const
{
    return case_parms_;
}

/// Default parameters for each employee class.

inline std::vector<Input> const& multiple_cell_document::class_parms() const
{
    return class_parms_;
}

/// Parameters for each cell.

inline std::vector<Input> const& multiple_cell_document::cell_parms() const
{
    return cell_parms_;
}

#endif // multiple_cell_document_hpp
