// Account value: monthly detail.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ihs_avdebug.cpp,v 1.1 2005-01-14 19:47:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_acctval.hpp"

#include "calendar_date.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "ihs_dbughdr.hpp"
#include "ihs_deathbft.hpp"
#include "ihs_irc7702.hpp"
#include "ihs_irc7702a.hpp"
#include "ihs_ldginvar.hpp"
#include "ihs_ldgvar.hpp"
#include "ihs_proddata.hpp"
#include "ihs_rnddata.hpp"
#include "inputs.hpp"
#include "inputstatus.hpp"
#include "interest_rates.hpp"
#include "loads.hpp"
#include "math_functors.hpp"
#include "ncnnnpnn.hpp"
#include "outlay.hpp"
#include "value_cast_ihs.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

// Display monthly detail of calculations for debugging.

// Debug functions that use the spreadsheet control are in dbugview.cpp .

namespace
{
    inline std::string const& not_applicable()
        {
        static char const* n_a = "---";
        static std::string const s(n_a);
        return s;
        }
}

//============================================================================
inline void AccountValue::SetMonthlyDetail(int enumerator, std::string const& s)
{
    DebugRecord[enumerator] = s;
}

//============================================================================
inline void AccountValue::SetMonthlyDetail(int enumerator, double d)
{
    DebugRecord[enumerator] = value_cast_ihs<std::string>(d);
}

//============================================================================
void AccountValue::SetDebugFilename(std::string const& s)
{
    DebugFilename = s;
}

//============================================================================
void AccountValue::DebugPrintInit()
{
    DebugRecord.assign(eLast, "EMPTY");

    if(!Debugging)
        {
        return;
        }
    if("" == DebugFilename)
        {
        throw std::logic_error("Debug filename not specified.");
        }

    DebugStream.open
        (DebugFilename.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    std::copy
        (DebugColHeaders().begin()
        ,DebugColHeaders().end()
        ,std::ostream_iterator<std::string>(DebugStream, "\t")
        );
    DebugStream << '\n';
}

//============================================================================
void AccountValue::DebugEndBasis()
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return;
        }
    DebugStream << '\n';
}

//============================================================================
void AccountValue::DebugRestart(std::string const& reason)
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return;
        }
    DebugStream << reason << '\n';
}

