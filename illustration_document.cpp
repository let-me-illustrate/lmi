// Document class for illustrations.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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

#include "illustration_document.hpp"
#include "view_ex.tpp"

#include "alert.hpp"
#include "illustration_view.hpp"
#include "illustrator.hpp"              // default_cell()
#include "miscellany.hpp"
#include "wx_utility.hpp"

#include <fstream>

IMPLEMENT_DYNAMIC_CLASS(IllustrationDocument, wxDocument)

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
        doc_.input_data_ = default_cell();
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

/// Override wx's built-in file management: doc_ handles that.
///
/// Override DoOpenDocument() instead of OnOpenDocument(): the latter
/// doesn't permit customizing its diagnostic messages.

bool IllustrationDocument::DoOpenDocument(wxString const&)
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
