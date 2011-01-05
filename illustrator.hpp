// Sole top-level facility for illustration generation.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#ifndef illustrator_hpp
#define illustrator_hpp

#include "config.hpp"

#include "mc_enum_type_enums.hpp" // enum mcenum_emission
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <functional>
#include <vector>

class Input;
class Ledger;

/// Sole top-level facility for illustration generation.
///
/// Implicitly-declared special member functions do the right thing.

class LMI_SO illustrator
    :public std::unary_function<fs::path const&, bool>
    ,virtual private obstruct_slicing<illustrator>
{
  public:
    explicit illustrator(mcenum_emission);
    ~illustrator();

    bool operator()(fs::path const&);
    bool operator()(fs::path const&, Input const&);
    bool operator()(fs::path const&, std::vector<Input> const&);

    void conditionally_show_timings_on_stdout() const;

    boost::shared_ptr<Ledger const> principal_ledger() const;

    double usec_for_input       () const;
    double usec_for_calculations() const;
    double usec_for_output      () const;

  private:
    mcenum_emission emission_;
    boost::shared_ptr<Ledger const> principal_ledger_;
    double usec_for_input_;
    double usec_for_calculations_;
    double usec_for_output_;
};

Input const& LMI_SO default_cell();

void LMI_SO assert_consistency
    (Input const& case_default
    ,Input const& cell
    );

#endif // illustrator_hpp

