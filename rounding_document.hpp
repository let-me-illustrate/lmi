// Document class for Rounding rules.
//
// Copyright (C) 2007 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: rounding_document.hpp,v 1.2 2007-02-21 03:07:24 chicares Exp $

#ifndef rounding_document_hpp
#define rounding_document_hpp

#include "config.hpp"

#include "ihs_rnddata.hpp"
#include "product_editor.hpp"

#include <wx/string.h>

#include <map>
#include <string>

class RoundingView;

class RoundingDocument
    :public ProductEditorDocument
{
  public:
    RoundingDocument();
    virtual ~RoundingDocument();

    typedef std::map<std::string, round_to<double>*> values_type;
    values_type&       values();
    values_type const& values() const;

  private:
    // ProductEditorDocument overrides.
    virtual void ReadDocument(wxString const& filename);
    virtual void WriteDocument(wxString const& filename);

    RoundingView& PredominantView() const;

    StreamableRoundingRules rounding_rules_;

    values_type values_;

    DECLARE_DYNAMIC_CLASS(RoundingDocument)
};

inline RoundingDocument::values_type& RoundingDocument::values()
{
    return values_;
}

inline RoundingDocument::values_type const& RoundingDocument::values() const
{
    return values_;
}

#endif // rounding_document_hpp

