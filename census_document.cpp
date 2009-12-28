// Document class for censuses.
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

// $Id: census_document.cpp,v 1.20 2009-03-22 17:32:24 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "census_document.hpp"
#include "view_ex.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "census_view.hpp"
#include "illustrator.hpp"
#include "miscellany.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(CensusDocument, wxDocument)

CensusDocument::CensusDocument()
    :wxDocument()
{
}

CensusDocument::~CensusDocument()
{
}

wxListView& CensusDocument::PredominantViewWindow() const
{
    return ::PredominantViewWindow<CensusView,wxListView>
        (*this
        ,&CensusView::list_window_
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
        std::ifstream ifs(filename.mb_str());
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

#if !wxCHECK_VERSION(2,9,0)
bool CensusDocument::OnNewDocument()
{
    Modify(true);
    SetDocumentSaved(false);

#if wxCHECK_VERSION(2,8,8)
    wxString const name = GetDocumentManager()->MakeNewDocumentName();
#else  // !wxCHECK_VERSION(2,8,8)
    wxString name;
    GetDocumentManager()->MakeDefaultName(name);
#endif // !wxCHECK_VERSION(2,8,8)
    SetTitle(name);
    SetFilename(name, true);

    return true;
}
#endif // !wxCHECK_VERSION(2,9,0)

/// See documentation for IllustrationDocument::DoOpenDocument().

bool CensusDocument::DoOpenDocument(wxString const& filename)
{
    return true;
}

/// See documentation for IllustrationDocument::DoSaveDocument().

bool CensusDocument::DoSaveDocument(wxString const& filename)
{
    std::ofstream ofs(filename.mb_str(), ios_out_trunc_binary());
    doc_.write(ofs);
    if(!ofs)
        {
        warning() << "Unable to save '" << filename << "'." << LMI_FLUSH;
        return false;
        }

    status() << "Saved '" << filename << "'." << std::flush;
    return true;
}

