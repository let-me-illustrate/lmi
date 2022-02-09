// Permit running the system iff data files and date are valid.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "authenticity.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "contains.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "md5.hpp"
#include "md5sum.hpp"
#include "path.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "timer.hpp"

#include <cstdio>                       // fclose(), fopen()
#include <cstdlib>                      // exit(), EXIT_FAILURE
#include <cstring>                      // memcpy()
#include <iostream>                     // cout, endl
#include <sstream>
#include <stdexcept>
#include <vector>

// TODO ?? Known security hole: data files can be modified after they
// have been validated.

Authenticity& Authenticity::Instance()
{
    try
        {
        static Authenticity z;
        return z;
        }
    catch(...)
        {
        report_exception();
        alarum() << "Instantiation failed." << LMI_FLUSH;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

void Authenticity::ResetCache()
{
    Instance().CachedDate_ = jdn_t(0);
}

std::string Authenticity::Assay
    (calendar_date const& candidate
    ,fs::path const&      data_path
    )
{
    Timer timer;

    // The cached date is valid unless it's the peremptorily-invalid
    // default value of JDN zero.
    if
        (  calendar_date(jdn_t(0)) != Instance().CachedDate_
        && candidate               == Instance().CachedDate_
        // MD5 !! Revert "measure_md5" instrumentation soon.
        && !contains(global_settings::instance().pyx(), "measure_md5")
        )
        {
        return "cached";
        }

    ResetCache();

    std::ostringstream oss;

    // Read the passkey and valid-date-range files each time
    // because they might change while the program is running.
    // They'll be validated against validated md5sums a fraction
    // of a second later, to guard against fraudulent manipulation.

    // Read saved passkey from file.
    std::string passkey;
    {
    fs::path passkey_path(data_path / "passkey");
    fs::ifstream is(passkey_path);
    if(!is)
        {
        oss
            << "Unable to read passkey file '"
            << passkey_path
            << "'. Try reinstalling."
            ;
        return oss.str();
        }

    is >> passkey;
    if(!is.eof())
        {
        oss
            << "Error reading passkey file '"
            << passkey_path
            << "'. Try reinstalling."
            ;
        return oss.str();
        }

    if(passkey.size() != chars_per_formatted_hex_byte * md5len)
        {
        oss
            << "Length of passkey '"
            << passkey
            << "' is "
            << passkey.size()
            << " but should be "
            << chars_per_formatted_hex_byte * md5len
            << ". Try reinstalling."
            ;
        return oss.str();
        }
    }

    // Read valid date range [begin, end) from file.
    calendar_date begin(last_yyyy_date ());
    calendar_date end  (gregorian_epoch());
    {
    fs::path expiry_path(data_path / "expiry");
    fs::ifstream is(expiry_path);
    if(!is)
        {
        oss
            << "Unable to read expiry file '"
            << expiry_path
            << "'. Try reinstalling."
            ;
        return oss.str();
        }

    is >> begin >> end;
    if(!is || !is.eof())
        {
        oss
            << "Error reading expiry file '"
            << expiry_path
            << "'. Try reinstalling."
            ;
        return oss.str();
        }
    }

    // Make sure candidate date is within valid range.
    if(candidate < begin)
        {
        oss
            << "Current date "
            << candidate.str()
            << " is invalid: this system cannot be used before "
            << begin.str()
            << ". Contact the home office."
            ;
        return oss.str();
        }
    if(end <= candidate)
        {
        oss
            << "Current date "
            << candidate.str()
            << " is invalid: this system cannot be used after "
            << (-1 + end).str()
            << ". Contact the home office."
            ;
        return oss.str();
        }

    // Validate all data files.
    try
        {
        auto const sums = md5_read_checksum_file(data_path / md5sum_file());
        for(auto const& s : sums)
            {
            auto const file_path = data_path / s.filename;
            auto const md5 = md5_calculate_file_checksum
                (data_path / s.filename
                ,s.file_mode
                );
            if(md5 != s.md5sum)
                {
                    throw std::runtime_error
                        ( "Integrity check failed for '"
                        + s.filename.string()
                        + "'"
                        );
                }
            }
        }
    catch(...)
        {
        report_exception();
        oss
            << "At least one required file is missing, altered, or invalid."
            << " Try reinstalling."
            ;
        return oss.str();
        }

    // The passkey must match the md5 sum of the md5 sum of the file
    // of md5 sums of secured files.

    char c_passkey[md5len];
    unsigned char u_passkey[md5len];
    std::FILE* md5sums_file = std::fopen
        ((data_path / md5sum_file()).string().c_str()
        ,"rb"
        );
    md5_stream(md5sums_file, u_passkey);
    std::fclose(md5sums_file);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::memcpy(c_passkey, u_passkey, md5len);
    md5_buffer(c_passkey, md5len, u_passkey);
    std::string expected = md5_hex_string
        (std::vector<unsigned char>(u_passkey, u_passkey + md5len)
        );
    if(passkey != expected)
        {
        oss
            << "Passkey is incorrect for this version."
            << " Contact the home office."
            ;
        return oss.str();
        }
    // Cache the validated date.
    Instance().CachedDate_ = candidate;

    // MD5 !! Revert "measure_md5" instrumentation soon. Use
    //   git diff c029dd3248 authenticity.cpp
    // to see whether reversion is complete.
    std::cout << "authentication: " << timer.stop().elapsed_msec_str() << std::endl;

    return "validated";
}

void authenticate_system()
{
    if(global_settings::instance().ash_nazg())
        {
        return;
        }

    calendar_date const
        prospicience_date = global_settings::instance().prospicience_date();
    std::string const diagnostic_message = Authenticity::Assay
        (prospicience_date == last_yyyy_date()
            ? today()
            : prospicience_date
        ,global_settings::instance().data_directory()
        );
    if
        (  "validated" != diagnostic_message
        && "cached"    != diagnostic_message
        )
        {
        warning() << diagnostic_message << LMI_FLUSH;
        std::exit(EXIT_FAILURE);
        }
}
