// Product database.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: ihs_dbdict.cpp,v 1.12 2006-01-20 05:59:50 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "ihs_dbdict.hpp"

#include "alert.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "data_directory.hpp"
#include "xenumtypes.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <limits>
#include <sstream>

std::string DBDictionary::CachedFilename;

int const NumberOfEntries = DB_LAST;

//============================================================================
DBDictionary& DBDictionary::instance()
{
    static DBDictionary z;
    return z;
}

//============================================================================
DBDictionary::DBDictionary()
{
}

//============================================================================
DBDictionary::~DBDictionary()
{
}

//============================================================================
void DBDictionary::Init(std::string const& NewFilename)
{
    // Perform the expensive operation of reading the dictionary from
    // file only if the cached file name doesn't match the new file
    // name, or if the cached file name is an empty string--which
    // means either that no dictionary has yet been read, or that the
    // cached file name was deliberately set to an empty string in
    // order to invalidate the cached database.
    //
    // TODO ?? We ought to address the problem that someone might have
    // modified that file in the meantime.

    if
        (
           ""          != CachedFilename
        && NewFilename == CachedFilename
        )
        {
        return;
        }

    CachedFilename = NewFilename;
    dictionary.erase(dictionary.begin(), dictionary.end());

    JRPS::JrPs_ifpstream ips
        (NewFilename.c_str()
        );
    if(!ips)
        {
        BadFile(NewFilename, "could not be found.");
        }
    int n;
    ips >> n;
    if(NumberOfEntries != n)
        {
        std::ostringstream oss;
        oss
            << "is not up to date or is corrupted."
            << " It should contain " << NumberOfEntries
            << " elements, but it actually contains " << n
            << " elements."
            ;
        BadFile(NewFilename, oss.str());
        }
    for(int j = 0; j < n; j++)
        {
        TDBValue* temp;
        ips >> temp;
        if(0 == temp)
            {
            std::ostringstream oss;
            oss
                << "is not up to date or is corrupted."
                << " Its element number " << j
                << ", which is '" << GetDBNames()[j].ShortName
                << "', cannot be read."
                ;
            BadFile(NewFilename, oss.str());
            break;
            }
        dictionary[temp->GetKey()] = *temp;
        delete temp;
        }
}

//============================================================================
// TODO ?? Does this function make the code clearer, or less clear?
void DBDictionary::BadFile(std::string const& Filename, std::string const& why)
{
    // Invalidate cache.
    CachedFilename = "";

    std::string s = ", which is required for the product selected, ";
    s += why;
    s += " Try reinstalling. Other products might work in the meantime.";

    // It's generally pointless to proceed.
    if(global_settings::instance().mellon())
        {
        hobsons_choice() << "File '" << Filename << "'" << s << LMI_FLUSH;
        }
    else
        {
        fatal_error() << "File '" << Filename << "'" << s << LMI_FLUSH;
        }
}

//============================================================================
void DBDictionary::WriteDB(std::string const& filename)
{
    JRPS::JrPs_ofpstream ops
        (filename.c_str()
        ,JRPS::JrPs_pstream::xxtrunc | JRPS::JrPs_pstream::xxcreat
        );
    if(!ops)
        {
        fatal_error()
            << "Cannot open database file '" << filename << "'."
            << LMI_FLUSH
            ;
        }
    if(NumberOfEntries != static_cast<int>(dictionary.size()))
        {
        warning()
            << "Error writing database '"
            << filename
            << "': the database has " << dictionary.size()
            << " entries, but should have " << NumberOfEntries << '.'
            ;
        for(int j = 0; j < NumberOfEntries; j++)
            {
            if(!dictionary.count(j))
                {
                warning() << " Key " << j << " not found.";
                }
            }
        warning() << LMI_FLUSH;
        }
    ops << dictionary.size();
    for(unsigned int j = 0; j < dictionary.size(); j++)
        {
        ops << &dictionary[j];
        }
}

//===========================================================================
void DBDictionary::Add(TDBValue const& e)
{
    dictionary.erase(e.GetKey());
    dictionary.insert(dict_map_val(e.GetKey(), e));
}

