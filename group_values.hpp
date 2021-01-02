// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef group_values_hpp
#define group_values_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp"       // enum mcenum_emission
#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <memory>                       // shared_ptr
#include <vector>

class Input;
class Ledger;

/// Result of running a census.
///
/// completed_normally_ is true if the process was allowed to run to
/// completion, and false if it was cancelled, e.g. by cancelling a
/// GUI progress dialog.
///
/// Time is measured for calculations and output but not for input,
/// because the census-run classes accept only preread input.
///
/// Implicitly-declared special member functions do the right thing.

struct census_run_result
{
    census_run_result()
        :completed_normally_       {true}
        ,seconds_for_calculations_ {0.0}
        ,seconds_for_output_       {0.0}
        {}

    bool completed_normally_;
    double seconds_for_calculations_;
    double seconds_for_output_;
};

/// Run all cells in a census.
///
/// Output is emitted to specified targets for all cells as well as
/// the composite. When output is wanted only for the composite, use
/// target 'mce_emit_nothing' and handle output explicitly for the
/// composite, which is accessible through composite(). That usage
/// normally arises when the target isn't known at the time the
/// composite is generated, so adding an emit-composite-only flag here
/// would make little sense.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO run_census final
{
  public:
    run_census() = default;
    ~run_census() = default;

    census_run_result operator()
        (fs::path           const& file
        ,mcenum_emission           emission
        ,std::vector<Input> const& cells
        );

    std::shared_ptr<Ledger const> composite() const;

  private:
    std::shared_ptr<Ledger> composite_;
};

#endif // group_values_hpp
