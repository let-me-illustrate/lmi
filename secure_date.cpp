// Moderately secure system date validation--tells whether system has expired.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: secure_date.cpp,v 1.5 2005-12-16 11:02:59 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "secure_date.hpp"

#include "alert.hpp"
#include "md5.hpp"
#include "platform_dependent.hpp" // chdir()
#include "system_command.hpp"

#include <boost/filesystem/fstream.hpp>

#include <cstdio>
#include <cstring>
#include <iomanip>
#include <memory>
#include <sstream>

// TODO ?? Known security hole: data files can be modified after we've
// validated them with md5sum . This will become a bigger problem when
// we replace the binary database files with xml.

std::auto_ptr<secure_date> foo(secure_date::instance());

secure_date* secure_date::instance_ = 0;

namespace
{
    int const chars_per_formatted_hex_byte = CHAR_BIT / 4;
}

//============================================================================
secure_date* secure_date::instance()
{
    if(0 == instance_)
        {
        instance_ = new secure_date;
        }
    return instance_;
}

//============================================================================
// Initialize to julian day number zero, which is 4713 BC, a date we
// will treat as always invalid. It is important to have such a known
// and implausible initial value in case the validation code fails in
// some unanticipated way that 'ratifies' the default date.
secure_date::secure_date()
{
    julian_day_number(0);
}

//============================================================================
std::string secure_date::validate
    (calendar_date const& candidate
    ,fs::path const&      path
    )
{
    // The date last validated is valid unless it's JDN zero.
    if
        (  0 != candidate.julian_day_number()
        && candidate == dynamic_cast<calendar_date const&>(*instance())
        )
        {
        return "";
        }

    std::ostringstream oss;

    // Read the passkey and valid-date-range files each time
    // because they might change while the program is running.
    // They'll be validated against validated md5sums a fraction
    // of a second later, to guard against fraudulent manipulation.

    // Read saved passkey from file.
    std::string passkey;
    {
    fs::path passkey_path(path / "passkey");
    fs::ifstream is(passkey_path);
    if(!is)
        {
        oss
            << "Unable to read passkey file '"
            << passkey_path.string()
            << "'. Try reinstalling."
            ;
        return oss.str();
        }

    is >> passkey;
    if(!is.eof())
        {
        oss
            << "Error reading passkey file '"
            << passkey_path.string()
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
    calendar_date begin;
    calendar_date end;
    {
    fs::path expiry_path(path / "expiry");
    fs::ifstream is(expiry_path);
    if(!is)
        {
        oss
            << "Unable to read expiry file '"
            << expiry_path.string()
            << "'. Try reinstalling."
            ;
        return oss.str();
        }

    is >> begin >> end;
    if(!is.eof())
        {
        oss
            << "Error reading expiry file '"
            << expiry_path.string()
            << "'. Try reinstalling."
            ;
        return oss.str();
        }
    }

    // Make sure candidate date is within valid range.
    if(candidate < begin)
        {
        oss
            << "Current date '"
            << candidate.str()
            << "' is invalid: this system cannot be used before '"
            << begin.str()
            << "'. Contact the home office."
            ;
        return oss.str();
        }
    if(end <= candidate)
        {
        oss
            << "Current date '"
            << candidate.str()
            << "' is invalid: this system expired on '"
            << end.str()
            << "'. Contact the home office."
            ;
        return oss.str();
        }

    // Validate all data files.
    chdir(path.string().c_str());
    if(system_command("md5sum --check --status " + std::string(md5sum_file())))
        {
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
    FILE* md5sums_file = std::fopen
        ((path / md5sum_file()).string().c_str()
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
    dynamic_cast<calendar_date&>(*instance()) = candidate;
    return "";
}

//============================================================================
std::string md5_hex_string(std::vector<unsigned char> const& vuc)
{
    LMI_ASSERT(md5len == vuc.size());
    std::stringstream oss;
    oss << std::hex;
    for(int j = 0; j < md5len; ++j)
        {
        oss
            << std::setw(chars_per_formatted_hex_byte)
            << std::setfill('0')
            << static_cast<unsigned int>(vuc[j])
            ;
        }
    return oss.str();
}

