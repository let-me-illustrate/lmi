// MVC Model for life-insurance illustrations--unit test.
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

// Facilities offered by all of these headers are tested here.
// Class product_database might appear not to belong, but it's
// intimately entwined with input.
#include "database.hpp"
#include "input.hpp"
#include "multiple_cell_document.hpp"
#include "single_cell_document.hpp"
#include "yare_input.hpp"
// End of headers tested here.

#include "assert_lmi.hpp"
#include "dbdict.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "miscellany.hpp"
#include "test_tools.hpp"
#include "timer.hpp"
#include "xml_lmi.hpp"

#include <boost/bind.hpp>

#include <xmlwrapp/document.h>

#if defined BOOST_MSVC || defined __BORLANDC__
#   include <cfloat>                    // floating-point hardware control
#endif // defined BOOST_MSVC || defined __BORLANDC__
#include <cstdio>                       // std::remove()
#include <fstream>
#include <ios>
#include <string>

class input_test
{
  public:
    static void test()
        {
        test_product_database();
        test_input_class();
        test_document_classes();
        test_obsolete_history();
        assay_speed();
        }

  private:
    static void test_product_database();
    static void test_input_class();
    static void test_document_classes();
    static void test_obsolete_history();
    static void assay_speed();

    template<typename DocumentClass>
    static void test_document_io
        (std::string const& original_filename
        ,std::string const& replica_filename
        ,char const*        file
        ,int                line
        ,bool               test_speed_only
        );

    static void mete_copy_ctor();
    static void mete_assign_op();
    static void mete_equal_op();
    static void mete_overhead();
    static void mete_read(xml::element& xml_data);
    static void mete_write();
    static void mete_cns_io();
    static void mete_ill_io();
    static void mete_cns_xsd();
    static void mete_ill_xsd();
};

void input_test::test_product_database()
{
    Input input;
    yare_input yi(input);
    product_database db(yi);
    std::vector<double> v;
    std::vector<double> w;

    // This vector's last element must be replicated.
    int dims_stat[e_number_of_axes] = {1, 1, 1, 1, 1, 1, 10};
    double stat[10] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.05};
    DBDictionary::instance().datum("StatVxQ") = database_entity
        (DB_StatVxQ
        ,e_number_of_axes
        ,dims_stat
        ,stat
        );
    db.Query(v, DB_StatVxQ);
    w.assign(stat, stat + 10);
    w.insert(w.end(), db.length() - w.size(), w.back());
    BOOST_TEST(v == w);

    // This vector must be truncated.
    int dims_tax[e_number_of_axes] = {1, 1, 1, 1, 1, 1, 100};
    double tax[100] =
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
        ,0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1
        ,0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2, 0.2
        ,0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3, 0.3
        ,0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4, 0.4
        ,0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
        ,0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6, 0.6
        ,0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7, 0.7
        ,0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8, 0.8
        ,0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9
        };
    DBDictionary::instance().datum("TaxVxQ") = database_entity
        (DB_TaxVxQ
        ,e_number_of_axes
        ,dims_tax
        ,tax
        );
    db.Query(v, DB_TaxVxQ);
    w.assign(tax, tax + db.length());
    BOOST_TEST(v == w);

    // Scalar access is forbidden when entity varies by duration.
    BOOST_TEST_THROW
        (db.Query(DB_StatVxQ)
        ,std::runtime_error
        ,"Assertion '1 == v.extent()' failed."
        );

    // Use bind<R> where compilation errors would occur without <R>.
    // The "recommended" solution forces the "right" overload by
    // writing an explicit pointer to member:
    //   http://lists.boost.org/boost-users/2007/06/28832.php
    // but if that becomes necessary, then bind should be abandoned:
    // writing individual functions by hand is simpler and clearer.
    std::cout
        << "\n  Database speed tests..."
        << "\n  initialize()      : " << TimeAnAliquot(boost::bind        (&product_database::initialize,      &db, "sample.database"))
        << "\n  Query(vector)     : " << TimeAnAliquot(boost::bind<void  >(&product_database::Query,           &db, v, DB_MaturityAge))
        << "\n  Query(scalar)     : " << TimeAnAliquot(boost::bind<double>(&product_database::Query,           &db,    DB_MaturityAge))
        << "\n  entity_from_key() : " << TimeAnAliquot(boost::bind        (&product_database::entity_from_key, &db,    DB_MaturityAge))
        << '\n'
        ;

    database_entity const maturity = db.entity_from_key(DB_MaturityAge);

    // Maturity age must not vary by duration.
    DBDictionary::instance().datum("MaturityAge") = database_entity
        (DB_StatVxQ
        ,e_number_of_axes
        ,dims_stat
        ,stat
        );
    BOOST_TEST_THROW
        (db.Query(DB_MaturityAge)
        ,std::runtime_error
        ,"Assertion '1 == v.extent()' failed."
        );
    DBDictionary::instance().datum("MaturityAge") = maturity;

    // A nondefault lookup index with a different issue age changes
    // the length of a queried vector.
    int dims_snflq[e_number_of_axes] = {1, 1, 1, e_max_dim_issue_age, 1, 1, 1};
    DBDictionary::instance().datum("SnflQ") = database_entity
        (DB_SnflQ
        ,e_number_of_axes
        ,dims_snflq
        ,tax
        );
    db.Query(v, DB_SnflQ);
    BOOST_TEST_EQUAL(55, db.length());
    BOOST_TEST_EQUAL(55, v.size());
    database_index index = db.index().issue_age(29);
    db.Query(v, DB_SnflQ, index);
    BOOST_TEST_EQUAL(55, db.length());
    BOOST_TEST_EQUAL(71, v.size());

    // Test presumptive issue-age bounds in class database_index.
    BOOST_TEST_THROW
        (index.issue_age(100)
        ,std::runtime_error
        ,"Assertion '0 <= z && z < e_max_dim_issue_age' failed."
        );
    BOOST_TEST_THROW
        (index.issue_age(-1)
        ,std::runtime_error
        ,"Assertion '0 <= z && z < e_max_dim_issue_age' failed."
        );

    index.issue_age(99);
    db.Query(v, DB_SnflQ, index);
    BOOST_TEST_EQUAL( 1, v.size());

    // Force the product to mature at 98.
    db.maturity_age_ = 98;
    index.issue_age(98);
    db.Query(DB_MaturityAge, index); // Accepted because maturity age is scalar.
    BOOST_TEST_THROW
        (db.Query(v, DB_SnflQ, index)
        ,std::runtime_error
        ,"Assertion '0 < local_length && local_length <= methuselah' failed."
        );

    index.issue_age(97);
    db.Query(v, DB_SnflQ, index);
    BOOST_TEST_EQUAL( 1, v.size());

    index.issue_age(0);
    db.Query(v, DB_SnflQ, index);
    BOOST_TEST_EQUAL(98, v.size());
}

