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
#include "ce_product_name.hpp"
#include "cso_table.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "database.hpp"
#include "input.hpp"
#include "mc_enum.hpp"                  // all_strings<>()
#include "product_data.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"
#include "yare_input.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
void verify_one_cell
    (std::string const& product_name
    ,std::string const& gender
    ,std::string const& smoking
    )
{
    Input input;
    input["ProductName"] = product_name;
    input["Gender"     ] = gender;
    input["Smoking"    ] = smoking;

    int const min_age = product_database(yare_input(input)).query<int>(DB_MinIssAge);
    input["IssueAge"   ] = value_cast<std::string>(min_age);

    yare_input       const yi(input);
    product_database const db(yi);
    auto const era    = db.query<oenum_cso_era   >(DB_CsoEra);
    auto const a_b    = db.query<oenum_alb_or_anb>(DB_AgeLastOrNearest);
    auto const t      = db.query<int             >(DB_Irc7702QTable);
    auto const axis_g = db.query<bool            >(DB_Irc7702QAxisGender);
    auto const axis_s = db.query<bool            >(DB_Irc7702QAxisSmoking);

    if((!axis_g && "Unisex" != gender) || (!axis_s && "Unismoke" != smoking))
        {
        std::cout << "  skipping " << gender << ' ' << smoking << std::endl;
        return;
        }

    if(0 == t)
        {
        std::cout
            << "7702 q PROBLEM: " << product_name
            << " nonexistent table zero"
            << ' ' << gender
            << ' ' << smoking
            << std::endl
            ;
        return;
        }

    std::vector<double> const v0 = cso_table
        (era
        ,oe_orthodox
        ,a_b
        ,mce_gender (gender).value()
        ,mce_smoking(smoking).value()
        );
    product_data const p(product_name);
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
            << ' ' << gender
            << ' ' << smoking
            << std::endl
            ;
        }
    else
        {
        std::cout
            << "7702 q PROBLEM: " << product_name
            << ' ' << gender
            << ' ' << smoking
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
} // Unnamed namespace.

/// Verify all products.
///
/// Class product_database (with the instance of class database_index
/// that it contains) suffices for retrieving data tabular data from
/// internal sources such as cso_table(). Class product_data provides
/// the names of tables stored in external database files.
///
/// For now at least, class Input is used merely as a convenient means
/// of instantiating the necessary product_database object. No MVC
/// Model function such as Reconcile() need be called to ensure the
/// internal consistency of the Input instance.
///
/// Only 7702 tables are validated for now. Products have two distinct
/// sets of gender axes: one for underwriting, and another for 7702
/// Those axes needn't be the same. For example, a product might be
/// issued only on a sex-distinct basis, yet use unisex 7702 tables
/// (to stay within IRS Notice 88-128's safe harbor, or to use more
/// liberal rates for one market segment at the cost of disadvantaging
/// another. The smoking axes may differ likewise. Calling Reconcile()
/// on an Input object enforces the underwriting axes, but it is the
/// 7702 axes that are tested here.
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
