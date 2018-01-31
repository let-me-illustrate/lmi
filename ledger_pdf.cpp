// Ledger PDF generation.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "ledger_pdf.hpp"

#include "configurable_settings.hpp"
#include "global_settings.hpp" // PDF !! expunge
#include "ledger.hpp"
#include "ledger_pdf_generator.hpp"
#include "ledger_xsl.hpp" // PDF !! expunge
#include "path_utility.hpp"             // unique_filepath()

/// Write ledger as pdf.

std::string write_ledger_as_pdf(Ledger const& ledger, fs::path const& filepath)
{
    // PDF !! Expunge this conditional block:
    if(global_settings::instance().ash_nazg())
        {
        // Execute both the new and the old code so that their results
        // may be compared.
        write_ledger_as_pdf_via_xsl(ledger, filepath);
        }

    throw_if_interdicted(ledger);

    fs::path print_dir(configurable_settings::instance().print_directory());
    // PDF !! Either orthodox_filename() should be used here, or its
    // use should be reconsidered everywhere else.
    fs::path pdf_out_file = unique_filepath(print_dir / filepath, ".pdf");

    auto const pdf = ledger_pdf_generator::create();
    pdf->write(ledger, pdf_out_file);

    return pdf_out_file.string();
}
