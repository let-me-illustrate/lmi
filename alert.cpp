// Alert messages.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: alert.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#include <ios>
#include <sstream>   // std::stringbuf
#include <stdexcept>

// TODO ?? Track down this problem:
//   Caught exception: basic_ios::clear(iostate) caused exception
// from test_exception().

namespace
{
typedef void (*alert_function_pointer)(std::string const&);
alert_function_pointer status_alert_function         = 0;
alert_function_pointer warning_alert_function        = 0;
alert_function_pointer hobsons_choice_alert_function = 0;
alert_function_pointer fatal_error_alert_function    = 0;

inline bool all_function_pointers_have_been_set()
{
    return
            0 != status_alert_function
        &&  0 != warning_alert_function
        &&  0 != hobsons_choice_alert_function
        &&  0 != fatal_error_alert_function
        ;
}

inline bool any_function_pointer_has_been_set()
{
    return
            0 != status_alert_function
        ||  0 != warning_alert_function
        ||  0 != hobsons_choice_alert_function
        ||  0 != fatal_error_alert_function
        ;
}
} // Unnamed namespace

bool set_alert_functions
    (void(*status_alert_function_pointer        )(std::string const&)
    ,void(*warning_alert_function_pointer       )(std::string const&)
    ,void(*hobsons_choice_alert_function_pointer)(std::string const&)
    ,void(*fatal_error_alert_function_pointer   )(std::string const&)
    )
{
    if(any_function_pointer_has_been_set())
        {
        throw std::runtime_error
            ("At least one alert function pointer had already been set."
            );
        }
    status_alert_function         = status_alert_function_pointer        ;
    warning_alert_function        = warning_alert_function_pointer       ;
    hobsons_choice_alert_function = hobsons_choice_alert_function_pointer;
    fatal_error_alert_function    = fatal_error_alert_function_pointer   ;
    return true;
}

class alert_buf
    :public std::stringbuf
{
  protected:
    alert_buf()
        :std::stringbuf()
        {
        if(!all_function_pointers_have_been_set())
            {
            throw std::runtime_error
                ("Not all alert function pointers have been set."
                );
            }
        }
    int sync()
        {
        raise_alert();
        str("");
        return 0;
        }

  private:
    virtual void raise_alert() = 0;
};

class status_buf
    :public alert_buf
{
    void raise_alert()
        {
        status_alert_function(str());
        }
};

class warning_buf
    :public alert_buf
{
    void raise_alert()
        {
        warning_alert_function(str());
        }
};

class hobsons_choice_buf
    :public alert_buf
{
    void raise_alert()
        {
        hobsons_choice_alert_function(str());
        }
};

class fatal_error_buf
    :public alert_buf
{
    void raise_alert()
        {
        fatal_error_alert_function(str());
        }
};

std::ostream& status()
{
    static status_buf buffer_;
    static std::ostream stream_(&buffer_);
    return stream_;
}

std::ostream& warning()
{
    static warning_buf buffer_;
    static std::ostream stream_(&buffer_);
    return stream_;
}

std::ostream& hobsons_choice()
{
    static hobsons_choice_buf buffer_;
    static std::ostream stream_(&buffer_);
#ifndef BC_BEFORE_5_5
    stream_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
#endif // Not old borland compiler.
    return stream_;
}

std::ostream& fatal_error()
{
    static fatal_error_buf buffer_;
    static std::ostream stream_(&buffer_);
#ifndef BC_BEFORE_5_5
    stream_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
#endif // Not old borland compiler.
    return stream_;
}

std::string const& hobsons_prompt()
{
    static std::string s
        ("Attempt to continue, probably with erroneous results?"
        );
    return s;
}

std::ostream& hobsons_prompt(std::ostream& os)
{
    os << hobsons_prompt();
    return os;
}

