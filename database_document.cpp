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

// $Id: database_document.cpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

#include "database_document.hpp"

#include "alert.hpp"
#include "database_view.hpp"
#include "database_view_editor.hpp"
#include "ihs_dbvalue.hpp"
#include "ihs_dbdict.hpp"
#include "xenumtypes.hpp"

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
/// is restored a helper class dict_map_swap_guard is used.
/// Used in DatabaseDocument::OnOpenDocument, DatabaseDocument::OnSaveDocument

class dict_map_swap_guard
{
  public:
    dict_map_swap_guard(dict_map& m1, dict_map& m2);
    ~dict_map_swap_guard();

  private:
    dict_map& m1_;
    dict_map& m2_;
};

inline dict_map_swap_guard::dict_map_swap_guard
    (dict_map& m1
    ,dict_map& m2
    )
    :m1_(m1)
    ,m2_(m2)
{
}
inline dict_map_swap_guard::~dict_map_swap_guard()
{
    m1_.swap(m2_);
}

} // unnamed namespace


IMPLEMENT_DYNAMIC_CLASS(DatabaseDocument, ProductEditorDocument)

DatabaseDocument::DatabaseDocument()
    :ProductEditorDocument()
    ,dict_()
{
    // Initialize database dictionary
    DBDictionary& instance = DBDictionary::instance();

    // double-swap workaround for the singleton
    dict_.swap(instance.GetDictionary());
    dict_map_swap_guard guard(dict_, instance.GetDictionary());

    instance.InitDB();
}

DatabaseDocument::~DatabaseDocument()
{
}

TDBValue* DatabaseDocument::GetTDBValue(std::size_t index)
{
    if(dict_.find(index) == dict_.end())
        fatal_error() << "index out of bounds" << LMI_FLUSH;
    return &dict_[index];
}

void DatabaseDocument::ReadDocument(wxString const& filename)
{
    DBDictionary& instance = DBDictionary::instance();

    // double-swap workaround for the singleton
    dict_.swap(instance.GetDictionary());
    dict_map_swap_guard guard(dict_, instance.GetDictionary());

    std::string const old_cached_filename = DBDictionary::CachedFilename;
    instance.Init(filename);
    DBDictionary::CachedFilename = old_cached_filename;
}

void DatabaseDocument::WriteDocument(wxString const& filename)
{
    DBDictionary& instance = DBDictionary::instance();

    // double-swap workaround for the singleton
    dict_.swap(instance.GetDictionary());
    dict_map_swap_guard guard(dict_, instance.GetDictionary());

    instance.WriteDB(filename);
}

