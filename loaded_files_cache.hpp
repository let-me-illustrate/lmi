// Caching of loaded files.
//
// Copyright (C) 2012 Gregory W. Chicares.
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

#ifndef loaded_files_cache_hpp
#define loaded_files_cache_hpp

#include "config.hpp"

#include "alert.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

/// Cache of objects loaded from files.
///
/// Used for holding objects that are expensively loaded from a file in memory.
/// For example, loading actuarial_table from an XML file is costly, so we keep
/// loaded instances in memory until the program terminates or until the file
/// is modified.
///
/// T must have a constructor that takes a single string argument, the
/// filename, as well as the default constructor.
template<typename T>
class loaded_files_cache
    :private lmi::uncopyable< loaded_files_cache<T> >
{
  public:
    typedef T value_type;
    typedef boost::shared_ptr<value_type const> value_ptr_type;
    typedef std::string key_type;

    // Get object corresponding to the given file, either getting it from the
    // cache or loading from the file. Checks file's timestamp and reloads it
    // if it was modified since the cached copy was read.
    value_ptr_type get(key_type const& filename)
        {
        try
            {
            std::time_t const write_time = fs::last_write_time(filename);

            typename map_type::iterator i = cache_.lower_bound(filename);
            if
                (  i != cache_.end()
                && i->first == filename
                && write_time == i->second.write_time
                )
                {
                return i->second.data;
                }

            // create the value before calling insert() because the call may throw
            value_ptr_type value(new value_type(filename));

            // insert() doesn't update the value if the key is already present, so
            // insert a dummy value and then modify it -- this will work for both
            // existing and new keys.
            i = cache_.insert(i, typename map_type::value_type(filename, record()));
            record& rec = i->second;
            rec.write_time = write_time;
            rec.data = value;

            return value;
            }
        catch(fs::filesystem_error const&)
            {
            fatal_error()
                << "File '"
                << filename
                << "' is required but could not be found. Try reinstalling."
                << LMI_FLUSH
                ;
            }
        }

    // Return singleton instance of the cache.
    static loaded_files_cache& instance()
        {
        static loaded_files_cache inst;
        return inst;
        }

  private:
    struct record
    {
        std::time_t    write_time;
        value_ptr_type data;
    };

    typedef std::map<key_type, record> map_type;
    map_type cache_;
};

/// Base class for cached types.
/// Its only purpose is providing convenience get_cached() function.
template<typename T>
class loaded_from_cache
{
  public:
    typedef loaded_files_cache<T> cache_type;
    typedef typename cache_type::value_ptr_type ptr_type;

    static ptr_type get_cached(std::string const& filename)
        {
        return cache_type::instance().get(filename);
        }
};

#endif // loaded_files_cache_hpp
