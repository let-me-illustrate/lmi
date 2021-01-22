// Cache class instances constructed from files.
//
// Copyright (C) 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef cache_file_reads_hpp
#define cache_file_reads_hpp

#include "config.hpp"

#include "assert_lmi.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <ctime>                        // time_t
#include <map>
#include <memory>                       // shared_ptr
#include <string>
#include <utility>                      // make_pair()

namespace detail
{
/// Cache of class T instances constructed from files.
///
/// Motivation: It is costly to deserialize objects from xml, so cache
/// them for reuse. The cache persists until the program terminates.
///
/// Requires: T::T(fs::path const& filename), though not T::T().
///
/// For each filename, the cache stores one instance, which is
/// replaced by reloading the file if its write time has changed.
///
/// Instances are retrieved as shared_ptr<T> so that they remain
/// valid even when the file changes. The client is responsible for
/// updating any stale pointers it holds. An earlier version returned
/// shared_ptr<T const> instead, but legitimate non-const use cases
/// exist, so managing constness is better left to each client.
///
/// Implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.

template<typename T>
class file_cache
{
  public:
    using retrieved_type = std::shared_ptr<T>;

    static file_cache<T>& instance()
        {
        static file_cache<T> z;
        return z;
        }

    retrieved_type retrieve_or_reload(fs::path const& filename)
        {
        // Throws if !exists(filename).
        std::time_t const write_time = fs::last_write_time(filename);

        auto i = cache_.lower_bound(filename);
        if
            (  cache_.end() == i
            || filename     != i->first
            || write_time   != i->second.write_time
            )
            {
            // Construct before inserting because ctor might throw.
            retrieved_type value(new T(filename));

            // insert() doesn't update the value if the key is already
            // present, so insert a dummy value and then modify it.
            // This works for both existing and new keys.
            i = cache_.insert(i, std::make_pair(filename, record()));
            i->second.data = value;
            i->second.write_time = write_time;
            }

        LMI_ASSERT(i->second.data);
        return i->second.data;
        }

  private:
    file_cache() = default;
    file_cache(file_cache const&) = delete;
    file_cache& operator=(file_cache const&) = delete;

    struct record
    {
        retrieved_type data;
        std::time_t    write_time;
    };

    std::map<fs::path,record> cache_;
};
} // namespace detail

/// Mixin to cache parent instances constructed from files.
///
/// Implemented in terms of class file_cache (q.v.).

template<typename T>
class cache_file_reads
{
    using retrieved_type = typename detail::file_cache<T>::retrieved_type;

  public:
    /// Return parent instance (constructed from file) via cache.
    ///
    /// Postcondition: returned pointer is not null; otherwise,
    /// file_cache::retrieve_or_reload() throws.

    static retrieved_type read_via_cache(fs::path const& filename)
        {
        return detail::file_cache<T>::instance().retrieve_or_reload(filename);
        }
};

#endif // cache_file_reads_hpp
