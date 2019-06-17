// Verify product files.
//
// Copyright (C) 2019 Gregory W. Chicares.
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

#include "verify_products.hpp"

#include "actuarial_table.hpp"
#include "basic_tables.hpp"
#include "ce_product_name.hpp"
#include "cso_table.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "database.hpp"
#include "mc_enum.hpp"                  // all_strings<>()
#include "product_data.hpp"
#include "ssize_lmi.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
void verify_one_cell
    (std::string const& product_name
    ,std::string const& gender_str
    ,std::string const& smoking_str
    )
{
    mcenum_gender  const gender  = mce_gender (gender_str ).value();
    mcenum_smoking const smoking = mce_smoking(smoking_str).value();

    product_database const db0
        (product_name
        ,gender
        ,mce_standard
        ,smoking
        ,45
        ,mce_medical
        ,mce_s_XX
        );
    int const min_age = db0.query<int>(DB_MinIssAge);

    product_database const db
        (product_name
        ,gender
        ,mce_standard
        ,smoking
        ,min_age
        ,mce_medical
        ,mce_s_XX
        );

    auto const era    = db.query<oenum_cso_era   >(DB_CsoEra);
    auto const a_b    = db.query<oenum_alb_or_anb>(DB_AgeLastOrNearest);
    auto const t      = db.query<int             >(DB_Irc7702QTable);
    auto const axis_g = db.query<bool            >(DB_Irc7702QAxisGender);
    auto const axis_s = db.query<bool            >(DB_Irc7702QAxisSmoking);
    auto const omega  = db.query<int             >(DB_MaturityAge);

    int const years_to_maturity = omega - min_age;

    product_data const p(product_name);

    if
        (   (!axis_g && "Unisex"   != gender_str )
        ||  (!axis_s && "Unismoke" != smoking_str)
        )
        {
        std::cout
            << "  skipping"
            << ' ' << gender_str
            << ' ' << smoking_str
            << std::endl
            ;
        return;
        }

    switch(db.query<oenum_7702_q_whence>(DB_Irc7702QWhence))
        {
        // Validate irc_7702_q_builtin(), which is implemented in
        // terms of cso_table(). The interface of irc_7702_q()
        // (which delegates to irc_7702_q_builtin()) specifies the
        // [begin, end) age interval, so cso_table() is called with
        // those arguments here. Passing this test means only that
        // the irc_7702_q_builtin() logic is correct; the correctness
        // of cso_table() is established elsewhere.
        //
        // This will fail for a product that incorrectly specifies a
        // minimum age of zero for smoker-distinct CSO tables, which
        // never begin at age zero.
        case oe_7702_q_builtin:
            {
            std::vector<double> const v0 = cso_table
                (era
                ,oe_orthodox // No other option currently supported for 7702.
                ,a_b
                ,mce_gender (gender_str).value()
                ,mce_smoking(smoking_str).value()
                ,min_age
                ,omega
                );
            std::vector<double> const v1 = irc_7702_q
                (p
                ,db
                ,min_age
                ,years_to_maturity
                );
            std::cout
                << "7702 q okay: builtin "
                << std::string((v0 == v1) ? "validated" : "PROBLEM")
                << ' ' << gender_str
                << ' ' << smoking_str
                << std::endl
                ;
            }
            break;
        // Validate an external table. Passing this test means that
        // the external table is identical to the published CSO table,
        // and that the external table can be discarded and its
        // internal equivalent used instead. This is stricter than
        // necessary: a product with an age range of [20, 95) could
        // use an external table containing values only in that range;
        // but it would be foolish to fabricate such a table when the
        // full published table is available, as errors have been
        // known to occur in fabrication.
        case oe_7702_q_external_table:
            {
            if(0 == t)
                {
                std::cout
                    << "7702 q PROBLEM: " << product_name
                    << " nonexistent table zero"
                    << ' ' << gender_str
                    << ' ' << smoking_str
                    << std::endl
                    ;
                return;
                }

            std::vector<double> const v0 = cso_table
                (era
                ,oe_orthodox // No other option currently supported for 7702.
                ,a_b
                ,mce_gender (gender_str).value()
                ,mce_smoking(smoking_str).value()
                );
            std::string const f = AddDataDir(p.datum("Irc7702QFilename"));
            actuarial_table const a(f, t);
            std::vector<double> const v1 = a.values
                (a.min_age()
                ,1 + a.max_age() - a.min_age()
                );

            if(v0 == v1)
                {
                std::cout
                    << "7702 q okay: table " << t
                    << ' ' << gender_str
                    << ' ' << smoking_str
                    << std::endl
                    ;
                }
            else
                {
                std::cout
                    << "7702 q PROBLEM: " << product_name
                    << ' ' << gender_str
                    << ' ' << smoking_str
                    << std::endl
                    ;
                std::cout
                    << "\n  CSO era: " << era
                    << "\n  ALB or ANB: " << a_b
                    << "\n  table file: " << f
                    << "\n  table number: " << t
                    << "\n  min age: " << a.min_age()
                    << "\n  max age: " << a.max_age()
                    << "\n  cso length: " << lmi::ssize(v0)
                    << "\n  table length: " << lmi::ssize(v1)
                    << "\n  v0.front(): " << v0.front()
                    << "\n  v1.front(): " << v1.front()
                    << "\n  v0.back (): " << v0.back ()
                    << "\n  v1.back (): " << v1.back ()
                    << std::endl
                    ;
                }
            }
            break;
        }
}
} // Unnamed namespace.

