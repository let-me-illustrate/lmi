// Document class for censuses.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#ifndef census_document_hpp
#define census_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "multiple_cell_document.hpp"
#include "uncopyable_lmi.hpp"

#include <wx/docview.h>

class WXDLLIMPEXP_FWD_ADV wxDataViewCtrl;

class CensusDocument
    :public  wxDocument
    ,private lmi::uncopyable<CensusDocument>
{
    friend class CensusView;
    friend class CensusViewOld; // Obsolescent listview version.

  public:
    CensusDocument();
    virtual ~CensusDocument();

  private:
// Not used at present; restore when obsolescent class CensusView is expunged.
#if 0
    wxDataViewCtrl& PredominantViewWindow() const;
#endif // 0

    // wxDocument overrides.
    virtual bool OnCreate(wxString const& filename, long int flags);
#if !wxCHECK_VERSION(2,9,0)
    virtual bool OnNewDocument();
#endif // !wxCHECK_VERSION(2,9,0)
    virtual bool DoOpenDocument(wxString const& filename);
    virtual bool DoSaveDocument(wxString const& filename);

    multiple_cell_document doc_;

    DECLARE_DYNAMIC_CLASS(CensusDocument)
};

#endif // census_document_hpp