//============================================================================
// To add a new column, see ihs_dbughdr.hpp .
void AccountValue::DebugPrint()
{
    if(!Debugging || Solving || SolvingForGuarPremium)
        {
        return; // Show detail on final run, not every solve iteration.
        }

    DebugRecord.assign(eLast, "EMPTY");

    SetMonthlyDetail(eYear               ,Year);
    SetMonthlyDetail(eMonth              ,Month);
    SetMonthlyDetail(eRateBasis          ,RateBasis.str());
    SetMonthlyDetail(eAge                ,InvariantValues().Age + Year);

    // Initial values at beginning of run, reflecting inforce if applicable.
    if(InforceYear == Year && InforceMonth == Month)
        {
        PriorAVGenAcct  = InforceAVGenAcct;
        PriorAVSepAcct  = InforceAVSepAcct;
        PriorAVRegLn    = InforceAVRegLn;
        PriorAVPrfLn    = InforceAVPrfLn;
        PriorRegLnBal   = InforceRegLnBal;
        PriorPrfLnBal   = InforcePrfLnBal;
        }

    // Beginning of month values.
    SetMonthlyDetail(eGenAcctBOMAV       ,PriorAVGenAcct);
    SetMonthlyDetail(eSepAcctBOMAV       ,PriorAVSepAcct);
    SetMonthlyDetail(eUnloanedBOMAV      ,PriorAVGenAcct + PriorAVSepAcct);
    SetMonthlyDetail(eRegularLoanBOMAV   ,PriorAVRegLn);
    SetMonthlyDetail(ePrefLoanBOMAV      ,PriorAVPrfLn);
    SetMonthlyDetail
        (eTotalBOMAV
        ,PriorAVGenAcct + PriorAVSepAcct + PriorAVRegLn + PriorAVPrfLn
        );

    PriorAVGenAcct  = AVGenAcct;
    PriorAVSepAcct  = AVSepAcct;
    PriorAVRegLn    = AVRegLn;
    PriorAVPrfLn    = AVPrfLn;

    // TODO ?? Should loan balance columns be similarly offset?

    SetMonthlyDetail(eRegLoanBal         ,RegLnBal                         );
    SetMonthlyDetail(ePrefLoanBal        ,PrfLnBal                         );
    SetMonthlyDetail(eDBOption           ,YearsDBOpt.str()                 );
    SetMonthlyDetail(eSpecAmt            ,ActualSpecAmt                    );
    SetMonthlyDetail(eCorridorFactor     ,YearsCorridorFactor              );
    SetMonthlyDetail(eDeathBft           ,DBReflectingCorr                 );
    SetMonthlyDetail(eForceout           ,GptForceout                      );
    SetMonthlyDetail(eEePrem             ,EeGrossPmts[Month]               );
    SetMonthlyDetail(eErPrem             ,ErGrossPmts[Month]               );
    SetMonthlyDetail(eTotalPrem          ,GrossPmts[Month]                 );
    SetMonthlyDetail(eTargetPrem         ,AnnualTargetPrem                 );
    SetMonthlyDetail(ePremLoad           ,GrossPmts[Month] - NetPmts[Month]);
    SetMonthlyDetail(eNetPrem            ,NetPmts[Month]                   );
    SetMonthlyDetail(eMlyPolicyFee       ,YearsMlyPolFee                   );
    SetMonthlyDetail(eAnnPolicyFee       ,YearsAnnPolFee                   );
    SetMonthlyDetail(eSpecAmtLoad        ,DetermineSpecAmtLoad()           );
    SetMonthlyDetail(eAcctValLoadBOM     ,DetermineAcctValLoadBOM()        );
    SetMonthlyDetail(eNAAR               ,NAAR                             );
    SetMonthlyDetail(eCOIRate            ,ActualCoiRate                    );
    SetMonthlyDetail(eCOICharge          ,COI                              );
    SetMonthlyDetail(eADDRate            ,YearsADDRate                     );
    SetMonthlyDetail(eADDCharge          ,ADDChg                           );
    SetMonthlyDetail(eWPRate             ,YearsWPRate                      );
    SetMonthlyDetail(eWPCharge           ,WPChg                            );
    SetMonthlyDetail(eTermAmount         ,TermDB                           );
    SetMonthlyDetail(eTermRate           ,YearsTermRate                    );
    SetMonthlyDetail(eTermCharge         ,TermChg                          );
    SetMonthlyDetail(eTotalRiderCharge   ,RiderDeductions                  );
    SetMonthlyDetail(eTotalMonthlyDeds   ,MlyDed                           );

    SetMonthlyDetail(eGenAcctIntRate     ,ActualMonthlyRate(YearsGenAcctIntRate              ));
    SetMonthlyDetail(eGenAcctIntCred     ,GenAcctIntCred                   );
    SetMonthlyDetail(eSepAcctIntRate     ,ActualMonthlyRate(YearsSepAcctIntRate              ));
    SetMonthlyDetail(eSepAcctIntCred     ,SepAcctIntCred                   );
    SetMonthlyDetail(eAcctValLoadAMD     ,DetermineAcctValLoadAMD()        );
    SetMonthlyDetail(eRegLnIntRate       ,ActualMonthlyRate(YearsRegLnIntCredRate            ));
    SetMonthlyDetail(eRegLnIntCred       ,RegLnIntCred                     );
    SetMonthlyDetail(ePrfLnIntRate       ,ActualMonthlyRate(YearsPrfLnIntCredRate            ));
    SetMonthlyDetail(ePrfLnIntCred       ,PrfLnIntCred                     );
    SetMonthlyDetail(eYearsHMValueRate   ,ActualMonthlyRate(YearsHoneymoonValueRate          ));
    SetMonthlyDetail(eYearsPostHMRate    ,ActualMonthlyRate(YearsPostHoneymoonGenAcctIntRate ));

    // Items that are used only on anniversary.
    if(0 == Month)
        {
        SetMonthlyDetail(eRequestedWD    ,RequestedWD                      );
        SetMonthlyDetail(eMaxWD          ,MaxWD                            );
        SetMonthlyDetail(eGrossWD        ,GrossWD                          );
        SetMonthlyDetail(eNetWD          ,NetWD                            );
        SetMonthlyDetail(eRequestedLoan  ,RequestedLoan                    );
        SetMonthlyDetail(eMaxLoan        ,MaxLoan                          );
        SetMonthlyDetail(eNewLoan        ,ActualLoan                       );
        }
    else
        {
        SetMonthlyDetail(eRequestedWD    ,not_applicable()                 );
        SetMonthlyDetail(eMaxWD          ,not_applicable()                 );
        SetMonthlyDetail(eGrossWD        ,not_applicable()                 );
        SetMonthlyDetail(eNetWD          ,not_applicable()                 );
        SetMonthlyDetail(eRequestedLoan  ,not_applicable()                 );
        SetMonthlyDetail(eMaxLoan        ,not_applicable()                 );
        SetMonthlyDetail(eNewLoan        ,not_applicable()                 );
        }

    SetMonthlyDetail(eTaxBasis           ,TaxBasis                         );
    SetMonthlyDetail(eCumNoLapsePrem     ,CumNoLapsePrem                   );
    SetMonthlyDetail(eNoLapseActive      ,NoLapseActive                    );
    SetMonthlyDetail(eEOMAV              ,TotalAccountValue()              );
    SetMonthlyDetail(eHMValue            ,std::max(HoneymoonValue, 0.0)    );
    SetMonthlyDetail(eSurrChg            ,SurrChg()                        );

// TODO ?? Unfortunately duplicated from AccountValue::FinalizeYear().
    // AV already includes any experience refund credited.
    double total_av = TotalAccountValue();
    double csv_net =
            total_av
        -   SurrChg()
        -   RegLnBal
        +   GetRefundableSalesLoad()
        +   std::max(0.0, ExpRatReserve)
        ;
    csv_net = std::max(HoneymoonValue, csv_net);

    SetMonthlyDetail(eEOMCSVNet          ,csv_net                          );
    SetMonthlyDetail(eEOMCV7702          ,CashValueFor7702()               );

    SetMonthlyDetail(eInforceFactor      ,InforceFactor                    );
    SetMonthlyDetail(eExpRatReserve      ,ExpRatReserve                    );
    SetMonthlyDetail(eExpRatStabReserve  ,ExpRatStabReserve                );
    SetMonthlyDetail(eExpRatIBNRReserve  ,ExpRatIBNRReserve                );
    SetMonthlyDetail(eExpRatRfd          ,ExpRatRfd                        );
    SetMonthlyDetail
        (   eClaimsPaid
        ,   GetPartMortQ(Year) * InvariantValues().SpecAmt[Year]
        );

    bool irc7702a_data_irrelevant =
           InvariantValues().MecYear < Year
        ||    InvariantValues().MecYear == Year
           && InvariantValues().MecMonth < Month
        || e_run_curr_basis != RateBasis
        ;

    if(!irc7702a_data_irrelevant)
        {
        SetMonthlyDetail(e7702ATestDur       ,IRC7702A->DebugGetTestDur  ());
        SetMonthlyDetail(e7702A7ppRate       ,IRC7702A->DebugGet7ppRate  ());
        SetMonthlyDetail(e7702ANsp           ,IRC7702A->DebugGetNsp      ());
        SetMonthlyDetail(e7702ALowestDb      ,IRC7702A->DebugGetLowestBft());
        // This one's a little tricky. We show the DCV actually used in
        // material change calculations, iff there is a material change
        // in the current month. Otherwise, we show the DCV at the end
        // of the month; we could have shown zero instead, but that
        // wouldn't be useful. We could always have shown DCV at the end
        // of the month, but that wouldn't show what was actually used in
        // material change processing.
        SetMonthlyDetail
            (e7702ADeemedCv
            ,IRC7702A->DebugGetIsMatChg()
                ? IRC7702A->DebugGetSavedDCV()
                : Dcv
            );
        SetMonthlyDetail(e7702ANetMaxNecPm   ,NetMaxNecessaryPremium       );
        SetMonthlyDetail(e7702AGrossMaxNecPm ,GrossMaxNecessaryPremium     );

        SetMonthlyDetail(e7702AUnnecPm       ,UnnecessaryPremium           );
        SetMonthlyDetail(e7702ADbAdj         ,IRC7702A->DebugGetDbAdj    ());
        SetMonthlyDetail(e7702A7pp           ,IRC7702A->DebugGet7pp      ());
        SetMonthlyDetail(e7702ACumPmts       ,IRC7702A->DebugGetCumPmts  ());
        SetMonthlyDetail(e7702ACum7pp        ,IRC7702A->DebugGetCum7pp   ());
        SetMonthlyDetail(e7702AIsMatChg      ,IRC7702A->DebugGetIsMatChg ());
        }
    else
        {
        SetMonthlyDetail(e7702ATestDur       ,not_applicable()             );
        SetMonthlyDetail(e7702A7ppRate       ,not_applicable()             );
        SetMonthlyDetail(e7702ANsp           ,not_applicable()             );
        SetMonthlyDetail(e7702ALowestDb      ,not_applicable()             );
        SetMonthlyDetail(e7702ADeemedCv      ,not_applicable()             );
        SetMonthlyDetail(e7702ANetMaxNecPm   ,not_applicable()             );
        SetMonthlyDetail(e7702AGrossMaxNecPm ,not_applicable()             );
        SetMonthlyDetail(e7702AUnnecPm       ,not_applicable()             );
        SetMonthlyDetail(e7702ADbAdj         ,not_applicable()             );
        SetMonthlyDetail(e7702A7pp           ,not_applicable()             );
        SetMonthlyDetail(e7702ACumPmts       ,not_applicable()             );
        SetMonthlyDetail(e7702ACum7pp        ,not_applicable()             );
        SetMonthlyDetail(e7702AIsMatChg      ,not_applicable()             );
        }
    SetMonthlyDetail(e7702AIsMec         ,InvariantValues().IsMec          );

    SetMonthlyDetail(eGSP                ,IRC7702->RoundedGSP            ());
    SetMonthlyDetail(eGLP                ,IRC7702->RoundedGLP            ());

    std::copy
        (DebugRecord.begin()
        ,DebugRecord.end()
        ,std::ostream_iterator<std::string>(DebugStream, "\t")
        );
    DebugStream << '\n';
    DebugRecord.assign(eLast, "EMPTY");
}

