// Product database map.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#include "database.hpp"
#include "dbvalue.hpp"
#include "obstruct_slicing.hpp"
#include "so_attributes.hpp"

#include <boost/utility.hpp>

#include <map>
#include <string>

typedef std::map<int, TDBValue, std::less<int> > TDBDictionary;
typedef TDBDictionary::value_type TDBEntry;

class DBDictionary
    :private boost::noncopyable
    ,virtual private obstruct_slicing<DBDictionary>
{
  public:
    static DBDictionary& instance();
    ~DBDictionary();
    void Init(std::string const& NewFilename);
    TDBEntry* Find(TDBEntry const& t);
    TDBDictionary const& GetDictionary();

  private:
    DBDictionary();

    static std::string CachedFilename;
    void Init();

    void AddEntry(TDBEntry const& e);

    TDBDictionary* dictionary;
};

inline TDBDictionary const& DBDictionary::GetDictionary()
{return *dictionary;}

void LMI_SO print_databases();

#endif // dbdict_hpp

