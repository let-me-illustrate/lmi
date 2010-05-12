// Product-database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// $Id$

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "dbdict.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "data_directory.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "mc_enum_type_enums.hpp"
#include "miscellany.hpp"
#include "oecumenic_enumerations.hpp"
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <limits>

std::string DBDictionary::cached_filename_;

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

namespace xml_serialize
{
template<> struct xml_io<database_entity>
{
    typedef database_entity T;
    static void   to_xml(xml::element& e, T const& t) {t.write(e);}
    static void from_xml(xml::element const& e, T& t) {t.read (e);}
};

/// Specialize xml_io<> for dict_map rather than coding a generic
/// xml_io<std::map>, because the key would be stored redundantly:
/// it's already part of class database_entity.

template<> struct xml_io<dict_map>
{
    static void to_xml(xml::element& e, dict_map const& t)
    {
        e.erase(e.begin(), e.end());
        typedef dict_map::const_iterator tci;
        for(tci i = t.begin(); i != t.end(); ++i)
            {
            // This is not equivalent to calling set_element():
            // multiple <item> elements are expressly permitted.
            xml::element z("item");
            xml_serialize::to_xml(z, i->second);
            e.push_back(z);
            }
    }

    static void from_xml(xml::element const& e, dict_map& t)
    {
        t.clear();
        xml::const_nodes_view const items(e.elements("item"));
        typedef xml::const_nodes_view::const_iterator cnvi;
        for(cnvi i = items.begin(); i != items.end(); ++i)
            {
            database_entity z;
            xml_serialize::from_xml(*i, z);
            t[z.key()] = z;
            }
    }
};
} // namespace xml_serialize

namespace
{
std::string xml_root_name()
{
    return "database";
}
} // Unnamed namespace.

dict_map const& DBDictionary::GetDictionary() const
{
    return dictionary_;
}

/// Read and cache a database file.
///
/// Perform the expensive operation of reading the dictionary from
/// file only if the cached file name doesn't match the new filename.

void DBDictionary::Init(std::string const& filename)
{
    LMI_ASSERT(!filename.empty());
    if(filename == cached_filename_)
        {
        return;
        }

    cached_filename_ = filename;

    if(access(filename.c_str(), R_OK))
        {
        InvalidateCache();
        fatal_error()
            << "File '"
            << filename
            << "' is required but could not be found. Try reinstalling."
            << LMI_FLUSH
            ;
        }

    xml_lmi::dom_parser parser(filename);
    xml::element const& root = parser.root_node(xml_root_name());

    xml_serialize::from_xml(root, dictionary_);

    if(NumberOfEntries != static_cast<int>(dictionary_.size()))
        {
        InvalidateCache();
        fatal_error()
            << "is not up to date or is corrupted."
            << " It should contain " << NumberOfEntries
            << " elements, but it actually contains " << dictionary_.size()
            << " elements."
            << LMI_FLUSH
            ;
        }
}

/// Cause next Init() call to read from file instead of using cache.
///
/// The implementation simply sets the cached filename to an empty
/// string, which is its initial value upon (static) construction and
/// cannot validly name any file.

void DBDictionary::InvalidateCache()
{
    cached_filename_.clear();
}

//============================================================================
void DBDictionary::WriteDB(std::string const& filename)
{
    InvalidateCache();
    if(NumberOfEntries != static_cast<int>(dictionary_.size()))
        {
        fatal_error()
            << "Error writing database '"
            << filename
            << "': the database has " << dictionary_.size()
            << " entries, but should have " << NumberOfEntries << '.'
            ;
        for(int j = 0; j < NumberOfEntries; j++)
            {
            if(!dictionary_.count(j))
                {
                fatal_error() << " Key " << j << " not found.";
                }
            }
        fatal_error() << LMI_FLUSH;
        }

    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();

    xml_lmi::set_attr(root, "version", "0");
    xml_serialize::to_xml(root, dictionary_);

    // Instead of this:
//    document.save(filename);
    // for the nonce, explicitly change the extension, in order to
    // force external product-file code to use the new extension.
    fs::path path(filename, fs::native);
    path = fs::change_extension(path, ".database");
    document.save(path.string());
}