// Support for 12-month projection of values.
// TODO ?? Expunge this--it's unsupported.

// The obsolete section from here to the end of the file is
// Copyright (C) 2002 Gregory W. Chicares and Joseph L. Murdzek.
// Authors: GWC and JLM.

//============================================================================
void AccountValue::SetProject12Filename(std::string const& s)
{
    Project12MosFilename = s;
}

//============================================================================
void AccountValue::Project12MosPrintInit()
{
    if(!Projecting12Mos || Solving || e_run_curr_basis != RateBasis)
        {
        return;
        }
    if("" == Project12MosFilename)
        {
        throw std::logic_error("Projection letter filename not specified.");
        }

    // This code is simplistic; we presently need it only under the
    // limited circumstances asserted here, and believe it would be
    // incorrect otherwise.
    //
    // TODO ?? JOE: are there any others?

    // Deach benefit option changes not allowed.
    int beg_yr = (     InforceMonth + 12 * InforceYear) / 12;
    int end_yr = (11 + InforceMonth + 12 * InforceYear) / 12;
    if(InvariantValues().DBOpt[beg_yr] != InvariantValues().DBOpt[end_yr])
        {
        throw std::runtime_error("Death benefit option changes not allowed.");
        }

    // Interest rate changes not allowed.
    if
        (  InterestRates_->GenAcctNetRate(e_basis(e_currbasis), e_rate_period(e_monthly_rate))[beg_yr]
        != InterestRates_->GenAcctNetRate(e_basis(e_currbasis), e_rate_period(e_monthly_rate))[end_yr]
        )
        {
        throw std::runtime_error("Interest rate changes not allowed.");
        }

    // General account only.
    if(Database->Query(DB_AllowSepAcct))
        {
        throw std::runtime_error("Separate account not allowed.");
        }

    int tab_stop = 40;

// TODO ?? All this 'pad' stuff should be factored into a subroutine.
    std::string name_str = "Name: " + InvariantValues().Insured1;
    {
    std::string pad(tab_stop - name_str.size(),' ');
    name_str += pad;
    }

    std::ostringstream os2;
    os2 << Input->PolicyNumber;
    std::string number_str("Policy Number: ");
    number_str += os2.str();
    {
    std::string pad(tab_stop - number_str.size(), ' ');
    number_str += pad;
    }

    std::string date_str =
          "Policy Date: "
        + calendar_date(Input->PolicyDate).str()
        ;
    {
    std::string pad(tab_stop - date_str.size(), ' ');
    date_str += pad;
    }

    // TODO ?? Don't need a new ostringstream each time;
    // if you do it that way, though, there should be
    // some consistency to the numbering.
    // But use value_cast_ihs instead.
    std::ostringstream os;
    os << Input->Status[0].IssueAge;
    std::string age_str("Age at Issue: ");
    age_str += os.str();
    {
    std::string pad(tab_stop - age_str.size(), ' ');
    age_str += pad;
    }

    std::string gender_str = "Gender: " + Input->Status[0].Gender.str();
    {
    std::string pad(tab_stop - gender_str.size(), ' ');
    gender_str += pad;
    }

    calendar_date as_of_date(Input->EffDate);
    as_of_date.add_years_and_months(InforceYear, InforceMonth, true);

    double init_value =
          InforceAVGenAcct
        + InforceAVSepAcct
        + InforceAVRegLn
        + InforceAVPrfLn
        ;

    std::string str_init_value = ncnnnpnn(init_value);

    Project12MosStream.open
        (Project12MosFilename.c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );
    Project12MosStream.setf(std::ios_base::fixed, std::ios_base::floatfield);

    Project12MosStream
        << "\n\n\n"

        << "Policyowner: "
        << InvariantValues().CorpName << '\n'

        << "Franchise Number: "
        << Input->Franchise << '\n'
        ;

    if(0 == InforceYear && 0 == InforceMonth)
        {
        Project12MosStream
            << name_str
            << "To be issued: "
            << as_of_date.str() << '\n'
            ;
        }
    else
        {
        // Input values are as of the day before a monthiversary, so that
        // they don't reflect the following day's monthly deduction.
        Project12MosStream
            << name_str
            << "Policy as of: "
            << (as_of_date - 1).str() << '\n'
            ;
        }

    Project12MosStream
        << number_str
        << "Starting Policy Value: "
        << std::setprecision(2)
        << str_init_value << '\n'

        << date_str
        << "Effective Interest Rate*: "
        << std::setprecision(2)
        << 100.0 * InterestRates_->GenAcctNetRate
            (e_basis(e_currbasis)
            ,e_rate_period(e_annual_rate)
            )[Year]
        << "%\n"

        << age_str
        << "Death Benefit Option: "
        << DeathBfts->GetDBOpt()[Year] << '\n'

        << gender_str
        << "Tobacco Status: "
        << InvariantValues().Smoker << '\n'

        << '\n'

        << "       Month  Mortality &     Interest       Policy         Death\n"
        << "             Admin Cost**     Credited        Value       Benefit\n"
        ;
}

