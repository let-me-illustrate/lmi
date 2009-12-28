// Document class for illustrations.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: illustration_document.cpp,v 1.24 2009-03-13 16:21:30 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "illustration_document.hpp"
#include "view_ex.tpp"

#include "alert.hpp"
#include "illustration_view.hpp"
#include "illustrator.hpp"
#include "miscellany.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(IllustrationDocument, wxDocument)

IllustrationDocument::IllustrationDocument()
    :is_phony_(false)
{
}

IllustrationDocument::~IllustrationDocument()
{
}

IllustrationView& IllustrationDocument::PredominantView() const
{
    return ::PredominantView<IllustrationView>(*this);
}

wxHtmlWindow& IllustrationDocument::PredominantViewWindow() const
{
    return ::PredominantViewWindow<IllustrationView,wxHtmlWindow>
        (*this
        ,&IllustrationView::html_window_
        );
}

/// Class IllustrationView overloads wxView::OnCreate() to display a
/// dialog that allow input parameters to be edited before the view is
/// shown. Cancelling that dialog deliberately prevents that view from
/// being created: it is not tasteful to show a blank view window
/// underneath the dialog and destroy it on wxID_CANCEL.
///
/// But wxDocManager::CreateDocument() calls wxDocument::OnCreate()
/// (which calls wxView::OnCreate() to create a view) before it calls
/// wxDocument::DoOpenDocument() (where, by default, wx would read the
/// document's data). Yet here it is required to read the document's
/// data before the view is created.
///
/// Resolution: Read document data in IllustrationDocument::OnCreate()
/// instead of in IllustrationDocument::DoOpenDocument(). Invoke
/// wxView::OnCreate() from IllustrationView::OnCreate() only when the
/// initial dialog is not cancelled.
///
/// Alternative not used: Because wxDocument::OnCreate() only creates
/// a view and does nothing else, another resolution is to call that
/// base-class function not from IllustrationDocument::OnCreate() but
/// rather from IllustrationDocument::DoOpenDocument(). That is
/// rejected for two reasons: it seems unnatural; and, far worse, it
/// wreaks havoc on wx's object management--for instance, the document
/// destroys itself when its last view ceases to exist, and much labor
/// is required to prevent memory leaks or segfaults in that case.

bool IllustrationDocument::OnCreate(wxString const& filename, long int flags)
{
    if(LMI_WX_CHILD_DOCUMENT & flags)
        {
        is_phony_ = true;
        }
    else if(wxDOC_NEW & flags)
        {
        *doc_.input_data_ = default_cell();
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
/// Formerly, wxDocument::OnNewDocument() called OnSaveModified().
/// That would seem appropriate if an existing document were being
/// overlaid, but the function is designed to create an entirely new
/// document. It was a problem here because this class sets the dirty
/// flag earlier. wxDocument::OnNewDocument() also clears the dirty
/// flag, but it seems more sensible to set it. For these and perhaps
/// other reasons, the base-class function must not be called here.
///
/// Setting the dirty flag peremptorily here was intemperate. The
/// behavior with wx-2.9 or later is preferable: the dirty flag is set
/// iff any change is made, so that when no change is made...
///   File | New | Illustration
///   OK
///   File | Close
/// ...this messagebox...
///   "Do you want to save changes to unnamed1?"
/// is no longer presented.

bool IllustrationDocument::OnNewDocument()
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

/// Override wx's built-in file management: doc_ handles that.
///
/// Override DoOpenDocument() instead of OnOpenDocument(): the latter
/// doesn't permit customizing its diagnostic messages.

bool IllustrationDocument::DoOpenDocument(wxString const& filename)
{
    return true;
}

/// Override wx's built-in file management: doc_ handles that.
///
/// Override DoSaveDocument() instead of OnSaveDocument(): the latter
/// doesn't permit customizing its diagnostic messages.

bool IllustrationDocument::DoSaveDocument(wxString const& filename)
{
    if(is_phony_)
        {
        warning() << "Impossible to save '" << filename << "'." << LMI_FLUSH;
        return false;
        }

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

