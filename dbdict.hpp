// Product database map.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: dbdict.hpp,v 1.3 2005-04-10 14:54:35 chicares Exp $

#ifndef dbdict_hpp
#define dbdict_hpp

#include "config.hpp"

#include "database.hpp"
#include "dbvalue.hpp"
#include "obstruct_slicing.hpp"

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

#endif // dbdict_hpp

