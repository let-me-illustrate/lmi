// Run an individual illustration, producing a ledger.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledgervalues.cpp,v 1.37 2008-12-12 13:32:19 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledgervalues.hpp"

#include "account_value.hpp"
#include "assert_lmi.hpp"
#include "fenv_guard.hpp"
#include "input.hpp"
#include "ledger.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

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

    fs::path debug_filename = fs::change_extension
        (fs::path(filename_)
        ,".debug"
        );
    av.SetDebugFilename(debug_filename.string());

    double z = av.RunAV();
    ledger_ = av.ledger_from_av();

    return z;
}

boost::shared_ptr<Ledger const> IllusVal::ledger() const
{
    LMI_ASSERT(ledger_.get());
    return ledger_;
}

