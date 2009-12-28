// Ledger xsl operations.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: ledger_xsl.cpp,v 1.41 2009-03-30 12:07:19 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "ledger_xsl.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"
#include "ledger.hpp"
#include "miscellany.hpp"
#include "path_utility.hpp"
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <ios>
#include <sstream>

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
            << xsl_file
            << "' required for ledger type '"
            << ledger.GetLedgerType()
            << "'."
            << LMI_FLUSH
            ;
        }
    return xsl_file;
}

/// Write ledger as pdf via xsl-fo.
///
/// Ensure that the output filename is portable. Apache fop rejects
/// some names for '-xml' that it accepts for pdf output, without
/// documenting what names it considers valid, so using the boost
///   http://www.boost.org/doc/libs/1_37_0/libs/filesystem/doc/portability_guide.htm
/// conventions here is just a guess. Nonportable names that apache
/// fop accepts for pdf output seem not to work well with the most
/// popular msw pdf viewer, at least in a msw 'dde conversation',
/// so use a similar portable name for pdf output.
///
/// USER !! This filename change should be documented for users.
/// Ultimately, for fop, the reason why their nonportable msw
/// filenames must be transformed is that apache fop is java, and
/// java is "portable".

std::string write_ledger_as_pdf(Ledger const& ledger, fs::path const& filepath)
{
    fs::path print_dir(configurable_settings::instance().print_directory());

    fs::path real_filepath(orthodox_filename(filepath.leaf()));
    LMI_ASSERT(fs::portable_name(real_filepath.string()));

    if(std::string::npos != global_settings::instance().pyx().find("xml"))
        {
        fs::path xml_file = unique_filepath(print_dir / real_filepath, ".xml");

        fs::ofstream ofs(xml_file, ios_out_trunc_binary());
        ledger.write(ofs);
        ofs.close();
        if(!ofs.good())
            {
            fatal_error()
                << "Unable to write output file '"
                << xml_file
                << "'."
                << LMI_FLUSH
                ;
            }
        }

    fs::path xml_fo_file = unique_filepath(print_dir / real_filepath, ".fo.xml");

    fs::ofstream ofs(xml_fo_file, ios_out_trunc_binary());
    ledger.write_xsl_fo(ofs);
    ofs.close();
    if(!ofs.good())
        {
        fatal_error()
            << "Unable to write output file '"
            << xml_fo_file
            << "'."
            << LMI_FLUSH
            ;
        }

    fs::path pdf_out_file = unique_filepath(print_dir / real_filepath, ".pdf");

    std::ostringstream oss;
    oss
        << configurable_settings::instance().xsl_fo_command()
        << " -fo "  << '"' << xml_fo_file  << '"'
        << " -pdf " << '"' << pdf_out_file << '"'
        ;
    system_command(oss.str());
    return pdf_out_file.string();
}

