// Database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2005 Gregory W. Chicares.
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

// $Id: ihs_dbdict.hpp,v 1.2 2005-02-14 04:35:18 chicares Exp $

#ifndef ihs_dbdict_hpp
#define ihs_dbdict_hpp

#ifdef dbdict_hpp
#   error Probable lmi/ihs header conflict.
#endif // dbdict_hpp

#include "config.hpp"

#include "expimp.hpp"
#include "ihs_dbvalue.hpp"      // needed here for map
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <map>
#include <string>

typedef std::map<int, TDBValue, std::less<int> > dict_map;
typedef dict_map::value_type dict_map_val;

class LMI_EXPIMP DBDictionary
    :private boost::noncopyable
    ,virtual private obstruct_slicing<DBDictionary>
{
    friend class DatabaseDocument;

  public:
    static DBDictionary& instance();
    ~DBDictionary();
    void Init(std::string const& NewFilename);
// TODO ?? This should be const.
    dict_map /*const*/& GetDictionary(){return dictionary;}
    void WriteSampleDBFile();   // A sample product you can play with.
    static void WriteDBFiles();
    void WriteProprietaryDBFiles();

  private:
    DBDictionary();

    void WriteDB(std::string const& filename);
    void Add(TDBValue const& e);
    void BadFile(std::string const& Filename, std::string const& why);
    void InitDB();  // TODO ?? A KLUDGE.

    static std::string CachedFilename;
    dict_map dictionary;
};

#endif // ihs_dbdict_hpp

