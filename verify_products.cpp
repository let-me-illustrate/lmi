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
    input["UseDOB"     ] = "No"; // Prevent Reconcile() from changing age.
    input["IssueAge"   ] = value_cast<std::string>(min_age);

#if 0
    input.Reconcile();
    LMI_ASSERT(product_name == input["ProductName"].str());
    LMI_ASSERT(gender       == input["Gender"     ].str());
    LMI_ASSERT(smoking      == input["Smoking"    ].str());
    LMI_ASSERT("No"         == input["UseDOB"     ].str());
    LMI_ASSERT(min_age      == input.issue_age());
#endif // 0

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

    input.Reconcile();
    if(product_name != input["ProductName"].str())
        std::cout << product_name << " != " << input["ProductName"].str() << std::endl;
    if(gender       != input["Gender"     ].str())
        std::cout << gender       << " != " << input["Gender"     ].str() << std::endl;
    if(smoking      != input["Smoking"    ].str())
        std::cout << smoking      << " != " << input["Smoking"    ].str() << std::endl;
    if("No"         != input["UseDOB"     ].str())
        std::cout << "No"         << " != " << input["UseDOB"     ].str() << std::endl;
    if(min_age      != input.issue_age()         )
        std::cout << min_age      << " != " << input.issue_age() << std::endl;

    // These four distinct diagnostics are observed with about a
    // hundred real-world products:
    //
    // Female != Unisex
    // Male != Unisex
    // Unisex != Female
    // Unismoke != Smoker
    //
    // All seem harmless:
    //
    // female or male forced to unisex: This occurs when
    //   Add({DB_Irc7702QAxisGender  , true});
    //   Add({DB_AllowSexDistinct    , false});
    //   Add({DB_AllowUnisex         , true});
    // i.e., for products that are issued only on a unisex basis,
    // but which have 7702 tables for male and female as well as
    // unisex, merely because it seemed expedient to copy the whole
    // multidimensional set. The sex-distinct tables are superfluous
    // and will never be read, so this oddity is benign.
    //
    // unisex forced to female: This occurs when
    //   Add({DB_Irc7702QAxisGender  , true});
    //   Add({DB_AllowSexDistinct    , [everywhere but MT]});
    //   Add({DB_AllowUnisex         , false});
    // so that all insureds are either male or female (except in MT),
    // while unisex is forbidden (even in MT--so such a product cannot
    // be issued in MT at all), yet unisex 7702 tables are provided.
    // Those extra (unisex) tables are superfluous but harmless.
    //
    // unismoke forced to smoker: This occurs when
    //   Add({DB_Irc7702QAxisSmoking , true});
    //   Add({DB_AllowSmokeDistinct  , true});
    //   Add({DB_AllowUnismoke       , [conditionally true or false]});
    // so that all insureds are either smoker or nonsmoker (except,
    // say, for guaranteed-issue underwriting), and in the particular
    // scenario tested (say, medical underwriting), unisex is entered
    // but forced to change to a different classification (smoker,
    // only because it's the first available enumerator). Unisex 7702
    // rates are provided, and might be required in a different
    // scenario, so all is well.
    //
    // Conclusions.
    //
    // Reconcile() should not be called here at all. Products have
    // two different sets of gender axes, one for underwriting, and
    // another for 7702. Those axes needn't be the same: e.g., a
    // product may be issued only on a sex-distinct basis, yet use
    // unisex 7702 tables--to stay within IRS Notice 88-128's safe
    // harbor, or to use more liberal rates for one market segment
    // at the cost of disadvantaging another. The smoking axes may
    // differ likewise. Calling Reconcile() enforces the underwriting
    // axes, but it is the 7702 axes that are tested here.
    //
    // DB_Irc7702QAxisGender and DB_Irc7702QAxisSmoking are not
    // adequate to describe all permissible variations; arguably
    // something beyond that simple boolean pair would be desirable,
    // e.g., because they cannot represent a smoker-distinct-only
    // product that needs no unismoke tables. In practice, though,
    // unismoke tables generally are provided, even when superfluous.
    // Furthermore, while some products may not allow unisex rating,
    // an exception is usually made for MT, so a more complicated
    // paradigm does not seem practically necessary.
    //
    // It is often convenient to provide a full set of 7702 q tables
    // even for a product that cannot use some of them: ideally, a
    // single set of {1980, 2001, 2017} X {ALB, ANB} tables could
    // then be shared by all products. The presence of superfluous
    // tables is not an anomaly, and product verification need take
    // no note of it. (It is not generally possible to share 7PP and
    // corridor tables tables across products, though, because those
    // tables depend on maturity age.)

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
