// Ledger xsl operations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: ledger_xsl.cpp,v 1.34 2008-11-17 00:37:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_xsl.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"       // "idiosyncrasy_spreadsheet" workaround
#include "ledger_formatter.hpp"
#include "miscellany.hpp"
#include "null_stream.hpp"            // "idiosyncrasy_spreadsheet" workaround
#include "path_utility.hpp"
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <ios>
#include <ostream>                    // "idiosyncrasy_spreadsheet" workaround
#include <sstream>

// Define this macro here, after including all the headers that it
// could potentially affect.

#define LMI_USE_NEW_REPORTS

namespace
{
std::string xsl_filename(Ledger const& ledger)
{
    mcenum_ledger_type const z = ledger.GetLedgerType();
    switch(z)
        {
        case mce_ill_reg:                      return "illustration_reg.xsl";
        case mce_nasd:                         return "nasd.xsl";
        case mce_group_private_placement:      return "reg_d_group.xsl";
        case mce_offshore_private_placement:   return "reg_d_offshore.xsl";
        case mce_individual_private_placement: return "reg_d_individual.xsl";
        case mce_variable_annuity:             return "variable_annuity.xsl";
        default:
            {
            fatal_error() << "Case '" << z << "' not found." << LMI_FLUSH;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}
} // Unnamed namespace.

/// File path for xsl-fo file appropriate for the given ledger.

fs::path xsl_filepath(Ledger const& ledger)
{
    std::string xsl_name = xsl_filename(ledger);
    fs::path xsl_file(global_settings::instance().data_directory() / xsl_name);
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

std::string write_ledger_as_pdf(Ledger const& ledger, fs::path const& filepath)
{
    fs::path print_dir(configurable_settings::instance().print_directory());

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
    fs::path real_filepath
        (fs::portable_name(filepath.string())
        ?   filepath
        :   fs::path("output")
        );

    // EXPERIMENTAL This variable should be renamed, e.g., to 'xml_fo_file'.
    fs::path xml_out_file = unique_filepath(print_dir / real_filepath, ".fo.xml");

    fs::ofstream ofs(xml_out_file, ios_out_trunc_binary());

if(std::string::npos != global_settings::instance().pyx().find("new") || global_settings::instance().pyx().empty())
{
    ledger.write_xsl_fo(ofs);
}
else
{
    // Scale a copy of the 'ledger' argument. The original must not be
    // modified because scaling is not reentrant. However, copying
    // does not prevent that problem here, because what is scaled is
    // actually not copied due to use of shared_ptr; see comment on
    //   https://savannah.nongnu.org/bugs/?13599
    // in the ledger-class implementation.
    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();
#if defined LMI_USE_NEW_REPORTS
    LedgerFormatterFactory& factory = LedgerFormatterFactory::Instance();
    LedgerFormatter formatter(factory.CreateFormatter(scaled_ledger));
    formatter.FormatAsXslFo(ofs);
    // TODO ?? Eliminate the problem that this works around.
    if
        (   ledger.GetIsComposite()
        &&  std::string::npos != ledger.GetLedgerInvariant().Comments.find("idiosyncrasy_spreadsheet")
        )
        {
        scaled_ledger.write(null_stream());
        }
#else  // !defined LMI_USE_NEW_REPORTS
    scaled_ledger.write(ofs);
#endif // !defined LMI_USE_NEW_REPORTS
}
    ofs.close();

    fs::path xsl_file = xsl_filepath(ledger);

    fs::path pdf_out_file = unique_filepath(print_dir / real_filepath, ".pdf");

    std::ostringstream oss;
#if defined LMI_USE_NEW_REPORTS
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -fo "  << '"' << xml_out_file.string() << '"'
        << " -pdf " << '"' << pdf_out_file.string() << '"'
        ;
#else  // !defined LMI_USE_NEW_REPORTS
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -xsl "  << '"' << xsl_file.string()     << '"'
        << " -xml "  << '"' << xml_out_file.string() << '"'
        << " "       << '"' << pdf_out_file.string() << '"'
        ;
#endif // !defined LMI_USE_NEW_REPORTS

    system_command(oss.str());
    return pdf_out_file.string();
}

