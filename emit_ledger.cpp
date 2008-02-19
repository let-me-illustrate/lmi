// Emit a ledger in various guises.
//
// Copyright (C) 2005, 2006, 2007, 2008 Gregory W. Chicares.
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

// $Id: emit_ledger.cpp,v 1.11 2008-02-19 00:46:28 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "emit_ledger.hpp"

#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "file_command.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "miscellany.hpp"          // ios_out_trunc_binary()
#include "path_utility.hpp"
#include "timer.hpp"

#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <string>

// EVGENIY !! EXPERIMENTAL.
void experiment(fs::path const&, int, Ledger const&);

/// Emit a ledger in various guises.
///
/// The commands for mce_emit_pdf_file and mce_emit_pdf_to_printer are
/// spelled out separately and in full, though one uses a copy of the
/// other. Reason: in the future, they may be implemented differently,
/// and mce_emit_pdf_to_printer may write directly to the printer
/// without creating any file.

double emit_ledger
    (fs::path const& filepath
    ,int             serial_index
    ,Ledger const&   ledger
    ,mcenum_emission emission
    )
{
    Timer timer;
    if((emission & mce_emit_composite_only) && !ledger.GetIsComposite())
        {
        goto done;
        }

    if(emission & mce_emit_pdf_file)
        {
// EVGENIY !! EXPERIMENTAL.
        experiment(filepath, serial_index, ledger);
// Has nondefault behavior due to experimental argument; except for
// that, this might already work from the command line (untested).
        write_ledger_as_pdf
            (ledger
            ,serialized_file_path(filepath, serial_index, "ill").string()
            ,true // EVGENIY !! EXPERIMENTAL
            );
        }
    if(emission & mce_emit_pdf_to_printer)
        {
// EVGENIY !! EXPERIMENTAL.
// Does not yet work from command line interface: file_command() unimplemented.
// Should we pass '-print' to 'fop' instead of using wxTheMimeTypesManager?
        std::string pdf_out_file = write_ledger_as_pdf
            (ledger
            ,serialized_file_path(filepath, serial_index, "ill").string()
            );
        file_command()(pdf_out_file, "print");
        }
    if(emission & mce_emit_test_data)
        {
        fs::ofstream ofs
            (serialized_file_path(filepath, serial_index, "test")
            ,ios_out_trunc_binary()
            );
        ledger.Spew(ofs);
        }
    if(emission & mce_emit_spreadsheet)
        {
        PrintFormTabDelimited
            (ledger
            ,   filepath.string()
            +   configurable_settings::instance().spreadsheet_file_extension()
            );
        }
    if(emission & mce_emit_text_stream)
        {
        PrintLedgerFlatText(ledger, std::cout);
        }
    if(emission & mce_emit_custom_0)
        {
        custom_io_0_write(ledger, filepath.string());
        }

  done:
    return timer.stop().elapsed_usec();
}

// EVGENIY !! EXPERIMENTAL.
//
// For now, "emit_pdf_file" writes some intermediate files, but no pdf.
// As experimentally implemented here, it needs these extra headers:

#include "system_command.hpp"

#include <boost/filesystem/convenience.hpp>

#include <sstream>

void experiment
    (fs::path const& filepath
    ,int             serial_index
    ,Ledger const&   ledger
    )
{
    std::string exemplar = filepath.leaf() + serialize_extension(serial_index, "change_this");
std::cout << "Processing file stem " << fs::basename(exemplar) << '\n';
Timer timer0;
    write_ledger_as_xml(ledger, fs::basename(exemplar));
std::cout
    << "...wrote '"
    << fs::change_extension(exemplar, ".xml").string()
    << "'.\n"
    << "  time: " << timer0.stop().elapsed_msec_str()
    << std::endl
    ;
timer0.restart();
    write_ledger_as_xml2(ledger, fs::basename(exemplar));
std::cout
    << "...wrote '"
    << fs::change_extension(exemplar, ".v2.xml").string()
    << "'.\n"
    << "  time: " << timer0.stop().elapsed_msec_str()
    << std::endl
    ;
timer0.restart();
    write_ledger_as_fo_xml(ledger, fs::basename(exemplar));
std::cout
    << "...wrote '"
    << fs::change_extension(exemplar, ".fo.xml").string()
    << "'.\n"
    << "  time: " << timer0.stop().elapsed_msec_str()
    << std::endl
    ;
timer0.restart();
    std::ostringstream oss;
    oss
        << "xsltproc"
        << " -o " << fs::change_extension(exemplar, ".xsltproc.xml").string()
        << " "    << xsl_filepath(ledger).string()
        << " "    << fs::change_extension(exemplar, ".xml").string()
        ;
std::cout << "...executing command:\n    " << oss.str() << '\n';
    system_command(oss.str());
std::cout
    << "...wrote '"
    << fs::change_extension(exemplar, ".xsltproc.xml").string()
    << "'.\n"
    << "  time: " << timer0.stop().elapsed_msec_str()
    << std::endl
    ;
}

