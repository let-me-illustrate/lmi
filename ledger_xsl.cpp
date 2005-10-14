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

// $Id: ledger_xsl.cpp,v 1.9 2005-10-14 13:40:44 chicares Exp $

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
fs::path xsl_filepath(Ledger const& ledger)
{
    std::string xsl_name = ledger.GetLedgerType().str() + ".xsl";
    fs::path fo_dir(configurable_settings::instance().xsl_fo_directory());
    fs::path xsl_file(fo_dir / xsl_name);
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

std::string write_ledger_to_pdf
    (Ledger const&      ledger
    ,std::string const& filename
    )
{
    fs::path fo_dir(configurable_settings::instance().xsl_fo_directory());

    // Ensure that the output filename is portable. Apache fop rejects
    // some names for '-xml' that it accepts for pdf output, without
    // documenting what names it considers valid, so using the boost
    // conventions here is just a guess. Nonportable names that apache
    // fop accepts for pdf output seem not to work well with the most
    // popular msw pdf viewer, at least in a msw 'dde conversation',
    // so use a similar portable name for pdf output.
    //
    // SOMEDAY !! It might be nicer to transform a nonportable name to
    // a portable one, preserving as many original characters as
    // possible--for instance, by filtering out everything but
    // [.-_A-Za-z0-9] and forbidding a terminal period '.'. OTOH, some
    // users might find that more astonishing than the behavior
    // implemented here.
    //
    // USER !! This filename change should be documented for users.
    // Ultimately, for fop, the reason why their nonportable msw
    // filenames must be transformed is that apache fop is java, and
    // java is "portable".
    //
    std::string real_filename
        (fs::portable_name(filename)
        ?   filename
        :   "output"
        );

    fs::path xml_out_file = unique_filepath(fo_dir / real_filename, ".xml");

    fs::ofstream ofs(xml_out_file, std::ios_base::out | std::ios_base::trunc);
    // Scale a copy of the 'ledger' argument. The original must not be
    // modified because scaling is not reentrant. TODO ?? However,
    // that problem is not avoided here, because what is scaled is
    // actually not copied due to use of shared_ptr; see description
    // of problems in the ledger-class implementation.
    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();
    scaled_ledger.write(ofs);
    ofs.close();

    fs::path xsl_file = xsl_filepath(ledger);

    fs::path pdf_out_file = unique_filepath(fo_dir / real_filename, ".pdf");

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
    // catch all problems--perhaps because they use a batch file that
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
        }
    return pdf_out_file.string();
}

