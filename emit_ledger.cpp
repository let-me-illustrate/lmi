// Emit a ledger or a group of ledgers in various guises.
//
// Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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
#include "custom_io_1.hpp"
#include "file_command.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "ledger_xsl.hpp"
#include "miscellany.hpp"               // ios_out_trunc_binary()
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <cstdio>                       // std::remove()
#include <iostream>
#include <string>

/// Emit a group of ledgers in various guises.
///
/// The ledgers constitute a 'case' consisting of 'cells' as those
/// concepts are defined for class multiple_cell_document.

ledger_emitter::ledger_emitter
    (fs::path const& case_filepath
    ,mcenum_emission emission
    )
    :case_filepath_ (case_filepath)
    ,emission_      (emission)
{}

ledger_emitter::~ledger_emitter()
{}

/// Perform initial case-level steps such as writing headers.

double ledger_emitter::initiate()
{
    Timer timer;

    if(emission_ & mce_emit_spreadsheet)
        {
        LMI_ASSERT(!case_filepath_.empty());
        std::string spreadsheet_filename =
                case_filepath_.string()
            +   configurable_settings::instance().spreadsheet_file_extension()
            ;
        std::remove(spreadsheet_filename.c_str());
        }
    if(emission_ & mce_emit_group_roster)
        {
        LMI_ASSERT(!case_filepath_.empty());
        std::string spreadsheet_filename =
                case_filepath_.string()
            +   ".roster"
            +   configurable_settings::instance().spreadsheet_file_extension()
            ;
        std::remove(spreadsheet_filename.c_str());
        PrintRosterHeaders(spreadsheet_filename);
        }

    return timer.stop().elapsed_seconds();
}

/// Perform cell-level steps.

double ledger_emitter::emit_cell
    (fs::path const& cell_filepath
    ,Ledger const& ledger
    )
{
    Timer timer;
    if((emission_ & mce_emit_composite_only) && !ledger.GetIsComposite())
        {
        goto done;
        }

    if(emission_ & mce_emit_pdf_file)
        {
        write_ledger_as_pdf(ledger, cell_filepath);
        }
    if(emission_ & mce_emit_pdf_to_printer)
        {
        std::string pdf_out_file = write_ledger_as_pdf(ledger, cell_filepath);
        file_command()(pdf_out_file, "print");
        }
    if(emission_ & mce_emit_pdf_to_viewer)
        {
        std::string pdf_out_file = write_ledger_as_pdf(ledger, cell_filepath);
        file_command()(pdf_out_file, "open");
        }
    if(emission_ & mce_emit_test_data)
        {
        fs::ofstream ofs
            (fs::change_extension(cell_filepath, ".test")
            ,ios_out_trunc_binary()
            );
        ledger.Spew(ofs);
        }
    if(emission_ & mce_emit_spreadsheet)
        {
        LMI_ASSERT(!case_filepath_.empty());
        PrintCellTabDelimited
            (ledger
            ,   case_filepath_.string()
            +   configurable_settings::instance().spreadsheet_file_extension()
            );
        }
    if(emission_ & mce_emit_group_roster)
        {
        LMI_ASSERT(!case_filepath_.empty());
        PrintRosterTabDelimited
            (ledger
            ,   case_filepath_.string()
            +   ".roster"
            +   configurable_settings::instance().spreadsheet_file_extension()
            );
        }
    if(emission_ & mce_emit_text_stream)
        {
        PrintLedgerFlatText(ledger, std::cout);
        }
    if(emission_ & mce_emit_custom_0)
        {
        configurable_settings const& c = configurable_settings::instance();
        fs::path out_file =
            cell_filepath.string() == c.custom_input_0_filename()
            ? c.custom_output_0_filename()
            : fs::change_extension(cell_filepath, ".test0")
            ;
        custom_io_0_write(ledger, out_file.string());
        }
    if(emission_ & mce_emit_custom_1)
        {
        configurable_settings const& c = configurable_settings::instance();
        fs::path out_file =
            cell_filepath.string() == c.custom_input_1_filename()
            ? c.custom_output_1_filename()
            : fs::change_extension(cell_filepath, ".test1")
            ;
        custom_io_1_write(ledger, out_file.string());
        }

  done:
    return timer.stop().elapsed_seconds();
}

/// Perform final case-level steps such as numbering output pages.

double ledger_emitter::finish()
{
    Timer timer;

    ; // Nothing to do for now.

    return timer.stop().elapsed_seconds();
}

/// Emit a single ledger in various guises.
///
/// Return time spent, which is almost always wanted.

double emit_ledger
    (fs::path const& cell_filepath
    ,Ledger const&   ledger
    ,mcenum_emission emission
    )
{
    ledger_emitter emitter("", emission);
    return emitter.emit_cell(cell_filepath, ledger);
}

