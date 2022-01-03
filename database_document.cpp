// Document class for Database dictionary.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "database_document.hpp"

#include "alert.hpp"
#include "contains.hpp"

IMPLEMENT_DYNAMIC_CLASS(DatabaseDocument, ProductEditorDocument)

DatabaseDocument::DatabaseDocument()
    :ProductEditorDocument {}
{
    db_.InitDB();
}

database_entity& DatabaseDocument::GetTDBValue(e_database_key index)
{
    std::string const& s = db_name_from_key(index);
    if(contains(db_.member_names(), s))
        {
        return db_.datum(s);
        }
    else
        {
        // A dummy entity ought to be good enough for non-leaf treenodes.
        static database_entity dummy;
        return dummy;
        }
}

void DatabaseDocument::ReadDocument(std::string const& filename)
{
    db_.Init(filename);
}

void DatabaseDocument::WriteDocument(std::string const& filename)
{
    db_.WriteDB(filename);
}