void input_test::test_input_class()
{
    // Test << and >> operators.
    Input original;
    Input replica;

    std::ofstream os0("eraseme0.xml", ios_out_trunc_binary());
    BOOST_TEST(os0.good());

    original.AgentName           = "Herbert Cassidy";
    original.AgentPhone          = "123-4567";
    original.InsuredName         = "Full Name";
    original.Address             = "address";
    original.City                = "city";
//    original.FundAllocations     = "0.4 0.3 0.2 0.1";
    original.SeparateAccountRate = "0.03125";

    original.RealizeAllSequenceInput();
/*
    original.FundAllocs[0]       = 1.0;
    original.SeparateAccountRateRealized_[0] = .01234567890123456789;
    original.SeparateAccountRateRealized_[1] = .12345678901234567890;
    original.SeparateAccountRateRealized_[2] = .23456789012345678901;
    original.SeparateAccountRateRealized_[3] = .34567890123456789012;
    original.SeparateAccountRateRealized_[4] = .45678901234567890123;
    original.SeparateAccountRateRealized_[5] = .56789012345678901234;
    original.SeparateAccountRateRealized_[6] = .67890123456789012345;
    original.SeparateAccountRateRealized_[7] = .78901234567890123456;
    original.SeparateAccountRateRealized_[8] = .89012345678901234567;
    original.SeparateAccountRateRealized_[9] = .90123456789012345678;
*/

    xml_lmi::xml_document xml_document0("root");
    xml::element& xml_root0 = xml_document0.root_node();
    xml_root0 << original;
    os0 << xml_document0;
    os0.close();

    xml::node::const_iterator i = xml_root0.begin();
    LMI_ASSERT(!i->is_text());
    xml::element const& xml_node = *i;

    xml_node >> replica;
    std::ofstream os1("eraseme1.xml", ios_out_trunc_binary());
    BOOST_TEST(os1.good());

    xml_lmi::xml_document xml_document1("root");
    xml::element& xml_root1 = xml_document1.root_node();
    xml_root1 << replica;
    os1 << xml_document1;
    os1.close();

    BOOST_TEST(original == replica);
    bool okay = files_are_identical("eraseme0.xml", "eraseme1.xml");
    BOOST_TEST(okay);
    // Leave the files for analysis if they didn't match.
    if(okay)
        {
        BOOST_TEST(0 == std::remove("eraseme0.xml"));
        BOOST_TEST(0 == std::remove("eraseme1.xml"));
        }

    BOOST_TEST(0.03125 == original.SeparateAccountRateRealized_[0]);
    BOOST_TEST(replica.SeparateAccountRateRealized_.empty());
    replica.RealizeAllSequenceInput();
    BOOST_TEST(!replica.SeparateAccountRateRealized_.empty());
    BOOST_TEST(0.03125 == replica.SeparateAccountRateRealized_[0]);

/* TODO ?? The code this tests is defective--fix it someday.
    BOOST_TEST(0.4 == original.FundAllocs[0]);
    BOOST_TEST(0.4 == replica.FundAllocs[0]);
std::cout << "original.FundAllocs[0] is " << original.FundAllocs[0] << '\n';
std::cout << "replica.FundAllocs[0] is " << replica.FundAllocs[0] << '\n';

std::cout << "original.FundAllocs.size() is " << original.FundAllocs.size() << '\n';
std::cout << "replica.FundAllocs.size() is " << replica.FundAllocs.size() << '\n';
*/

    BOOST_TEST(0 == original.InforceYear);
    original["InforceYear"] = std::string("3");
    BOOST_TEST(3 == original.InforceYear);

// Fails--need to change initialization.
    BOOST_TEST(45 == original.IssueAge);
    original["IssueAge"] = std::string("57");
    BOOST_TEST(57 == original.IssueAge);

    // Test copy constructor.
    Input copy0(original);
    BOOST_TEST(original == copy0);
    copy0["InsuredName"] = "Claude Proulx";
    BOOST_TEST(original != copy0);
    BOOST_TEST(std::string("Claude Proulx") == copy0   .InsuredName.value());
    BOOST_TEST(std::string("Full Name")     == original.InsuredName.value());

    // Test assignment operator.
    Input copy1;
    copy1 = original;
    BOOST_TEST(original == copy1);
    copy1["InsuredName"] = "Angela";
    BOOST_TEST(original != copy1);
    BOOST_TEST(std::string("Angela")    == copy1   .InsuredName.value());
    BOOST_TEST(std::string("Full Name") == original.InsuredName.value());

    // For now at least, just test that this compiles and runs.
    yare_input y(original);
}

