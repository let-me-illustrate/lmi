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

// $Id: census_document.hpp,v 1.4 2005-12-02 22:44:22 chicares Exp $

// WX !! Override OnOpenDocument() and OnSaveDocument() instead of
// LoadObject() and SaveObject(): the latter would probably work, but
// their diagnostic messages aren't customizable.

#ifndef census_document_hpp
#define census_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "multiple_cell_document.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT
#include <wx/docview.h>

class WXDLLEXPORT wxListView;

// TODO ?? This class and the illustration-document class have much in
// common that should be factored into a base class.

class CensusDocument
    :public wxDocument
    ,private boost::noncopyable
{
    friend class CensusView;

  public:
    CensusDocument();
    virtual ~CensusDocument();

  private:
    wxListView& PredominantViewWindow() const;

    // wxDocument overrides.
    virtual bool OnCreate(wxString const& filename, long int flags);
    virtual bool OnNewDocument();
    virtual bool OnOpenDocument(wxString const& filename);
    virtual bool OnSaveDocument(wxString const& filename);

    multiple_cell_document doc_;

    std::vector<Input> case_parms_;
    std::vector<Input> cell_parms_;
    std::vector<Input> class_parms_;

    DECLARE_DYNAMIC_CLASS(CensusDocument)
};

#endif // census_document_hpp

