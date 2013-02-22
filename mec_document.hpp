// Document class for MEC testing.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013 Gregory W. Chicares.
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

#ifndef mec_document_hpp
#define mec_document_hpp

#include "config.hpp"

#include "mec_input.hpp"
#include "mec_xml_document.hpp"
#include "uncopyable_lmi.hpp"

#include <wx/docview.h>

class mec_view;
class WXDLLIMPEXP_FWD_CORE wxHtmlWindow;

class mec_document
    :public  wxDocument
    ,private lmi::uncopyable<mec_document>
{
    friend class mec_view;

  public:
    mec_document();
    virtual ~mec_document();

    mec_view& PredominantView() const;

  private:
    wxHtmlWindow& PredominantViewWindow() const;

    // wxDocument overrides.
    virtual bool OnCreate(wxString const& filename, long int flags);
#if !wxCHECK_VERSION(2,9,0)
    virtual bool OnNewDocument();
#endif // !wxCHECK_VERSION(2,9,0)
    virtual bool DoOpenDocument(wxString const& filename);
    virtual bool DoSaveDocument(wxString const& filename);

    mec_xml_document doc_;

    DECLARE_DYNAMIC_CLASS(mec_document)
};

#endif // mec_document_hpp

