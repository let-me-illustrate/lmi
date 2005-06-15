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

// $Id: illustration_document.hpp,v 1.2 2005-06-15 05:05:04 chicares Exp $

// Because illustration windows have their own functions for loading
// and saving files, override OnOpenDocument() and OnSaveDocument()
// instead of LoadObject() and SaveObject().

#ifndef illustration_document_hpp
#define illustration_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "single_cell_document.hpp"

#include <boost/utility.hpp>

#include <wx/docview.h>

/// WX !! The wx document-view implementation has no notion of 'child'
/// documents, but sometimes lmi creates a document that logically is
/// a 'child' of a parent CensusDocument: it corresponds to no actual,
/// distinct document, can't be opened or saved separately, and should
/// be closed, along with all its views, when its parent closes; and,
/// accordingly, it should never be added to any wxFileHistory. This
/// set of behaviors is implemented here by implicitly defining a new
/// document-creation flag, appropriating an unused bit in the flags
/// word. This is brittle, but then again it seems unlikely that
/// anyone will change this aspect of wx.

enum {LMI_WX_CHILD_DOCUMENT = 8};

class WXDLLEXPORT wxHtmlWindow;

class IllustrationDocument
    :public wxDocument
    ,private boost::noncopyable
{
    friend class IllustrationView;

  public:
    IllustrationDocument();
    virtual ~IllustrationDocument();

  private:
    wxHtmlWindow* DominantViewWindow() const;

    // wxDocument overrides.
    virtual bool OnCreate(wxString const& filename, long int flags);
    virtual bool OnNewDocument();
    virtual bool OnOpenDocument(wxString const& filename);
    virtual bool OnSaveDocument(wxString const& filename);

    // TODO ?? Consider using smart pointers, and forward-declaring
    // classes single_cell_document and Input.

    single_cell_document doc_;

    Input input_;

    bool is_phony_;

    DECLARE_DYNAMIC_CLASS(IllustrationDocument)
};

#endif // illustration_document_hpp

