// Cache class instances constructed from files.
//
// Copyright (C) 2016 Gregory W. Chicares.
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

#ifndef cache_file_reads_hpp
#define cache_file_reads_hpp

#include "config.hpp"

#include "uncopyable_lmi.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/shared_ptr.hpp>

#include <ctime>                        // std::time_t
#include <map>
#include <string>

/// Cache of class T instances constructed from files.
///
/// Motivation: It is costly to deserialize objects from xml, so cache
/// them for reuse. The cache persists until the program terminates.
///
/// Requires: T::T() and T::T(std::string const& filename).
///
/// For each filename, the cache stores one instance, which is
/// replaced by reloading the file if its write time has changed.
///
/// Instances are retrieved as shared_ptr<T const> so that they remain
/// valid even when the file changes. The client is responsible for
/// updating any stale pointers it holds.
///
/// Implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.

template<typename T>
class file_cache
    :private lmi::uncopyable<file_cache<T> >
{
  public:
    typedef T value_type;
    typedef boost::shared_ptr<value_type const> value_ptr_type;
    typedef std::string key_type;

    static file_cache<T>& instance()
        {
        static file_cache<T> z;
        return z;
        }

    value_ptr_type retrieve_or_reload(key_type const& filename)
        {
        // Throws if !exists(filename).
        std::time_t const write_time = fs::last_write_time(filename);

        typename map_type::iterator i = cache_.lower_bound(filename);
        if
            (  cache_.end() != i
            && filename     == i->first
            && write_time   == i->second.write_time
            )
            {
            return i->second.data;
            }

        // Construction may throw, so do it before updating the cache.
        value_ptr_type value(new value_type(filename));

        // insert() doesn't update the value if the key is already
        // present, so insert a dummy value and then modify it--this
        // works for both existing and new keys.
        i = cache_.insert(i, typename map_type::value_type(filename, record()));
        record& rec = i->second;
        rec.data = value;
        rec.write_time = write_time;

        return value;
        }

  private:
    struct record
    {
        value_ptr_type data;
        std::time_t    write_time;
    };

    typedef std::map<key_type,record> map_type;
    map_type cache_;
};

/// Mixin to cache parent instances constructed from files.
///
/// Implemented in terms of class file_cache (q.v.).

template<typename T>
class cache_file_reads
{
  public:
    typedef file_cache<T> cache_type;
    typedef typename cache_type::value_ptr_type ptr_type;

    static ptr_type read_from_cache(std::string const& filename)
        {
        return cache_type::instance().retrieve_or_reload(filename);
        }
};

#endif // cache_file_reads_hpp

