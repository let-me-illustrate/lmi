// Permit running the system iff data files and date are valid.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018 Gregory W. Chicares.
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

#include "pchfile.hpp"

#include "authenticity.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "md5.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "platform_dependent.hpp"       // chdir()
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <cstdio>                       // fclose(), fopen()
#include <cstdlib>                      // exit(), EXIT_FAILURE
#include <cstring>                      // memcpy()
#include <iomanip>
#include <sstream>

// TODO ?? Known security hole: data files can be modified after they
// have been validated with 'md5sum'. This problem will grow worse
// when the binary database files are replaced with xml.

namespace
{
    int const chars_per_formatted_hex_byte = CHAR_BIT / 4;
}

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
    // The cached date is valid unless it's the peremptorily-invalid
    // default value of JDN zero.
    if
        (  calendar_date(jdn_t(0)) != Instance().CachedDate_
        && candidate               == Instance().CachedDate_
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
    fs::path original_path(fs::current_path());
    if(0 != chdir(data_path.string().c_str()))
        {
        oss
            << "Unable to change directory to '"
            << data_path
            << "'. Try reinstalling."
            ;
        return oss.str();
        }
    try
        {
        system_command("md5sum --check --status " + std::string(md5sum_file()));
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
    if(0 != chdir(original_path.string().c_str()))
        {
        oss
            << "Unable to restore directory to '"
            << original_path
            << "'. Try reinstalling."
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

std::string md5_hex_string(std::vector<unsigned char> const& vuc)
{
    LMI_ASSERT(md5len == vuc.size());
    std::stringstream oss;
    oss << std::hex;
    for(auto const& j : vuc)
        {
        oss
            << std::setw(chars_per_formatted_hex_byte)
            << std::setfill('0')
            << static_cast<unsigned int>(j)
            ;
        }
    return oss.str();
}

