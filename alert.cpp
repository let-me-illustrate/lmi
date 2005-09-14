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

// $Id: alert.cpp,v 1.4 2005-09-14 14:16:30 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

#include <ios>
#include <sstream>   // std::stringbuf
#include <stdexcept>

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

// Member function alert_buf::alert_string() provides get-reset-use
// semantics to ensure that the std::stringbuf is reset even if an
// exception is thrown by alert_buf::raise_alert(). Performing the
// reset in alert_buf::sync() after calling alert_buf::raise_alert()
// would give get-use-[throw]-reset semantics, which wouldn't work
// correctly: in the event of an exception, the std::stringbuf would
// not be cleared of its former contents.

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
        return 0;
        }

  protected:
    std::string const& alert_string()
        {
        alert_string_ = str();
        str("");
        return alert_string_;
        }

  private:
    virtual void raise_alert() = 0;

    std::string alert_string_;
};

class status_buf
    :public alert_buf
{
    void raise_alert()
        {
        status_alert_function(alert_string());
        }
};

class warning_buf
    :public alert_buf
{
    void raise_alert()
        {
        warning_alert_function(alert_string());
        }
};

class hobsons_choice_buf
    :public alert_buf
{
    void raise_alert()
        {
        hobsons_choice_alert_function(alert_string());
        }
};

class fatal_error_buf
    :public alert_buf
{
    void raise_alert()
        {
        fatal_error_alert_function(alert_string());
        }
};

// Exceptions must be cleared here: otherwise, any prior exception
// would cause this function to fail when it calls exceptions().
//
// Both 'failbit' [27.6.2.5.3/8] and 'badbit' [27.6.2.1/3] must be
// specified in the call to exceptions().
//
template<typename T>
inline std::ostream& alert_stream()
{
    // Double parentheses: don't parse comma as a macro parameter separator.
    BOOST_STATIC_ASSERT((boost::is_base_and_derived<alert_buf,T>::value));
    static T buffer_;
    static std::ostream stream_(&buffer_);
    stream_.clear();
    stream_.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    return stream_;
}

std::ostream& status()
{
    return alert_stream<status_buf>();
}

std::ostream& warning()
{
    return alert_stream<warning_buf>();
}

std::ostream& hobsons_choice()
{
    return alert_stream<hobsons_choice_buf>();
}

std::ostream& fatal_error()
{
    return alert_stream<fatal_error_buf>();
}

std::string const& hobsons_prompt()
{
    static std::string s
        ("Stop the current operation and attempt to resume safely?"
        );
    return s;
}

hobsons_choice_exception::hobsons_choice_exception()
{
}

void LMI_EXPIMP test_status()
{
    status()         << "Test status()"         << LMI_FLUSH;
}

void LMI_EXPIMP test_warning()
{
    warning()        << "Test warning()"        << LMI_FLUSH;
}

void LMI_EXPIMP test_hobsons_choice()
{
    hobsons_choice() << "Test hobsons_choice()" << LMI_FLUSH;
}

void LMI_EXPIMP test_fatal_error()
{
    fatal_error()    << "Test fatal_error()"    << LMI_FLUSH;
}

void LMI_EXPIMP test_standard_exception()
{
    throw std::runtime_error("Test a standard exception.");
}

void LMI_EXPIMP test_arbitrary_exception()
{
    throw "Test an arbitrary exception.";
}

