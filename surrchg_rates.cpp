// Life insurance illustrations: surrender charge rates.
//
// Copyright (C) 2003, 2005 Gregory W. Chicares.
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

// $Id: surrchg_rates.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "surrchg_rates.hpp"

#include "database.hpp"
#include "dbnames.hpp"

//============================================================================
SurrChgRates::SurrChgRates(TDatabase const& database)
{
    Initialize(database);
}

//============================================================================
void SurrChgRates::Initialize(TDatabase const& database)
{
    database.Query(RatePerDollarOfPremium_     , DB_SurrChgPremMult   );
    database.Query(RatePerDollarOfAcctval_     , DB_SurrChgAVMult     );
    database.Query(RatePerDollarOfSpecamt_     , DB_SurrChgSAMult     );
    database.Query(AcctvalRateDurationalFactor_, DB_SurrChgAVDurFactor);
    database.Query(SpecamtRateDurationalFactor_, DB_SurrChgSADurFactor);
}

