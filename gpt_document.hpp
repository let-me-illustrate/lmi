// Document class for guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef gpt_document_hpp
#define gpt_document_hpp

#include "config.hpp"

#include "gpt_input.hpp"
#include "gpt_xml_document.hpp"

#include <wx/docview.h>

class gpt_view;
class WXDLLIMPEXP_FWD_CORE wxHtmlWindow;

class gpt_document
    :public wxDocument
{
    friend class gpt_view;

  public:
    gpt_document() = default;
    ~gpt_document() override = default;

    gpt_view& PredominantView() const;

  private:
    gpt_document(gpt_document const&) = delete;
    gpt_document& operator=(gpt_document const&) = delete;

    wxHtmlWindow& PredominantViewWindow() const;

    // wxDocument overrides.
    bool OnCreate(wxString const& filename, long int flags) override;
    bool DoOpenDocument(wxString const& filename) override;
    bool DoSaveDocument(wxString const& filename) override;

    gpt_xml_document doc_;

    DECLARE_DYNAMIC_CLASS(gpt_document)
};

#endif // gpt_document_hpp
