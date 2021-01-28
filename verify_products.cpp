// Verify product files.
//
// Copyright (C) 2019, 2020, 2021 Gregory W. Chicares.
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
class product_verifier
{
  public:
    product_verifier
        (std::string const& product_name
        ,std::string const& gender_str
        ,std::string const& smoking_str
        );
    void verify();

  private:
    void verify_7702q();

    std::string      const  product_name_;
    std::string      const  gender_str_  ;
    std::string      const  smoking_str_ ;
    product_data     const& p_           ;
    mcenum_gender    const  gender_      ;
    mcenum_smoking   const  smoking_     ;
    product_database const  db0_         ;
    int              const  min_age_     ;
    product_database const  db_          ;
    mcenum_cso_era   const  era_         ;
    oenum_alb_or_anb const  a_b_         ;
    int              const  t_           ;
    bool             const  axis_g_      ;
    bool             const  axis_s_      ;
    int              const  omega_       ;
};

product_verifier::product_verifier
    (std::string const& product_name
    ,std::string const& gender_str
    ,std::string const& smoking_str
    )
    :product_name_ {product_name}
    ,gender_str_   {gender_str}
    ,smoking_str_  {smoking_str}
    ,p_            (*product_data::read_via_cache(filename_from_product_name(product_name)))
    ,gender_       {mce_gender (gender_str ).value()}
    ,smoking_      {mce_smoking(smoking_str).value()}
    ,db0_
        (product_name
        ,gender_
        ,mce_standard
        ,smoking_
        ,45
        ,mce_medical
        ,mce_s_XX
        )
    ,min_age_      {db0_.query<int>(DB_MinIssAge)}
    ,db_
        (product_name
        ,gender_
        ,mce_standard
        ,smoking_
        ,min_age_
        ,mce_medical
        ,mce_s_XX
        )
    ,era_          {db_.query<mcenum_cso_era  >(DB_CsoEra)}
    ,a_b_          {db_.query<oenum_alb_or_anb>(DB_AgeLastOrNearest)}
    ,t_            {db_.query<int             >(DB_Irc7702QTable)}
    ,axis_g_       {db_.query<bool            >(DB_Irc7702QAxisGender)}
    ,axis_s_       {db_.query<bool            >(DB_Irc7702QAxisSmoking)}
    ,omega_        {db_.query<int             >(DB_MaturityAge)}
{
}

void product_verifier::verify()
{
    if
        (   (!axis_g_ && mce_unisex   != gender_ )
        ||  (!axis_s_ && mce_unismoke != smoking_)
        )
        {
        std::cout
            << "  skipping"
            << ' ' << gender_str_
            << ' ' << smoking_str_
            << std::endl
            ;
        return;
        }

    verify_7702q();
}

void product_verifier::verify_7702q()
{
    switch(db_.query<oenum_7702_q_whence>(DB_Irc7702QWhence))
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
                (era_
                ,oe_orthodox // No other option currently supported for 7702.
                ,a_b_
                ,gender_
                ,smoking_
                ,min_age_
                ,omega_
                );
            std::vector<double> const v1 = irc_7702_q
                (p_
                ,db_
                ,min_age_
                ,omega_ - min_age_
                );
            std::cout
                << "7702 q okay: builtin "
                << std::string((v0 == v1) ? "validated" : "PROBLEM")
                << ' ' << gender_str_
                << ' ' << smoking_str_
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
            if(0 == t_)
                {
                std::cout
                    << "7702 q PROBLEM: " << product_name_
                    << " nonexistent table zero"
                    << ' ' << gender_str_
                    << ' ' << smoking_str_
                    << std::endl
                    ;
                return;
                }

            std::vector<double> const v0 = cso_table
                (era_
                ,oe_orthodox // No other option currently supported for 7702.
                ,a_b_
                ,gender_
                ,smoking_
                );
            std::string const f = AddDataDir(p_.datum("Irc7702QFilename"));
            actuarial_table const a(f, t_);
            std::vector<double> const v1 = a.values
                (a.min_age()
                ,1 + a.max_age() - a.min_age()
                );

            if(v0 == v1)
                {
                std::cout
                    << "7702 q okay: table " << t_
                    << ' ' << gender_str_
                    << ' ' << smoking_str_
                    << std::endl
                    ;
                }
            else
                {
                std::cout
                    << "7702 q PROBLEM: " << product_name_
                    << ' ' << gender_str_
                    << ' ' << smoking_str_
                    << std::endl
                    ;
                std::cout
                    << "\n  CSO era: " << era_
                    << "\n  ALB or ANB: " << a_b_
                    << "\n  table file: " << f
                    << "\n  table number: " << t_
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
    for(auto const& p : products)
        {
        std::cout << "Testing product " << p << '\n';
        for(auto const& g : all_strings<mcenum_gender>())
            {
            for(auto const& s : all_strings<mcenum_smoking>())
                {
                product_verifier(p, g, s).verify();
                }
            }
        }
    std::cout << std::endl;
}
