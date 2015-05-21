// Tools for working with SOA tables represented in binary format.
//
// Copyright (C) 2015 Gregory W. Chicares.
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

#ifndef soa_database_hpp
#define soa_database_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"
#include "uncopyable_lmi.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

/// Namespace containing classes working with databases in SOA binary format.
///
/// Support for other versions of the format, such as XML-based XTbML, could be
/// added in the future and this namespace exists to facilitate replacing the
/// binary format with another one by just changing the name of the namespace.
namespace soa_binary_format
{

class table_impl;
class database_impl;

/// A single table in SOA database.
class table
    :        private lmi::uncopyable <table>
    ,virtual private obstruct_slicing<table>
{
  public:
    // table number: just an integer wrapped in a class for type-safety.
    class Number
    {
      public:
        explicit Number(int number) : number_(number) {}
        int value() const { return number_; }

        bool operator==(Number other) const { return number_ == other.number_; }
        bool operator!=(Number other) const { return number_ != other.number_; }
        bool operator<(Number other) const { return number_ < other.number_; }

      private:
        // The number can't really change after construction but is non-const
        // to allow storing Number objects in standard containers.
        int number_;
    };

    // Read a table from a text file, throws on failure.
    static table read_from_text(fs::path const& file);

    // Save the table in the format understood by read_from_text().
    void save_as_text(fs::path const& file) const;

    // The only currently defined mutating operation: change table name.
    void name(std::string const& n);

    // Observers for all the table fields.
    Number number() const;
    std::string const& name() const;

    unsigned long compute_hash_value() const;

  private:
    // Private ctor used only by database.
    explicit table(boost::shared_ptr<table_impl> const& impl)
        :impl_(impl)
    {
    }

    boost::shared_ptr<table_impl> impl_;

    friend database_impl;
};

/// A database in SOA binary format.
///
/// A database contains 0 or more tables, uniquely identified by their numbers.
/// Tables can added to or deleted from the database.
///
/// It is represented by two disk files with the extensions .dat and .ndx, the
/// first containing the tables data and the second being the index allowing to
/// locate a table by its number.
class database
    :        private lmi::uncopyable <database>
    ,virtual private obstruct_slicing<database>
{
  public:
    // Initialize an empty database.
    //
    // Call append_table() or add_or_replace_table() later to add tables to the
    // database and eventually save() it.
    database();

    // Constructor takes the name of the associated file, which may include the
    // path to it but not the extension.
    //
    // Both path.dat and path.ndx files must exist, otherwise an exception is
    // thrown.
    explicit database(fs::path const& path);

    // table access by index, only useful for iterating over all of them (using
    // iterators could be an alternative approach, but would be heavier without
    // providing much gain).
    int tables_count() const;
    table get_nth_table(int idx) const;

    // table access by number, throws if there is no table with this number.
    table find_table(table::Number number) const;

    // Add a new table, throws if a table with the same number already exists.
    //
    // Notice that the addition of the new tables only are taken into account
    // when and if the database is saved using save() method. However
    // tables_count() return value is updated immediately and calling this
    // method invalidates the previously valid indices.
    void append_table(table const& table);

    // Add a new table, replacing the existing one with the same number, if any.
    //
    // The notes for append_table() also apply to this method.
    void add_or_replace_table(table const& table);

    // Save the current database contents to the specified file.
    void save(fs::path const& path);

    ~database();

  private:
    database_impl* const impl_;
};

} // namespace soa_binary_format

#endif // soa_database_hpp
