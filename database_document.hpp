// Document class for Database dictionary.
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

#ifndef database_document_hpp
#define database_document_hpp

#include "config.hpp"

#include "product_editor.hpp"

#include "dbdict.hpp"
#include "dbnames.hpp"

class LMI_SO database_entity;

class DatabaseDocument
    :public ProductEditorDocument
{
  public:
    DatabaseDocument();
    virtual ~DatabaseDocument();

    database_entity& GetTDBValue(e_database_key index);

  private:
    // ProductEditorDocument overrides.
    virtual void ReadDocument (std::string const& filename);
    virtual void WriteDocument(std::string const& filename);

    dict_map dict_;

    DECLARE_DYNAMIC_CLASS(DatabaseDocument)
};

#endif // database_document_hpp

