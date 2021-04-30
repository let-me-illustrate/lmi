// Ledger PDF generation.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#include "pchfile.hpp"

#include "ledger_pdf.hpp"

#include "configurable_settings.hpp"
#include "ledger.hpp"
#include "path_utility.hpp"             // unique_filepath()
#include "pdf_command.hpp"

/// Write a scaled copy of the ledger to a PDF file.
///
/// PDF !! Does the following block comment actually apply here? and
/// is it time to address the comment about shared_ptr members in
/// class ledger?
///
/// The original ledger must not be modified because scaling is not
/// reentrant. However, copying does not prevent that problem here,
/// because what is scaled is actually not copied due to use of
/// shared_ptr; see comment on
///   https://savannah.nongnu.org/bugs/index.php?13599
/// in the ledger-class implementation.

std::string write_ledger_as_pdf(Ledger const& ledger, fs::path const& filepath)
{
    throw_if_interdicted(ledger);

    fs::path print_dir(configurable_settings::instance().print_directory());
    // PDF !! Either portable_filename() should be used here, or its
    // use should be reconsidered everywhere else.
    fs::path pdf_out_file = unique_filepath(print_dir / filepath, ".pdf");

    Ledger scaled_ledger(ledger);
    scaled_ledger.AutoScale();
    pdf_command(scaled_ledger, pdf_out_file);

    return pdf_out_file.string();
}
