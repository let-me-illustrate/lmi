// Ledger data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: ledgervalues.cpp,v 1.23 2007-05-25 02:31:18 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledgervalues.hpp"

#include "account_value.hpp"
#include "assert_lmi.hpp"
#include "fenv_guard.hpp"
#include "inputs.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp" // PrintLedgerFlatText()

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <ostream>
#include <string>

//============================================================================
IllusVal::IllusVal(std::string const& filename)
    :filename_ (filename)
{
}

// TODO ?? Pointless: expunge at the same time as struct RunCensus.
//============================================================================
IllusVal::IllusVal(Ledger* ledger, std::string const& filename)
    :filename_ (filename)
    ,ledger_   (ledger)
{
}

//============================================================================
IllusVal::~IllusVal()
{
}

//============================================================================
double IllusVal::Run(InputParms const* IP)
{
    fenv_guard fg;
    AccountValue av(*IP);

    fs::path debug_filename = fs::change_extension
        (fs::path(filename_)
        ,".debug"
        );
    av.SetDebugFilename(debug_filename.string());

    double z = av.RunAV();
    // TODO ?? This seems nasty, but probably this whole class should
    // be eradicated.
    LMI_ASSERT(av.ledger_from_av().get());
    ledger_ = boost::shared_ptr<Ledger>(new Ledger(*av.ledger_from_av()));

// TODO ?? Temporary code for trying to track down a problem.
#if 0
//LMI_ASSERT(*av.ledger_from_av().GetLedgerMap() == ledger_->GetLedgerMap());
//LMI_ASSERT(*av.ledger_from_av().GetLedgerInvariant() == ledger_->GetLedgerInvariant());
LMI_ASSERT(*av.ledger_from_av().GetLedgerInvariant().GetInforceLives() == ledger_->GetLedgerInvariant().GetInforceLives());

LMI_ASSERT(*av.ledger_from_av().GetLedgerType() == ledger_->GetLedgerType());
LMI_ASSERT(*av.ledger_from_av().GetRunBases() == ledger_->GetRunBases());
LMI_ASSERT(*av.ledger_from_av().GetIsComposite() == ledger_->GetIsComposite());
#endif // 0

    return z;
}

//============================================================================
void IllusVal::Print(std::ostream& os) const
{
    PrintLedgerFlatText(ledger(), os);
}

