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

// $Id: group_values.hpp,v 1.1 2005-04-16 02:05:41 chicares Exp $

#ifndef group_values_hpp
#define group_values_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

class IllusInputParms;
class Ledger;

std::string LMI_EXPIMP serialize_extension
    (int                serial_number
    ,std::string const& extension
    );

// TODO ?? Make this a functor.
void LMI_EXPIMP RunCensusInSeries
    (fs::directory_iterator       const& file
    ,std::vector<IllusInputParms> const& cells
    ,Ledger                            & composite
    );

struct LMI_EXPIMP RunCensusInParallel
{
    explicit RunCensusInParallel()
// TODO ?? Add timing code to implementation:
//        ,time_for_calculations(0.0)
//        ,time_for_output      (0.0)
        {}

    void operator()
        (fs::directory_iterator       const& file
        ,std::vector<IllusInputParms> const& cells
        ,Ledger                            & composite
        );

//    double time_for_calculations;
//    double time_for_output;
};

#endif // group_values_hpp

