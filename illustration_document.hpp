// Document class for illustrations.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

// $Id$

#ifndef illustration_document_hpp
#define illustration_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "single_cell_document.hpp"
#include "uncopyable_lmi.hpp"

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

class IllustrationView;
class WXDLLIMPEXP_FWD_CORE wxHtmlWindow;

class IllustrationDocument
    :public  wxDocument
    ,private lmi::uncopyable<IllustrationDocument>
{
    friend class IllustrationView;

  public:
    IllustrationDocument();
    virtual ~IllustrationDocument();

    IllustrationView& PredominantView() const;

  private:
    wxHtmlWindow& PredominantViewWindow() const;

    // wxDocument overrides.
    virtual bool OnCreate(wxString const& filename, long int flags);
#if !wxCHECK_VERSION(2,9,0)
    virtual bool OnNewDocument();
#endif // !wxCHECK_VERSION(2,9,0)
    virtual bool DoOpenDocument(wxString const& filename);
    virtual bool DoSaveDocument(wxString const& filename);

    single_cell_document doc_;

    bool is_phony_;

    DECLARE_DYNAMIC_CLASS(IllustrationDocument)
};

#endif // illustration_document_hpp

