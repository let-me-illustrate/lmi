// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
//
// Copyright (C) 2004 Gregory W. Chicares.
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
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: mc_enum_test.cpp,v 1.1 2005-02-03 16:03:37 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include <algorithm>
#include <exception>
#include <ios>
#include <iterator>
#include <sstream>

#include "mc_enum.hpp"
#include "mc_enum.tpp" // Template class implementation.
#include "mc_enum_test_aux.hpp"

#define BOOST_INCLUDE_MAIN
#include "test_tools.hpp"

enum enum_island {i_Easter = 37, i_Pago_Pago = -17};
extern enum_island const island_enums[] = {i_Easter, i_Pago_Pago};
extern char const*const island_strings[] = {"Easter", "Pago Pago"};
template class mc_enum<enum_island, 2, island_enums, island_strings>;
typedef mc_enum<enum_island, 2, island_enums, island_strings> e_island;

// Enumerative type 'e_holiday' is explicitly instantiated in a
// different translation unit.

int test_main(int, char*[])
{
    // Default ctor.
    e_holiday holiday0;
    BOOST_TEST_EQUAL("Theophany", holiday0);

    // Construct from enumerator.
    e_holiday holiday1(h_Easter);
    BOOST_TEST_EQUAL(holiday1, "Easter");

    // Construct from std::string.
    std::string const s_Pentecost("Pentecost");
    e_holiday holiday2(s_Pentecost);
    BOOST_TEST_EQUAL(holiday2, "Pentecost");

    // Construct from char const* implicitly converted to std::string.
    e_holiday holiday3("Pentecost");
    BOOST_TEST_EQUAL(holiday3, "Pentecost");

    // Copy ctor.
    e_holiday holiday4(holiday1);
    BOOST_TEST_EQUAL(holiday4, "Easter");

    // Copy assignment operator.
    holiday4 = holiday2;
    BOOST_TEST_EQUAL(holiday4, "Pentecost");

    // Assign from enumerator.
    holiday4 = h_Theophany;
    BOOST_TEST_EQUAL(holiday4, "Theophany");

    // Assign from std::string.
    holiday4 = s_Pentecost;
    BOOST_TEST_EQUAL(holiday4, "Pentecost");

    // Assign from char const* implicitly converted to std::string.
    holiday4 = "Easter";
    BOOST_TEST_EQUAL(holiday4, "Easter");

    // Equivalence to same type.
    BOOST_TEST_EQUAL(holiday4, holiday1);

    // Equivalence to enumerator.
    BOOST_TEST_EQUAL(holiday4, h_Easter);

    // Equivalence to std::string.
    std::string const s_Easter("Easter");
    BOOST_TEST_EQUAL(holiday4, s_Easter);

    // Equivalence to char const* implicitly converted to std::string.
    BOOST_TEST_EQUAL(holiday4, "Easter");

    // Operators added by boost::equality_comparable.
    BOOST_TEST_EQUAL  (h_Easter,    holiday4);
    BOOST_TEST_EQUAL  (s_Easter,    holiday4);
    BOOST_TEST_EQUAL  ("Easter",    holiday4);
    BOOST_TEST_UNEQUAL(holiday3,    holiday4);
    BOOST_TEST_UNEQUAL(h_Pentecost, holiday4);
    BOOST_TEST_UNEQUAL(s_Pentecost, holiday4);
    BOOST_TEST_UNEQUAL("Pentecost", holiday4);

    // Ordinal.
    BOOST_TEST_EQUAL(holiday4.ordinal(), 1);

    // Explicit conversion to std::string.
    // Not tested because it's private.
//    BOOST_TEST_EQUAL(holiday4.str(), "Easter");

    // Explicit conversion to enumerator.
    BOOST_TEST_EQUAL(holiday4.value(), h_Easter);

    // Cardinality.
    BOOST_TEST_EQUAL(holiday4.cardinality(), 3);
    mc_enum_base* base_pointer = dynamic_cast<mc_enum_base*>(&holiday4);
    BOOST_TEST_UNEQUAL(base_pointer, 0);
    int expected_cardinality = 0;
    if(base_pointer)
        {
        expected_cardinality = base_pointer->cardinality();
        }
    BOOST_TEST_EQUAL(expected_cardinality, 3);

    // Strings.
    BOOST_TEST_EQUAL("Theophany", holiday4.str(0));
    BOOST_TEST_EQUAL("Easter"   , holiday4.str(1));
    BOOST_TEST_EQUAL("Pentecost", holiday4.str(2));

    // Stream operators.
    e_holiday const Easter(h_Easter);
    e_holiday holiday5;
    std::stringstream ss;
    ss << Easter;
    ss >> holiday5;
    BOOST_TEST_EQUAL(Easter, holiday5);

    ss.str("");
    ss.clear();
    e_holiday const Pentecost(h_Pentecost);
    ss << Pentecost;
    ss >> holiday5;
    BOOST_TEST_EQUAL(Pentecost, holiday5);

    // Test an enum class defined in this translation unit that has a
    // std::string name that matches one in a different class.
    e_island island0;
    BOOST_TEST_EQUAL("Easter", island0);
    BOOST_TEST_EQUAL(i_Easter, island0);

    // Streaming shouldn't stop reading at blanks, which are not
    // treated as whitespace.
    e_island island1(i_Pago_Pago);
    ss.clear();
    ss.str("");
    ss << island1;
    ss >> island1;
    BOOST_TEST_EQUAL("Pago Pago", island1);

    BOOST_TEST_THROW
        (e_island unknown("Borneo")
        ,std::runtime_error
        ,""
        );

    return 0;
}