//============================================================================
// Initialize all database entities to not-necessarily-plausible values.
void DBDictionary::InitDB()
{
    dictionary.erase(dictionary.begin(), dictionary.end());
    for(int j = DB_FIRST; j < DB_LAST; ++j)
        {
        Add(TDBValue(j, 0.0));
        }

    // It would be dangerous to set these to zero.
    Add(TDBValue(DB_CCOIMultiplier      , 1.0));
    Add(TDBValue(DB_GCOIMultiplier      , 1.0));
    Add(TDBValue(DB_SubstdTblMult       , 1.0));
    Add(TDBValue(DB_SurrChgSADurFactor  , 1.0));
    Add(TDBValue(DB_SurrChgAVDurFactor  , 1.0));

    // Generally you would want a value such as 1/12 or 1/11 here.
    Add(TDBValue(DB_MaxMonthlyCoiRate   , 1.0 / 12.0));

    Add(TDBValue(DB_GuarIntSpread, std::numeric_limits<double>::max()));

    Add(TDBValue(DB_CurrCOITable0Limit  , std::numeric_limits<double>::max()));
    Add(TDBValue(DB_CurrCOITable1       , 999));
    Add(TDBValue(DB_CurrCOITable1Limit  , std::numeric_limits<double>::max()));
    Add(TDBValue(DB_CurrCOITable2       , 999));

    double const bignum = std::numeric_limits<double>::max();
    int premium_tax_dimensions[TDBValue::e_number_of_axes] = {1, 1, 1, 1, 1, 53, 1};
    double premium_tax_retaliation_threshold[53] =
        {
    //  AL      AK      AZ      AR      CA      CO      CT
        bignum, 0.0   , bignum, bignum, bignum, bignum, bignum,
    //  DE      DC      FL      GA      HI      ID
        bignum, bignum, bignum, bignum, bignum, bignum,
    //  IL      IN      IA      KS      KY      LA      ME
        bignum, bignum, bignum, bignum, bignum, bignum, bignum,
    //  MD      MA      MI      MN      MS      MO
        bignum, bignum, bignum, bignum, bignum, bignum,
    //  MT      NE      NV      NH      NJ      NM      NY
        bignum, bignum, bignum, bignum, bignum, bignum, bignum,
    //  NC      ND      OH      OK      OR      PA
        bignum, bignum, bignum, bignum, bignum, bignum,
    //  PR      RI      SC      SD      TN      TX      UT
        bignum, bignum, bignum, 1E6   , bignum, bignum, bignum,
    //  VT      VA      WA      WV      WI      WY      XX
        bignum, bignum, bignum, bignum, bignum, bignum, 0.0   ,
        };
    Add
        (TDBValue
            (DB_PremTaxRetalLimit
            ,TDBValue::e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_retaliation_threshold
            )
        );
}