/// Verify all products.
///
/// Class product_database (with the instance of class database_index
/// that it contains) suffices for retrieving data tabular data from
/// internal sources such as cso_table(). Class product_data provides
/// the names of tables stored in external database files.
///
/// Only 7702 tables are validated for now. Products have two distinct
/// sets of gender axes: one for underwriting, and another for 7702
/// Those axes needn't be the same. For example, a product might be
/// issued only on a sex-distinct basis, yet use unisex 7702 tables
/// (to stay within IRS Notice 88-128's safe harbor, or to use more
/// liberal rates for one market segment at the cost of disadvantaging
/// another. The smoking axes may differ likewise.
///
/// Two booleans {DB_Irc7702QAxisGender, DB_Irc7702QAxisSmoking} are
/// not adequate to describe all permissible variations. Arguably,
/// a more complex paradigm would be desirable, e.g., because two
/// booleans cannot represent a smoker-distinct-only product that
/// needs no unismoke tables. However, the usual practice is to
/// specify unismoke tables even when superfluous, and cso_table()
/// of course provides a complete set. Furthermore, while a product
/// may normally require sex-distinct rating, an exception is usually
/// made for MT, so the simple boolean pair is adequate in practice.
/// It would theoretically be possible to design a unisex or unismoke
/// product with smoker- or sex-distinct 7702 calculations, but that
/// seems most unlikely. Thus, the booleans may be read as meaning:
///   DB_Irc7702QAxisGender:  iff false, force unisex   for 7702
///   DB_Irc7702QAxisSmoking: iff false, force unismoke for 7702
/// but honor the underwriting distinction otherwise (iff true).
///
/// It is often convenient to provide a full set of 7702 q tables even
/// for products that cannot use some of them: ideally, a single set
/// of {1980, 2001, 2017} X {ALB, ANB} tables would then be shared by
/// all products. The presence of superfluous tables is not anomalous,
/// and product verification need take no note of it. (It is generally
/// not possible to share 7PP and corridor tables tables across all
/// products, though, because those tables depend on maturity age.)

void verify_products()
{
    std::vector<std::string> const& products = ce_product_name().all_strings();
    for(auto const& z : products)
        {
        std::cout << "Testing product " << z << '\n';
        for(auto const& g : all_strings<mcenum_gender>())
            {
            for(auto const& s : all_strings<mcenum_smoking>())
                {
                verify_one_cell(z, g, s);
                }
            }
        }
    std::cout << std::endl;
}
