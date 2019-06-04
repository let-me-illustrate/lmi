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
//#include "assert_lmi.hpp"
#include "ce_product_name.hpp"
#include "cso_table.hpp"
#include "data_directory.hpp"           // AddDataDir()
#include "database.hpp"
#include "input.hpp"
#include "irc7702_tables.hpp"
#include "mc_enum.hpp"                  // all_strings<>()
#include "product_data.hpp"
#include "ssize_lmi.hpp"
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
    yare_input       const yi(input);
    product_database const db(yi);
    auto const era    = db.query<oenum_cso_era   >(DB_CsoEra);
    auto const a_b    = db.query<oenum_alb_or_anb>(DB_AgeLastOrNearest);
    auto const t      = db.query<int             >(DB_Irc7702QTable);
    auto const axis_g = db.query<bool            >(DB_Irc7702QAxisGender);
    auto const axis_s = db.query<bool            >(DB_Irc7702QAxisSmoking);
    if
        (  0 == t
        || (!axis_g && "Unisex"   != gender)
        || (!axis_s && "Unismoke" != smoking)
        )
        {
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

//  CvatCorridorFilename
//  DB_CorridorTable
    std::string const f7pp = AddDataDir(p.datum("SevenPayFilename"));
    auto const t7pp        = db.query<int>   (DB_SevenPayTable);
    if(!t7pp)
        {
        std::cout
            << "7pp table 0: " << product_name
            << ' ' << gender
            << ' ' << smoking
            << std::endl
            ;
        return;
        }
    actuarial_table const a7pp(f7pp, t7pp);
    std::vector<double> const v1_7pp = a7pp.values
        (a7pp.min_age()
        ,1 + a7pp.max_age() - a7pp.min_age()
        );

    if(db.query<int>(DB_MinIssAge  ) !=     a7pp.min_age())
        std::cout << "  7pp table " << t7pp << ": min age discrepancy" << std::endl;
    if(db.query<int>(DB_MaturityAge) != 1 + a7pp.max_age())
        std::cout << "  7pp table " << t7pp << ": max age discrepancy" << std::endl;

    std::vector<double> ig;
    db.query_into(DB_NaarDiscount, ig);
    irc7702_tables z
        (era
        ,oe_orthodox
        ,a_b
        ,mce_gender (gender).value()
        ,mce_smoking(smoking).value()
        ,ig
        ,1.0 / db.query<double>(DB_MaxMonthlyCoiRate)
        ,a7pp.min_age()//,db.query<int   >(DB_MinIssAge)
        ,1 + a7pp.max_age()//,db.query<int   >(DB_MaturityAge)
        );
//  std::vector<double> const& ul_corr = z.ul_corr();
//  std::vector<double> const& ul_7pp  = z.ul_7pp ();
//  std::vector<double> const& ol_corr = z.ol_corr();
    std::vector<double> const& ol_7pp  = z.ol_7pp ();

    std::cout
        << "7pp table " << t7pp
        << ' ' << lmi::ssize(v1_7pp)
        << ' ' << lmi::ssize(ol_7pp)
        << ' ' << ((lmi::ssize(v1_7pp) != lmi::ssize(ol_7pp)) ? "DIFF" : "SAME")
        << std::endl
        ;
    int length = std::min(lmi::ssize(ol_7pp), lmi::ssize(v1_7pp));
//  LMI_ASSERT(lmi::ssize(ol_7pp) == lmi::ssize(v1_7pp));
    for(int j = 0; j < length; ++j)
        {
        if(v1_7pp[j] <= ol_7pp[j])
            continue;
        std::cout
            << "7pp error:"
            << " table " << t7pp
            << ' ' << product_name
            << ' ' << gender
            << ' ' << smoking
            << " omega " << 1 + a7pp.max_age()
            << std::endl
            << " dur " << j
            << " tabular " << v1_7pp[j]
            << " exceeds calculated " << ol_7pp[j]
            << std::endl
            ;
        return;
        }
    std::cout
        << "7pp okay: table " << t7pp
        << ' ' << gender
        << ' ' << smoking
        << std::endl
        ;
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