//============================================================================
void DBDictionary::WriteSampleDBFile()
{
    InitDB();
    Add(TDBValue(DB_GuarPolFee          , 8.00));
    Add(TDBValue(DB_GuarSpecAmtLoad     , 0.0));
    Add(TDBValue(DB_GuarIssueFee        , 0.0));
    Add(TDBValue(DB_GuarFundAdminChg    , 0.0));
    Add(TDBValue(DB_GuarPremLoadTgt     , 0.07));
    Add(TDBValue(DB_GuarPremLoadExc     , 0.04));
    Add(TDBValue(DB_GuarPremLoadTgtRfd  , 0.00));
    Add(TDBValue(DB_GuarPremLoadExcRfd  , 0.00));
    Add(TDBValue(DB_GuarAcctValLoadAMD  , 0.0));
    Add(TDBValue(DB_CurrPolFee          , 5.00));
    Add(TDBValue(DB_CurrSpecAmtLoad     , 0.0));
    Add(TDBValue(DB_CurrIssueFee        , 0.0));
    Add(TDBValue(DB_CurrFundAdminChg    , 0.0));
    Add(TDBValue(DB_CurrPremLoadTgt     , 0.05));
    Add(TDBValue(DB_CurrPremLoadExc     , 0.02));
    Add(TDBValue(DB_CurrPremLoadTgtRfd  , 0.00));
    Add(TDBValue(DB_CurrPremLoadExcRfd  , 0.00));
    Add(TDBValue(DB_CurrAcctValLoadAMD  , 0.0));
    Add(TDBValue(DB_DACTaxPremLoad      , 0.01));
    Add(TDBValue(DB_FundCharge          , 0.0));
    Add(TDBValue(DB_PremTaxFundCharge   , 0.0));
    Add(TDBValue(DB_DACTaxFundCharge    , 0.0));
    Add(TDBValue(DB_WaivePmTxInt1035    , true));
    Add(TDBValue(DB_FirstWDYear         , 0.0));
    Add(TDBValue(DB_MaxWDAVMult         , 1.0));
    Add(TDBValue(DB_MaxWDDed            , e_to_next_anniversary));
    Add(TDBValue(DB_MinWD               , 100.0));
    Add(TDBValue(DB_WDFee               , 25.0));
    Add(TDBValue(DB_WDFeeRate           , 0.02));
    Add(TDBValue(DB_WDCanDecrSADBO1     , true));
    Add(TDBValue(DB_WDCanDecrSADBO2     , true));
    Add(TDBValue(DB_WDCanDecrSADBO3     , true));
    Add(TDBValue(DB_FirstLoanYear       , 0.0));
    Add(TDBValue(DB_AllowPrefLoan       , false));
    Add(TDBValue(DB_AllowFixedLoan      , true));
    Add(TDBValue(DB_FixedLoanRate       , 0.06));
    Add(TDBValue(DB_AllowVLR            , true));
    Add(TDBValue(DB_MaxLoanAVMult       , 1.0));
    Add(TDBValue(DB_MaxLoanDed          , e_to_next_anniversary));
    Add(TDBValue(DB_GuarPrefLoanSpread  , 0.0));
    Add(TDBValue(DB_GuarRegLoanSpread   , 0.04));
    Add(TDBValue(DB_CurrPrefLoanSpread  , 0.0));
    Add(TDBValue(DB_CurrRegLoanSpread   , 0.02));
    Add(TDBValue(DB_GuarInt             , 0.03));
    Add(TDBValue(DB_NAARDiscount        , 1.0 / 1.00246627));
    Add(TDBValue(DB_GuarIntSpread       , 0.03));
    Add(TDBValue(DB_GuarMandE           , 0.009));
    Add(TDBValue(DB_CurrIntSpread       , 0.01));
    Add(TDBValue(DB_CurrMandE           , 0.009));
    Add(TDBValue(DB_BonusInt            , 0.0));
    Add(TDBValue(DB_IntFloor            , 0.0));
    Add(TDBValue(DB_SepAcctSpreadMethod , e_spread_is_effective_annual));
    Add(TDBValue(DB_DynamicMandE        , false));

    // gender, smoker
    int dims313[TDBValue::e_number_of_axes] = {3, 1, 3, 1, 1, 1, 1};

    // US 1980 CSO age last; unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    double TgCOI[9] =
        {
         39,  37,  35, // female: sm ns us
         45,  57,  41, // male:   sm ns us
        111, 109, 107, // unisex: sm ns us
        };

    // For now at least, just use (a multiple of) guaranteed COI rates
    // as current.
    Add(TDBValue(DB_CurrCOITable, TDBValue::e_number_of_axes, dims313, TgCOI));
    Add(TDBValue(DB_GuarCOITable, TDBValue::e_number_of_axes, dims313, TgCOI));

    Add(TDBValue(DB_COINYMinTable       , 0.0));

    double coimult[9] =
        {
        0.40, 0.30, 0.35, // female: sm ns us
        0.60, 0.50, 0.55, // male:   sm ns us
        0.50, 0.40, 0.45, // unisex: sm ns us
        };
    Add(TDBValue(DB_CCOIMultiplier, TDBValue::e_number_of_axes, dims313, coimult));

    Add(TDBValue(DB_UseNYCOIFloor       , 0.0));
    Add(TDBValue(DB_GuarCOICeiling      , 0.0));
    Add(TDBValue(DB_COIGuarIsMin        , 0.0));
    Add(TDBValue(DB_COINonforfIsGuar    , 0.0));
    Add(TDBValue(DB_CCoiIsAnnual        , true));
    Add(TDBValue(DB_GCoiIsAnnual        , true));
    Add(TDBValue(DB_MCoiIsAnnual        , true));
    Add(TDBValue(DB_AgeLastOrNearest    , 0)); // ALB
    Add(TDBValue(DB_AllowRetirees       , true));
    Add(TDBValue(DB_MinSpecAmt          , 100000.0));
    Add(TDBValue(DB_AllowSubstdTable    , true));
    Add(TDBValue(DB_AllowFlatExtras     , true));
    Add(TDBValue(DB_MinIssAge           , 15));
    Add(TDBValue(DB_MaxIssAge           , 70));
    Add(TDBValue(DB_MinIssFace          , 0.0));
    Add(TDBValue(DB_MaxIssFace          , 0.0));
    Add(TDBValue(DB_MinRenlBaseFace     , 50000.0));
    Add(TDBValue(DB_MinRenlFace         , 50000.0));
    Add(TDBValue(DB_MaxRenlFace         , 0.0));
    Add(TDBValue(DB_MinFaceIncr         , 0.0));
    Add(TDBValue(DB_MaxIncrAge          , 99));
    Add(TDBValue(DB_MinPmt              , 0.0));
    Add(TDBValue(DB_SmokeOrTobacco      , e_tobacco_nontobacco));
    Add(TDBValue(DB_AllowUnisex         , true));
    Add(TDBValue(DB_AllowSexDistinct    , true));
    Add(TDBValue(DB_AllowUnismoke       , true));
    Add(TDBValue(DB_AllowSmokeDistinct  , true));
    Add(TDBValue(DB_AllowFullUW         , true));
    Add(TDBValue(DB_AllowSimpUW         , true));
    Add(TDBValue(DB_AllowGuarUW         , true));
    Add(TDBValue(DB_AllowMortBlendSex   , true));
    Add(TDBValue(DB_AllowMortBlendSmoke , true));
    Add(TDBValue(DB_AllowRatedWP        , true));
    Add(TDBValue(DB_AllowRatedADD       , true));
    Add(TDBValue(DB_AllowRatedTerm      , true));
    Add(TDBValue(DB_Allowable           , true));
    Add(TDBValue(DB_AllowPreferredClass , true));
    Add(TDBValue(DB_AllowCVAT           , true));
    Add(TDBValue(DB_AllowGPT            , true));

    // This is just a sample product, so we make do with plausible
    // all-male seven-pay premiums, and use GPT corridor factors for
    // CVAT.
    Add(TDBValue(DB_CorridorTable       , 7));
    Add(TDBValue(DB_TAMRA7PayTable      , 10));

    // Following IRS Notice 88-128, use only the male and female
    // tables with no smoker distinction, and a unisex table where
    // required by state law.
    //
    // US 1980 CSO age last, not smoker distinct. Unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    int dims311[TDBValue::e_number_of_axes] = {3, 1, 1, 1, 1, 1, 1}; // gender
    double T7702q[9] = {35, 41, 107,}; // Female, male, unisex.
    Add(TDBValue(DB_IRC7702QTable, TDBValue::e_number_of_axes, dims311, T7702q));

    Add(TDBValue(DB_PremLoad7702        , 0.02));
    Add(TDBValue(DB_AllowDBO1           , true));
    Add(TDBValue(DB_AllowDBO2           , true));
    Add(TDBValue(DB_AllowDBO3           , true));
    Add(TDBValue(DB_OptChgCanIncrSA     , true));
    Add(TDBValue(DB_OptChgCanDecrSA     , true));
    Add(TDBValue(DB_NonforfQTable       , 0.0));
    Add(TDBValue(DB_SurrChgByFormula    , 0.0));
    Add(TDBValue(DB_SurrChgPeriod       , 0.0));
    Add(TDBValue(DB_SurrChgZeroDur      , 0.0));
    Add(TDBValue(DB_SurrChgNLPMult      , 0.0));
    Add(TDBValue(DB_SurrChgNLPMax       , 0.0));
    Add(TDBValue(DB_SurrChgEAMax        , 0.0));
    Add(TDBValue(DB_SurrChgPremMult     , 0.0));
    Add(TDBValue(DB_SurrChgIsMly        , 0.0));

    // These aren't actual premium tax rates. Actual rates change
    // often, and depend on the insurer's domicile because of
    // retaliation. Instead of giving rates that appear to be 'right'
    // but could be valid only on a certain date in a certain
    // domicile, we use two percent in every state except AK and SD
    // because those two states have a tiered premium tax that this
    // program can handle, and except fictitious state XX, which may
    // be used where no premium tax applies, as for offshore business.
    // DE has a tiered premium tax that this program cannot yet
    // handle, so we punt and use two percent in DE.
    int premium_tax_dimensions[TDBValue::e_number_of_axes] = {1, 1, 1, 1, 1, 53, 1};
    double const tiered = 0.0;
    double premium_tax_rates[53] =
        {
    //  AL      AK      AZ      AR      CA      CO      CT
        0.0200, tiered, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  DE      DC      FL      GA      HI      ID
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  IL      IN      IA      KS      KY      LA      ME
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  MD      MA      MI      MN      MS      MO
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  MT      NE      NV      NH      NJ      NM      NY
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  NC      ND      OH      OK      OR      PA
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200,
    //  PR      RI      SC      SD      TN      TX      UT
        0.0200, 0.0200, 0.0200, tiered, 0.0200, 0.0200, 0.0200,
    //  VT      VA      WA      WV      WI      WY      XX
        0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0200, 0.0000,
        };
    Add
        (TDBValue
            (DB_PremTaxRate
            ,TDBValue::e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_rates
            )
        );

    Add(TDBValue(DB_PremTaxState        , e_ee_state));
    Add(TDBValue(DB_EndtAge             , 100));
    Add(TDBValue(DB_AllowExtEndt        , true));
    Add(TDBValue(DB_AllowGenAcct        , true));
    Add(TDBValue(DB_AllowSepAcct        , true));
    Add(TDBValue(DB_MinPremType         , e_monthly_deduction));
    Add(TDBValue(DB_TgtPremType         , e_modal_nonmec));
    Add(TDBValue(DB_TgtPmFixedAtIssue   , false));
    Add(TDBValue(DB_TgtPmIgnoreSubstd   , true));
    Add(TDBValue(DB_NoLapseMinDur       , 0.0));
    Add(TDBValue(DB_NoLapseMinAge       , 0.0));
    Add(TDBValue(DB_NoLapseUnratedOnly  , false));
    Add(TDBValue(DB_NoLapseOpt1Only     , false));
    Add(TDBValue(DB_PremRefund          , 0.0));
    // Reuse current COI rates as current and guaranteed term rates.
    Add(TDBValue(DB_TermTable, TDBValue::e_number_of_axes, dims313, TgCOI));
    Add(TDBValue(DB_GuarTermTable, TDBValue::e_number_of_axes, dims313, TgCOI));
    Add(TDBValue(DB_AllowTerm           , true));
    Add(TDBValue(DB_TermMinIssAge       , 0.0));
    Add(TDBValue(DB_TermMaxIssAge       , 0.0));
    Add(TDBValue(DB_TermForcedConvAge   , 0.0));
    Add(TDBValue(DB_MaxTermProportion   , 0.0));
    Add(TDBValue(DB_TermCOIRate         , 0.0));
    Add(TDBValue(DB_TermPremRate        , 0.0));
    Add(TDBValue(DB_WPTable             , 8));
    Add(TDBValue(DB_AllowWP             , true));
    Add(TDBValue(DB_WPMinIssAge         , 0.0));
    Add(TDBValue(DB_WPMaxIssAge         , 0.0));
    Add(TDBValue(DB_WPMax               , 0.0));
    Add(TDBValue(DB_WPCOIRate           , 0.0));
    Add(TDBValue(DB_WPPremRate          , 0.0));
    // SOA qx_ins table 708 is 70-75 US ADB experience.
    Add(TDBValue(DB_ADDTable            , 708));
    Add(TDBValue(DB_AllowADD            , true));
    Add(TDBValue(DB_ADDMinIssAge        , 0.0));
    Add(TDBValue(DB_ADDMaxIssAge        , 0.0));
    Add(TDBValue(DB_ADDLimit            , 1000000.0));
    Add(TDBValue(DB_ADDCOIRate          , 0.0));
    Add(TDBValue(DB_ADDPremRate         , 0.0));
    Add(TDBValue(DB_WeightClass         , 0.0));
    Add(TDBValue(DB_WeightGender        , 0.0));
    Add(TDBValue(DB_WeightSmoking       , 0.0));
    Add(TDBValue(DB_WeightAge           , 0.0));
    Add(TDBValue(DB_WeightFace          , 0.0));
    Add(TDBValue(DB_WeightState         , 0.0));
    Add(TDBValue(DB_FullExpPol          , 0.0));
    Add(TDBValue(DB_FullExpPrem         , 0.0));
    Add(TDBValue(DB_FullExpDumpin       , 0.0));
    Add(TDBValue(DB_FullExpPerK         , 0.0));
    Add(TDBValue(DB_VarExpPol           , 0.0));
    Add(TDBValue(DB_VarExpPrem          , 0.0));
    Add(TDBValue(DB_VarExpDumpin        , 0.0));
    Add(TDBValue(DB_VarExpPerK          , 0.0));
    Add(TDBValue(DB_ExpPerKLimit        , 0.0));
    Add(TDBValue(DB_MedicalProportion   , 0.0));
    Add(TDBValue(DB_UWTestCost          , 0.0));
    Add(TDBValue(DB_VxBasicQTable       , 0.0));
    Add(TDBValue(DB_VxDeficQTable       , 0.0));
    Add(TDBValue(DB_VxTaxQTable         , 0.0));
    Add(TDBValue(DB_StatVxInt           , 0.0));
    Add(TDBValue(DB_TaxVxInt            , 0.0));
    Add(TDBValue(DB_StatVxQ             , 0.0));
    Add(TDBValue(DB_TaxVxQ              , 0.0));
    Add(TDBValue(DB_DefVxQ              , 0.0));
    Add(TDBValue(DB_NonforfQ            , 0.0));
    Add(TDBValue(DB_CompTarget          , 0.0));
    Add(TDBValue(DB_CompExcess          , 0.0));
    Add(TDBValue(DB_CompChargeBack      , 0.0));
    Add(TDBValue(DB_LapseRate           , 0.0));
    Add(TDBValue(DB_ReqSurpNAAR         , 0.0));
    Add(TDBValue(DB_ReqSurpVx           , 0.0));
    Add(TDBValue(DB_LICFitRate          , 0.0));
    Add(TDBValue(DB_LicDacTaxRate       , 0.0));
    Add(TDBValue(DB_GDBVxMethod         , 0.0));
    Add(TDBValue(DB_PrimaryHurdle       , 0.0));
    Add(TDBValue(DB_SecondaryHurdle     , 0.0));
    Add(TDBValue(DB_LedgerType          , e_ill_reg));
    Add(TDBValue(DB_AllowExpRating      , false));
    // These aren't really NY Table Y group rates--in fact, they're
    // US 65-70 male ALB. But Table Y is not commonly used anyway.
    Add(TDBValue(DB_TableYTable         , 358));

    // Use male rates for unisex--1983 GAM seems to have no unisex version.
    double T83Gam[3] = {825, 826, 826,};
    Add(TDBValue(DB_83GamTable, TDBValue::e_number_of_axes, dims311, T83Gam));

    Add(TDBValue(DB_AllowWD             , true));
    Add(TDBValue(DB_AllowLoan           , true));
    Add(TDBValue(DB_AllowChangeToDBO2   , true));
    Add(TDBValue(DB_AllowSAIncr         , true));
    Add(TDBValue(DB_NoLapseAlwaysActive , false));
    Add(TDBValue(DB_PrefOrSelect        , e_called_select));
    Add(TDBValue(DB_ExpRatStdDevMult    , 0.0));
    Add(TDBValue(DB_ExpRatIBNRMult      , 0.0));
    Add(TDBValue(DB_ExpRatCOIRetention  , 0.0));
    Add(TDBValue(DB_StableValFundCharge , 0.0));
    Add(TDBValue(DB_AmortPmLdFundCharge , 0.0030));
    Add(TDBValue(DB_AllowAmortPremLoad  , false));
    Add(TDBValue(DB_PmTxAmortPeriod     , 0));
    Add(TDBValue(DB_PmTxAmortIntRate    , 0.0));
    // Pass through premium tax.
    Add
        (TDBValue
            (DB_PremTaxLoad
            ,TDBValue::e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_rates
            )
        );
    Add(TDBValue(DB_AllowHoneymoon      , true));
    // Set target equal to seven-pay premium.
    Add(TDBValue(DB_TgtPremTable        , 10));
    Add(TDBValue(DB_TgtPremPolFee       , 0.0));
    Add(TDBValue(DB_AllowExtraAssetComp , true));
    Add(TDBValue(DB_AllowExtraPremComp  , true));
    Add(TDBValue(DB_AssetChargeType     , e_asset_charge_spread));
    Add(TDBValue(DB_AllowUltraPrefClass , false));
    Add(TDBValue(DB_MaxGenAcctRate      , 0.06));
    Add(TDBValue(DB_MaxSepAcctRate      , 0.12));
    Add(TDBValue(DB_MaxVLRRate          , 0.18));
    Add(TDBValue(DB_SurrChgAVMult       , 0.0));
    Add(TDBValue(DB_IntSpreadFreq       , e_spread_daily));
    Add(TDBValue(DB_StateApproved       , true));
    Add(TDBValue(DB_AllowStateXX        , true));
    Add(TDBValue(DB_AllowForeign        , true));
    Add(TDBValue(DB_AllowCustomFund     , false));
    Add(TDBValue(DB_AllowNo7702         , false));
    Add(TDBValue(DB_EnforceNAARLimit    , true));
    Add(TDBValue(DB_DynamicSepAcctLoad  , false));
    Add(TDBValue(DB_SpecAmtLoadLimit    , 10000000.0));
    Add(TDBValue(DB_Equiv7702DBO3       , 0));
    Add(TDBValue(DB_NonUSCorridorPivot  , 0));
    Add(TDBValue(DB_ExpRatRiskCOIMult   , 0));
    Add(TDBValue(DB_SurrChgSAMult       , 0.0));
    Add(TDBValue(DB_AllowSpouse         , false));
    Add(TDBValue(DB_AllowChild          , false));

    // Spouse and child riders unavailable, so it doesn't matter
    // what table we specify.
    Add(TDBValue(DB_SpouseRiderTable    , 708));
    Add(TDBValue(DB_ChildRiderTable     , 708));

    Add(TDBValue(DB_GAIntBonus          , 0.0));

    // Allow experience rating.
    Add(TDBValue(DB_AllowExpRating      , 1.0));
    Add(TDBValue(DB_ExpRatIBNRMult      , 6.0));
    Add(TDBValue(DB_ExpRatAmortPeriod   , 4.0));

    WriteDB(AddDataDir("sample.db4"));
}

//============================================================================
void print_databases()
{
    fs::path path(global_settings::instance().data_directory());
    fs::directory_iterator i(path);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        if(is_directory(*i) || ".db4" != fs::extension(*i))
            {
            continue;
            }

        DBDictionary::instance().Init(i->string());
        fs::path out_file = fs::change_extension(*i, ".dbt");
        fs::ofstream os(out_file, std::ios_base::out | std::ios_base::trunc);
        dict_map& dictionary = DBDictionary::instance().GetDictionary();
        // std::ostream_iterator not used because it doesn't work
        // nicely with std::map (a name-lookup issue).
        for(unsigned int j = 0; j < dictionary.size(); j++)
            {
            os << dictionary[j];
            }
        }
}

