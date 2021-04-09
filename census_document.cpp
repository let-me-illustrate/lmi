// Document class for censuses.
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

#include "pchfile_wx.hpp"

#include "census_document.hpp"
#include "view_ex.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "census_view.hpp"
#include "illustrator.hpp"              // default_cell()
#include "miscellany.hpp"
#include "wx_utility.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(CensusDocument, wxDocument)

wxGrid& CensusDocument::PredominantViewWindow() const
{
    return ::PredominantViewWindow<CensusView,wxGrid>
        (*this
        ,&CensusView::grid_window_
        );
}

bool CensusDocument::OnCreate(wxString const& filename, long int flags)
{
    if(wxDOC_NEW & flags)
        {
        LMI_ASSERT(1 == doc_.case_parms_ .size());
        LMI_ASSERT(1 == doc_.cell_parms_ .size());
        LMI_ASSERT(1 == doc_.class_parms_.size());

        doc_.case_parms_ [0] = default_cell();
        doc_.cell_parms_ [0] = default_cell();
        doc_.class_parms_[0] = default_cell();
        }
    else
        {
        std::string f = ValidateAndConvertFilename(filename);
        std::ifstream ifs(f.c_str());
        if(!ifs)
            {
            warning()
                << "Unable to read file '"
                << filename
                << "'."
                << LMI_FLUSH
                ;
            return false;
            }
        doc_.read(ifs);
        }

    return wxDocument::OnCreate(filename, flags);
}

/// See documentation for IllustrationDocument::DoOpenDocument().

bool CensusDocument::DoOpenDocument(wxString const&)
{
    return true;
}

/// See documentation for IllustrationDocument::DoSaveDocument().

bool CensusDocument::DoSaveDocument(wxString const& filename)
{
    std::string f = ValidateAndConvertFilename(filename);
    std::ofstream ofs(f.c_str(), ios_out_trunc_binary());
    doc_.write(ofs);
    if(!ofs)
        {
        warning() << "Unable to save '" << filename << "'." << LMI_FLUSH;
        return false;
        }

    status() << "Saved '" << filename << "'." << std::flush;
    return true;
}
