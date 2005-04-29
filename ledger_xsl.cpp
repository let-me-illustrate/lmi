// Ledger xsl operations.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: ledger_xsl.cpp,v 1.2 2005-04-29 16:14:08 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_xsl.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "ledger.hpp"
#include "path_utility.hpp"
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <ios>
#include <sstream>

namespace
{
/// Return the appropriate xsl filepath for a ledger.
///
/// TODO ?? The only reason for e_ledger_type enumerators to differ is
/// that each requires a distinct output format. Therefore, it makes
/// no sense to use a switch statement at all: instead, it should
/// suffice to append '.xsl' to the string associated with the
/// enumerator.

fs::path xsl_filepath(Ledger const& ledger)
{
    std::string xsl_name;
    switch(ledger.GetLedgerType())
        {
        case e_ill_reg:
        case e_ill_reg_private_placement:  // TODO ?? Should have its own xsl.
            {
            xsl_name = "IllReg.xsl";
            }
            break;
        case e_individual_private_placement:
            {
            xsl_name = "IllIndivPP.xsl";
            }
            break;
        case e_nasd:
        case e_group_private_placement:    // TODO ?? Should have its own xsl.
            {
            xsl_name = "IllNASD.xsl";
            }
            break;
        case e_offshore_private_placement: // TODO ?? Should have its own xsl.
        default:
            {
            fatal_error()
                << "Case '"
                << ledger.GetLedgerType()
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }

    fs::path fop(configurable_settings::instance().xsl_fo_directory());
    fs::path xsl_file(fop / xsl_name);
    if(!fs::exists(xsl_file))
        {
        fatal_error()
            << "Unable to read file '"
            << xsl_file.string()
            << "' required for ledger type '"
            << ledger.GetLedgerType()
            << "'."
            << LMI_FLUSH
            ;
        }
    return xsl_file;
}
} // Unnamed namespace.

/* Yet-untested rewrite of a function in another file:
void IllustrationView::Pdf(std::string action)
{
    LMI_ASSERT(ledger_values_.get());
    wxString z;
    document().GetPrintableName(z);
    std::string document_file(z.c_str());
    write_ledger_to_pdf(ledger_values_.get(), document_file);

    if("open" != action && "print" != action)
        {
        warning()
            << "Action '"
            << action
            << "' unrecognized."
            << LMI_FLUSH
            ;
        return;
        }

    // TODO ?? Experimental. Want a platform-independent solution.
    // Consider this one:
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/583826b8d2f14f2c?dmode=source
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/b05623f68906edbd?dmode=source
// and also
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/900fd00738b9b71a?dmode=source
    // Putting this system call in a gui module means the wx facility
    // can be used; and there's no need to perform such an operation
    // with any other interface.

    HINSTANCE rc = ShellExecute
        ((HWND)GetFrame()->GetHandle()
        ,action.c_str()
        ,pdf_out_file.string().c_str()
        ,0
        ,0 // directory
        ,SW_SHOWNORMAL
        );
    if(reinterpret_cast<long int>(rc) <= 32)
        {
        warning()
            << "Failed to "
            << action
            << " file '"
            << pdf_out_file.string()
            << "'. If it was already open, then it could not"
            << " be updated--close it first, then try again."
            ;
        return;
        }
}
*/

void write_ledger_to_pdf
    (Ledger const&      ledger
    ,std::string const& filename
    )
{
    fs::path fop(configurable_settings::instance().xsl_fo_directory());

    fs::path xml_out_file = unique_filepath(fop / filename, ".xml");

    fs::ofstream ofs(xml_out_file, std::ios_base::out | std::ios_base::trunc);
    ledger.write(ofs);
    ofs.close();

    fs::path xsl_file = xsl_filepath(ledger);

    // TODO ?? Use xml_out_file as exemplar instead?
    fs::path pdf_out_file = unique_filepath(fop / filename, ".pdf");

    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -xsl "  << '"' << xsl_file.string()     << '"'
        << " -xml "  << '"' << xml_out_file.string() << '"'
        << " "       << '"' << pdf_out_file.string() << '"'
        ;

    int rc = system_command(oss.str());

    // TODO ?? Using apache fop on the msw platform, following the
    // procedure suggested at the apache website, this seems not to
    // catch any problem--perhaps because they use a batch file that
    // eats the error code?
    if(rc)
        {
        warning()
            << "Report formatting failed.\n"
            << "The specific command that failed was '"
            << oss.str()
            << "'."
            << LMI_FLUSH
            ;
        return;
        }
}