/// Add an entry to the dictionary.

void DBDictionary::Add(database_entity const& e)
{
    dictionary_[e.key()] = e;
}

/// Initialize all database entities to not-necessarily-plausible values.

void DBDictionary::InitDB()
{
    static double const bignum = std::numeric_limits<double>::max();

    dictionary_.clear();
    for(int j = DB_FIRST; j < DB_LAST; ++j)
        {
        Add(database_entity(j, 0.0));
        }

    // It would be dangerous to set these to zero.
    Add(database_entity(DB_CCOIMultiplier      , 1.0));
    Add(database_entity(DB_GCOIMultiplier      , 1.0));
    Add(database_entity(DB_SubstdTblMult       , 1.0));
    Add(database_entity(DB_SurrChgSADurFactor  , 1.0));
    Add(database_entity(DB_SurrChgAVDurFactor  , 1.0));

    // Usually the maximum is a reciprocal, e.g., 1/11 or 1/12; for
    // greatest precision, store the reciprocal of that reciprocal,
    // e.g., 11 or 12.
    Add(database_entity(DB_MaxMonthlyCoiRate   , 12.0));

    Add(database_entity(DB_GuarIntSpread       , bignum));

    Add(database_entity(DB_CurrCOITable0Limit  , bignum));
    Add(database_entity(DB_CurrCOITable1       , 999));
    Add(database_entity(DB_CurrCOITable1Limit  , bignum));
    Add(database_entity(DB_CurrCOITable2       , 999));

    Add(database_entity(DB_SpecAmtLoadLimit    , bignum));
    Add(database_entity(DB_DynSepAcctLoadLimit , bignum));
    Add(database_entity(DB_ADDLimit            , bignum));
    Add(database_entity(DB_ExpPerKLimit        , bignum));

    // SD Chapter 260 (HB 1200), signed 2008-02-19, amended 58-6-70
    // by removing the former million-dollar threshold.
    //
    // TODO ?? For now, only the threshold here is changed. Much
    // complex code elsewhere can be removed when time permits.

    int premium_tax_dimensions[e_number_of_axes] = {1, 1, 1, 1, 1, 53, 1};
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
        bignum, bignum, bignum, 0.0   , bignum, bignum, bignum,
    //  VT      VA      WA      WV      WI      WY      XX
        bignum, bignum, bignum, bignum, bignum, bignum, 0.0   ,
        };
    Add
        (database_entity
            (DB_PremTaxRetalLimit
            ,e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_retaliation_threshold
            )
        );
}

