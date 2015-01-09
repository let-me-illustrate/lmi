// Document class for Rounding rules.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifndef rounding_document_hpp
#define rounding_document_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include "rounding_rules.hpp"

#include <map>
#include <string>

class RoundingView;

class RoundingDocument
    :public ProductEditorDocument
{
  public:
    RoundingDocument();
    virtual ~RoundingDocument();

    typedef std::map<std::string, rounding_parameters*> values_type;
    values_type&       values();
    values_type const& values() const;

  private:
    // ProductEditorDocument overrides.
    virtual void ReadDocument (std::string const& filename);
    virtual void WriteDocument(std::string const& filename);

    RoundingView& PredominantView() const;

    rounding_rules rounding_rules_;

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

