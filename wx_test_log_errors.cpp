// Test that wxLog error messages are shown to the user.
//
// Copyright (C) 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "assert_lmi.hpp"
#include "wx_test_case.hpp"

#include <wx/docview.h>
#include <wx/log.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

LMI_WX_TEST_CASE(log_error)
{
    wxDocManager* const doc_manager = wxDocManager::GetDocumentManager();
    LMI_ASSERT(doc_manager);

    // Silently opening a file without extension must fail as the doc/view
    // framework has no way to find the template associated with it.
    doc_manager->CreateDocument("file_without_extension", wxDOC_SILENT);

    wxTEST_DIALOG
        (wxLog::FlushActive()
        ,wxExpectModal<wxMessageDialog>(wxOK).
            Describe("warning about being unable to determine file format")
        );
}
