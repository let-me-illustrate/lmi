// Alert messages--unit test.
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

// $Id: alert_test.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

#include <iterator>
#include <typeinfo>
#include <vector>

int test_main(int, char*[])
{
    status()
        << "This should have no effect, because the handler for"
        << " the command-line interface gobbles status messages."
        << std::flush
        ;

    warning() << "This should be a single line," << std::flush;
    warning() << "and this should be another single line." << std::flush;

    warning() << "This";
    warning() << " sentence";
    warning() << " should";
    warning() << " occupy";
    warning() << " only";
    warning() << " one";
    warning() << " line." << std::flush;

    std::vector<double> v;
    v.push_back(2.718281828459045);
    v.push_back(1.0);
    warning() << "One would expect the natural logarithm of the first\n";
    warning() << " number in this list approximately to equal the second.\n";
    warning() << " list: ";
    std::copy
        (v.begin()
        ,v.end()
        ,std::ostream_iterator<double>(warning(), " ")
        );
    warning() << LMI_FLUSH;

    int return_value = -1;
    try
        {
        // This should throw:
        fatal_error() <<  "Simulated fatal error--location: " << LMI_FLUSH;
        std::cout << "This line shouldn't be reached." << std::endl;
        BOOST_TEST(false);
        }
    catch(std::exception const& e)
        {
        std::cout
            << "Caught exception '"
            << e.what()
            << "' of type '" << typeid(e).name()
            << "'." << std::endl
            ;
        if(typeid(e).name() == typeid(std::runtime_error).name())
            {
            return_value = 0;
            }
        else
            {
            std::cout
                << "\nHowever, type '" << typeid(std::runtime_error).name()
                << "' was expected." << std::endl
                ;
#if defined __GLIBCPP__ && __GLIBCPP__==20030426
            std::cout
                << "This is due to a defect in libstdc++-v3\n"
                << "that appears to have been fixed in CVS.\n"
                << "Try using ostream.tcc version 1.30.2.14,\n"
                << "dated 2003-12-01T19:39:49 .\n"
                << std::endl
                ;
#endif // Not defective libstdc++-v3.
            }
        }

    return return_value;
}

