// Alert messages.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Gregory W. Chicares.
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

#include "alert.hpp"

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>

#if !defined LMI_MSW
#   include <cstdio>
#else  // defined LMI_MSW
#   include <windows.h>
#endif // defined LMI_MSW

#include <ios>
#include <sstream>                      // std::stringbuf
#include <stdexcept>

namespace
{
typedef void (*alert_function_pointer)(std::string const&);
alert_function_pointer status_alert_function         = nullptr;
alert_function_pointer warning_alert_function        = nullptr;
alert_function_pointer hobsons_choice_alert_function = nullptr;
alert_function_pointer fatal_error_alert_function    = nullptr;

typedef void (*message_function_pointer)(char const*);
message_function_pointer safe_message_alert_function = nullptr;

inline bool all_function_pointers_have_been_set()
{
    return
            nullptr != status_alert_function
        &&  nullptr != warning_alert_function
        &&  nullptr != hobsons_choice_alert_function
        &&  nullptr != fatal_error_alert_function
        &&  nullptr != safe_message_alert_function
        ;
}

inline bool any_function_pointer_has_been_set()
{
    return
            nullptr != status_alert_function
        ||  nullptr != warning_alert_function
        ||  nullptr != hobsons_choice_alert_function
        ||  nullptr != fatal_error_alert_function
        ||  nullptr != safe_message_alert_function
        ;
}

void report_catastrophe(char const* message)
{
#if !defined LMI_MSW
    std::fputs(message, stderr);
    std::fputc('\n'   , stderr);
    std::fflush(stderr);
#else  // defined LMI_MSW
    ::MessageBoxA
        (0
        ,message
        ,"Catastrophic error"
        ,MB_OK | MB_SETFOREGROUND | MB_ICONHAND | MB_SYSTEMMODAL
        );
#endif // defined LMI_MSW
}
} // Unnamed namespace.

bool set_alert_functions
    (void(*status_alert_function_pointer        )(std::string const&)
    ,void(*warning_alert_function_pointer       )(std::string const&)
    ,void(*hobsons_choice_alert_function_pointer)(std::string const&)
    ,void(*fatal_error_alert_function_pointer   )(std::string const&)
    ,void(*safe_message_alert_function_pointer  )(char const*)
    )
{
    if(any_function_pointer_has_been_set())
        {
        report_catastrophe
            ("At least one alert function pointer had already been set."
            );
        }
    status_alert_function         = status_alert_function_pointer        ;
    warning_alert_function        = warning_alert_function_pointer       ;
    hobsons_choice_alert_function = hobsons_choice_alert_function_pointer;
    fatal_error_alert_function    = fatal_error_alert_function_pointer   ;
    safe_message_alert_function   = safe_message_alert_function_pointer  ;
    return true;
}

/// Member function alert_buf::alert_string() provides get-reset-use
/// semantics to ensure that the std::stringbuf is reset even if an
/// exception is thrown by alert_buf::raise_alert(). Performing the
/// reset in alert_buf::sync() after calling alert_buf::raise_alert()
/// would give get-use-[throw]-reset semantics, which wouldn't work
/// correctly: in the event of an exception, the std::stringbuf would
/// not be cleared of its former contents.

class alert_buf
    :public std::stringbuf
{
  protected:
    alert_buf()
        :std::stringbuf()
        {
        if(!all_function_pointers_have_been_set())
            {
            report_catastrophe
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

/// Exceptions must be cleared here: otherwise, any prior exception
/// would cause this function to fail when it calls exceptions().
///
/// Both 'failbit' [27.6.2.5.3/8] and 'badbit' [27.6.2.1/3] must be
/// specified in the call to exceptions().

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

void safely_show_message(char const* message)
{
    if(nullptr == safe_message_alert_function)
        {
        report_catastrophe
            ("No function defined for reporting a problem safely."
            );
        }
    safe_message_alert_function(message);
}

void safely_show_message(std::string const& message)
{
    safely_show_message(message.c_str());
}

std::string const& hobsons_prompt()
{
    static std::string s
        ("Stop the current operation and attempt to resume safely?"
        );
    return s;
}

void test_status()
{
    status()         << "Test status()"         << LMI_FLUSH;
}

void test_warning()
{
    warning()        << "Test warning()"        << LMI_FLUSH;
}

void test_hobsons_choice()
{
    hobsons_choice() << "Test hobsons_choice()" << LMI_FLUSH;
}

void test_fatal_error()
{
    fatal_error()    << "Test fatal_error()"    << LMI_FLUSH;
}

void test_standard_exception()
{
    throw std::runtime_error("Test a standard exception.");
}

void test_arbitrary_exception()
{
    throw "Test an arbitrary exception.";
}

void test_catastrophe_report()
{
    report_catastrophe("Test catastrophe report.");
}

