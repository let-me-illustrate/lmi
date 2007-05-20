// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005, 2006, 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: group_values.hpp,v 1.17 2007-05-20 21:39:29 chicares Exp $

#ifndef group_values_hpp
#define group_values_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

class IllusInputParms;
class Input;
class Ledger;

/// Output target for census runs.
///
/// Enumerators are binary powers so that more than one can be
/// specified in a single scalar entity.

enum enum_emission
    {e_emit_nothing        =    0
    ,e_emit_composite_only =    1
    ,e_emit_quietly        =    2
    ,e_emit_timings        =    4
    ,e_emit_pdf_file       =    8 // Not yet implemented.
    ,e_emit_pdf_to_printer =   16
    ,e_emit_test_data      =   32
    ,e_emit_spreadsheet    =   64
    ,e_emit_text_stream    =  128
    ,e_emit_custom_0       =  256 // Not yet implemented.
    };

/// Run all cells in a census.
///
/// operator() returns true if the process was allowed to run to
/// completion, or false if it was cancelled.
///
/// Output is emitted to specified targets for all cells as well as
/// the composite. When output is wanted only for the composite, use
/// target 'e_emit_nothing' and handle output explicitly for the
/// composite, which is accessible through composite(). That usage
/// normally arises when the target isn't known at the time the
/// composite is generated, so adding an emit-composite-only flag here
/// would make little sense.
///
/// assert_consistency() throws if an inconsistency is detected
/// between a cell and its corresponding case default.
/// TODO ?? Instead, this should be enforced when data is entered.

class LMI_SO run_census
{
  public:
    explicit run_census()
        {}

    bool operator()
        (fs::path const&                     file
        ,enum_emission                       emission
        ,std::vector<IllusInputParms> const& cells
        );

    boost::shared_ptr<Ledger const> composite();

    static void assert_consistency
        (IllusInputParms const& case_default
        ,IllusInputParms const& cell
        );

    static void assert_consistency
        (Input const& case_default
        ,Input const& cell
        );

  private:
    boost::shared_ptr<Ledger> composite_;
};

#endif // group_values_hpp

