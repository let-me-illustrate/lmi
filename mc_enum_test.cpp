// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mc_enum.hpp"
#include "mc_enum.tpp" // Template class implementation.
#include "mc_enum_test_aux.hpp"

#include "test_tools.hpp"

#include <sstream>
#include <stdexcept>

// Ni'ihau is deliberately (mis)spelled with an underbar instead of an
// apostrophe, in order to test provide_for_backward_compatibility().

enum enum_island {i_Easter = 37, i_Pago_Pago = -17, i_Ni_ihau = 13};
extern enum_island const island_enums[] = {i_Easter, i_Pago_Pago, i_Ni_ihau};
extern char const*const island_strings[] = {"Easter", "Pago Pago", "Ni_ihau"};
template class mc_enum<enum_island, 3, island_enums, island_strings>;
typedef mc_enum<enum_island, 3, island_enums, island_strings> e_island;

// Enumerative type 'e_holiday' is explicitly instantiated in a
// different translation unit.

struct mc_enum_test
{
    static void test();
};

int test_main(int, char*[])
{
    mc_enum_test::test();
    return 0;
}

void mc_enum_test::test()
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
    BOOST_TEST_EQUAL(holiday4.str(), "Easter");

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
    std::vector<std::string> v;
    v.push_back(holiday4.str(0));
    v.push_back(holiday4.str(1));
    v.push_back(holiday4.str(2));
    BOOST_TEST(v == e_holiday::all_strings());

    // Forced validity.
    holiday3.enforce_proscription();
    BOOST_TEST_EQUAL(holiday3, "Pentecost");

    // If current value isn't allowed, pick the first one that is.
    holiday3.allow(2, false);
    BOOST_TEST(!holiday3.is_allowed(2));
    BOOST_TEST( holiday4.is_allowed(2));
    BOOST_TEST_EQUAL(holiday3, "Pentecost");
    holiday3.enforce_proscription();
    BOOST_TEST_EQUAL(holiday3, "Theophany");

    holiday3.allow(0, false);
    BOOST_TEST_EQUAL(holiday3, "Theophany");
    holiday3.enforce_proscription();
    BOOST_TEST_EQUAL(holiday3, "Easter");

    // If no value is allowed, pick the current ordinal.
    holiday3.allow(1, false);
    BOOST_TEST_EQUAL(holiday3, "Easter");
    holiday3.enforce_proscription();
    BOOST_TEST_EQUAL(holiday3, "Easter");

    // That which is inconceivable is not to be allowed.
    // COMPILER !! Here, como catches std::range_error when at()
    // throws, whereas 23.1.1/13 requires std::out_of_range.
    BOOST_TEST_THROW(holiday3.allow( 3, false), std::out_of_range, "");
    BOOST_TEST_THROW(holiday3.allow(17, false), std::out_of_range, "");
    BOOST_TEST_THROW(holiday3.allow(-1, false), std::out_of_range, "");

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

    // A long time ago, a predecessor of this class replaced spaces
    // with underbars in its stream inserter. To maintain backward
    // compatibilitiy with old files: if stream extraction would fail,
    // then it's retried, substituting spaces for underbars.
    e_island island2;
    ss.clear();
    ss.str("Pago_Pago");
    ss >> island2;
    BOOST_TEST_EQUAL("Pago Pago", island2);

    // Backward compatibility, however, must not come at the expense
    // of correctness. Underbars can legitimately appear in strings
    // (though they never did in the era of underbar substitution),
    // and they mustn't be replaced by spaces blithely.
    e_island island3;
    ss.clear();
    ss.str("Ni_ihau");
    ss >> island3;
    BOOST_TEST_EQUAL("Ni_ihau", island3);

    BOOST_TEST_THROW
        (e_island unknown("Borneo")
        ,std::runtime_error
        ,""
        );
}