void input_test::test_document_classes()
{
    typedef multiple_cell_document M;
    test_document_io<M>("sample.cns", "replica.cns", __FILE__, __LINE__, false);
    typedef single_cell_document S;
    test_document_io<S>("sample.ill", "replica.ill", __FILE__, __LINE__, false);
}

void input_test::test_obsolete_history()
{
    Input z;
    z.InforceYear  = 0;
    z.InforceMonth = 0;
    // For new business, history is irrelevant.
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("0"    , "1"           , true));
    // Year 0, month 1: one year of history.
    z.InforceYear  = 0;
    z.InforceMonth = 1;
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("0"    , "1"           , true));
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1"    , "1"           , true));
    // Year 1, month 0: one year of history.
    z.InforceYear  = 1;
    z.InforceMonth = 0;
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("0"    , "1"           , true));
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1"    , "1"           , true));
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1"    , "1;2"         , true));
    BOOST_TEST(1 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "2"           , true));
    // "History" after first year doesn't matter.
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "1"           , true));
    // Year 1, month 1: two years of history.
    z.InforceYear  = 1;
    z.InforceMonth = 1;
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1"    , "1"           , true));
    // No conflict: history is a "subset" of specamt.
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "1;2;3"       , true));
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "1"           , true));
    BOOST_TEST(1 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "2"           , true));
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1"    , "1;2"         , true));
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "2;3"         , true));
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("1;2;3", "1;2;3"       , true));
    // Warn if a keyword is used--this one's meaning depends on context.
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "1;corridor"  , true));
    // Keyword may engender "obvious" false positives.
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1;2"  , "1;2;corridor", true));
    // This would have been forbidden: history was numeric only.
