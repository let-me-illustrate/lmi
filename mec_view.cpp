// Document view for MEC testing.
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

// $Id: mec_view.cpp,v 1.21 2009-07-27 20:05:43 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_view.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"          // lowest_premium_tax_load()
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "et_vector.hpp"
#include "handle_exceptions.hpp"
#include "ihs_commfns.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_proddata.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "mec_document.hpp"
#include "mec_input.hpp"
#include "miscellany.hpp"            // each_equal(), htmlize()
#include "mvc_controller.hpp"
#include "oecumenic_enumerations.hpp"
#include "safely_dereference_as.hpp"
#include "stratified_algorithms.hpp" // TieredGrossToNet()
#include "stratified_charges.hpp"
#include "timer.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <fstream>
#include <limits>
#include <sstream>
#include <vector>

mec_mvc_view::mec_mvc_view()
{
}

mec_mvc_view::~mec_mvc_view()
{
}

char const* mec_mvc_view::DoBookControlName() const
{
    return "mec_notebook";
}

char const* mec_mvc_view::DoMainDialogName() const
{
    return "dialog_containing_mec_notebook";
}

char const* mec_mvc_view::DoResourceFileName() const
{
    return "mec.xrc";
}

IMPLEMENT_DYNAMIC_CLASS(mec_view, ViewEx)

BEGIN_EVENT_TABLE(mec_view, ViewEx)
    EVT_MENU(XRCID("edit_cell"             ),mec_view::UponProperties)
    EVT_UPDATE_UI(wxID_SAVE                 ,mec_view::UponUpdateFileSave)
    EVT_UPDATE_UI(wxID_SAVEAS               ,mec_view::UponUpdateFileSaveAs)
    EVT_UPDATE_UI(XRCID("edit_cell"        ),mec_view::UponUpdateProperties)

