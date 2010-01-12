// MEC-testing server.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

// $Id: mec_server.cpp,v 1.6 2009-10-03 18:13:40 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_server.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "basic_values.hpp"          // lowest_premium_tax_load()
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "et_vector.hpp"
#include "ihs_commfns.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_proddata.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "mec_input.hpp"
#include "mec_xml_document.hpp"
#include "miscellany.hpp"            // ios_out_trunc_binary()
#include "oecumenic_enumerations.hpp"
#include "path_utility.hpp"          // fs::path inserter
#include "round_to.hpp"
#include "stratified_algorithms.hpp" // TieredGrossToNet()
#include "stratified_charges.hpp"
#include "timer.hpp"
#include "value_cast.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <algorithm>                 // std::min()
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{
mec_state test_one_days_7702A_transactions
    (fs::path  const& file_path
    ,mec_input const& input
    )
{
    bool                        Use7702ATables               = exact_cast<mce_yes_or_no           >(input["Use7702ATables"              ])->value();
//  int                         IssueAge                     = exact_cast<tnr_issue_age           >(input["IssueAge"                    ])->value();
    mcenum_gender               Gender                       = exact_cast<mce_gender              >(input["Gender"                      ])->value();
    mcenum_smoking              Smoking                      = exact_cast<mce_smoking             >(input["Smoking"                     ])->value();
    mcenum_class                UnderwritingClass            = exact_cast<mce_class               >(input["UnderwritingClass"           ])->value();
//  calendar_date               DateOfBirth                  = exact_cast<tnr_date                >(input["DateOfBirth"                 ])->value();
//  mcenum_table_rating         SubstandardTable             = exact_cast<mce_table_rating        >(input["SubstandardTable"            ])->value();
    std::string                 ProductName                  = exact_cast<ce_product_name         >(input["ProductName"                 ])->value();
    double                      External1035ExchangeAmount   = exact_cast<tnr_nonnegative_double  >(input["External1035ExchangeAmount"  ])->value();
//  bool                        External1035ExchangeFromMec  = exact_cast<mce_yes_or_no           >(input["External1035ExchangeFromMec" ])->value();
    double                      Internal1035ExchangeAmount   = exact_cast<tnr_nonnegative_double  >(input["Internal1035ExchangeAmount"  ])->value();
//  bool                        Internal1035ExchangeFromMec  = exact_cast<mce_yes_or_no           >(input["Internal1035ExchangeFromMec" ])->value();
//  calendar_date               EffectiveDate                = exact_cast<tnr_date                >(input["EffectiveDate"               ])->value();
    mcenum_defn_life_ins        DefinitionOfLifeInsurance    = exact_cast<mce_defn_life_ins       >(input["DefinitionOfLifeInsurance"   ])->value();
    mcenum_defn_material_change DefinitionOfMaterialChange   = exact_cast<mce_defn_material_change>(input["DefinitionOfMaterialChange"  ])->value();
    mcenum_uw_basis             GroupUnderwritingType        = exact_cast<mce_uw_basis            >(input["GroupUnderwritingType"       ])->value();
//  std::string                 Comments                     = exact_cast<datum_string            >(input["Comments"                    ])->value();
    int                         InforceYear                  = exact_cast<tnr_duration            >(input["InforceYear"                 ])->value();
    int                         InforceMonth                 = exact_cast<tnr_month               >(input["InforceMonth"                ])->value();
    double                      InforceTargetSpecifiedAmount = exact_cast<tnr_nonnegative_double  >(input["InforceTargetSpecifiedAmount"])->value();
    double                      InforceAccountValue          = exact_cast<tnr_nonnegative_double  >(input["InforceAccountValue"         ])->value();
//  calendar_date               InforceAsOfDate              = exact_cast<tnr_date                >(input["InforceAsOfDate"             ])->value();
    bool                        InforceIsMec                 = exact_cast<mce_yes_or_no           >(input["InforceIsMec"                ])->value();
//  calendar_date               LastMaterialChangeDate       = exact_cast<tnr_date                >(input["LastMaterialChangeDate"      ])->value();
    double                      InforceDcv                   = exact_cast<tnr_nonnegative_double  >(input["InforceDcv"                  ])->value();
    double                      InforceAvBeforeLastMc        = exact_cast<tnr_nonnegative_double  >(input["InforceAvBeforeLastMc"       ])->value();
    int                         InforceContractYear          = exact_cast<tnr_duration            >(input["InforceContractYear"         ])->value();
    int                         InforceContractMonth         = exact_cast<tnr_month               >(input["InforceContractMonth"        ])->value();
    double                      InforceLeastDeathBenefit     = exact_cast<tnr_nonnegative_double  >(input["InforceLeastDeathBenefit"    ])->value();
    mcenum_state                StateOfJurisdiction          = exact_cast<mce_state               >(input["StateOfJurisdiction"         ])->value();
//  std::string                 FlatExtra                    = exact_cast<datum_sequence          >(input["FlatExtra"                   ])->value();
//  std::string                 PaymentHistory               = exact_cast<datum_sequence          >(input["PaymentHistory"              ])->value();
//  std::string                 BenefitHistory               = exact_cast<datum_sequence          >(input["BenefitHistory"              ])->value();
//  bool                        DeprecatedUseDOB             = exact_cast<mce_yes_or_no           >(input["DeprecatedUseDOB"            ])->value();
    double                      Payment                      = exact_cast<tnr_nonnegative_double  >(input["Payment"                     ])->value();
    double                      BenefitAmount                = exact_cast<tnr_nonnegative_double  >(input["BenefitAmount"               ])->value();

    TProductData product_data(ProductName);

    TDatabase database
        (ProductName
        ,Gender
        ,UnderwritingClass
        ,Smoking
        ,input.issue_age()
        ,GroupUnderwritingType
        ,StateOfJurisdiction
        );

    stratified_charges stratified(AddDataDir(product_data.GetTierFilename()));

    // SOMEDAY !! Ideally these would be in the GUI (or read from product files).
    round_to<double> const RoundNonMecPrem(2, r_downward);
    round_to<double> const round_max_premium(2, r_downward);

    oenum_modal_prem_type const target_premium_type =
        static_cast<oenum_modal_prem_type>(static_cast<int>(database.Query(DB_TgtPremType)));
    std::vector<double> TargetPremiumRates(input.years_to_maturity());
    if(oe_modal_table == target_premium_type)
        {
        TargetPremiumRates = actuarial_table_rates
            (AddDataDir(product_data.GetTgtPremFilename())
            ,static_cast<long int>(database.Query(DB_TgtPremTable))
            ,input.issue_age()
            ,input.years_to_maturity()
            );
        }
    else
        {
        ; // Do nothing: 'TargetPremiumRates' won't be used.
        }

    std::vector<double> const CvatCorridorFactors = actuarial_table_rates
        (AddDataDir(product_data.GetCorridorFilename())
        ,static_cast<long int>(database.Query(DB_CorridorTable))
        ,input.issue_age()
        ,input.years_to_maturity()
        );

    std::vector<double> tabular_Ax;
    for(int j = 0; j < input.years_to_maturity(); ++j)
        {
        LMI_ASSERT(0.0 < CvatCorridorFactors[j]);
        tabular_Ax.push_back(1.0 / CvatCorridorFactors[j]);
        }
    tabular_Ax.push_back(1.0);

    std::vector<double> const tabular_7Px = actuarial_table_rates
        (AddDataDir(product_data.GetTAMRA7PayFilename())
        ,static_cast<long int>(database.Query(DB_TAMRA7PayTable))
        ,input.issue_age()
        ,input.years_to_maturity()
        );

    std::vector<double> Mly7702qc = actuarial_table_rates
        (AddDataDir(product_data.GetIRC7702Filename())
        ,static_cast<long int>(database.Query(DB_IRC7702QTable))
        ,input.issue_age()
        ,input.years_to_maturity()
        );
    double const max_coi_rate = database.Query(DB_MaxMonthlyCoiRate);
    // ET !! Mly7702qc = coi_rate_from_q(Mly7702qc, Database_->Query(DB_MaxMonthlyCoiRate));
    assign(Mly7702qc, apply_binary(coi_rate_from_q<double>(), Mly7702qc, max_coi_rate));

    std::vector<double> guar_int;
    database.Query(guar_int, DB_GuarInt);

    std::vector<double> const spread
        (input.years_to_maturity()
        ,stratified.minimum_tiered_spread_for_7702()
        );

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(.04, guar_int) - spread);
    std::vector<double> Mly7702iGlp(input.years_to_maturity());
    assign
        (Mly7702iGlp
        ,apply_unary
            (i_upper_12_over_12_from_i<double>()
            ,apply_binary(greater_of<double>(), 0.04, guar_int) - spread
            )
        );

    std::vector<double> DBDiscountRate;
    database.Query(DBDiscountRate, DB_NAARDiscount);
    // ET !! Mly7702ig = -1.0 + 1.0 / DBDiscountRate;
    std::vector<double> Mly7702ig(input.years_to_maturity());
    assign(Mly7702ig, -1.0 + 1.0 / DBDiscountRate);

    // Use zero if that's the guaranteed rate; else use the statutory rate.
    // ET !! Use each_equal() here because PETE seems to interfere with
    // the normal operator==(). Is that a PETE defect?
    std::vector<double> const zero(input.years_to_maturity(), 0.0);
    std::vector<double> const& naar_disc_rate =
          each_equal(Mly7702ig.begin(), Mly7702ig.end(), 0.0)
        ? zero
        : Mly7702iGlp
        ;
    ULCommFns commfns
        (Mly7702qc
        ,Mly7702iGlp
        ,naar_disc_rate
        ,mce_option1
        ,mce_monthly
        );

    std::vector<double> analytic_Ax(input.years_to_maturity());
    analytic_Ax += (commfns.kM() + commfns.aD().back()) / commfns.aD();

    std::vector<double> E7aN(commfns.aN());
    E7aN.insert(E7aN.end(), 7, 0.0);
    E7aN.erase(E7aN.begin(), 7 + E7aN.begin());
    std::vector<double> analytic_7Px(input.years_to_maturity());
    analytic_7Px += (commfns.kM() + commfns.aD().back()) / (commfns.aN() - E7aN);

    std::vector<double> const& chosen_Ax  = Use7702ATables ? tabular_Ax  : analytic_Ax ;
    std::vector<double> const& chosen_7Px = Use7702ATables ? tabular_7Px : analytic_7Px;

    Irc7702A z
        (0
        ,DefinitionOfLifeInsurance
        ,DefinitionOfMaterialChange
        ,false // Survivorship: hardcoded for now.
        ,mce_allow_mec
        ,true  // Use table for 7pp: hardcoded for now.
        ,true  // Use table for NSP: hardcoded for now.
        ,chosen_7Px
        ,chosen_Ax
        ,RoundNonMecPrem
        );

    z.Initialize7702A
        (false       // a_Ignore
        ,InforceIsMec
        ,input.issue_age()
        ,input.maturity_age()
        ,InforceYear
        ,InforceMonth
        ,InforceContractYear
        ,InforceContractMonth
        ,InforceAvBeforeLastMc
        ,InforceLeastDeathBenefit
        ,input.PaymentHistoryRealized()
        ,input.BenefitHistoryRealized()
        );
    z.UpdateBOY7702A(InforceYear);
    z.UpdateBOM7702A(InforceMonth);

    // See the implementation of class BasicValues.
    long double const epsilon_plus_one =
        1.0L + std::numeric_limits<long double>::epsilon()
        ;

    double AnnualTargetPrem = 1000000000.0; // No higher premium is anticipated.
    int const target_year =
          database.Query(DB_TgtPmFixedAtIssue)
        ? 0
        : input.inforce_year()
        ;
    if(oe_monthly_deduction == target_premium_type)
        {
        warning() << "Unsupported modal premium type." << LMI_FLUSH;
        }
    else if(oe_modal_nonmec == target_premium_type)
        {
        // When 7Px is calculated from first principles, presumably
        // the target premium should be the same as for oe_modal_table
        // with a 7Px table and a DB_TgtPremPolFee of zero.
        AnnualTargetPrem = round_max_premium
            (   InforceTargetSpecifiedAmount
            *   epsilon_plus_one
            *   tabular_7Px[target_year]
            );
        }
    else if(oe_modal_table == target_premium_type)
        {
        AnnualTargetPrem = round_max_premium
            (   database.Query(DB_TgtPremPolFee)
            +       InforceTargetSpecifiedAmount
                *   epsilon_plus_one
                *   TargetPremiumRates[target_year]
            );
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << target_premium_type << '.'
            << LMI_FLUSH
            ;
        }

    double const premium_tax_load = lowest_premium_tax_load
        (database
        ,stratified
        ,StateOfJurisdiction
        ,false
        );

    std::vector<double> target_sales_load  ;
    std::vector<double> excess_sales_load  ;
    std::vector<double> target_premium_load;
    std::vector<double> excess_premium_load;
    std::vector<double> dac_tax_load       ;

    database.Query(target_sales_load  , DB_CurrPremLoadTgtRfd);
    database.Query(excess_sales_load  , DB_CurrPremLoadExcRfd);
    database.Query(target_premium_load, DB_CurrPremLoadTgt);
    database.Query(excess_premium_load, DB_CurrPremLoadExc);
    database.Query(dac_tax_load       , DB_DACTaxPremLoad);

    double const LoadTarget = target_sales_load[InforceYear] + target_premium_load[InforceYear] + dac_tax_load[InforceYear] + premium_tax_load;
    double const LoadExcess = excess_sales_load[InforceYear] + excess_premium_load[InforceYear] + dac_tax_load[InforceYear] + premium_tax_load;

    LMI_ASSERT(static_cast<unsigned int>(InforceContractYear) < input.BenefitHistoryRealized().size());
    double const old_benefit_amount = input.BenefitHistoryRealized()[InforceContractYear];

    double const total_1035_amount = round_max_premium
        (TieredGrossToNet
            (External1035ExchangeAmount + Internal1035ExchangeAmount
            ,AnnualTargetPrem
            ,LoadTarget
            ,LoadExcess
            )
        );
    if(0.0 != total_1035_amount)
        {
        z.Update1035Exch7702A
            (InforceDcv
            ,total_1035_amount
            ,old_benefit_amount
            );
        InforceAccountValue = InforceDcv;
        }

    if(BenefitAmount != old_benefit_amount)
        {
        z.UpdateBft7702A
            (InforceDcv          // Not actually used.
            ,BenefitAmount
            ,old_benefit_amount
            ,false               // Ignored.
            ,BenefitAmount
            ,old_benefit_amount
            ,InforceAccountValue // Not actually used.
            );
        }

    double const max_necessary_premium = z.MaxNecessaryPremium
        (InforceDcv
        ,AnnualTargetPrem
        ,LoadTarget
        ,LoadExcess
        ,InforceAccountValue
        );
    z.MaxNonMecPremium
        (InforceDcv
        ,AnnualTargetPrem
        ,LoadTarget
        ,LoadExcess
        ,InforceAccountValue
        );
    double const necessary_premium = std::min(Payment, max_necessary_premium);
    double const unnecessary_premium = material_difference(Payment, necessary_premium);

    if(!z.IsMecAlready() && 0.0 != necessary_premium)
        {
        z.UpdatePmt7702A
            (InforceDcv
            ,necessary_premium
            ,false
            ,AnnualTargetPrem    // Unused.
            ,LoadTarget          // Unused.
            ,LoadExcess          // Unused.
            ,InforceAccountValue // Unused.
            );
        double const net_necessary_premium = round_max_premium
            (TieredGrossToNet
                (necessary_premium
                ,AnnualTargetPrem
                ,LoadTarget
                ,LoadExcess
                )
            );
        InforceDcv          += net_necessary_premium;
        InforceAccountValue += net_necessary_premium;
        }

    if(0.0 < unnecessary_premium)
        {
        z.InduceMaterialChange();
        }
    if(z.IsMaterialChangeInQueue())
        {
        z.RedressMatChg
            (InforceDcv
            ,unnecessary_premium
            ,necessary_premium
            ,InforceAccountValue
            );
        }

    if(!z.IsMecAlready() && 0.0 != unnecessary_premium)
        {
        z.UpdatePmt7702A
            (InforceDcv
            ,unnecessary_premium
            ,true
            ,AnnualTargetPrem    // Unused.
            ,LoadTarget          // Unused.
            ,LoadExcess          // Unused.
            ,InforceAccountValue // Unused.
            );
        }

    std::vector<double> ratio_Ax (input.years_to_maturity());
    ratio_Ax  += tabular_Ax  / analytic_Ax ;
    std::vector<double> ratio_7Px(input.years_to_maturity());
    ratio_7Px += tabular_7Px / analytic_7Px;

    configurable_settings const& c = configurable_settings::instance();
    std::string const extension(".mec" + c.spreadsheet_file_extension());
    fs::path spreadsheet_filename = fs::change_extension(file_path, extension);
    fs::ofstream ofs(spreadsheet_filename, ios_out_trunc_binary());
    ofs << "This temporary output file will be removed in a future release.\n";
    ofs
        << "t\t"
        << "ic\t"
        << "ig\t"
        << "qc\t"
        << "aD\t"
        << "kC\t"
        << "aN\t"
        << "kM\t"
        << "E7aN\t"
        << "Ax\t"
        << "tabular\t"
        << "ratio\t"
        << "7Px\t"
        << "tabular\t"
        << "ratio\t"
        << '\n'
        ;
    for(int j = 0; j < input.years_to_maturity(); ++j)
        {
        ofs
            <<               j  << '\t'
            << value_cast<std::string>(Mly7702iGlp    [j]) << '\t'
            << value_cast<std::string>(naar_disc_rate [j]) << '\t'
            << value_cast<std::string>(Mly7702qc      [j]) << '\t'
            << value_cast<std::string>(commfns.aD()   [j]) << '\t'
            << value_cast<std::string>(commfns.kC()   [j]) << '\t'
            << value_cast<std::string>(commfns.aN()   [j]) << '\t'
            << value_cast<std::string>(commfns.kM()   [j]) << '\t'
            << value_cast<std::string>(E7aN           [j]) << '\t'
            << value_cast<std::string>(analytic_Ax    [j]) << '\t'
            << value_cast<std::string>(tabular_Ax     [j]) << '\t'
            << value_cast<std::string>(ratio_Ax       [j]) << '\t'
            << value_cast<std::string>(analytic_7Px   [j]) << '\t'
            << value_cast<std::string>(tabular_7Px    [j]) << '\t'
            << value_cast<std::string>(ratio_7Px      [j]) << '\t'
            << '\n'
        ;
        }
    ofs
        << input.years_to_maturity()
        << "\t\t\t\t"
        << value_cast<std::string>(commfns.aD().back())
        << "\t\t\t\t\t\t\t\t\t\t\t"
        << '\n'
        ;
    if(!ofs)
        {
        warning()
            << "Unable to write '"
            << spreadsheet_filename
            << "'."
            << LMI_FLUSH
            ;
        }

    return z.state();
}
} // Unnamed namespace.

