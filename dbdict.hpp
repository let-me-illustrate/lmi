// Product-database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef dbdict_hpp
#define dbdict_hpp

#include "config.hpp"

#include "dbvalue.hpp" // Needed here for map declaration.
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <map>
#include <string>

typedef std::map<int, database_entity> dict_map;

/// Cached product database.

class LMI_SO DBDictionary
    :private boost::noncopyable
    ,virtual private obstruct_slicing<DBDictionary>
{
    friend class DatabaseDocument;
    friend class input_test;
    friend class product_file_test;

  public:
    static DBDictionary& instance();
    ~DBDictionary();

    dict_map const& GetDictionary() const;

    void Init(std::string const& filename);
    void WriteSampleDBFile();
    void WriteProprietaryDBFiles();

    void InitAntediluvian();

  private:
    DBDictionary();

    void WriteDB(std::string const& filename);
    void Add(database_entity const&);
    void InitDB();

    static void InvalidateCache();

    static std::string cached_filename_;

    dict_map dictionary_;
};

void LMI_SO print_databases();

#endif // dbdict_hpp