//============================================================================
// To add a new column, see ihs_dbughdr.hpp .
void AccountValue::Project12MosPrintMonth()
{
    if
        (  !Projecting12Mos
        || Solving
        || SolvingForGuarPremium
        || e_run_curr_basis != RateBasis
        )
        {
        return; // Show detail on final run, not every solve iteration.
        }

    if
        (
            (     Month        + 12 * Year       )
        <   (12 + InforceMonth + 12 * InforceYear)
        )
        {
// TODO ?? JOE: An earlier comment, now removed, suggested that
// results would be incorrect if a payment were made during the
// projection period. Yet the following code would seem to handle
// payments during the projection period. Please comment.
        double loads =
            GrossPmts[Month] - NetPmts[Month]
            +   YearsAnnPolFee
            +   YearsMlyPolFee
            +   DetermineSpecAmtLoad()
            +   DetermineAcctValLoadBOM()
            ;
        double interest = GenAcctIntCred + SepAcctIntCred;
        double totalav = AVGenAcct + AVSepAcct + AVRegLn + AVPrfLn;

        // Note that rounding is not required here. Read the html
        // documentation for rounding.
        std::string str_chgs = ncnnnpnn(COI + loads);

        // TODO ?? Here the 'pad' stuff becomes more insidious
        // with the introduction of unexplained manifest constants.
        // How could this ever be maintained?
        //
        // --A maintenence disaster, but here is some explanation:
//        Month  Mortality &     Interest       Policy         Death
//              Admin Cost**     Credited        Value       Benefit
// <----12----><----13-----><-----13----><----13-----><-----14----->
//     November        47.89        89.22    20,041.33
//
// TODO ?? JOE--five fields (widths 12,13,13,13,14) filled with four data?

        {
        std::string pad(13 - str_chgs.length(), ' ');
        str_chgs.insert(0, pad);
        }

        std::string str_interest = ncnnnpnn(interest);
        {
        std::string pad(13-str_interest.length(),' ');
        str_interest.insert(0, pad);
        }

        std::string str_totalav = ncnnnpnn(totalav);
        {
        std::string pad(13-str_totalav.length(),' ');
        str_totalav.insert(0, pad);
        }

        std::string str_DB = ncnnnpnn(DBReflectingCorr);
        {
        std::string pad(14-str_DB.length(),' ');
        str_DB.insert(0, pad);
        }

        // Label each row with the date on which the corresponding
        // monthiversary period ends, which is the day before the
        // next succeeding monthiversary. Note that monthiversary
        // dates must be calculated wrt the effective date rather
        // than wrt any anniversary or monthiversary date because
        // information is lost if the day of the former exceeds
        // the number of days in the latter: for instance, if the
        // contract effective date is 2002-03-31, the eleventh
        // monthiversary is 2003-02-28, but the twelfth is not the
        // twenty-eighth of 2003-03 but rather the thirty-first.
        calendar_date end_of_contract_month(Input->EffDate);
        end_of_contract_month.add_years_and_months(Year, 1 + Month, true);
        --end_of_contract_month;

#ifdef LMI_SHOW_PROJECTION_DATES_ON_LETTER
        Project12MosStream
            << std::setw(0) << "  "
            << std::setprecision(0) << std::setw(12)
            << end_of_contract_month.str()
#else // Not defined(LMI_SHOW_PROJECTION_DATES_ON_LETTER).
// TRICKY !! We use 'c_str()' below because it makes setw() right-align
// the month names.
        Project12MosStream
            << std::setprecision(0) << std::setw(12)
            << calendar_date::month_name(end_of_contract_month.month()).c_str()
#endif // Not defined(LMI_SHOW_PROJECTION_DATES_ON_LETTER).
            << str_chgs
            << str_interest
            << str_totalav
            << str_DB
            << '\n';
        }
}

