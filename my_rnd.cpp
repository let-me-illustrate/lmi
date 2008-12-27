// Product-specific fund data.
//
// Copyright (C) 2001, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: my_rnd.cpp,v 1.6 2008-12-27 02:56:51 chicares Exp $

// This file is a template for embedding product-specific data. Doing
// that creates a derived work covered by the GPL. But you may prefer
// not to publish your data, for instance because it is proprietary.
// In that case, the GPL does not permit you to distribute the derived
// work at all. But read the second paragraph of section 0 of the GPL
// carefully: it permits you to run your modified version of the
// program--and to distribute its output, which is not a derived work
// because it's merely your data, trivially cast in a format suitable
// for use with lmi. You can therefore distribute the files created by
// your modified version of this program, but not that program itself.
// Those files are all you need: distributing the program itself isn't
// necessary anyway.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_rnddata.hpp"

#include "data_directory.hpp"

// TODO ?? It would be better to call StreamableRoundingRules::WriteRndFiles()
// here than to duplicate what it does. However, it would be better still
// to use xml for all product data files.

//============================================================================
void StreamableRoundingRules::WriteProprietaryRndFiles()
{
    // Sample policy form.
    StreamableRoundingRules sample;

    sample.round_specamt_         = round_to<double>(0, r_upward    );
    sample.round_death_benefit_   = round_to<double>(2, r_to_nearest);
    sample.round_naar_            = round_to<double>(2, r_to_nearest);
    sample.round_coi_rate_        = round_to<double>(8, r_downward  );
    sample.round_coi_charge_      = round_to<double>(2, r_to_nearest);
    sample.round_gross_premium_   = round_to<double>(2, r_to_nearest);
    sample.round_net_premium_     = round_to<double>(2, r_to_nearest);
    sample.round_interest_rate_   = round_to<double>(0, r_not_at_all);
    sample.round_interest_credit_ = round_to<double>(2, r_to_nearest);
    sample.round_withdrawal_      = round_to<double>(2, r_to_nearest);
    sample.round_loan_            = round_to<double>(2, r_to_nearest);
    sample.round_corridor_factor_ = round_to<double>(2, r_to_nearest);
    sample.round_surrender_charge_= round_to<double>(2, r_to_nearest);
    sample.round_irr_             = round_to<double>(5, r_downward  );

    sample.Write(AddDataDir("sample.rnd"));

    // Another policy form....
}

