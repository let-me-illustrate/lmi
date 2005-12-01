// Document class for illustrations.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: illustration_document.cpp,v 1.4 2005-12-01 04:06:34 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "illustration_document.hpp"

#include "alert.hpp"
#include "illustration_view.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(IllustrationDocument, wxDocument)

IllustrationDocument::IllustrationDocument()
    :is_phony_(false)
{
}

IllustrationDocument::~IllustrationDocument()
{
}

// Somewhat naively, assume that the first view of the appropriate
// type is the one that contains the authoritative data.
// TODO ?? Shouldn't MVC take care of that, if used correctly?
//
IllustrationView& IllustrationDocument::DominantView() const
{
    IllustrationView* view = 0;
    wxList const& vl = GetViews();
    for(wxList::const_iterator i = vl.begin(); i != vl.end(); ++i)
        {
        wxObject* p = *i;
        LMI_ASSERT(0 != p);
        if(p->IsKindOf(CLASSINFO(IllustrationView)))
            {
            view = dynamic_cast<IllustrationView*>(p);
            break;
            }
        }
    if(!view)
        {
        fatal_error() << "Illustration view not found." << LMI_FLUSH;
        }
    return *view;
}

// INELEGANT !! This wants to be a template in a base class or at
// least in a separate header.
//
wxHtmlWindow& IllustrationDocument::DominantViewWindow() const
{
    IllustrationView const& view = DominantView();
    if(!view.html_window_)
        {
        fatal_error() << "Illustration window not found." << LMI_FLUSH;
        }
    return *view.html_window_;
}

// TODO ?? Revise:
// IllustrationDocument::OnOpenDocument() has not yet been called,
// Defer calling base-class virtual.

// Class IllustrationView overloads wxView::OnCreate() to display a
// dialog that allow input parameters to be edited before the view is
// shown. Canceling that dialog deliberately prevents that view from
// being created: it is not tasteful to show a blank view window
// underneath the dialog and destroy it on wxID_CANCEL.
//
// But wxDocManager::CreateDocument() calls wxDocument::OnCreate()
// (which calls wxView::OnCreate() to create a view) before it calls
// wxDocument::OnOpenDocument() (which, notionally at least, reads the
// document's data). Yet here it is required to read the document's
// data before the view is created.
//
// Resolution: Read document data in IllustrationDocument::OnCreate()
// instead of in IllustrationDocument::OnOpenDocument(). Invoke
// wxView::OnCreate() from IllustrationView::OnCreate() only when the
// initial dialog is not canceled.
//
// Alternative not used: Because wxDocument::OnCreate() simply creates
// a view and does nothing else, another resolution is to call that
// base-class function not from IllustrationDocument::OnCreate() but
// rather from IllustrationDocument::OnOpenDocument(). That is
// rejected for two reasons: it seems unnatural; and, far worse, it
// wreaks havoc on wx's object management--for instance, the document
// destroys itself when its last view ceases to exist, and much labor
// is required to prevent memory leaks or segfaults in that case.
//
bool IllustrationDocument::OnCreate(wxString const& filename, long int flags)
{
// TODO ?? Why not offer doc_.read(filename)?
// TODO ?? Would a vector<char> help?

    if(LMI_WX_CHILD_DOCUMENT & flags)
        {
        is_phony_ = true;
        }

    if(!(wxDOC_NEW & flags) && !(is_phony_))
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
    convert_from_ihs(doc_.input_data(), input_);

    return wxDocument::OnCreate(filename, flags);
}

// WX !! Oddly, wxDocument::OnNewDocument() calls OnSaveModified().
// That would seem appropriate if an existing document were being
// overlaid, but the function is designed to create an entirely new
// document. The apparent design flaw manifests itself here because
// this class sets the dirty flag earlier. wxDocument::OnNewDocument()
// also clears the dirty flag, but it seems more sensible to set it.
// For these and perhaps other reasons, the base-class function must
// not be called here.
//
bool IllustrationDocument::OnNewDocument()
{
    Modify(true);
    SetDocumentSaved(false);

    wxString name;
    GetDocumentManager()->MakeDefaultName(name);
    SetTitle(name);
    SetFilename(name, true);

    return true;
}

// Do not call the base-class function--storage is managed in this
// derived class, but in a different function; and the base-class
// function would clear the 'modified' flag, which would be incorrect
// here.
//
bool IllustrationDocument::OnOpenDocument(wxString const& filename)
{
    SetFilename(filename, true);
    // WX !! This necessary function is undocumented. It must be
    // called here; otherwise, wxView::OnSave() pops up an annoying
    // 'Save As' dialog downstream when the file loaded here is saved.
    SetDocumentSaved();
    UpdateAllViews();
    return true;
}

bool IllustrationDocument::OnSaveDocument(wxString const& filename)
{
    if(is_phony_)
        {
        warning() << "This document cannot be saved." << LMI_FLUSH;
        return false;
        }

    convert_to_ihs(*doc_.input_data_, input_);
    std::ofstream ofs(filename.c_str());
    doc_.write(ofs);
    Modify(false);
    status() << "Document saved.";
    return true;
}

