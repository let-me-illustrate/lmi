// Basic tables.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#include "basic_tables.hpp"

#include "actuarial_table.hpp"
#include "assert_lmi.hpp"
#include "cso_table.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"                  // e_database_key
#include "oecumenic_enumerations.hpp"
#include "product_data.hpp"

#if 0 // For debugging only.
#include "alert.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#endif // 0

#include <string>

namespace
{
std::vector<double> basic_table_rates
    (product_data     const& product
    ,product_database const& database
    ,std::string      const& table_name
    ,e_database_key          table_id
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
#if 1
    return actuarial_table_rates
        (AddDataDir(product.datum(table_name))
        ,database.query<int>(table_id)
        ,issue_age
        ,years_to_maturity
        );
#else // For debugging only. "Unsuitable for production" as explained below.
    std::string const f = AddDataDir(product.datum(table_name));
    int const t = database.query<int>(table_id);
    try
        {
        return actuarial_table_rates(f, t, issue_age, years_to_maturity);
        }
    catch(...)
        {
        report_exception();
        if(0 == t)
            {
            warning() << "Table number zero is always invalid." << LMI_FLUSH;
            }
        else
            {
            actuarial_table const a(f, t);
            warning()
                << "'" << f << "' rate-table database\n"
                << t << " table number\n"
                << issue_age << " issue_age argument\n"
                << years_to_maturity << " years_to_maturity argument\n"
                << a.min_age() << " table minimum age\n"
                << a.max_age() << " table maximum age\n"
                << LMI_FLUSH
                ;
            }
        // Unsuitable for production, which must throw instead.
        return std::vector<double>(years_to_maturity);
        }
#endif // 0
}

std::vector<double> irc_7702_q_builtin
    (product_database const& db
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    auto const era    = db.query<mcenum_cso_era  >(DB_CsoEra);
    auto const a_b    = db.query<oenum_alb_or_anb>(DB_AgeLastOrNearest);
    auto const axis_g = db.query<bool            >(DB_Irc7702QAxisGender);
    auto const axis_s = db.query<bool            >(DB_Irc7702QAxisSmoking);
    auto const omega  = db.query<int             >(DB_MaturityAge);
    LMI_ASSERT(db.index().issue_age() == issue_age);
    LMI_ASSERT(omega - issue_age == years_to_maturity);

    // Use 7702 axes for gender and smoker--for 7702, not guar mort.
    return cso_table
        (era
        ,oe_orthodox // No other option currently supported for 7702.
        ,a_b
        ,axis_g ? db.index().gender () : mce_unisex
        ,axis_s ? db.index().smoking() : mce_unismoke
        ,issue_age
        ,omega
        );
}
} // Unnamed namespace.

std::vector<double> irc_7702_q
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    switch(database.query<oenum_7702_q_whence>(DB_Irc7702QWhence))
        {
        case oe_7702_q_builtin:
            {
            return irc_7702_q_builtin
                (database
                ,issue_age
                ,years_to_maturity
                );
            }
        case oe_7702_q_external_table:
            {
            return basic_table_rates
                (product
                ,database
                ,"Irc7702QFilename"
                ,DB_Irc7702QTable
                ,issue_age
                ,years_to_maturity
                );
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

std::vector<double> irc_7702A_7pp
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    switch(database.query<oenum_7702_7pp_whence>(DB_SevenPayWhence))
        {
        case oe_7702_7pp_first_principles:
            {
            throw "Not yet implemented (and don't forget rounding).";
            }
        case oe_7702_7pp_from_table:
            {
            return basic_table_rates
                (product
                ,database
                ,"SevenPayFilename"
                ,DB_SevenPayTable
                ,issue_age
                ,years_to_maturity
                );
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

std::vector<double> irc_7702_nsp
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    switch(database.query<oenum_7702_nsp_whence>(DB_Irc7702NspWhence))
        {
        case oe_7702_nsp_first_principles:
            {
            throw "Not yet implemented (and don't forget rounding).";
            }
        case oe_7702_nsp_from_table:
            {
            return basic_table_rates
                (product
                ,database
                ,"Irc7702NspFilename"
                ,DB_Irc7702NspTable
                ,issue_age
                ,years_to_maturity
                );
            }
        case oe_7702_nsp_reciprocal_cvat_corridor:
            {
            throw "Not yet implemented (and don't forget rounding).";
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

std::vector<double> cvat_corridor_factors
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    switch(database.query<oenum_7702_corr_whence>(DB_CorridorWhence))
        {
        case oe_7702_corr_first_principles:
            {
            throw "Not yet implemented (and don't forget rounding).";
            }
        case oe_7702_corr_from_table:
            {
            return basic_table_rates
                (product
                ,database
                ,"CvatCorridorFilename"
                ,DB_CorridorTable
                ,issue_age
                ,years_to_maturity
                );
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

std::vector<double> target_premium_rates
    (product_data     const& product
    ,product_database const& database
    ,int                     issue_age
    ,int                     years_to_maturity
    )
{
    switch(database.query<oenum_modal_prem_type>(DB_TgtPremType))
        {
        case oe_monthly_deduction:
            {
            throw "Not yet implemented (and don't forget rounding).";
            }
        case oe_modal_nonmec:
            {
            return irc_7702A_7pp
                (product
                ,database
                ,issue_age
                ,years_to_maturity
                );
            }
        case oe_modal_table:
            {
            return basic_table_rates
                (product
                ,database
                ,"TgtPremFilename"
                ,DB_TgtPremTable
                ,issue_age
                ,years_to_maturity
                );
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}