//============================================================================
void AccountValue::Project12MosPrintEnd()
{
    if
        (  !Projecting12Mos
        || Solving
        || SolvingForGuarPremium
        || e_run_curr_basis != RateBasis
        )
        {
        return;
        }

    Project12MosStream
        << '\n'
        << "*Effective Interest Rate: the effective interest rate is "
        << "equal to the declared crediting rate. The declared "
        << "crediting rate is reviewed on a calendar quarter basis "
        << "and is subject to change by "
        << InvariantValues().InsCoShortName
        << ".\n"
        << "**Mortality and Admin costs include charges for COIs, "
        << "monthly policy fees, and full underwriting charges, if "
        << "any.\n"
        << '\n'
        << "Interest is credited after the deduction of expense and "
        << "mortality charges. The minimum guaranteed annual "
        << "interest rate is 3%.\n"
        << '\n'
        << "Policy Values are projected assuming current mortality "
        << "and current expense charges and an effective interest "
        << "rate. Current mortality and current expense charge "
        << "assumptions and the assumed effective interest rate are "
        << "not guaranteed elements and are subject to change by "
        << InvariantValues().InsCoShortName
        << ". "
        << "Policy values are not guaranteed. It is assumed that no "
        << "changes are made to this policy, that no policy loans "
        << "are issued, that interest on existing policy loans is "
        << "paid when due, and that no further premium payments are "
        << "made.\n"
        << '\n'
        << "***Important Policyowner Notice: You should consider "
        << "requesting more detailed information about your policy "
        << "to understand how it may perform in the future. You "
        << "should not consider replacement of your policy or make "
        << "changes in your coverage without requesting a current "
        << "illustration. You may annually request, without charge "
        << "such an illustration by calling "
        << InvariantValues().InsCoPhone
        << "; by writing to "
        << InvariantValues().InsCoName
        << " at "
        << InvariantValues().InsCoStreet
        << ", "
        << InvariantValues().InsCoAddr
        << " or contacting your agent. If you do not receive a "
        << "current illustration of your policy within 30 days from "
        << "your request, you should contact your state insurance "
        << "department.\n"
        << '\n'
        << InvariantValues().InsCoName
        << ", "
        << InvariantValues().InsCoStreet
        << ", "
        << InvariantValues().InsCoAddr
        << '\n'
        ;
}