//  BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("X"    , "1"           , true));
    BOOST_TEST(2 == z.must_overwrite_specamt_with_obsolete_history("1"    , "X"           , true));
    // This case differs little from the preceding one, but has a different
    // outcome. Reason: non-numeric substrings are interepreted as zero.
    // This oddity doesn't matter, because specified amount by its nature
    // must be positive, and zero is returned so that nothing is changed.
    BOOST_TEST(0 == z.must_overwrite_specamt_with_obsolete_history("0"    , "X"           , true));
}

void input_test::assay_speed()
{
    Input raw_data;
    xml_lmi::xml_document document("root");
    xml::element& root = document.root_node();
    root << raw_data;

    xml::node::const_iterator i = root.begin();
    LMI_ASSERT(!i->is_text());
    xml::element const& e = *i;

    std::cout
        << "\n  Class 'Input' speed tests..."
        << "\n  Copy ctor: " << TimeAnAliquot(mete_copy_ctor           )
        << "\n  Assign   : " << TimeAnAliquot(mete_assign_op           )
        << "\n  Equals   : " << TimeAnAliquot(mete_equal_op            )
        << "\n  Overhead : " << TimeAnAliquot(mete_overhead            )
        << "\n  Read     : " << TimeAnAliquot(boost::bind(mete_read, e))
        << "\n  Write    : " << TimeAnAliquot(mete_write               )
        << "\n  'cns' io : " << TimeAnAliquot(mete_cns_io              )
        << "\n  'ill' io : " << TimeAnAliquot(mete_ill_io              )
        << "\n  'cns' xsd: " << TimeAnAliquot(mete_cns_xsd             )
        << "\n  'ill' xsd: " << TimeAnAliquot(mete_ill_xsd             )
        << '\n'
        ;
}

template<typename DocumentClass>
void input_test::test_document_io
    (std::string const& original_filename
    ,std::string const& replica_filename
    ,char const*        file
    ,int                line
    ,bool               test_speed_only
    )
{
    DocumentClass document(original_filename);
    std::ofstream ofs(replica_filename.c_str(), ios_out_trunc_binary());
    document.write(ofs);
    if(test_speed_only)
        {
        return;
        }

    ofs.close();
    bool okay = files_are_identical(original_filename, replica_filename);
    INVOKE_BOOST_TEST(okay, file, line);
    // Leave the file for analysis if it didn't match.
    if(okay)
        {
        INVOKE_BOOST_TEST
            (0 == std::remove(replica_filename.c_str())
            ,file
            ,line
            );
        }
}

void input_test::mete_copy_ctor()
{
    static Input const x;
    Input volatile y(x);
}

void input_test::mete_assign_op()
{
    static Input const x;
    static Input y;
    // Neither 'x' nor 'y' can be volatile, but analysis of the
    // timings may indicate whether this operation is optimized away.
    y = x;
}

void input_test::mete_equal_op()
{
    static Input const x;
    static Input const y(x);
    bool volatile b(x == y);
    stifle_warning_for_unused_value(b);
}

void input_test::mete_overhead()
{
    xml_lmi::xml_document document("root");
    xml::element& root = document.root_node();
    stifle_warning_for_unused_value(root);
}

void input_test::mete_read(xml::element& xml_data)
{
    static Input raw_data;
    xml_data >> raw_data;
    // Realizing sequence input might be done separately, but it must
    // somehow be done.
    raw_data.RealizeAllSequenceInput();
}

void input_test::mete_write()
{
    static Input raw_data;
    xml_lmi::xml_document document("root");
    xml::element& root = document.root_node();
    root << raw_data;
}

void input_test::mete_cns_io()
{
    typedef multiple_cell_document M;
    test_document_io<M>("sample.cns", "replica.cns", __FILE__, __LINE__, true);
}

void input_test::mete_ill_io()
{
    typedef single_cell_document S;
    test_document_io<S>("sample.ill", "replica.ill", __FILE__, __LINE__, true);
}

void input_test::mete_cns_xsd()
{
    static xml::document const cns = xml_lmi::dom_parser("sample.cns").document();
    static multiple_cell_document const mcd;
    mcd.validate_with_xsd_schema(cns);
}

void input_test::mete_ill_xsd()
{
    static xml::document const ill = xml_lmi::dom_parser("sample.ill").document();
    static single_cell_document const scd;
    scd.validate_with_xsd_schema(ill);
}

int test_main(int, char*[])
{
    // Location of '*.xsd' files.
    global_settings::instance().set_data_directory("/opt/lmi/data");

    input_test::test();
    return EXIT_SUCCESS;
}

