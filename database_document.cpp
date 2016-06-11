// Document class for Database dictionary.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "database_document.hpp"

#include "alert.hpp"

// EVGENIY !! Doesn't it seem strange that this wx header appears
// to be needed here? I don't see it included in similar files.
// I tried omitting it, but wasn't able to figure out what the
// diagnostics really meant.

#include <wx/defs.h>

#include <algorithm>                    // std::swap()

namespace
{

/// DBDictionary class is a singleton class, which does not allow more
/// than one instance. To load/save DBDictionary objects data from/to
/// multiple files at the same time one could workaround the singleton
/// constraint by:
///   - swapping DBDictionary::instance() internal data into temporary variable
///   - performing operation
///   - swapping data back into singleton
/// To ensure that in case of an exception, singleton's internal state
/// is restored a helper class swap_workaround_for_singleton is used.

class swap_workaround_for_singleton
{
  public:
    swap_workaround_for_singleton
        (std::map<std::string,database_entity>&
        ,DBDictionary&
        );
    ~swap_workaround_for_singleton();

  private:
    std::map<std::string,database_entity>& m1_;
    DBDictionary&                          m2_;
};

inline swap_workaround_for_singleton::swap_workaround_for_singleton
    (std::map<std::string,database_entity>& m1
    ,DBDictionary&                          m2
    )
    :m1_(m1)
    ,m2_(m2)
{
    DatabaseDocument::swap_kludge(m1_, m2_);
}

inline swap_workaround_for_singleton::~swap_workaround_for_singleton()
{
    DatabaseDocument::swap_kludge(m1_, m2_);
}

} // Unnamed namespace.

IMPLEMENT_DYNAMIC_CLASS(DatabaseDocument, ProductEditorDocument)

void DatabaseDocument::swap_kludge
    (std::map<std::string,database_entity>& m
    ,DBDictionary&                          d
    )
{
    typedef std::vector<std::string>::const_iterator svci;
    for(svci i = d.member_names().begin(); i != d.member_names().end(); ++i)
        {
        std::swap(m[*i], d.datum(*i));
        }
}

DatabaseDocument::DatabaseDocument()
    :ProductEditorDocument()
    ,dict_()
{
    DBDictionary& instance = DBDictionary::instance();
    swap_workaround_for_singleton workaround(dict_, instance);
    instance.InitDB();
}

DatabaseDocument::~DatabaseDocument()
{
}

database_entity& DatabaseDocument::GetTDBValue(e_database_key index)
{
    std::string const& s = db_name_from_key(index);
    if(dict_.find(s) == dict_.end())
        {
        // A dummy entity ought to be good enough for non-leaf treenodes.
        static database_entity dummy;
        return dummy;
        }
    else
        {
        return dict_[s];
        }
}

void DatabaseDocument::ReadDocument(std::string const& filename)
{
    DBDictionary& instance = DBDictionary::instance();
    swap_workaround_for_singleton workaround(dict_, instance);
    DBDictionary::InvalidateCache();
    instance.Init(filename);
}

void DatabaseDocument::WriteDocument(std::string const& filename)
{
    DBDictionary& instance = DBDictionary::instance();
    swap_workaround_for_singleton workaround(dict_, instance);
    instance.WriteDB(filename);
}

