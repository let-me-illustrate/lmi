// Permit running the system iff data files and date are valid.
//
// Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef authenticity_hpp
#define authenticity_hpp

#include "config.hpp"

#include "calendar_date.hpp"
#include "so_attributes.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

/// Permit running the system iff data files and date are valid.
///
/// Implemented as a simple Meyers singleton, with the expected
/// dead-reference and threading issues.
///
/// 'cached_date_' holds the most-recently-validated date; it is
/// initialized to a peremptorily-invalid default value of JDN zero.

class Authenticity final
{
    friend class PasskeyTest;

  public:
    static Authenticity& Instance();
    static std::string Assay
        (calendar_date const& candidate
        ,fs::path const&      data_path
        );

  private:
    Authenticity() = default;
    ~Authenticity() = default;
    Authenticity(Authenticity const&) = delete;
    Authenticity& operator=(Authenticity const&) = delete;

    static void ResetCache();

    mutable calendar_date CachedDate_ {jdn_t(0)};
};

/// Authenticate production system and its crucial data files.
///
/// Terminate the program immediately if authentication fails. But
/// skip authentication altogether for the most-privileged password.

LMI_SO void authenticate_system();

/// Name of file containing md5sums of secured files.

inline char const* md5sum_file() {return "validated.md5";}

#endif // authenticity_hpp