//============================================================================
void DBDictionary::WriteSampleDBFile()
{
    InitDB();
    Add(database_entity(DB_GuarPolFee          , 8.00));
    Add(database_entity(DB_GuarSpecAmtLoad     , 0.0));
    Add(database_entity(DB_GuarIssueFee        , 0.0));
    Add(database_entity(DB_GuarFundAdminChg    , 0.0));
    Add(database_entity(DB_GuarPremLoadTgt     , 0.07));
    Add(database_entity(DB_GuarPremLoadExc     , 0.04));
    Add(database_entity(DB_GuarPremLoadTgtRfd  , 0.00));
    Add(database_entity(DB_GuarPremLoadExcRfd  , 0.00));
    Add(database_entity(DB_GuarAcctValLoadAMD  , 0.0));
    Add(database_entity(DB_CurrPolFee          , 5.00));
    Add(database_entity(DB_CurrSpecAmtLoad     , 0.0));
    Add(database_entity(DB_CurrIssueFee        , 0.0));
    Add(database_entity(DB_CurrFundAdminChg    , 0.0));
    Add(database_entity(DB_CurrPremLoadTgt     , 0.05));
    Add(database_entity(DB_CurrPremLoadExc     , 0.02));
    Add(database_entity(DB_CurrPremLoadTgtRfd  , 0.00));
    Add(database_entity(DB_CurrPremLoadExcRfd  , 0.00));
    Add(database_entity(DB_CurrAcctValLoadAMD  , 0.0));
    Add(database_entity(DB_DACTaxPremLoad      , 0.01));
    Add(database_entity(DB_FundCharge          , 0.0));
    Add(database_entity(DB_PremTaxFundCharge   , 0.0));
    Add(database_entity(DB_DACTaxFundCharge    , 0.0));
    Add(database_entity(DB_WaivePmTxInt1035    , true));
    Add(database_entity(DB_FirstWDYear         , 0.0));
    Add(database_entity(DB_MaxWDAVMult         , 1.0));
    Add(database_entity(DB_MaxWDDed            , mce_to_next_anniversary));
    Add(database_entity(DB_MinWD               , 100.0));
    Add(database_entity(DB_WDFee               , 25.0));
    Add(database_entity(DB_WDFeeRate           , 0.02));
    Add(database_entity(DB_WDCanDecrSADBO1     , true));
    Add(database_entity(DB_WDCanDecrSADBO2     , true));
    Add(database_entity(DB_WDCanDecrSADBO3     , true));
    Add(database_entity(DB_FirstLoanYear       , 0.0));
    Add(database_entity(DB_AllowPrefLoan       , false));
    Add(database_entity(DB_AllowFixedLoan      , true));
    Add(database_entity(DB_FixedLoanRate       , 0.06));
    Add(database_entity(DB_AllowVLR            , true));
    Add(database_entity(DB_MaxLoanAVMult       , 1.0));
    Add(database_entity(DB_MaxLoanDed          , mce_to_next_anniversary));
    Add(database_entity(DB_GuarPrefLoanSpread  , 0.0));
    Add(database_entity(DB_GuarRegLoanSpread   , 0.04));
    Add(database_entity(DB_CurrPrefLoanSpread  , 0.0));
    Add(database_entity(DB_CurrRegLoanSpread   , 0.02));
    Add(database_entity(DB_GuarInt             , 0.03));
    Add(database_entity(DB_NAARDiscount        , 0.00246627));
    Add(database_entity(DB_GuarIntSpread       , 0.03));
    Add(database_entity(DB_GuarMandE           , 0.009));
    Add(database_entity(DB_CurrIntSpread       , 0.01));
    Add(database_entity(DB_CurrMandE           , 0.009));
    Add(database_entity(DB_BonusInt            , 0.0));
    Add(database_entity(DB_IntFloor            , 0.0));
    Add(database_entity(DB_SepAcctSpreadMethod , mce_spread_is_effective_annual));
    Add(database_entity(DB_DynamicMandE        , false));

    // gender, smoker
    int dims313[e_number_of_axes] = {3, 1, 3, 1, 1, 1, 1};

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
    Add(database_entity(DB_CurrCOITable, e_number_of_axes, dims313, TgCOI));
    Add(database_entity(DB_GuarCOITable, e_number_of_axes, dims313, TgCOI));

    Add(database_entity(DB_COINYMinTable       , 0.0));

    double coimult[9] =
        {
        0.40, 0.30, 0.35, // female: sm ns us
        0.60, 0.50, 0.55, // male:   sm ns us
        0.50, 0.40, 0.45, // unisex: sm ns us
        };
    Add(database_entity(DB_CCOIMultiplier, e_number_of_axes, dims313, coimult));

    Add(database_entity(DB_UseNYCOIFloor       , 0.0));
    Add(database_entity(DB_GuarCOICeiling      , 0.0));
    Add(database_entity(DB_COIGuarIsMin        , 0.0));
    Add(database_entity(DB_COINonforfIsGuar    , 0.0));
    Add(database_entity(DB_CCoiIsAnnual        , true));
    Add(database_entity(DB_GCoiIsAnnual        , true));
    Add(database_entity(DB_MCoiIsAnnual        , true));
    Add(database_entity(DB_AgeLastOrNearest    , 0, "0 = ALB")); // ALB
    Add(database_entity(DB_AllowRetirees       , true));
    Add(database_entity(DB_MinSpecAmt          , 100000.0));
    Add(database_entity(DB_AllowSubstdTable    , true));
    Add(database_entity(DB_AllowFlatExtras     , true));
    Add(database_entity(DB_MinIssAge           , 15));
    Add(database_entity(DB_MaxIssAge           , 70));
    Add(database_entity(DB_MinIssSpecAmt       , 0.0));
    Add(database_entity(DB_MaxIssSpecAmt       , 0.0));
    Add(database_entity(DB_MinRenlBaseSpecAmt  , 50000.0));
    Add(database_entity(DB_MinRenlSpecAmt      , 50000.0));
    Add(database_entity(DB_MaxRenlSpecAmt      , 0.0));
    Add(database_entity(DB_MinSpecAmtIncr      , 0.0));
    Add(database_entity(DB_MaxIncrAge          , 99));
    Add(database_entity(DB_MinPmt              , 0.0));
    Add(database_entity(DB_SmokeOrTobacco      , oe_tobacco_nontobacco));
    Add(database_entity(DB_AllowUnisex         , true));
    Add(database_entity(DB_AllowSexDistinct    , true));
    Add(database_entity(DB_AllowUnismoke       , true));
    Add(database_entity(DB_AllowSmokeDistinct  , true));
    Add(database_entity(DB_AllowFullUW         , true));
    Add(database_entity(DB_AllowSimpUW         , true));
    Add(database_entity(DB_AllowGuarUW         , true));
    Add(database_entity(DB_AllowMortBlendSex   , true));
    Add(database_entity(DB_AllowMortBlendSmoke , true));
    Add(database_entity(DB_AllowRatedWP        , true));
    Add(database_entity(DB_AllowRatedADD       , true));
    Add(database_entity(DB_AllowRatedTerm      , true));
    Add(database_entity(DB_Allowable           , true));
    Add(database_entity(DB_AllowPreferredClass , true));
    Add(database_entity(DB_AllowCVAT           , true));
    Add(database_entity(DB_AllowGPT            , true));

    // This is just a sample product, so we make do with plausible
    // all-male seven-pay premiums, and use GPT corridor factors for
    // CVAT.
    Add(database_entity(DB_CorridorTable       , 7));
    Add(database_entity(DB_TAMRA7PayTable      , 10));

    // Following IRS Notice 88-128, use only the male and female
    // tables with no smoker distinction, and a unisex table where
    // required by state law.
    //
    // US 1980 CSO age last, not smoker distinct. Unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    int dims311[e_number_of_axes] = {3, 1, 1, 1, 1, 1, 1}; // gender
    double T7702q[9] = {35, 41, 107,}; // Female, male, unisex.
    Add(database_entity(DB_IRC7702QTable, e_number_of_axes, dims311, T7702q));

    Add(database_entity(DB_PremLoad7702        , 0.02));
    Add(database_entity(DB_AllowDBO1           , true));
    Add(database_entity(DB_AllowDBO2           , true));
    Add(database_entity(DB_AllowDBO3           , true));
    Add(database_entity(DB_OptChgCanIncrSA     , true));
    Add(database_entity(DB_OptChgCanDecrSA     , true));
    Add(database_entity(DB_NonforfQTable       , 0.0));
    Add(database_entity(DB_SurrChgByFormula    , 0.0));
    Add(database_entity(DB_SurrChgPeriod       , 0.0));
    Add(database_entity(DB_SurrChgZeroDur      , 0.0));
    Add(database_entity(DB_SurrChgNLPMult      , 0.0));
    Add(database_entity(DB_SurrChgNLPMax       , 0.0));
    Add(database_entity(DB_SurrChgEAMax        , 0.0));
    Add(database_entity(DB_SurrChgPremMult     , 0.0));
    Add(database_entity(DB_SurrChgIsMly        , 0.0));

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
    int premium_tax_dimensions[e_number_of_axes] = {1, 1, 1, 1, 1, 53, 1};
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
        (database_entity
            (DB_PremTaxRate
            ,e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_rates
            )
        );

    Add(database_entity(DB_PremTaxState        , oe_ee_state));
    Add(database_entity(DB_EndtAge             , 100));
    Add(database_entity(DB_AllowExtEndt        , true));
    Add(database_entity(DB_AllowGenAcct        , true));
    Add(database_entity(DB_AllowSepAcct        , true));
    Add(database_entity(DB_MinPremType         , oe_monthly_deduction));
    Add(database_entity(DB_TgtPremType         , oe_modal_nonmec));
    Add(database_entity(DB_TgtPmFixedAtIssue   , false));
    Add(database_entity(DB_TgtPmIgnoreSubstd   , true));
    Add(database_entity(DB_NoLapseMinDur       , 0.0));
    Add(database_entity(DB_NoLapseMinAge       , 0.0));
    Add(database_entity(DB_NoLapseUnratedOnly  , false));
    Add(database_entity(DB_NoLapseOpt1Only     , false));
    Add(database_entity(DB_PremRefund          , 0.0));
    // Reuse current COI rates as current and guaranteed term rates.
    Add(database_entity(DB_TermTable, e_number_of_axes, dims313, TgCOI));
    Add(database_entity(DB_GuarTermTable, e_number_of_axes, dims313, TgCOI));
    Add(database_entity(DB_AllowTerm           , true));
    Add(database_entity(DB_TermMinIssAge       , 0.0));
    Add(database_entity(DB_TermMaxIssAge       , 0.0));
    Add(database_entity(DB_TermForcedConvAge   , 0.0));
    Add(database_entity(DB_MaxTermProportion   , 0.0));
    Add(database_entity(DB_TermCOIRate         , 0.0));
    Add(database_entity(DB_TermPremRate        , 0.0));
    Add(database_entity(DB_WPTable             , 8));
    Add(database_entity(DB_AllowWP             , true));
    Add(database_entity(DB_WPMinIssAge         , 0.0));
    Add(database_entity(DB_WPMaxIssAge         , 0.0));
    Add(database_entity(DB_WPMax               , 0.0));
    Add(database_entity(DB_WPCOIRate           , 0.0));
    Add(database_entity(DB_WPPremRate          , 0.0));
    // SOA qx_ins table 708 is 70-75 US ADB experience.
    Add(database_entity(DB_ADDTable            , 708));
    Add(database_entity(DB_AllowADD            , true));
    Add(database_entity(DB_ADDMinIssAge        , 0.0));
    Add(database_entity(DB_ADDMaxIssAge        , 0.0));
    Add(database_entity(DB_ADDLimit            , 1000000.0));
    Add(database_entity(DB_ADDCOIRate          , 0.0));
    Add(database_entity(DB_ADDPremRate         , 0.0));
    Add(database_entity(DB_WeightClass         , 0.0));
    Add(database_entity(DB_WeightGender        , 0.0));
    Add(database_entity(DB_WeightSmoking       , 0.0));
    Add(database_entity(DB_WeightAge           , 0.0));
    Add(database_entity(DB_WeightSpecAmt       , 0.0));
    Add(database_entity(DB_WeightState         , 0.0));
    Add(database_entity(DB_FullExpPol          , 0.0));
    Add(database_entity(DB_FullExpPrem         , 0.0));
    Add(database_entity(DB_FullExpDumpin       , 0.0));
    Add(database_entity(DB_FullExpPerK         , 0.0));
    Add(database_entity(DB_VarExpPol           , 0.0));
    Add(database_entity(DB_VarExpPrem          , 0.0));
    Add(database_entity(DB_VarExpDumpin        , 0.0));
    Add(database_entity(DB_VarExpPerK          , 0.0));
    Add(database_entity(DB_MedicalProportion   , 0.0));
    Add(database_entity(DB_UWTestCost          , 0.0));
    Add(database_entity(DB_VxBasicQTable       , 0.0));
    Add(database_entity(DB_VxDeficQTable       , 0.0));
    Add(database_entity(DB_VxTaxQTable         , 0.0));
    Add(database_entity(DB_StatVxInt           , 0.0));
    Add(database_entity(DB_TaxVxInt            , 0.0));
    Add(database_entity(DB_StatVxQ             , 0.0));
    Add(database_entity(DB_TaxVxQ              , 0.0));
    Add(database_entity(DB_DefVxQ              , 0.0));
    Add(database_entity(DB_NonforfQ            , 0.0));
    Add(database_entity(DB_CompTarget          , 0.0));
    Add(database_entity(DB_CompExcess          , 0.0));
    Add(database_entity(DB_CompChargeBack      , 0.0));
    Add(database_entity(DB_LapseRate           , 0.0));
    Add(database_entity(DB_ReqSurpNAAR         , 0.0));
    Add(database_entity(DB_ReqSurpVx           , 0.0));
    Add(database_entity(DB_LICFitRate          , 0.0));
    Add(database_entity(DB_LicDacTaxRate       , 0.0));
    Add(database_entity(DB_GDBVxMethod         , 0.0));
    Add(database_entity(DB_PrimaryHurdle       , 0.0));
    Add(database_entity(DB_SecondaryHurdle     , 0.0));
    Add(database_entity(DB_LedgerType          , mce_ill_reg));
    Add(database_entity(DB_AllowExpRating      , false));

    // These aren't really NY Table Y group rates--in fact, they're
    // US 65-70 male ALB. Though NY Table Y is occasionally
    // encountered in the group-carveout market, it's not included
    // in the SOA's databases; for default initialization, a widely-
    // available table is preferred.
    //
    // DATABASE !! Hence, the entity is misnamed; it really means
    // something like "group proxy rate". However, what's really
    // wanted is a choice among tables. The same can be said of
    // 'DB_83GamTable', which really means "partial-mortality table";
    // this support request:
    //   http://savannah.nongnu.org/support/?105593
    // would offer a choice and make that database entity unnecessary.
    Add(database_entity(DB_TableYTable         , 358));

    // Use male rates for unisex--1983 GAM seems to have no unisex version.
    double T83Gam[3] = {825, 826, 826,};
    Add(database_entity(DB_83GamTable, e_number_of_axes, dims311, T83Gam, "Use male rates for unisex--1983 GAM seems to have no unisex version."));

    Add(database_entity(DB_AllowWD             , true));
    Add(database_entity(DB_AllowLoan           , true));
    Add(database_entity(DB_AllowChangeToDBO2   , true));
    Add(database_entity(DB_AllowSAIncr         , true));
    Add(database_entity(DB_NoLapseAlwaysActive , false));
    Add(database_entity(DB_PrefOrSelect        , oe_called_select));
    Add(database_entity(DB_ExpRatStdDevMult    , 0.0));
    Add(database_entity(DB_ExpRatIBNRMult      , 0.0));
    Add(database_entity(DB_ExpRatCOIRetention  , 0.0));
    Add(database_entity(DB_StableValFundCharge , 0.0));
    Add(database_entity(DB_AmortPmLdFundCharge , 0.0030));
    Add(database_entity(DB_AllowAmortPremLoad  , false));
    Add(database_entity(DB_PmTxAmortPeriod     , 0));
    Add(database_entity(DB_PmTxAmortIntRate    , 0.0));
    // Pass through premium tax.
    Add
        (database_entity
            (DB_PremTaxLoad
            ,e_number_of_axes
            ,premium_tax_dimensions
            ,premium_tax_rates
            )
        );
    Add(database_entity(DB_AllowHoneymoon      , true));
    // Set target equal to seven-pay premium.
    Add(database_entity(DB_TgtPremTable        , 10));
    Add(database_entity(DB_TgtPremPolFee       , 0.0));
    Add(database_entity(DB_AllowExtraAssetComp , true));
    Add(database_entity(DB_AllowExtraPremComp  , true));
    Add(database_entity(DB_AssetChargeType     , oe_asset_charge_spread));
    Add(database_entity(DB_AllowUltraPrefClass , false));
    Add(database_entity(DB_MaxGenAcctRate      , 0.06));
    Add(database_entity(DB_MaxSepAcctRate      , 0.12));
    Add(database_entity(DB_MaxVLRRate          , 0.18));
    Add(database_entity(DB_SurrChgAVMult       , 0.0));
    Add(database_entity(DB_IntSpreadFreq       , mce_spread_daily));
    Add(database_entity(DB_StateApproved       , true));
    Add(database_entity(DB_AllowStateXX        , true));
    Add(database_entity(DB_AllowForeign        , true));
    Add(database_entity(DB_AllowCustomFund     , false));
    Add(database_entity(DB_AllowNo7702         , false));
    Add(database_entity(DB_EnforceNAARLimit    , true));
    Add(database_entity(DB_DynamicSepAcctLoad  , false));
    Add(database_entity(DB_SpecAmtLoadLimit    , 10000000.0));
    Add(database_entity(DB_Equiv7702DBO3       , 0));
    Add(database_entity(DB_ExpRatRiskCOIMult   , 0));
    Add(database_entity(DB_SurrChgSAMult       , 0.0));
    Add(database_entity(DB_AllowSpouse         , false));
    Add(database_entity(DB_AllowChild          , false));

    // Spouse and child riders unavailable, so it doesn't matter
    // what table we specify.
    Add(database_entity(DB_SpouseRiderTable    , 708));
    Add(database_entity(DB_ChildRiderTable     , 708));

    Add(database_entity(DB_GAIntBonus          , 0.0));

    // Allow experience rating.
    Add(database_entity(DB_AllowExpRating      , 1.0));
    Add(database_entity(DB_ExpRatIBNRMult      , 6.0));
    Add(database_entity(DB_ExpRatAmortPeriod   , 4.0));

    WriteDB(AddDataDir("sample.database"));
}