// There has to be a better way to inhibit these inapplicable ids.
    EVT_UPDATE_UI(XRCID("edit_class"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"          ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),mec_view::UponUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),mec_view::UponUpdateInapplicable)
END_EVENT_TABLE()

mec_view::mec_view()
    :ViewEx       ()
    ,html_content_("Unable to display results.")
    ,html_window_ (0)
{
}

mec_view::~mec_view()
{
}

inline mec_input& mec_view::input_data()
{
    return *document().doc_.input_data_;
}

mec_document& mec_view::document() const
{
    return safely_dereference_as<mec_document>(GetDocument());
}

wxWindow* mec_view::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

int mec_view::EditProperties()
{
    bool dirty = document().IsModified();
    mec_input edited_input = input_data();
    mec_mvc_view const v;
    MvcController controller(GetFrame(), edited_input, v);
    int rc = controller.ShowModal();
    if(wxID_OK == rc)
        {
        if(edited_input != input_data())
            {
            input_data() = edited_input;
            dirty = true;
            }
        document().Modify(dirty);
        }
    return rc;
}

wxIcon mec_view::Icon() const
{
    return IconFromXmlResource("mec_view_icon");
}

wxMenuBar* mec_view::MenuBar() const
{
    return MenuBarFromXmlResource("mec_view_menu");
}

/// This virtual function calls its base-class namesake explicitly.
///
/// Trap any exception thrown by EditProperties() to ensure that this
/// function returns 'false' on failure, lest wx's doc-view framework
/// create a zombie view. See:
///   http://lists.nongnu.org/archive/html/lmi/2008-12/msg00017.html

bool mec_view::OnCreate(wxDocument* doc, long int flags)
{
    bool has_view_been_created = false;
    try
        {
        if(wxID_OK != EditProperties())
            {
            return has_view_been_created;
            }

        has_view_been_created = ViewEx::OnCreate(doc, flags);
        if(!has_view_been_created)
            {
            return has_view_been_created;
            }

        Run();
        }
    catch(...)
        {
        report_exception();
        }

    return has_view_been_created;
}

wxPrintout* mec_view::OnCreatePrintout()
{
    wxHtmlPrintout* z = new(wx) wxHtmlPrintout;
    safely_dereference_as<wxHtmlPrintout>(z).SetHtmlText(html_content_.c_str());
    return z;
}

void mec_view::UponProperties(wxCommandEvent&)
{
    if(wxID_OK == EditProperties())
        {
        Run();
        }
}

/// This complete replacement for wxDocManager::OnUpdateFileSave()
/// should not call Skip().

void mec_view::UponUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(document().IsModified());
}

/// This complete replacement for wxDocManager::OnUpdateFileSaveAs()
/// should not call Skip().

void mec_view::UponUpdateFileSaveAs(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void mec_view::UponUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

void mec_view::UponUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

namespace
{
template<typename T>
std::string f(T t)
{
    static double const bignum = std::numeric_limits<double>::max();
    if(bignum == t)
        {
        return "&nbsp;&nbsp;&nbsp;BIGNUM";
        }
    else
        {
        return "&nbsp;&nbsp;&nbsp;" + value_cast<std::string>(t);
        }
}
} // Unnamed namespace.

void mec_view::Run()
{
    input_data().RealizeAllSequenceInput();

    bool                        Use7702ATables               = exact_cast<mce_yes_or_no           >(input_data()["Use7702ATables"              ])->value();
//  int                         IssueAge                     = exact_cast<tnr_issue_age           >(input_data()["IssueAge"                    ])->value();
    mcenum_gender               Gender                       = exact_cast<mce_gender              >(input_data()["Gender"                      ])->value();
    mcenum_smoking              Smoking                      = exact_cast<mce_smoking             >(input_data()["Smoking"                     ])->value();
    mcenum_class                UnderwritingClass            = exact_cast<mce_class               >(input_data()["UnderwritingClass"           ])->value();
//  calendar_date               DateOfBirth                  = exact_cast<tnr_date                >(input_data()["DateOfBirth"                 ])->value();
//  mcenum_table_rating         SubstandardTable             = exact_cast<mce_table_rating        >(input_data()["SubstandardTable"            ])->value();
    std::string                 ProductName                  = exact_cast<ce_product_name         >(input_data()["ProductName"                 ])->value();
    double                      External1035ExchangeAmount   = exact_cast<tnr_nonnegative_double  >(input_data()["External1035ExchangeAmount"  ])->value();
//  bool                        External1035ExchangeFromMec  = exact_cast<mce_yes_or_no           >(input_data()["External1035ExchangeFromMec" ])->value();
    double                      Internal1035ExchangeAmount   = exact_cast<tnr_nonnegative_double  >(input_data()["Internal1035ExchangeAmount"  ])->value();
//  bool                        Internal1035ExchangeFromMec  = exact_cast<mce_yes_or_no           >(input_data()["Internal1035ExchangeFromMec" ])->value();
//  calendar_date               EffectiveDate                = exact_cast<tnr_date                >(input_data()["EffectiveDate"               ])->value();
    mcenum_defn_life_ins        DefinitionOfLifeInsurance    = exact_cast<mce_defn_life_ins       >(input_data()["DefinitionOfLifeInsurance"   ])->value();
    mcenum_defn_material_change DefinitionOfMaterialChange   = exact_cast<mce_defn_material_change>(input_data()["DefinitionOfMaterialChange"  ])->value();
    mcenum_uw_basis             GroupUnderwritingType        = exact_cast<mce_uw_basis            >(input_data()["GroupUnderwritingType"       ])->value();
    std::string                 Comments                     = exact_cast<datum_string            >(input_data()["Comments"                    ])->value();
    int                         InforceYear                  = exact_cast<tnr_duration            >(input_data()["InforceYear"                 ])->value();
    int                         InforceMonth                 = exact_cast<tnr_month               >(input_data()["InforceMonth"                ])->value();
    double                      InforceTargetSpecifiedAmount = exact_cast<tnr_nonnegative_double  >(input_data()["InforceTargetSpecifiedAmount"])->value();
    double                      InforceAccountValue          = exact_cast<tnr_nonnegative_double  >(input_data()["InforceAccountValue"         ])->value();
//  calendar_date               InforceAsOfDate              = exact_cast<tnr_date                >(input_data()["InforceAsOfDate"             ])->value();
    bool                        InforceIsMec                 = exact_cast<mce_yes_or_no           >(input_data()["InforceIsMec"                ])->value();
    calendar_date               LastMaterialChangeDate       = exact_cast<tnr_date                >(input_data()["LastMaterialChangeDate"      ])->value();
    double                      InforceDcv                   = exact_cast<tnr_nonnegative_double  >(input_data()["InforceDcv"                  ])->value();
    double                      InforceAvBeforeLastMc        = exact_cast<tnr_nonnegative_double  >(input_data()["InforceAvBeforeLastMc"       ])->value();
    int                         InforceContractYear          = exact_cast<tnr_duration            >(input_data()["InforceContractYear"         ])->value();
    int                         InforceContractMonth         = exact_cast<tnr_month               >(input_data()["InforceContractMonth"        ])->value();
    double                      InforceLeastDeathBenefit     = exact_cast<tnr_nonnegative_double  >(input_data()["InforceLeastDeathBenefit"    ])->value();
    mcenum_state                StateOfJurisdiction          = exact_cast<mce_state               >(input_data()["StateOfJurisdiction"         ])->value();
//  std::string                 FlatExtra                    = exact_cast<datum_sequence          >(input_data()["FlatExtra"                   ])->value();
//  std::string                 PaymentHistory               = exact_cast<datum_sequence          >(input_data()["PaymentHistory"              ])->value();
//  std::string                 BenefitHistory               = exact_cast<datum_sequence          >(input_data()["BenefitHistory"              ])->value();
//  bool                        DeprecatedUseDOB             = exact_cast<mce_yes_or_no           >(input_data()["DeprecatedUseDOB"            ])->value();
    double                      Payment                      = exact_cast<tnr_nonnegative_double  >(input_data()["Payment"                     ])->value();
    double                      BenefitAmount                = exact_cast<tnr_nonnegative_double  >(input_data()["BenefitAmount"               ])->value();

    TProductData product_data(ProductName);

    TDatabase database
        (ProductName
        ,Gender
        ,UnderwritingClass
        ,Smoking
        ,input_data().issue_age()
        ,GroupUnderwritingType
        ,StateOfJurisdiction
        );

    stratified_charges stratified(AddDataDir(product_data.GetTierFilename()));

    // SOMEDAY !! Ideally these would be in the GUI (or read from product files).
    round_to<double> const RoundNonMecPrem(2, r_downward);
    round_to<double> const round_max_premium(2, r_downward);

    oenum_modal_prem_type const target_premium_type =
        static_cast<oenum_modal_prem_type>(static_cast<int>(database.Query(DB_TgtPremType)));
    std::vector<double> TargetPremiumRates(input_data().years_to_maturity());
    if(oe_modal_table == target_premium_type)
        {
        TargetPremiumRates = actuarial_table_rates
            (AddDataDir(product_data.GetTgtPremFilename())
            ,static_cast<long int>(database.Query(DB_TgtPremTable))
            ,input_data().issue_age()
            ,input_data().years_to_maturity()
            );
        }
    else
        {
        ; // Do nothing: 'TargetPremiumRates' won't be used.
        }

    std::vector<double> const CvatCorridorFactors = actuarial_table_rates
        (AddDataDir(product_data.GetCorridorFilename())
        ,static_cast<long int>(database.Query(DB_CorridorTable))
        ,input_data().issue_age()
        ,input_data().years_to_maturity()
        );

    std::vector<double> tabular_Ax;
    for(int j = 0; j < input_data().years_to_maturity(); ++j)
        {
        LMI_ASSERT(0.0 < CvatCorridorFactors[j]);
        tabular_Ax.push_back(1.0 / CvatCorridorFactors[j]);
        }
    tabular_Ax.push_back(1.0);

    std::vector<double> const tabular_7Px = actuarial_table_rates
        (AddDataDir(product_data.GetTAMRA7PayFilename())
        ,static_cast<long int>(database.Query(DB_TAMRA7PayTable))
        ,input_data().issue_age()
        ,input_data().years_to_maturity()
        );

    std::vector<double> Mly7702qc = actuarial_table_rates
        (AddDataDir(product_data.GetIRC7702Filename())
        ,static_cast<long int>(database.Query(DB_IRC7702QTable))
        ,input_data().issue_age()
        ,input_data().years_to_maturity()
        );
    double const max_coi_rate = database.Query(DB_MaxMonthlyCoiRate);
    // ET !! Mly7702qc = coi_rate_from_q(Mly7702qc, Database_->Query(DB_MaxMonthlyCoiRate));
    assign(Mly7702qc, apply_binary(coi_rate_from_q<double>(), Mly7702qc, max_coi_rate));

    std::vector<double> guar_int;
    database.Query(guar_int, DB_GuarInt);

    std::vector<double> const spread
        (input_data().years_to_maturity()
        ,stratified.minimum_tiered_spread_for_7702()
        );

    // ET !! Mly7702iGlp = i_upper_12_over_12_from_i(max(.04, guar_int) - spread);
    std::vector<double> Mly7702iGlp(input_data().years_to_maturity());
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
    std::vector<double> Mly7702ig(input_data().years_to_maturity());
    assign(Mly7702ig, -1.0 + 1.0 / DBDiscountRate);

    // Use zero if that's the guaranteed rate; else use the statutory rate.
    // ET !! Use each_equal() here because PETE seems to interfere with
    // the normal operator==(). Is that a PETE defect?
    std::vector<double> const zero(input_data().years_to_maturity(), 0.0);
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
        ,mce_monthly
        ,mce_monthly
        );

    std::vector<double> analytic_Ax(input_data().years_to_maturity());
    analytic_Ax += (commfns.kM() + commfns.aD().back()) / commfns.aD();

    std::vector<double> E7aN(commfns.aN());
    E7aN.insert(E7aN.end(), 7, 0.0);
    E7aN.erase(E7aN.begin(), 7 + E7aN.begin());
    std::vector<double> analytic_7Px(input_data().years_to_maturity());
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
        ,input_data().issue_age()
        ,input_data().maturity_age()
        ,InforceYear
        ,InforceMonth
        ,InforceContractYear
        ,InforceContractMonth
        ,InforceAvBeforeLastMc
        ,InforceLeastDeathBenefit
        ,input_data().PaymentHistoryRealized()
        ,input_data().BenefitHistoryRealized()
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
        : input_data().inforce_year()
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

    std::ostringstream oss;

    oss
        << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\"\n"
        << "    \"http://www.w3.org/TR/html4/loose.dtd\">\n"
        << "<html>\n"
        << "<head>\n"
        << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\n"
        << "<title>Let me illustrate...</title>\n"
        << "</head>\n"
        << "<body>\n"
        ;

    oss << "<p>" << htmlize(Comments) << "</p>\n";

    LMI_ASSERT(static_cast<unsigned int>(InforceContractYear) < input_data().BenefitHistoryRealized().size());
    double const old_benefit_amount = input_data().BenefitHistoryRealized()[InforceContractYear];

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

    mec_state const& state = z.state();
    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "policy year"                     << "</td>\n"
        << "<td nowrap>" << f(state.B0_deduced_policy_year   ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "contract year"                   << "</td>\n"
        << "<td nowrap>" << f(state.B1_deduced_contract_year ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "seven-pay rate"                  << "</td>\n"
        << "<td nowrap>" << f(state.B2_deduced_px7_rate      ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "nsp rate"                        << "</td>\n"
        << "<td nowrap>" << f(state.B3_deduced_nsp_rate      ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "target premium"                  << "</td>\n"
        << "<td nowrap>" << f(state.B4_deduced_target_premium) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "target load"                     << "</td>\n"
        << "<td nowrap>" << f(state.B5_deduced_target_load   ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "excess load"                     << "</td>\n"
        << "<td nowrap>" << f(state.B6_deduced_excess_load   ) << "</td>\n"
        << "</tr>\n"
        << "</table>\n"
        ;

    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"

        << "<tr align=\"right\">\n"
        << "<td valign=\"bottom\" width=\"14%\">" << ""           << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "initial"    << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "incr"       << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "decr"       << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "nec_prem"   << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "MC"         << " </td>\n"
        << "<td valign=\"bottom\" width=\"14%\">" << "unnec_prem" << " </td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "benefit"                   << "</td>\n"
        << "<td nowrap>" << f(state.C0_init_bft       ) << "</td>\n"
        << "<td nowrap>" << f(state.D0_incr_bft       ) << "</td>\n"
        << "<td nowrap>" << f(state.E0_decr_bft       ) << "</td>\n"
        << "<td nowrap>" << f(state.F0_nec_pm_bft     ) << "</td>\n"
        << "<td nowrap>" << f(state.G0_do_mc_bft      ) << "</td>\n"
        << "<td nowrap>" << f(state.H0_unnec_pm_bft   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "LDB"                       << "</td>\n"
        << "<td nowrap>" << f(state.C1_init_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(state.D1_incr_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(state.E1_decr_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(state.F1_nec_pm_ldb     ) << "</td>\n"
        << "<td nowrap>" << f(state.G1_do_mc_ldb      ) << "</td>\n"
        << "<td nowrap>" << f(state.H1_unnec_pm_ldb   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "amts pd"                   << "</td>\n"
        << "<td nowrap>" << f(state.C2_init_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(state.D2_incr_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(state.E2_decr_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(state.F2_nec_pm_amt_pd  ) << "</td>\n"
        << "<td nowrap>" << f(state.G2_do_mc_amt_pd   ) << "</td>\n"
        << "<td nowrap>" << f(state.H2_unnec_pm_amt_pd) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "MC"                        << "</td>\n"
        << "<td nowrap>" << f(state.C3_init_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(state.D3_incr_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(state.E3_decr_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(state.F3_nec_pm_is_mc   ) << "</td>\n"
        << "<td nowrap>" << f(state.G3_do_mc_is_mc    ) << "</td>\n"
        << "<td nowrap>" << f(state.H3_unnec_pm_is_mc ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "DCV"                       << "</td>\n"
        << "<td nowrap>" << f(state.C4_init_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(state.D4_incr_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(state.E4_decr_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(state.F4_nec_pm_dcv     ) << "</td>\n"
        << "<td nowrap>" << f(state.G4_do_mc_dcv      ) << "</td>\n"
        << "<td nowrap>" << f(state.H4_unnec_pm_dcv   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "7PP"                       << "</td>\n"
        << "<td nowrap>" << f(state.C5_init_px7       ) << "</td>\n"
        << "<td nowrap>" << f(state.D5_incr_px7       ) << "</td>\n"
        << "<td nowrap>" << f(state.E5_decr_px7       ) << "</td>\n"
        << "<td nowrap>" << f(state.F5_nec_pm_px7     ) << "</td>\n"
        << "<td nowrap>" << f(state.G5_do_mc_px7      ) << "</td>\n"
        << "<td nowrap>" << f(state.H5_unnec_pm_px7   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "MEC"                       << "</td>\n"
        << "<td nowrap>" << f(state.C6_init_mec       ) << "</td>\n"
        << "<td nowrap>" << f(state.D6_incr_mec       ) << "</td>\n"
        << "<td nowrap>" << f(state.E6_decr_mec       ) << "</td>\n"
        << "<td nowrap>" << f(state.F6_nec_pm_mec     ) << "</td>\n"
        << "<td nowrap>" << f(state.G6_do_mc_mec      ) << "</td>\n"
        << "<td nowrap>" << f(state.H6_unnec_pm_mec   ) << "</td>\n"
        << "</tr>\n"

        << "</table>\n"
        ;

    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "net 1035 amount"              << "</td>\n"
        << "<td nowrap>" << f(state.Q0_net_1035          ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "net max necessary premium"    << "</td>\n"
        << "<td nowrap>" << f(state.Q1_max_nec_prem_net  ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gross max necessary premium"  << "</td>\n"
        << "<td nowrap>" << f(state.Q2_max_nec_prem_gross) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "CV before last MC"            << "</td>\n"
        << "<td nowrap>" << f(state.Q3_cv_before_last_mc ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "cumulative seven-pay premium" << "</td>\n"
        << "<td nowrap>" << f(state.Q4_cum_px7           ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "cumulative amounts paid"      << "</td>\n"
        << "<td nowrap>" << f(state.Q5_cum_amt_pd        ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "max non-MEC premium"          << "</td>\n"
        << "<td nowrap>" << f(state.Q6_max_non_mec_prem  ) << "</td>\n"
        << "</tr>\n"
        << "</table>\n"
        ;

    oss
        << "</body>\n"
        << "</html>\n"
        ;
    html_content_ = oss.str();
    html_window_->SetPage(html_content_);

    std::vector<double> ratio_Ax (input_data().years_to_maturity());
    ratio_Ax  += tabular_Ax  / analytic_Ax ;
    std::vector<double> ratio_7Px(input_data().years_to_maturity());
    ratio_7Px += tabular_7Px / analytic_7Px;

    configurable_settings const& c = configurable_settings::instance();
    std::string const extension(c.spreadsheet_file_extension());
    std::string spreadsheet_filename = base_filename() + extension;
    std::ofstream ofs(spreadsheet_filename.c_str(), ios_out_trunc_binary());
    ofs << "This temporary output file will be removed in a future release.\n";
    ofs
        << "t\t"
        << "i\t"
        << "q\t"
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
    for(int j = 0; j < input_data().years_to_maturity(); ++j)
        {
        ofs
            <<               j  << '\t'
            << value_cast<std::string>(Mly7702iGlp [j]) << '\t'
            << value_cast<std::string>(Mly7702qc   [j]) << '\t'
            << value_cast<std::string>(commfns.aD()[j]) << '\t'
            << value_cast<std::string>(commfns.kC()[j]) << '\t'
            << value_cast<std::string>(commfns.aN()[j]) << '\t'
            << value_cast<std::string>(commfns.kM()[j]) << '\t'
            << value_cast<std::string>(E7aN        [j]) << '\t'
            << value_cast<std::string>(analytic_Ax [j]) << '\t'
            << value_cast<std::string>(tabular_Ax  [j]) << '\t'
            << value_cast<std::string>(ratio_Ax    [j]) << '\t'
            << value_cast<std::string>(analytic_7Px[j]) << '\t'
            << value_cast<std::string>(tabular_7Px [j]) << '\t'
            << value_cast<std::string>(ratio_7Px   [j]) << '\t'
            << '\n'
        ;
        }
    ofs
        << input_data().years_to_maturity()
        << "\t\t\t"
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
}

