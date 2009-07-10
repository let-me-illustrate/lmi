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

// $Id: mec_view.cpp,v 1.5 2009-07-10 12:41:12 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_view.hpp"

#include "actuarial_table.hpp"
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "basic_values.hpp"          // lowest_premium_tax_load()
#include "comma_punct.hpp"
#include "data_directory.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "handle_exceptions.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_proddata.hpp"
#include "materially_equal.hpp"
#include "mec_document.hpp"
#include "mec_input.hpp"
#include "mvc_controller.hpp"
#include "oecumenic_enumerations.hpp"
#include "safely_dereference_as.hpp"
#include "stratified_algorithms.hpp" // TieredGrossToNet()
#include "stratified_charges.hpp"
#include "timer.hpp"
#include "wx_new.hpp"

#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/xrc/xmlres.h>

#include <string>
#include <sstream>

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

void mec_view::Run()
{
    input_data().RealizeAllSequenceInput();

//  int                         IssueAge                    = exact_cast<tnr_issue_age           >(input_data()["IssueAge"                   ])->value();
    mcenum_gender               Gender                      = exact_cast<mce_gender              >(input_data()["Gender"                     ])->value();
    mcenum_smoking              Smoking                     = exact_cast<mce_smoking             >(input_data()["Smoking"                    ])->value();
    mcenum_class                UnderwritingClass           = exact_cast<mce_class               >(input_data()["UnderwritingClass"          ])->value();
//  calendar_date               DateOfBirth                 = exact_cast<tnr_date                >(input_data()["DateOfBirth"                ])->value();
//  mcenum_table_rating         SubstandardTable            = exact_cast<mce_table_rating        >(input_data()["SubstandardTable"           ])->value();
    std::string                 ProductName                 = exact_cast<ce_product_name         >(input_data()["ProductName"                ])->value();
    double                      External1035ExchangeAmount  = exact_cast<tnr_nonnegative_double  >(input_data()["External1035ExchangeAmount" ])->value();
//  bool                        External1035ExchangeFromMec = exact_cast<mce_yes_or_no           >(input_data()["External1035ExchangeFromMec"])->value();
    double                      Internal1035ExchangeAmount  = exact_cast<tnr_nonnegative_double  >(input_data()["Internal1035ExchangeAmount" ])->value();
//  bool                        Internal1035ExchangeFromMec = exact_cast<mce_yes_or_no           >(input_data()["Internal1035ExchangeFromMec"])->value();
//  calendar_date               EffectiveDate               = exact_cast<tnr_date                >(input_data()["EffectiveDate"              ])->value();
    mcenum_defn_life_ins        DefinitionOfLifeInsurance   = exact_cast<mce_defn_life_ins       >(input_data()["DefinitionOfLifeInsurance"  ])->value();
    mcenum_defn_material_change DefinitionOfMaterialChange  = exact_cast<mce_defn_material_change>(input_data()["DefinitionOfMaterialChange" ])->value();
    mcenum_uw_basis             GroupUnderwritingType       = exact_cast<mce_uw_basis            >(input_data()["GroupUnderwritingType"      ])->value();
    std::string                 Comments                    = exact_cast<datum_string            >(input_data()["Comments"                   ])->value();
    int                         InforceYear                 = exact_cast<tnr_duration            >(input_data()["InforceYear"                ])->value();
    int                         InforceMonth                = exact_cast<tnr_month               >(input_data()["InforceMonth"               ])->value();
    double                      InforceAccountValue         = exact_cast<tnr_nonnegative_double  >(input_data()["InforceAccountValue"        ])->value();
//  calendar_date               InforceAsOfDate             = exact_cast<tnr_date                >(input_data()["InforceAsOfDate"            ])->value();
    double                      InforceSevenPayPremium      = exact_cast<tnr_nonnegative_double  >(input_data()["InforceSevenPayPremium"     ])->value();
    bool                        InforceIsMec                = exact_cast<mce_yes_or_no           >(input_data()["InforceIsMec"               ])->value();
    calendar_date               LastMaterialChangeDate      = exact_cast<tnr_date                >(input_data()["LastMaterialChangeDate"     ])->value();
    double                      InforceDcv                  = exact_cast<tnr_nonnegative_double  >(input_data()["InforceDcv"                 ])->value();
    double                      InforceAvBeforeLastMc       = exact_cast<tnr_nonnegative_double  >(input_data()["InforceAvBeforeLastMc"      ])->value();
    int                         InforceContractYear         = exact_cast<tnr_duration            >(input_data()["InforceContractYear"        ])->value();
    int                         InforceContractMonth        = exact_cast<tnr_month               >(input_data()["InforceContractMonth"       ])->value();
    double                      InforceLeastDeathBenefit    = exact_cast<tnr_nonnegative_double  >(input_data()["InforceLeastDeathBenefit"   ])->value();
    mcenum_state                StateOfJurisdiction         = exact_cast<mce_state               >(input_data()["StateOfJurisdiction"        ])->value();
//  std::string                 FlatExtra                   = exact_cast<datum_sequence          >(input_data()["FlatExtra"                  ])->value();
//  std::string                 PaymentHistory              = exact_cast<datum_sequence          >(input_data()["PaymentHistory"             ])->value();
//  std::string                 BenefitHistory              = exact_cast<datum_sequence          >(input_data()["BenefitHistory"             ])->value();
//  bool                        DeprecatedUseDOB            = exact_cast<mce_yes_or_no           >(input_data()["DeprecatedUseDOB"           ])->value();
//  bool                        EffectiveDateToday          = exact_cast<mce_yes_or_no           >(input_data()["EffectiveDateToday"         ])->value();
    double                      Payment                     = exact_cast<tnr_nonnegative_double  >(input_data()["Payment"                    ])->value();
    double                      BenefitAmount               = exact_cast<tnr_nonnegative_double  >(input_data()["BenefitAmount"              ])->value();

(void)InforceSevenPayPremium    ; // TODO ?? Unused?

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

    std::vector<double> SevenPayRates = actuarial_table_rates
        (AddDataDir(product_data.GetTAMRA7PayFilename())
        ,static_cast<long int>(database.Query(DB_TAMRA7PayTable))
        ,input_data().issue_age()
        ,input_data().years_to_maturity()
        );

    std::vector<double> TargetPremiumRates(input_data().years_to_maturity());
    if(oe_modal_table == database.Query(DB_TgtPremType))
        {
        TargetPremiumRates = actuarial_table_rates
            (AddDataDir(product_data.GetTgtPremFilename())
            ,static_cast<long int>(database.Query(DB_TgtPremTable))
            ,input_data().issue_age()
            ,input_data().years_to_maturity()
            );
        }

    std::vector<double> CvatCorridorFactors = actuarial_table_rates
        (AddDataDir(product_data.GetCorridorFilename())
        ,static_cast<long int>(database.Query(DB_CorridorTable))
        ,input_data().issue_age()
        ,input_data().years_to_maturity()
        );

    std::vector<double> CvatNspRates;
    for(int j = 0; j < input_data().years_to_maturity(); ++j)
        {
        LMI_ASSERT(0.0 < CvatCorridorFactors[j]);
        CvatNspRates.push_back(1.0 / CvatCorridorFactors[j]);
        }
    CvatNspRates.push_back(1.0);

    Irc7702A z
        (0
        ,DefinitionOfLifeInsurance
        ,DefinitionOfMaterialChange
        ,false // Survivorship: hardcoded for now.
        ,mce_allow_mec
        ,true  // Use table for 7pp: hardcoded for now.
        ,true  // Use table for NSP: hardcoded for now.
        ,SevenPayRates
        ,CvatNspRates
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

    // TODO ?? This should be an input field.
    double target_premium_specamt = input_data().BenefitHistoryRealized()[0];
    double AnnualTargetPrem = 1000000000.0; // No higher premium is anticipated.
    oenum_modal_prem_type const premium_type =
        static_cast<oenum_modal_prem_type>(static_cast<int>(database.Query(DB_TgtPremType)));
    if(oe_monthly_deduction == premium_type)
        {
        warning() << "Unsupported modal premium type." << LMI_FLUSH;
        }
    else if(oe_modal_nonmec == premium_type)
        {
        AnnualTargetPrem = round_max_premium
            (   target_premium_specamt
            *   epsilon_plus_one
            *   SevenPayRates[0]
            );
        }
    else if(oe_modal_table == premium_type)
        {
        AnnualTargetPrem = round_max_premium
            (   database.Query(DB_TgtPremPolFee)
            +       target_premium_specamt
                *   epsilon_plus_one
                *   TargetPremiumRates[0]
            );
        }
    else
        {
        fatal_error()
            << "Unknown modal premium type " << premium_type << '.'
            << LMI_FLUSH
            ;
        }

    double premium_tax_load = lowest_premium_tax_load
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

    double LoadTarget = target_sales_load[InforceYear] + target_premium_load[InforceYear] + dac_tax_load[InforceYear] + premium_tax_load;
    double LoadExcess = excess_sales_load[InforceYear] + excess_premium_load[InforceYear] + dac_tax_load[InforceYear] + premium_tax_load;

    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

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

    oss
        << Comments                                                     << "<br>\n"
        << InforceYear                << "\tPolicy year"                << "<br>\n"
        << InforceMonth               << "\tPolicy month"               << "<br>\n"
        << InforceContractYear        << "\tContract year"              << "<br>\n"
        << InforceContractMonth       << "\tContract month"             << "<br>\n"
        << AnnualTargetPrem           << "\tTarget premium"             << "<br>\n"
        << LoadTarget                 << "\tTarget load"                << "<br>\n"
        << LoadExcess                 << "\tExcess load"                << "<br>\n"
        << SevenPayRates[0]           << "\tInitial 7-pay rate"         << "<br>\n"
        << CvatCorridorFactors[0]     << "\tInitial corridor factor"    << "<br>\n"
        << CvatNspRates[0]            << "\tInitial NSP rate"           << "<br>\n"
        << z.IsMecAlready()           << "\tAlready a MEC"              << "<br>\n"
        << InforceDcv                 << "\tInitial DCV"                << "<br>\n"
        << z.GetPresent7pp()          << "\t7PP"                        << "<br>\n"
        ;

    LMI_ASSERT(static_cast<unsigned int>(InforceContractYear) < input_data().BenefitHistoryRealized().size());
    double old_benefit_amount = input_data().BenefitHistoryRealized()[InforceContractYear];

    double total_1035_amount = TieredGrossToNet
        (External1035ExchangeAmount + Internal1035ExchangeAmount
        ,AnnualTargetPrem
        ,LoadTarget
        ,LoadExcess
        );
    if(0.0 != total_1035_amount)
        {
        LMI_ASSERT(0 == InforceYear );
        LMI_ASSERT(0 == InforceMonth);
        z.Update1035Exch7702A
            (InforceDcv
            ,total_1035_amount
            ,old_benefit_amount
            );
        oss
            << "* 1035 exchange"                    << "<br>\n"
            << total_1035_amount << "\t1035 amount" << "<br>\n"
            << z.IsMecAlready()  << "\tMEC"         << "<br>\n"
            << InforceDcv        << "\tDCV"         << "<br>\n"
            << z.GetPresent7pp() << "\t7PP"         << "<br>\n"
            ;
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
        oss
            << "* benefit change"                             << "<br>\n"
            << old_benefit_amount          << "\told benefit" << "<br>\n"
            << BenefitAmount               << "\tnew benefit" << "<br>\n"
            << z.DebugGetLowestBft()       << "\tLDB"         << "<br>\n"
            << z.GetPresent7pp()           << "\t7PP"         << "<br>\n"
            << z.IsMecAlready()            << "\tMEC"         << "<br>\n"
            << z.IsMaterialChangeInQueue() << "\tMC pending"  << "<br>\n"
            ;
        }

    double kludge_account_value = InforceAccountValue;
    if(0 == InforceYear && 0 == InforceMonth)
        {
        kludge_account_value = InforceDcv; // TODO ?? Why?
        }
    double max_necessary_premium = z.MaxNecessaryPremium
        (InforceDcv
        ,AnnualTargetPrem
        ,LoadTarget
        ,LoadExcess
        ,kludge_account_value
        );
    double max_non_mec_premium = z.MaxNonMecPremium
        (InforceDcv
        ,AnnualTargetPrem
        ,LoadTarget
        ,LoadExcess
        ,kludge_account_value
        );
    if(!z.IsMecAlready())
        {
        oss
            << z.DebugGetGrossMaxNecPm() << "\tGross maximum necessary premium" << "<br>\n"
            << max_necessary_premium     << "\tMaximum necessary premium" << "<br>\n"
            << max_non_mec_premium       << "\tMaximum non-MEC premium" << "<br>\n"
            ;
        }

    double necessary_premium = std::min(Payment, max_necessary_premium);
    double unnecessary_premium = material_difference(Payment, necessary_premium);

    if(!z.IsMecAlready() && 0.0 != necessary_premium)
        {
        z.UpdatePmt7702A
            (InforceDcv           // Unused.
            ,necessary_premium
            ,false
            ,AnnualTargetPrem     // Unused.
            ,LoadTarget           // Unused.
            ,LoadExcess           // Unused.
            ,kludge_account_value // Unused.
            );
        oss
            << "* accept necessary premium"                       << "<br>\n"
            << z.DebugGetCumPmts() << "\tCumulative amounts paid" << "<br>\n"
            << z.DebugGetCum7pp()  << "\tCumulative 7PP"          << "<br>\n"
            << z.IsMecAlready()    << "\tMEC"                     << "<br>\n"
            ;
        }

    if(z.IsMaterialChangeInQueue())
        {
        z.RedressMatChg
            (InforceDcv
            ,unnecessary_premium
            ,necessary_premium
            ,InforceAccountValue // TODO ?? Update for payment?
            );
        oss
            << "* material change"                            << "<br>\n"
            << z.IsMecAlready()            << "\tMEC"         << "<br>\n"
            << InforceDcv                  << "\tDCV"         << "<br>\n"
            << z.DebugGetLowestBft()       << "\tLDB"         << "<br>\n"
            << z.GetPresent7pp()           << "\t7PP"         << "<br>\n"
            ;
        }

    if(!z.IsMecAlready() && 0.0 != unnecessary_premium)
        {
        z.UpdatePmt7702A
            (InforceDcv           // Unused.
            ,unnecessary_premium
            ,true
            ,AnnualTargetPrem     // Unused.
            ,LoadTarget           // Unused.
            ,LoadExcess           // Unused.
            ,kludge_account_value // Unused.
            );
        oss
            << "* accept unnecessary premium"                     << "<br>\n"
            << z.DebugGetCumPmts() << "\tCumulative amounts paid" << "<br>\n"
            << z.DebugGetCum7pp()  << "\tCumulative 7PP"          << "<br>\n"
            << z.IsMecAlready()    << "\tMEC"                     << "<br>\n"
            ;
        }

    oss
        << "</body>\n"
        << "</html>\n"
        ;
    html_window_->SetPage(oss.str());
}

