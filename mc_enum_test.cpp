// M C Enums: string-Mapped, value-Constrained Enumerations: unit test.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "ce_product_name.hpp"
#include "mc_enum.hpp"
#include "mc_enum_test_aux.hpp"

#include "test_tools.hpp"

#include <sstream>
#include <stdexcept>

// Enumerative types 'e_holiday' and 'e_island' are explicitly
// instantiated in a different translation unit.

struct mc_enum_test
{
    static void test();
    static void test_product_name();
};

int test_main(int, char*[])
{
    mc_enum_test::test();
    mc_enum_test::test_product_name();
    return 0;
}

void mc_enum_test::test()
{
    // Default ctor.
    e_holiday holiday0;
    LMI_TEST_EQUAL("Theophany", holiday0);

    // Construct from enumerator.
    e_holiday holiday1(h_Easter);
    LMI_TEST_EQUAL(holiday1, "Easter");

    // Construct from std::string.
    std::string const s_Pentecost("Pentecost");
    e_holiday holiday2(s_Pentecost);
    LMI_TEST_EQUAL(holiday2, "Pentecost");

    // Construct from char const* implicitly converted to std::string.
    e_holiday holiday3("Pentecost");
    LMI_TEST_EQUAL(holiday3, "Pentecost");

    // Copy ctor.
    e_holiday holiday4(holiday1);
    LMI_TEST_EQUAL(holiday4, "Easter");

    // Copy assignment operator.
    holiday4 = holiday2;
    LMI_TEST_EQUAL(holiday4, "Pentecost");

    // Assign from enumerator.
    holiday4 = h_Theophany;
    LMI_TEST_EQUAL(holiday4, "Theophany");

    // Assign from std::string.
    holiday4 = s_Pentecost;
    LMI_TEST_EQUAL(holiday4, "Pentecost");

    // Assign from char const* implicitly converted to std::string.
    holiday4 = "Easter";
    LMI_TEST_EQUAL(holiday4, "Easter");

    // Equivalence to same type.
    LMI_TEST_EQUAL(holiday4, holiday1);

    // Equivalence to enumerator.
    LMI_TEST_EQUAL(holiday4, h_Easter);

    // Equivalence to std::string.
    std::string const s_Easter("Easter");
    LMI_TEST_EQUAL(holiday4, s_Easter);

    // Equivalence to char const* implicitly converted to std::string.
    LMI_TEST_EQUAL(holiday4, "Easter");

    // Operators added by boost::equality_comparable.
    LMI_TEST_EQUAL  (h_Easter,    holiday4);
    LMI_TEST_EQUAL  (s_Easter,    holiday4);
    LMI_TEST_EQUAL  ("Easter",    holiday4);
    LMI_TEST_UNEQUAL(holiday3,    holiday4);
    LMI_TEST_UNEQUAL(h_Pentecost, holiday4);
    LMI_TEST_UNEQUAL(s_Pentecost, holiday4);
    LMI_TEST_UNEQUAL("Pentecost", holiday4);

    // Ordinal.
    LMI_TEST_EQUAL(holiday4.ordinal(), 1);

    // Explicit conversion to std::string.
    LMI_TEST_EQUAL(holiday4.str(), "Easter");

    // Explicit conversion to enumerator.
    LMI_TEST_EQUAL(holiday4.value(), h_Easter);

    // Cardinality.
    LMI_TEST_EQUAL(holiday4.cardinality(), 3);
    mc_enum_base* base_pointer = dynamic_cast<mc_enum_base*>(&holiday4);
    LMI_TEST(nullptr != base_pointer);
    int expected_cardinality = 0;
    if(base_pointer)
        {
        expected_cardinality = base_pointer->cardinality();
        }
    LMI_TEST_EQUAL(expected_cardinality, 3);

    // Strings.
    LMI_TEST_EQUAL("Theophany", holiday4.str(0));
    LMI_TEST_EQUAL("Easter"   , holiday4.str(1));
    LMI_TEST_EQUAL("Pentecost", holiday4.str(2));
    std::vector<std::string> v;
    v.push_back(holiday4.str(0));
    v.push_back(holiday4.str(1));
    v.push_back(holiday4.str(2));
    LMI_TEST(v == all_strings<enum_holiday>());
    LMI_TEST(v == holiday4.all_strings());

    // Forced validity.
    holiday3.enforce_proscription();
    LMI_TEST_EQUAL(holiday3, "Pentecost");

    // If current value isn't allowed, pick the first one that is.
    holiday3.allow(2, false);
    LMI_TEST(!holiday3.is_allowed(2));
    LMI_TEST( holiday4.is_allowed(2));
    LMI_TEST_EQUAL(holiday3, "Pentecost");
    holiday3.enforce_proscription();
    LMI_TEST_EQUAL(holiday3, "Theophany");

    holiday3.allow(0, false);
    LMI_TEST_EQUAL(holiday3, "Theophany");
    holiday3.enforce_proscription();
    LMI_TEST_EQUAL(holiday3, "Easter");

    // If no value is allowed, pick the current ordinal.
    holiday3.allow(1, false);
    LMI_TEST_EQUAL(holiday3, "Easter");
    holiday3.enforce_proscription();
    LMI_TEST_EQUAL(holiday3, "Easter");

    // That which is inconceivable is not to be allowed.
    // COMPILER !! Here, como catches std::range_error when at()
    // throws, whereas 23.1.1/13 requires std::out_of_range.
    LMI_TEST_THROW(holiday3.allow( 3, false), std::out_of_range, "");
    LMI_TEST_THROW(holiday3.allow(17, false), std::out_of_range, "");
    LMI_TEST_THROW(holiday3.allow(-1, false), std::out_of_range, "");

    // Stream operators.
    e_holiday const Easter(h_Easter);
    e_holiday holiday5;
    std::stringstream ss;
    ss << Easter;
    ss >> holiday5;
    LMI_TEST_EQUAL(Easter, holiday5);

    ss.str("");
    ss.clear();
    e_holiday const Pentecost(h_Pentecost);
    ss << Pentecost;
    ss >> holiday5;
    LMI_TEST_EQUAL(Pentecost, holiday5);

    // Test an enumerative class that has a std::string name that
    // matches one in a different enumerative class.
    e_island island0;
    LMI_TEST_EQUAL("Easter", island0);
    LMI_TEST_EQUAL(i_Easter, island0);

    // Streaming shouldn't stop reading at blanks, which are not
    // treated as whitespace.
    e_island island1(i_Pago_Pago);
    ss.clear();
    ss.str("");
    ss << island1;
    ss >> island1;
    LMI_TEST_EQUAL("Pago Pago", island1);

    // A long time ago, a predecessor of this class replaced spaces
    // with underbars in its stream inserter. To maintain backward
    // compatibilitiy with old files: if stream extraction would fail,
    // then it's retried, substituting spaces for underbars.
    e_island island2;
    ss.clear();
    ss.str("Pago_Pago");
    ss >> island2;
    LMI_TEST_EQUAL("Pago Pago", island2);

    // Backward compatibility, however, must not come at the expense
    // of correctness. Underbars can legitimately appear in strings
    // (though they never did in the era of underbar substitution),
    // and they mustn't be replaced by spaces blithely.
    e_island island3;
    ss.clear();
    ss.str("Ni_ihau");
    ss >> island3;
    LMI_TEST_EQUAL("Ni_ihau", island3);

    LMI_TEST_THROW
        (e_island unknown("Borneo")
        ,std::runtime_error
        ,""
        );
}

void mc_enum_test::test_product_name()
{
    std::stringstream ss;
    ss.str("invalid product");
    ce_product_name x;
    LMI_TEST_THROW
        (ss >> x
        ,std::runtime_error
        ,"Value 'invalid product' invalid for type 'ce_product_name'."
        );
}
