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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "database_document.hpp"

#include "alert.hpp"

// EVGENIY !! Doesn't it seem strange that this wx header appears
// to be needed here? I don't see it included in similar files.
// I tried omitting it, but wasn't able to figure out what the
// diagnostics really meant.

#include <wx/defs.h>

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
    swap_workaround_for_singleton(dict_map& m1, dict_map& m2);
    ~swap_workaround_for_singleton();

  private:
    dict_map& m1_;
    dict_map& m2_;
};

inline swap_workaround_for_singleton::swap_workaround_for_singleton
    (dict_map& m1
    ,dict_map& m2
    )
    :m1_(m1)
    ,m2_(m2)
{
    m1_.swap(m2_); // initially swap
}
inline swap_workaround_for_singleton::~swap_workaround_for_singleton()
{
    m1_.swap(m2_); // swap back
}

} // unnamed namespace

IMPLEMENT_DYNAMIC_CLASS(DatabaseDocument, ProductEditorDocument)

DatabaseDocument::DatabaseDocument()
    :ProductEditorDocument()
    ,dict_()
{
    // Initialize database dictionary
    DBDictionary& instance = DBDictionary::instance();

    swap_workaround_for_singleton workaround(dict_, instance.GetDictionary());

    instance.InitDB();
}

DatabaseDocument::~DatabaseDocument()
{
}

TDBValue& DatabaseDocument::GetTDBValue(DatabaseNames index)
{
    if(dict_.find(index) == dict_.end())
        {
        fatal_error() << "Index out of bounds." << LMI_FLUSH;
        }

    return dict_[index];
}

void DatabaseDocument::ReadDocument(std::string const& filename)
{
    DBDictionary& instance = DBDictionary::instance();

    swap_workaround_for_singleton workaround(dict_, instance.GetDictionary());

    DBDictionary::InvalidateCache();
    instance.Init(filename);
}

void DatabaseDocument::WriteDocument(std::string const& filename)
{
    DBDictionary& instance = DBDictionary::instance();

    swap_workaround_for_singleton workaround(dict_, instance.GetDictionary());

    instance.WriteDB(filename);
}

