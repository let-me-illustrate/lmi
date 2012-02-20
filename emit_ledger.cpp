// Emit a ledger in various guises.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#include "emit_ledger.hpp"

#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "file_command.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "miscellany.hpp"          // ios_out_trunc_binary()
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <string>

/// Emit a ledger in various guises.
///
/// The commands for
///   mce_emit_pdf_file
///   mce_emit_pdf_to_printer
///   mce_emit_pdf_to_viewer
/// are spelled out separately and in full, which is redundant.
/// Reason: in the future, they may be implemented differently.
/// In particular, mce_emit_pdf_to_printer might pass '-print' to
/// 'fop' instead of using wxTheMimeTypesManager, which would make
/// it available with the command-line interface. That interface
/// doesn't support mce_emit_pdf_to_viewer at all: it could be
/// supported in a platform-dependent way, but it would be strange
/// to require a command-line program to invoke an external GUI
/// program.
///
/// The 'tsv_filepath' argument is used only for mce_emit_spreadsheet,
/// for which a single output file encompasses all cells in a census,
/// whereas other output types produce a separate file for each cell.

double emit_ledger
    (fs::path const& filepath
    ,fs::path const& tsv_filepath
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
        write_ledger_as_pdf(ledger, filepath);
        }
    if(emission & mce_emit_pdf_to_printer)
        {
        std::string pdf_out_file = write_ledger_as_pdf(ledger, filepath);
        file_command()(pdf_out_file, "print");
        }
    if(emission & mce_emit_pdf_to_viewer)
        {
        std::string pdf_out_file = write_ledger_as_pdf(ledger, filepath);
        file_command()(pdf_out_file, "open");
        }
    if(emission & mce_emit_test_data)
        {
        fs::ofstream ofs
            (fs::change_extension(filepath, ".test")
            ,ios_out_trunc_binary()
            );
        ledger.Spew(ofs);
        }
    if(emission & mce_emit_spreadsheet)
        {
        LMI_ASSERT(!tsv_filepath.empty());
        PrintFormTabDelimited
            (ledger
            ,   tsv_filepath.string()
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
    return timer.stop().elapsed_seconds();
}