mec_server::mec_server(mcenum_emission emission)
    :emission_              (emission)
    ,usec_for_input_        (0.0)
    ,usec_for_calculations_ (0.0)
    ,usec_for_output_       (0.0)
{
}

mec_server::~mec_server()
{
}

bool mec_server::operator()(fs::path const& file_path)
{
    std::string const extension = fs::extension(file_path);
    if(".mec" == extension)
        {
        Timer timer;
        fs::ifstream ifs(file_path);
        if(!ifs)
            {
            fatal_error()
                << "Unable to read file '"
                << file_path
                << "'."
                << LMI_FLUSH
                ;
            }
        mec_xml_document doc;
        doc.read(ifs);
        usec_for_input_ = timer.stop().elapsed_usec();
        return operator()(file_path, doc.input_data());
        }
    else
        {
        fatal_error()
            << "File '"
            << file_path
            << "': extension '"
            << extension
            << "' not supported."
            << LMI_FLUSH
            ;
        return false;
        }
}

bool mec_server::operator()(fs::path const& file_path, mec_input const& z)
{
    Timer timer;
    state_ = test_one_days_7702A_transactions(file_path, z);
    usec_for_calculations_ = timer.stop().elapsed_usec();
    timer.restart();
    if(mce_emit_test_data && emission_)
        {
        state_.save(fs::change_extension(file_path, ".mec.xml"));
        }
    usec_for_output_       = timer.stop().elapsed_usec();
    conditionally_show_timings_on_stdout();
    return true;
}

void mec_server::conditionally_show_timings_on_stdout() const
{
    if(mce_emit_timings & emission_)
        {
        std::cout
            << "\n    Input:        "
            << Timer::elapsed_msec_str(usec_for_input_)
            << "\n    Calculations: "
            << Timer::elapsed_msec_str(usec_for_calculations_)
            << "\n    Output:       "
            << Timer::elapsed_msec_str(usec_for_output_)
            << '\n'
            ;
        }
}

mec_state mec_server::state() const
{
    return state_;
}

double mec_server::usec_for_input() const
{
    return usec_for_input_;
}

double mec_server::usec_for_calculations() const
{
    return usec_for_calculations_;
}

double mec_server::usec_for_output() const
{
    return usec_for_output_;
}

