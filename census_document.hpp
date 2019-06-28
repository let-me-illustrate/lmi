// Document class for censuses.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

#ifndef census_document_hpp
#define census_document_hpp

#include "config.hpp"

#include "input.hpp"
#include "multiple_cell_document.hpp"

#include <wx/docview.h>

class WXDLLIMPEXP_FWD_ADV wxGrid;

class CensusDocument
    :public wxDocument
{
    friend class CensusView;

  public:
    CensusDocument() = default;
    ~CensusDocument() override = default;

  private:
    CensusDocument(CensusDocument const&) = delete;
    CensusDocument& operator=(CensusDocument const&) = delete;

    wxGrid& PredominantViewWindow() const;

    // wxDocument overrides.
    bool OnCreate(wxString const& filename, long int flags) override;
    bool DoOpenDocument(wxString const& filename) override;
    bool DoSaveDocument(wxString const& filename) override;

    multiple_cell_document doc_;

    DECLARE_DYNAMIC_CLASS(CensusDocument)
};

#endif // census_document_hpp
