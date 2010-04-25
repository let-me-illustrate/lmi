// Document class for product data.
//
// Copyright (C) 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef policy_document_hpp
#define policy_document_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include "product_data.hpp"

#include <map>
#include <string>

class PolicyView;

class PolicyDocument
    :public ProductEditorDocument
{
  public:
    PolicyDocument();
    virtual ~PolicyDocument();

    typedef std::map<std::string, std::string*> values_type;
    values_type&       values();
    values_type const& values() const;

  private:
    // ProductEditorDocument overrides.
    virtual void ReadDocument (std::string const& filename);
    virtual void WriteDocument(std::string const& filename);

    PolicyView& PredominantView() const;

    product_data product_data_;

    values_type values_;

    DECLARE_DYNAMIC_CLASS(PolicyDocument)
};

inline PolicyDocument::values_type& PolicyDocument::values()
{
    return values_;
}

inline PolicyDocument::values_type const& PolicyDocument::values() const
{
    return values_;
}

#endif // policy_document_hpp