/// Initialize the built-in database for the antediluvian branch.

void DBDictionary::InitAntediluvian()
{
    dictionary_.clear();

    // Zero is inappropriate for some entities ("DB_CCOIMultiplier",
    // e.g.), but the antediluvian branch doesn't actually use most
    // database entities.
    for(int j = DB_FIRST; j < DB_LAST; ++j)
        {
        Add(database_entity(j, 0.0));
        }

    Add(database_entity(DB_GuarInt, 0.03));

    Add(database_entity(DB_FixedLoanRate, 0.06));

    Add(database_entity(DB_GuarRegLoanSpread, 0.0));
    Add(database_entity(DB_CurrRegLoanSpread, 0.0));
    Add(database_entity(DB_GuarPrefLoanSpread, 0.0));
    Add(database_entity(DB_CurrPrefLoanSpread, 0.0));

    Add(database_entity(DB_AllowGenAcct, 1.0));
    Add(database_entity(DB_AllowPreferredClass, 1.0));

    // premium loads

    Add(database_entity(DB_GuarPolFee, 12.00));
    Add(database_entity(DB_GuarSpecAmtLoad, 0.0));
    Add(database_entity(DB_GuarPremLoadTgt, 0.025));
    Add(database_entity(DB_GuarPremLoadExc, 0.025));
    Add(database_entity(DB_CurrPolFee, 5.00));
    Add(database_entity(DB_CurrSpecAmtLoad, 0.0));
    Add(database_entity(DB_CurrPremLoadTgt, 0.025));
    Add(database_entity(DB_CurrPremLoadExc, 0.025));

    Add(database_entity(DB_MinWD, 100.0));
    Add(database_entity(DB_WDFee, 5.0));
    Add(database_entity(DB_WDFeeRate, 0.01));

    int guar_coi_dims[e_number_of_axes] = {1, 1, 3, 1, 1, 1, 1};
    // smoker, nonsmoker, unismoke
    double guar_coi_tables[3] = {111, 109, 107};
    Add(database_entity(DB_GuarCOITable, e_number_of_axes, guar_coi_dims, guar_coi_tables));

    int curr_coi_dims[e_number_of_axes] = {1, 4, 3, 1, 1, 1, 1};
    // preferred, standard, rated, ultrapreferred by smoker, nonsmoker, unismoke
    double curr_coi_tables[] =
        {
        2, 3, 1, // pref  sm ns us
        5, 6, 4, // std   sm ns us
        5, 6, 4, // rated sm ns us
        0, 0, 0, // ultra sm ns us
        };
    Add(database_entity(DB_CurrCOITable, e_number_of_axes, curr_coi_dims, curr_coi_tables));

    Add(database_entity(DB_CorridorTable, 7));
    Add(database_entity(DB_WPTable, 8));
    Add(database_entity(DB_ADDTable, 9));
    Add(database_entity(DB_EndtAge, 100));
    Add(database_entity(DB_AgeLastOrNearest, 1.0));
    Add(database_entity(DB_MinSpecAmt, 10000.0));

    Add(database_entity(DB_MaxGenAcctRate, 0.12));
    Add(database_entity(DB_MaxSepAcctRate, 0.12));

    Add(database_entity(DB_AllowLoan, 1.0));
    Add(database_entity(DB_AllowWD, 1.0));
    Add(database_entity(DB_AllowFlatExtras, 1.0));
    Add(database_entity(DB_AllowChangeToDBO2, 1.0));
    Add(database_entity(DB_AllowDBO3, 1.0));

    Add(database_entity(DB_SurrChgPremMult, 0.0));
    Add(database_entity(DB_SurrChgAVMult, 0.0));
    Add(database_entity(DB_SurrChgSAMult, 0.0));
    Add(database_entity(DB_SurrChgAVDurFactor, 1.0));
    Add(database_entity(DB_SurrChgSADurFactor, 1.0));

    Add(database_entity(DB_LedgerType, mce_ill_reg));

    Add(database_entity(DB_NoLapseAlwaysActive, 0.0));
    Add(database_entity(DB_NoLapseMinDur, 0.0));
    Add(database_entity(DB_NoLapseMinAge, 0.0));

    Add(database_entity(DB_NominallyPar, 0.0));
    Add(database_entity(DB_Has1035ExchCharge, 0.0));
    Add(database_entity(DB_SmokeOrTobacco, 0.0));
    Add(database_entity(DB_DACTaxFundCharge, 0.0));
    Add(database_entity(DB_AllowWP, 0.0));
    Add(database_entity(DB_AllowADD, 0.0));
    Add(database_entity(DB_AllowSpouse, 0.0));
    Add(database_entity(DB_AllowChild, 0.0));

    Add(database_entity(DB_ExpRatAmortPeriod, 4.0));
}

/// Print databases to file in an alternative text format.
///
/// Unlike xml, the alternative shows multidimensional data in an
/// array format reminiscent of APL, interleaved with definitions
/// that good xml practice would put in a schema.
///
/// Every database file in the data directory is written in the
/// alternative format, with a distinct file extension.

void print_databases()
{
    fs::path path(global_settings::instance().data_directory());
    fs::directory_iterator i(path);
    fs::directory_iterator end_i;
    for(; i != end_i; ++i)
        {
        if(is_directory(*i) || ".database" != fs::extension(*i))
            {
            continue;
            }

        DBDictionary::instance().Init(i->string());
        fs::path out_file = fs::change_extension(*i, ".dbt");
        fs::ofstream os(out_file, ios_out_trunc_binary());
        dict_map const& dictionary = DBDictionary::instance().GetDictionary();
        // std::ostream_iterator not used because it doesn't work
        // nicely with std::map (a name-lookup issue).
        typedef dict_map::const_iterator dmci;
        for(dmci i = dictionary.begin(); i != dictionary.end(); ++i)
            {
            i->second.write(os);
            }
        }
}

