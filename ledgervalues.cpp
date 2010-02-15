// Run an individual illustration, producing a ledger.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledgervalues.hpp"

#include "account_value.hpp"
#include "assert_lmi.hpp"
#include "fenv_guard.hpp"
#include "input.hpp"
#include "ledger.hpp"

IllusVal::IllusVal(std::string const& filename)
    :filename_ (filename)
{
}

IllusVal::~IllusVal()
{
}

double IllusVal::run(Input const& input)
{
    fenv_guard fg;
    AccountValue av(input);
    av.SetDebugFilename(filename_);

    double z = av.RunAV();
    ledger_ = av.ledger_from_av();

    return z;
}

boost::shared_ptr<Ledger const> IllusVal::ledger() const
{
    LMI_ASSERT(ledger_.get());
    return ledger_;
}

