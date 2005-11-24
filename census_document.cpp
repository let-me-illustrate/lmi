// Document class for censuses.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: census_document.cpp,v 1.3 2005-11-24 05:22:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "census_document.hpp"

#include "alert.hpp"
#include "census_view.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(CensusDocument, wxDocument)

CensusDocument::CensusDocument()
    :wxDocument()
{
}

CensusDocument::~CensusDocument()
{
}

// INELEGANT !! This wants to be a template in a base class or at
// least in a separate header.
//
// Somewhat naively, assume that the first view of the appropriate
// type is the one that contains the authoritative data.
// TODO ?? Shouldn't MVC take care of that, if used correctly?
//
wxListView* CensusDocument::DominantViewWindow() const
{
    CensusView* view = 0;
    while(wxList::compatibility_iterator node = GetViews().GetFirst())
        {
        if(node->GetData()->IsKindOf(CLASSINFO(CensusView)))
            {
            view = dynamic_cast<CensusView*>(node->GetData());
            break;
            }
        node = node->GetNext();
        }
    if(!view)
        {
        fatal_error() << "Census view not found." << LMI_FLUSH;
        }
    if(!view->list_window_)
        {
        fatal_error() << "Census window not found." << LMI_FLUSH;
        }
    return view->list_window_;
}

bool CensusDocument::OnCreate(wxString const& filename, long int flags)
{
// TODO ?? Why not offer doc_.read(filename)?
// TODO ?? Would a vector<char> be faster?

    if(!(wxDOC_NEW & flags))
        {
        std::ifstream ifs(filename.c_str());
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

    convert_from_ihs(doc_.case_parms_ , case_parms_ );
    convert_from_ihs(doc_.cell_parms_ , cell_parms_ );
    convert_from_ihs(doc_.class_parms_, class_parms_);

    return wxDocument::OnCreate(filename, flags);
}

bool CensusDocument::OnNewDocument()
{
    Modify(true);
    SetDocumentSaved(false);

    wxString name;
    GetDocumentManager()->MakeDefaultName(name);
    SetTitle(name);
    SetFilename(name, true);

    return true;
}

bool CensusDocument::OnOpenDocument(wxString const& filename)
{
    SetFilename(filename, true);
    // WX !! This necessary function is undocumented. It must be
    // called here; otherwise, wxView::OnSave() pops up an annoying
    // 'Save As' dialog downstream when the file loaded here is saved.
    SetDocumentSaved();
//    Modify(false); // TODO ?? Is this merely pleonastic?
    UpdateAllViews();
    return true;
}

bool CensusDocument::OnSaveDocument(wxString const& filename)
{
    convert_to_ihs(doc_.case_parms_ , case_parms_ );
    convert_to_ihs(doc_.cell_parms_ , cell_parms_ );
    convert_to_ihs(doc_.class_parms_, class_parms_);

    std::ofstream ofs(filename.c_str());
    doc_.write(ofs);

    Modify(false);
    status() << "Document saved." << std::flush;
    return true;
}

