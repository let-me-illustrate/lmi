// Document class for Database dictionary.
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

// $Id: database_document.hpp,v 1.3 2007-03-03 19:44:17 chicares Exp $

#ifndef database_document_hpp
#define database_document_hpp

#include "config.hpp"

#include "ihs_dbvalue.hpp"
#include "ihs_dbdict.hpp"
#include "product_editor.hpp"
#include "xenumtypes.hpp"

#include <boost/utility.hpp>

#include <wx/defs.h>

class DatabaseDocument
    :public ProductEditorDocument
{
  public:
    DatabaseDocument();
    virtual ~DatabaseDocument();

    TDBValue* GetTDBValue(std::size_t index);

  private:
    // ProductEditorDocument overrides.
    virtual void ReadDocument (std::string const& filename);
    virtual void WriteDocument(std::string const& filename);

    dict_map dict_;

    DECLARE_DYNAMIC_CLASS(DatabaseDocument)
};

#endif // database_document_hpp

