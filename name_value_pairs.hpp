// Manage files of name-value pairs.
//
// Copyright (C) 2005 Gregory W. Chicares.
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

// $Id: name_value_pairs.hpp,v 1.4 2005-12-22 13:59:49 chicares Exp $

#ifndef name_value_pairs_hpp
#define name_value_pairs_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <map>

/// Design notes for class name_value_pairs.
///
/// This class stores name-value pairs as strings, providing read-only
/// access to values either as strings or as numbers.
///
/// Its design is deliberately quite limited. The motivation is only
/// to read flat-text configuration files with one name-value pair per
/// line, separated by an equal sign. For each line, every character
/// preceding the equal sign is part of the name, and every character
/// following the equal sign but preceding the end-of-line marker is
/// part of the value: leading or trailing blanks are not skipped.
///
/// This is similar to the file layout specified for wx class
/// wxFileConfig, but simpler in that 'group' names (special lines
/// consisting of a token in square brackets] are ignored, because
/// they aren't meaningful for the particular files lmi uses.
///
/// Eventually a facility to write such files might become desirable;
/// for now, they may only be read, and for simplicity the file to
/// read is given as a ctor argument.
///
/// It may seem odd to provide a distinct numeric accessor. The
/// rationale is that "missing" numeric data are to be treated as
/// zero; furthermore, leading and trailing blanks are disregarded
/// for numeric conversions.
///
/// A private accessor to the internal std::map data member is
/// provided only for use by the friend unit-test function.
///
/// Implementation notes for class name_value_pairs.
///
/// The 'configuration' file is deliberately read in text rather than
/// binary mode. For msw, this means that carriage returns in CR-LF
/// pairs are ignored. Using CR-LF delimiters in such files prevents
/// them from being portable to other operating systems, but the
/// author of such a file can choose to write portable files--that's
/// not the responsibility of this class.
///
/// If two lines have name-value pairs with the same name, then the
/// later line's value overwrites the first line's. This class's
/// responsibility is only to behave reasonably, not to diagnose
/// practices that the 'configuration' file's author would do well to
/// eschew.
///
/// Existence and accessibility of the 'configuration' file are not
/// checked; if the file cannot be read, then the std::map member is
/// simply not populated. This might be inappropriate for a general-
/// purpose library, but it meets lmi's specialized needs, and such
/// extra checks can easily be added if they become desirable.

class name_value_pairs
    :private boost::noncopyable
    ,virtual private obstruct_slicing<name_value_pairs>
{
    friend int test_main(int, char*[]);

    typedef std::map<std::string, std::string> string_map;

  public:
    name_value_pairs(std::string const& filename);
    ~name_value_pairs();

    std::string const& string_value(std::string const& key) const;
    double numeric_value(std::string const& key) const;

  private:
    std::map<std::string, std::string> const& map() const;

    string_map map_;
};

#endif // name_value_pairs_hpp

