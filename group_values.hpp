// Distinct and composite values for cells in a group.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: group_values.hpp,v 1.6 2005-05-05 15:22:14 chicares Exp $

#ifndef group_values_hpp
#define group_values_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <vector>

class IllusInputParms;
class Ledger;

// Enumerators are binary powers so that more than one can be
// specified in a single int.
enum e_emission_target
    {emit_to_nowhere     = 0
    ,emit_to_printer     = 1
    ,emit_to_spew_file   = 2
    ,emit_to_spreadsheet = 4
    };

void emit_ledger
    (fs::path const&               file
    ,int                           index
    ,Ledger const&                 ledger
    ,e_emission_target             emission_target
    );

// TODO ?? Sometimes all cells should be emitted; sometimes, only the composite.
    
// TODO ?? Should this be derived from std::*nary_function?

class LMI_EXPIMP run_census
{
  public:
    explicit run_census()
        {}

    bool operator()
        (fs::path const&                     file
        ,e_emission_target                   emission_target
        ,std::vector<IllusInputParms> const& cells
        );

    Ledger const& composite();

  private:
    boost::shared_ptr<Ledger> composite_;
// TODO ?? Consider adding data members for timings.
};

// TODO ?? Should these other functors have linkage?

class LMI_EXPIMP run_census_in_series
{
  public:
    explicit run_census_in_series()
// TODO ?? Add timing code to implementation:
//        ,time_for_calculations(0.0)
//        ,time_for_output      (0.0)
        {}

    bool operator()
        (fs::path const&                     file
        ,e_emission_target                   emission_target
        ,std::vector<IllusInputParms> const& cells
        ,Ledger&                             composite
        );

  private:
//    double time_for_calculations;
//    double time_for_output;
};

class LMI_EXPIMP run_census_in_parallel
{
  public:
    explicit run_census_in_parallel()
// TODO ?? Add timing code to implementation:
//        ,time_for_calculations(0.0)
//        ,time_for_output      (0.0)
        {}

    bool operator()
        (fs::path const&                     file
        ,e_emission_target                   emission_target
        ,std::vector<IllusInputParms> const& cells
        ,Ledger&                             composite
        );

  private:
//    double time_for_calculations;
//    double time_for_output;
};

#endif // group_values_hpp

