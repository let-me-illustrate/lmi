// Product-database map.
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

#include "dbdict.hpp"
#include "xml_serializable.tpp"

#include "actuarial_table.hpp"          // e_reenter_never
#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"            // gregorian_epoch(), last_yyyy_date()
#include "data_directory.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"        // report_exception()
#include "ieee754.hpp"                  // infinity<>()
#include "mc_enum_type_enums.hpp"
#include "miscellany.hpp"
#include "my_proem.hpp"                 // ::write_proem()
#include "oecumenic_enumerations.hpp"
#include "premium_tax.hpp"              // premium_tax_rates_for_life_insurance()
#include "sample.hpp"                   // superior::lingo
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include <vector>

template class xml_serializable<DBDictionary>;

namespace xml_serialize
{
template<> struct xml_io<database_entity>
{
    typedef database_entity T;
    static void   to_xml(xml::element& e, T const& t) {t.write(e);}
    static void from_xml(xml::element const& e, T& t) {t.read (e);}
};
} // namespace xml_serialize

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::str()
/// which is not useful here.

template<>
std::string value_cast<std::string>(database_entity const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

/// Specialize value_cast<>() to throw an exception.
///
/// This is required by
///   any_member::operator=(std::string const&)
/// which is not useful here.

template<>
database_entity value_cast<database_entity>(std::string const&)
{
    alarum() << "Invalid function call." << LMI_FLUSH;
    throw "Unreachable--silences a compiler diagnostic.";
}

DBDictionary::DBDictionary()
{
    ascribe_members();
    InitDB();
}

/// Construct from a '.database' file.
///
/// Call InitDB() unconditionally before Init(), even though that may
/// seem unnecessary, in case the file read by Init() lacks any member
/// entity (because it's an older version or has been edited, e.g.).

DBDictionary::DBDictionary(fs::path const& filename)
{
    ascribe_members();
    InitDB();
    Init(filename.string());
}

database_entity const& DBDictionary::datum(std::string const& name) const
{
    return *member_cast<database_entity>(operator[](name));
}

database_entity& DBDictionary::datum(std::string const& name)
{
    return *member_cast<database_entity>(operator[](name));
}

void DBDictionary::ascribe_members()
{
    ascribe("MinIssAge"                 , &DBDictionary::MinIssAge                 );
    ascribe("MaxIssAge"                 , &DBDictionary::MaxIssAge                 );
    ascribe("MaxIncrAge"                , &DBDictionary::MaxIncrAge                );
    ascribe("AllowFullUw"               , &DBDictionary::AllowFullUw               );
    ascribe("AllowParamedUw"            , &DBDictionary::AllowParamedUw            );
    ascribe("AllowNonmedUw"             , &DBDictionary::AllowNonmedUw             );
    ascribe("AllowSimpUw"               , &DBDictionary::AllowSimpUw               );
    ascribe("AllowGuarUw"               , &DBDictionary::AllowGuarUw               );
    ascribe("SmokeOrTobacco"            , &DBDictionary::SmokeOrTobacco            );
    ascribe("AllowPreferredClass"       , &DBDictionary::AllowPreferredClass       );
    ascribe("AllowUltraPrefClass"       , &DBDictionary::AllowUltraPrefClass       );
    ascribe("AllowSubstdTable"          , &DBDictionary::AllowSubstdTable          );
    ascribe("AllowFlatExtras"           , &DBDictionary::AllowFlatExtras           );
    ascribe("AllowRatedWp"              , &DBDictionary::AllowRatedWp              );
    ascribe("AllowRatedAdb"             , &DBDictionary::AllowRatedAdb             );
    ascribe("AllowRatedTerm"            , &DBDictionary::AllowRatedTerm            );
    ascribe("AllowRetirees"             , &DBDictionary::AllowRetirees             );
    ascribe("AllowUnisex"               , &DBDictionary::AllowUnisex               );
    ascribe("AllowSexDistinct"          , &DBDictionary::AllowSexDistinct          );
    ascribe("AllowUnismoke"             , &DBDictionary::AllowUnismoke             );
    ascribe("AllowSmokeDistinct"        , &DBDictionary::AllowSmokeDistinct        );
    ascribe("StateApproved"             , &DBDictionary::StateApproved             );
    ascribe("AllowStateXX"              , &DBDictionary::AllowStateXX              );
    ascribe("AllowForeign"              , &DBDictionary::AllowForeign              );
    ascribe("GroupIndivSelection"       , &DBDictionary::GroupIndivSelection       );
    ascribe("TxCallsGuarUwSubstd"       , &DBDictionary::TxCallsGuarUwSubstd       );
    ascribe("Allowable"                 , &DBDictionary::Allowable                 );
    ascribe("AllowCvat"                 , &DBDictionary::AllowCvat                 );
    ascribe("AllowGpt"                  , &DBDictionary::AllowGpt                  );
    ascribe("AllowNo7702"               , &DBDictionary::AllowNo7702               );
    ascribe("AnnInterestRate7702"       , &DBDictionary::AnnInterestRate7702       );
    ascribe("IgnoreLoanRateFor7702"     , &DBDictionary::IgnoreLoanRateFor7702     );
    ascribe("MlyDiscountRate7702Whence" , &DBDictionary::MlyDiscountRate7702Whence );
    ascribe("MlyDiscountRate7702"       , &DBDictionary::MlyDiscountRate7702       );
    ascribe("Irc7702Obreption"          , &DBDictionary::Irc7702Obreption          );
    ascribe("CorridorWhence"            , &DBDictionary::CorridorWhence            );
    ascribe("Irc7702NspWhence"          , &DBDictionary::Irc7702NspWhence          );
    ascribe("SevenPayWhence"            , &DBDictionary::SevenPayWhence            );
    ascribe("Irc7702QWhence"            , &DBDictionary::Irc7702QWhence            );
    ascribe("CorridorTable"             , &DBDictionary::CorridorTable             );
    ascribe("Irc7702NspTable"           , &DBDictionary::Irc7702NspTable           );
    ascribe("SevenPayTable"             , &DBDictionary::SevenPayTable             );
    ascribe("Irc7702QTable"             , &DBDictionary::Irc7702QTable             );
    ascribe("Irc7702QAxisGender"        , &DBDictionary::Irc7702QAxisGender        );
    ascribe("Irc7702QAxisSmoking"       , &DBDictionary::Irc7702QAxisSmoking       );
    ascribe("CvatMatChangeDefn"         , &DBDictionary::CvatMatChangeDefn         );
    ascribe("GptMatChangeDefn"          , &DBDictionary::GptMatChangeDefn          );
    ascribe("Irc7702BftIsSpecAmt"       , &DBDictionary::Irc7702BftIsSpecAmt       );
    ascribe("RiskyInitial7702Db"        , &DBDictionary::RiskyInitial7702Db        );
    ascribe("Irc7702Endowment"          , &DBDictionary::Irc7702Endowment          );
    ascribe("Effective7702DboRop"       , &DBDictionary::Effective7702DboRop       );
    ascribe("TermIsQABOrDb7702"         , &DBDictionary::TermIsQABOrDb7702         );
    ascribe("TermIsQABOrDb7702A"        , &DBDictionary::TermIsQABOrDb7702A        );
    ascribe("GioIsQAB"                  , &DBDictionary::GioIsQAB                  );
    ascribe("AdbIsQAB"                  , &DBDictionary::AdbIsQAB                  );
    ascribe("SpouseRiderIsQAB"          , &DBDictionary::SpouseRiderIsQAB          );
    ascribe("ChildRiderIsQAB"           , &DBDictionary::ChildRiderIsQAB           );
    ascribe("WpIsQAB"                   , &DBDictionary::WpIsQAB                   );
    ascribe("CsoEra"                    , &DBDictionary::CsoEra                    );
    ascribe("CsoMisprint"               , &DBDictionary::CsoMisprint               );
    ascribe("GuarCoiWhence"             , &DBDictionary::GuarCoiWhence             );
    ascribe("GuarCoiTable"              , &DBDictionary::GuarCoiTable              );
    ascribe("GuarCoiIsAnnual"           , &DBDictionary::GuarCoiIsAnnual           );
    ascribe("GuarCoiMultiplier"         , &DBDictionary::GuarCoiMultiplier         );
    ascribe("CurrCoiTable"              , &DBDictionary::CurrCoiTable              );
    ascribe("CurrCoiIsAnnual"           , &DBDictionary::CurrCoiIsAnnual           );
    ascribe("MinInputCoiMult"           , &DBDictionary::MinInputCoiMult           );
    ascribe("CurrCoiMultiplier"         , &DBDictionary::CurrCoiMultiplier         );
    ascribe("UnusualCoiBanding"         , &DBDictionary::UnusualCoiBanding         );
    ascribe("CurrCoiTable0Limit"        , &DBDictionary::CurrCoiTable0Limit        );
    ascribe("CurrCoiTable1"             , &DBDictionary::CurrCoiTable1             );
    ascribe("CurrCoiTable1Limit"        , &DBDictionary::CurrCoiTable1Limit        );
    ascribe("CurrCoiTable2"             , &DBDictionary::CurrCoiTable2             );
    ascribe("MdptCoiTable"              , &DBDictionary::MdptCoiTable              );
    ascribe("MdptCoiIsAnnual"           , &DBDictionary::MdptCoiIsAnnual           );
    ascribe("CoiNyMinTable"             , &DBDictionary::CoiNyMinTable             );
    ascribe("UseNyCoiFloor"             , &DBDictionary::UseNyCoiFloor             );
    ascribe("MaxMonthlyCoiRate"         , &DBDictionary::MaxMonthlyCoiRate         );
    ascribe("GuarCoiCeiling"            , &DBDictionary::GuarCoiCeiling            );
    ascribe("CoiGuarIsMin"              , &DBDictionary::CoiGuarIsMin              );
    ascribe("SubstdTableMult"           , &DBDictionary::SubstdTableMult           );
    ascribe("SubstdTableMultTable"      , &DBDictionary::SubstdTableMultTable      );
    ascribe("CoiUpper12Method"          , &DBDictionary::CoiUpper12Method          );
    ascribe("CoiInforceReentry"         , &DBDictionary::CoiInforceReentry         );
    ascribe("CoiResetMinDate"           , &DBDictionary::CoiResetMinDate           );
    ascribe("CoiResetMaxDate"           , &DBDictionary::CoiResetMaxDate           );
    ascribe("AllowMortBlendSex"         , &DBDictionary::AllowMortBlendSex         );
    ascribe("AllowMortBlendSmoke"       , &DBDictionary::AllowMortBlendSmoke       );
    ascribe("GuarInt"                   , &DBDictionary::GuarInt                   );
    ascribe("NaarDiscount"              , &DBDictionary::NaarDiscount              );
    ascribe("GuarIntSpread"             , &DBDictionary::GuarIntSpread             );
    ascribe("GuarMandE"                 , &DBDictionary::GuarMandE                 );
    ascribe("CurrIntSpread"             , &DBDictionary::CurrIntSpread             );
    ascribe("CurrMandE"                 , &DBDictionary::CurrMandE                 );
    ascribe("GenAcctIntBonus"           , &DBDictionary::GenAcctIntBonus           );
    ascribe("IntFloor"                  , &DBDictionary::IntFloor                  );
    ascribe("AllowGenAcct"              , &DBDictionary::AllowGenAcct              );
    ascribe("AllowSepAcct"              , &DBDictionary::AllowSepAcct              );
    ascribe("AllowGenAcctEarnRate"      , &DBDictionary::AllowGenAcctEarnRate      );
    ascribe("AllowSepAcctNetRate"       , &DBDictionary::AllowSepAcctNetRate       );
    ascribe("MaxGenAcctRate"            , &DBDictionary::MaxGenAcctRate            );
    ascribe("MaxSepAcctRate"            , &DBDictionary::MaxSepAcctRate            );
    ascribe("SepAcctSpreadMethod"       , &DBDictionary::SepAcctSpreadMethod       );
    ascribe("IntSpreadMode"             , &DBDictionary::IntSpreadMode             );
    ascribe("DynamicMandE"              , &DBDictionary::DynamicMandE              );
    ascribe("AllowAmortPremLoad"        , &DBDictionary::AllowAmortPremLoad        );
    ascribe("LoadAmortFundCharge"       , &DBDictionary::LoadAmortFundCharge       );
    ascribe("AllowImfOverride"          , &DBDictionary::AllowImfOverride          );
    ascribe("AssetChargeType"           , &DBDictionary::AssetChargeType           );
    ascribe("GuarFundAdminChg"          , &DBDictionary::GuarFundAdminChg          );
    ascribe("CurrFundAdminChg"          , &DBDictionary::CurrFundAdminChg          );
    ascribe("FundCharge"                , &DBDictionary::FundCharge                );
    ascribe("GuarMonthlyPolFee"         , &DBDictionary::GuarMonthlyPolFee         );
    ascribe("GuarAnnualPolFee"          , &DBDictionary::GuarAnnualPolFee          );
    ascribe("GuarPremLoadTgt"           , &DBDictionary::GuarPremLoadTgt           );
    ascribe("GuarPremLoadExc"           , &DBDictionary::GuarPremLoadExc           );
    ascribe("GuarPremLoadTgtRfd"        , &DBDictionary::GuarPremLoadTgtRfd        );
    ascribe("GuarPremLoadExcRfd"        , &DBDictionary::GuarPremLoadExcRfd        );
    ascribe("GuarSpecAmtLoad"           , &DBDictionary::GuarSpecAmtLoad           );
    ascribe("GuarSpecAmtLoadTable"      , &DBDictionary::GuarSpecAmtLoadTable      );
    ascribe("GuarAcctValLoad"           , &DBDictionary::GuarAcctValLoad           );
    ascribe("CurrMonthlyPolFee"         , &DBDictionary::CurrMonthlyPolFee         );
    ascribe("CurrAnnualPolFee"          , &DBDictionary::CurrAnnualPolFee          );
    ascribe("CurrPremLoadTgt"           , &DBDictionary::CurrPremLoadTgt           );
    ascribe("CurrPremLoadExc"           , &DBDictionary::CurrPremLoadExc           );
    ascribe("CurrPremLoadTgtRfd"        , &DBDictionary::CurrPremLoadTgtRfd        );
    ascribe("CurrPremLoadExcRfd"        , &DBDictionary::CurrPremLoadExcRfd        );
    ascribe("CurrSpecAmtLoad"           , &DBDictionary::CurrSpecAmtLoad           );
    ascribe("CurrSpecAmtLoadTable"      , &DBDictionary::CurrSpecAmtLoadTable      );
    ascribe("CurrAcctValLoad"           , &DBDictionary::CurrAcctValLoad           );
    ascribe("TgtPremMonthlyPolFee"      , &DBDictionary::TgtPremMonthlyPolFee      );
    ascribe("LoadRfdProportion"         , &DBDictionary::LoadRfdProportion         );
    ascribe("SpecAmtLoadLimit"          , &DBDictionary::SpecAmtLoadLimit          );
    ascribe("DynamicSepAcctLoad"        , &DBDictionary::DynamicSepAcctLoad        );
    ascribe("DynSepAcctLoadLimit"       , &DBDictionary::DynSepAcctLoadLimit       );
    ascribe("DacTaxFundCharge"          , &DBDictionary::DacTaxFundCharge          );
    ascribe("DacTaxPremLoad"            , &DBDictionary::DacTaxPremLoad            );
    ascribe("PremTaxFundCharge"         , &DBDictionary::PremTaxFundCharge         );
    ascribe("PremTaxLoad"               , &DBDictionary::PremTaxLoad               );
    ascribe("WaivePremTaxInt1035"       , &DBDictionary::WaivePremTaxInt1035       );
    ascribe("PremTaxRetalLimit"         , &DBDictionary::PremTaxRetalLimit         );
    ascribe("PremTaxTierGroup"          , &DBDictionary::PremTaxTierGroup          );
    ascribe("PremTaxTierPeriod"         , &DBDictionary::PremTaxTierPeriod         );
    ascribe("PremTaxTierNonDecr"        , &DBDictionary::PremTaxTierNonDecr        );
    ascribe("PremTaxAmortPeriod"        , &DBDictionary::PremTaxAmortPeriod        );
    ascribe("PremTaxAmortIntRate"       , &DBDictionary::PremTaxAmortIntRate       );
    ascribe("PremTaxRate"               , &DBDictionary::PremTaxRate               );
    ascribe("PremTaxState"              , &DBDictionary::PremTaxState              );
    ascribe("SurrChgOnIncr"             , &DBDictionary::SurrChgOnIncr             );
    ascribe("SurrChgOnDecr"             , &DBDictionary::SurrChgOnDecr             );
    ascribe("Has1035ExchCharge"         , &DBDictionary::Has1035ExchCharge         );
    ascribe("SnflQTable"                , &DBDictionary::SnflQTable                );
    ascribe("CoiSnflIsGuar"             , &DBDictionary::CoiSnflIsGuar             );
    ascribe("SurrChgByFormula"          , &DBDictionary::SurrChgByFormula          );
    ascribe("SurrChgPeriod"             , &DBDictionary::SurrChgPeriod             );
    ascribe("SurrChgZeroDur"            , &DBDictionary::SurrChgZeroDur            );
    ascribe("SurrChgNlpMult"            , &DBDictionary::SurrChgNlpMult            );
    ascribe("SurrChgNlpMax"             , &DBDictionary::SurrChgNlpMax             );
    ascribe("SurrChgEaMax"              , &DBDictionary::SurrChgEaMax              );
    ascribe("SurrChgAmort"              , &DBDictionary::SurrChgAmort              );
    ascribe("AllowSpecAmtIncr"          , &DBDictionary::AllowSpecAmtIncr          );
    ascribe("MinSpecAmtIncr"            , &DBDictionary::MinSpecAmtIncr            );
    ascribe("EnforceNaarLimit"          , &DBDictionary::EnforceNaarLimit          );
    ascribe("MinSpecAmt"                , &DBDictionary::MinSpecAmt                );
    ascribe("MinIssSpecAmt"             , &DBDictionary::MinIssSpecAmt             );
    ascribe("MinIssBaseSpecAmt"         , &DBDictionary::MinIssBaseSpecAmt         );
    ascribe("MinRenlSpecAmt"            , &DBDictionary::MinRenlSpecAmt            );
    ascribe("MinRenlBaseSpecAmt"        , &DBDictionary::MinRenlBaseSpecAmt        );
    ascribe("MaxIssSpecAmt"             , &DBDictionary::MaxIssSpecAmt             );
    ascribe("MaxRenlSpecAmt"            , &DBDictionary::MaxRenlSpecAmt            );
    ascribe("AllowDboLvl"               , &DBDictionary::AllowDboLvl               );
    ascribe("AllowDboInc"               , &DBDictionary::AllowDboInc               );
    ascribe("AllowDboRop"               , &DBDictionary::AllowDboRop               );
    ascribe("AllowDboMdb"               , &DBDictionary::AllowDboMdb               );
    ascribe("DboLvlChangeToWhat"        , &DBDictionary::DboLvlChangeToWhat        );
    ascribe("DboLvlChangeMethod"        , &DBDictionary::DboLvlChangeMethod        );
    ascribe("DboIncChangeToWhat"        , &DBDictionary::DboIncChangeToWhat        );
    ascribe("DboIncChangeMethod"        , &DBDictionary::DboIncChangeMethod        );
    ascribe("DboRopChangeToWhat"        , &DBDictionary::DboRopChangeToWhat        );
    ascribe("DboRopChangeMethod"        , &DBDictionary::DboRopChangeMethod        );
    ascribe("DboMdbChangeToWhat"        , &DBDictionary::DboMdbChangeToWhat        );
    ascribe("DboMdbChangeMethod"        , &DBDictionary::DboMdbChangeMethod        );
    ascribe("AllowChangeToDbo2"         , &DBDictionary::AllowChangeToDbo2         );
    ascribe("DboChgCanIncrSpecAmt"      , &DBDictionary::DboChgCanIncrSpecAmt      );
    ascribe("DboChgCanDecrSpecAmt"      , &DBDictionary::DboChgCanDecrSpecAmt      );
    ascribe("AllowExtEndt"              , &DBDictionary::AllowExtEndt              );
    ascribe("AllowTerm"                 , &DBDictionary::AllowTerm                 );
    ascribe("GuarTermTable"             , &DBDictionary::GuarTermTable             );
    ascribe("TermTable"                 , &DBDictionary::TermTable                 );
    ascribe("TermMinIssAge"             , &DBDictionary::TermMinIssAge             );
    ascribe("TermMaxIssAge"             , &DBDictionary::TermMaxIssAge             );
    ascribe("TermIsNotRider"            , &DBDictionary::TermIsNotRider            );
    ascribe("TermCanLapse"              , &DBDictionary::TermCanLapse              );
    ascribe("TermForcedConvAge"         , &DBDictionary::TermForcedConvAge         );
    ascribe("TermForcedConvDur"         , &DBDictionary::TermForcedConvDur         );
    ascribe("MaxTermProportion"         , &DBDictionary::MaxTermProportion         );
    ascribe("TermCoiRate"               , &DBDictionary::TermCoiRate               );
    ascribe("TermPremRate"              , &DBDictionary::TermPremRate              );
    ascribe("AllowWp"                   , &DBDictionary::AllowWp                   );
    ascribe("WpTable"                   , &DBDictionary::WpTable                   );
    ascribe("WpMinIssAge"               , &DBDictionary::WpMinIssAge               );
    ascribe("WpMaxIssAge"               , &DBDictionary::WpMaxIssAge               );
    ascribe("WpLimit"                   , &DBDictionary::WpLimit                   );
    ascribe("WpCoiRate"                 , &DBDictionary::WpCoiRate                 );
    ascribe("WpPremRate"                , &DBDictionary::WpPremRate                );
    ascribe("WpChargeMethod"            , &DBDictionary::WpChargeMethod            );
    ascribe("AllowAdb"                  , &DBDictionary::AllowAdb                  );
    ascribe("AdbTable"                  , &DBDictionary::AdbTable                  );
    ascribe("AdbMinIssAge"              , &DBDictionary::AdbMinIssAge              );
    ascribe("AdbMaxIssAge"              , &DBDictionary::AdbMaxIssAge              );
    ascribe("AdbLimit"                  , &DBDictionary::AdbLimit                  );
    ascribe("AdbCoiRate"                , &DBDictionary::AdbCoiRate                );
    ascribe("AdbPremRate"               , &DBDictionary::AdbPremRate               );
    ascribe("AllowSpouseRider"          , &DBDictionary::AllowSpouseRider          );
    ascribe("SpouseRiderMinAmt"         , &DBDictionary::SpouseRiderMinAmt         );
    ascribe("SpouseRiderMaxAmt"         , &DBDictionary::SpouseRiderMaxAmt         );
    ascribe("SpouseRiderMinIssAge"      , &DBDictionary::SpouseRiderMinIssAge      );
    ascribe("SpouseRiderMaxIssAge"      , &DBDictionary::SpouseRiderMaxIssAge      );
    ascribe("SpouseRiderGuarTable"      , &DBDictionary::SpouseRiderGuarTable      );
    ascribe("SpouseRiderTable"          , &DBDictionary::SpouseRiderTable          );
    ascribe("AllowChildRider"           , &DBDictionary::AllowChildRider           );
    ascribe("ChildRiderMinAmt"          , &DBDictionary::ChildRiderMinAmt          );
    ascribe("ChildRiderMaxAmt"          , &DBDictionary::ChildRiderMaxAmt          );
    ascribe("ChildRiderTable"           , &DBDictionary::ChildRiderTable           );
    ascribe("AllowWd"                   , &DBDictionary::AllowWd                   );
    ascribe("WdFee"                     , &DBDictionary::WdFee                     );
    ascribe("WdFeeRate"                 , &DBDictionary::WdFeeRate                 );
    ascribe("FreeWdProportion"          , &DBDictionary::FreeWdProportion          );
    ascribe("MinWd"                     , &DBDictionary::MinWd                     );
    ascribe("MaxWdGenAcctValMult"       , &DBDictionary::MaxWdGenAcctValMult       );
    ascribe("MaxWdSepAcctValMult"       , &DBDictionary::MaxWdSepAcctValMult       );
    ascribe("MaxWdDed"                  , &DBDictionary::MaxWdDed                  );
    ascribe("WdDecrSpecAmtDboLvl"       , &DBDictionary::WdDecrSpecAmtDboLvl       );
    ascribe("WdDecrSpecAmtDboInc"       , &DBDictionary::WdDecrSpecAmtDboInc       );
    ascribe("WdDecrSpecAmtDboRop"       , &DBDictionary::WdDecrSpecAmtDboRop       );
    ascribe("FirstWdMonth"              , &DBDictionary::FirstWdMonth              );
    ascribe("AllowLoan"                 , &DBDictionary::AllowLoan                 );
    ascribe("AllowPrefLoan"             , &DBDictionary::AllowPrefLoan             );
    ascribe("AllowFixedLoan"            , &DBDictionary::AllowFixedLoan            );
    ascribe("AllowVlr"                  , &DBDictionary::AllowVlr                  );
    ascribe("FixedLoanRate"             , &DBDictionary::FixedLoanRate             );
    ascribe("MaxVlrRate"                , &DBDictionary::MaxVlrRate                );
    ascribe("MinVlrRate"                , &DBDictionary::MinVlrRate                );
    ascribe("MaxLoanAcctValMult"        , &DBDictionary::MaxLoanAcctValMult        );
    ascribe("MaxLoanDed"                , &DBDictionary::MaxLoanDed                );
    ascribe("FirstPrefLoanYear"         , &DBDictionary::FirstPrefLoanYear         );
    ascribe("PrefLoanRateDecr"          , &DBDictionary::PrefLoanRateDecr          );
    ascribe("GuarPrefLoanSpread"        , &DBDictionary::GuarPrefLoanSpread        );
    ascribe("GuarRegLoanSpread"         , &DBDictionary::GuarRegLoanSpread         );
    ascribe("CurrPrefLoanSpread"        , &DBDictionary::CurrPrefLoanSpread        );
    ascribe("CurrRegLoanSpread"         , &DBDictionary::CurrRegLoanSpread         );
    ascribe("FirstLoanMonth"            , &DBDictionary::FirstLoanMonth            );
    ascribe("MinPremType"               , &DBDictionary::MinPremType               );
    ascribe("MinPremTable"              , &DBDictionary::MinPremTable              );
    ascribe("MinPremIntSpread"          , &DBDictionary::MinPremIntSpread          );
    ascribe("SplitMinPrem"              , &DBDictionary::SplitMinPrem              );
    ascribe("UnsplitSplitMinPrem"       , &DBDictionary::UnsplitSplitMinPrem       );
    ascribe("ErNotionallyPaysTerm"      , &DBDictionary::ErNotionallyPaysTerm      );
    ascribe("TgtPremType"               , &DBDictionary::TgtPremType               );
    ascribe("TgtPremTable"              , &DBDictionary::TgtPremTable              );
    ascribe("TgtPremFixedAtIssue"       , &DBDictionary::TgtPremFixedAtIssue       );
    ascribe("TgtPremIgnoreSubstd"       , &DBDictionary::TgtPremIgnoreSubstd       );
    ascribe("MinPmt"                    , &DBDictionary::MinPmt                    );
    ascribe("IsSinglePremium"           , &DBDictionary::IsSinglePremium           );
    ascribe("NoLapseMinDur"             , &DBDictionary::NoLapseMinDur             );
    ascribe("NoLapseMinAge"             , &DBDictionary::NoLapseMinAge             );
    ascribe("NoLapseUnratedOnly"        , &DBDictionary::NoLapseUnratedOnly        );
    ascribe("NoLapseDboLvlOnly"         , &DBDictionary::NoLapseDboLvlOnly         );
    ascribe("NoLapseAlwaysActive"       , &DBDictionary::NoLapseAlwaysActive       );
    ascribe("AllowHoneymoon"            , &DBDictionary::AllowHoneymoon            );
    ascribe("DeductionMethod"           , &DBDictionary::DeductionMethod           );
    ascribe("DeductionAcct"             , &DBDictionary::DeductionAcct             );
    ascribe("DistributionMethod"        , &DBDictionary::DistributionMethod        );
    ascribe("DistributionAcct"          , &DBDictionary::DistributionAcct          );
    ascribe("EePremMethod"              , &DBDictionary::EePremMethod              );
    ascribe("EePremAcct"                , &DBDictionary::EePremAcct                );
    ascribe("ErPremMethod"              , &DBDictionary::ErPremMethod              );
    ascribe("ErPremAcct"                , &DBDictionary::ErPremAcct                );
    ascribe("CalculateComp"             , &DBDictionary::CalculateComp             );
    ascribe("CompTarget"                , &DBDictionary::CompTarget                );
    ascribe("CompExcess"                , &DBDictionary::CompExcess                );
    ascribe("CompChargeback"            , &DBDictionary::CompChargeback            );
    ascribe("AssetComp"                 , &DBDictionary::AssetComp                 );
    ascribe("AllowExtraAssetComp"       , &DBDictionary::AllowExtraAssetComp       );
    ascribe("AllowExtraPremComp"        , &DBDictionary::AllowExtraPremComp        );
    ascribe("LedgerType"                , &DBDictionary::LedgerType                );
    ascribe("Nonillustrated"            , &DBDictionary::Nonillustrated            );
    ascribe("NoLongerIssued"            , &DBDictionary::NoLongerIssued            );
    ascribe("AgeLastOrNearest"          , &DBDictionary::AgeLastOrNearest          );
    ascribe("MaturityAge"               , &DBDictionary::MaturityAge               );
    ascribe("AllowCashValueEnh"         , &DBDictionary::AllowCashValueEnh         );
    ascribe("CashValueEnhMult"          , &DBDictionary::CashValueEnhMult          );
    ascribe("LapseIgnoresSurrChg"       , &DBDictionary::LapseIgnoresSurrChg       );
    ascribe("DefaultProcessOrder"       , &DBDictionary::DefaultProcessOrder       );
    ascribe("GroupProxyRateTable"       , &DBDictionary::GroupProxyRateTable       );
    ascribe("PartialMortTable"          , &DBDictionary::PartialMortTable          );
    ascribe("AllowGroupQuote"           , &DBDictionary::AllowGroupQuote           );
    ascribe("PolicyForm"                , &DBDictionary::PolicyForm                );
    ascribe("PolicyMktgName"            , &DBDictionary::PolicyMktgName            );
    ascribe("PolicyLegalName"           , &DBDictionary::PolicyLegalName           );
    ascribe("InsCoShortName"            , &DBDictionary::InsCoShortName            );
    ascribe("InsCoName"                 , &DBDictionary::InsCoName                 );
    ascribe("InsCoAddr"                 , &DBDictionary::InsCoAddr                 );
    ascribe("InsCoStreet"               , &DBDictionary::InsCoStreet               );
    ascribe("InsCoPhone"                , &DBDictionary::InsCoPhone                );
    ascribe("MainUnderwriter"           , &DBDictionary::MainUnderwriter           );
    ascribe("MainUnderwriterAddress"    , &DBDictionary::MainUnderwriterAddress    );
    ascribe("CoUnderwriter"             , &DBDictionary::CoUnderwriter             );
    ascribe("CoUnderwriterAddress"      , &DBDictionary::CoUnderwriterAddress      );
    ascribe("AvName"                    , &DBDictionary::AvName                    );
    ascribe("CsvName"                   , &DBDictionary::CsvName                   );
    ascribe("CsvHeaderName"             , &DBDictionary::CsvHeaderName             );
    ascribe("NoLapseProvisionName"      , &DBDictionary::NoLapseProvisionName      );
    ascribe("ContractName"              , &DBDictionary::ContractName              );
    ascribe("DboName"                   , &DBDictionary::DboName                   );
    ascribe("DboNameLevel"              , &DBDictionary::DboNameLevel              );
    ascribe("DboNameIncreasing"         , &DBDictionary::DboNameIncreasing         );
    ascribe("DboNameReturnOfPremium"    , &DBDictionary::DboNameReturnOfPremium    );
    ascribe("DboNameMinDeathBenefit"    , &DBDictionary::DboNameMinDeathBenefit    );
    ascribe("GenAcctName"               , &DBDictionary::GenAcctName               );
    ascribe("GenAcctNameElaborated"     , &DBDictionary::GenAcctNameElaborated     );
    ascribe("SepAcctName"               , &DBDictionary::SepAcctName               );
    ascribe("SpecAmtName"               , &DBDictionary::SpecAmtName               );
    ascribe("SpecAmtNameElaborated"     , &DBDictionary::SpecAmtNameElaborated     );
    ascribe("UwBasisMedical"            , &DBDictionary::UwBasisMedical            );
    ascribe("UwBasisParamedical"        , &DBDictionary::UwBasisParamedical        );
    ascribe("UwBasisNonmedical"         , &DBDictionary::UwBasisNonmedical         );
    ascribe("UwBasisSimplified"         , &DBDictionary::UwBasisSimplified         );
    ascribe("UwBasisGuaranteed"         , &DBDictionary::UwBasisGuaranteed         );
    ascribe("UwClassPreferred"          , &DBDictionary::UwClassPreferred          );
    ascribe("UwClassStandard"           , &DBDictionary::UwClassStandard           );
    ascribe("UwClassRated"              , &DBDictionary::UwClassRated              );
    ascribe("UwClassUltra"              , &DBDictionary::UwClassUltra              );
    ascribe("AccountValueFootnote"      , &DBDictionary::AccountValueFootnote      );
    ascribe("AttainedAgeFootnote"       , &DBDictionary::AttainedAgeFootnote       );
    ascribe("CashSurrValueFootnote"     , &DBDictionary::CashSurrValueFootnote     );
    ascribe("DeathBenefitFootnote"      , &DBDictionary::DeathBenefitFootnote      );
    ascribe("InitialPremiumFootnote"    , &DBDictionary::InitialPremiumFootnote    );
    ascribe("NetPremiumFootnote"        , &DBDictionary::NetPremiumFootnote        );
    ascribe("GrossPremiumFootnote"      , &DBDictionary::GrossPremiumFootnote      );
    ascribe("OutlayFootnote"            , &DBDictionary::OutlayFootnote            );
    ascribe("PolicyYearFootnote"        , &DBDictionary::PolicyYearFootnote        );
    ascribe("ADDTerseName"              , &DBDictionary::ADDTerseName              );
    ascribe("InsurabilityTerseName"     , &DBDictionary::InsurabilityTerseName     );
    ascribe("ChildTerseName"            , &DBDictionary::ChildTerseName            );
    ascribe("SpouseTerseName"           , &DBDictionary::SpouseTerseName           );
    ascribe("TermTerseName"             , &DBDictionary::TermTerseName             );
    ascribe("WaiverTerseName"           , &DBDictionary::WaiverTerseName           );
    ascribe("AccelBftRiderTerseName"    , &DBDictionary::AccelBftRiderTerseName    );
    ascribe("OverloanRiderTerseName"    , &DBDictionary::OverloanRiderTerseName    );
    ascribe("ADDFootnote"               , &DBDictionary::ADDFootnote               );
    ascribe("ChildFootnote"             , &DBDictionary::ChildFootnote             );
    ascribe("SpouseFootnote"            , &DBDictionary::SpouseFootnote            );
    ascribe("TermFootnote"              , &DBDictionary::TermFootnote              );
    ascribe("WaiverFootnote"            , &DBDictionary::WaiverFootnote            );
    ascribe("AccelBftRiderFootnote"     , &DBDictionary::AccelBftRiderFootnote     );
    ascribe("OverloanRiderFootnote"     , &DBDictionary::OverloanRiderFootnote     );
    ascribe("GroupQuoteShortProductName", &DBDictionary::GroupQuoteShortProductName);
    ascribe("GroupQuoteIsNotAnOffer"    , &DBDictionary::GroupQuoteIsNotAnOffer    );
    ascribe("GroupQuoteRidersFooter"    , &DBDictionary::GroupQuoteRidersFooter    );
    ascribe("GroupQuotePolicyFormId"    , &DBDictionary::GroupQuotePolicyFormId    );
    ascribe("GroupQuoteStateVariations" , &DBDictionary::GroupQuoteStateVariations );
    ascribe("GroupQuoteProspectus"      , &DBDictionary::GroupQuoteProspectus      );
    ascribe("GroupQuoteUnderwriter"     , &DBDictionary::GroupQuoteUnderwriter     );
    ascribe("GroupQuoteBrokerDealer"    , &DBDictionary::GroupQuoteBrokerDealer    );
    ascribe("GroupQuoteRubricMandatory" , &DBDictionary::GroupQuoteRubricMandatory );
    ascribe("GroupQuoteRubricVoluntary" , &DBDictionary::GroupQuoteRubricVoluntary );
    ascribe("GroupQuoteRubricFusion"    , &DBDictionary::GroupQuoteRubricFusion    );
    ascribe("GroupQuoteFooterMandatory" , &DBDictionary::GroupQuoteFooterMandatory );
    ascribe("GroupQuoteFooterVoluntary" , &DBDictionary::GroupQuoteFooterVoluntary );
    ascribe("GroupQuoteFooterFusion"    , &DBDictionary::GroupQuoteFooterFusion    );
    ascribe("MinimumPremiumFootnote"    , &DBDictionary::MinimumPremiumFootnote    );
    ascribe("PremAllocationFootnote"    , &DBDictionary::PremAllocationFootnote    );
    ascribe("InterestDisclaimer"        , &DBDictionary::InterestDisclaimer        );
    ascribe("GuarMortalityFootnote"     , &DBDictionary::GuarMortalityFootnote     );
    ascribe("ProductDescription"        , &DBDictionary::ProductDescription        );
    ascribe("StableValueFootnote"       , &DBDictionary::StableValueFootnote       );
    ascribe("NoVanishPremiumFootnote"   , &DBDictionary::NoVanishPremiumFootnote   );
    ascribe("RejectPremiumFootnote"     , &DBDictionary::RejectPremiumFootnote     );
    ascribe("ExpRatingFootnote"         , &DBDictionary::ExpRatingFootnote         );
    ascribe("MortalityBlendFootnote"    , &DBDictionary::MortalityBlendFootnote    );
    ascribe("HypotheticalRatesFootnote" , &DBDictionary::HypotheticalRatesFootnote );
    ascribe("SalesLoadRefundFootnote"   , &DBDictionary::SalesLoadRefundFootnote   );
    ascribe("NoLapseEverFootnote"       , &DBDictionary::NoLapseEverFootnote       );
    ascribe("NoLapseFootnote"           , &DBDictionary::NoLapseFootnote           );
    ascribe("CurrentValuesFootnote"     , &DBDictionary::CurrentValuesFootnote     );
    ascribe("DBOption1Footnote"         , &DBDictionary::DBOption1Footnote         );
    ascribe("DBOption2Footnote"         , &DBDictionary::DBOption2Footnote         );
    ascribe("DBOption3Footnote"         , &DBDictionary::DBOption3Footnote         );
    ascribe("MinDeathBenefitFootnote"   , &DBDictionary::MinDeathBenefitFootnote   );
    ascribe("ExpRatRiskChargeFootnote"  , &DBDictionary::ExpRatRiskChargeFootnote  );
    ascribe("ExchangeChargeFootnote1"   , &DBDictionary::ExchangeChargeFootnote1   );
    ascribe("FlexiblePremiumFootnote"   , &DBDictionary::FlexiblePremiumFootnote   );
    ascribe("GuaranteedValuesFootnote"  , &DBDictionary::GuaranteedValuesFootnote  );
    ascribe("CreditingRateFootnote"     , &DBDictionary::CreditingRateFootnote     );
    ascribe("GrossRateFootnote"         , &DBDictionary::GrossRateFootnote         );
    ascribe("NetRateFootnote"           , &DBDictionary::NetRateFootnote           );
    ascribe("MecFootnote"               , &DBDictionary::MecFootnote               );
    ascribe("GptFootnote"               , &DBDictionary::GptFootnote               );
    ascribe("MidpointValuesFootnote"    , &DBDictionary::MidpointValuesFootnote    );
    ascribe("SinglePremiumFootnote"     , &DBDictionary::SinglePremiumFootnote     );
    ascribe("MonthlyChargesFootnote"    , &DBDictionary::MonthlyChargesFootnote    );
    ascribe("UltCreditingRateFootnote"  , &DBDictionary::UltCreditingRateFootnote  );
    ascribe("UltCreditingRateHeader"    , &DBDictionary::UltCreditingRateHeader    );
    ascribe("MaxNaarFootnote"           , &DBDictionary::MaxNaarFootnote           );
    ascribe("PremTaxSurrChgFootnote"    , &DBDictionary::PremTaxSurrChgFootnote    );
    ascribe("PolicyFeeFootnote"         , &DBDictionary::PolicyFeeFootnote         );
    ascribe("AssetChargeFootnote"       , &DBDictionary::AssetChargeFootnote       );
    ascribe("InvestmentIncomeFootnote"  , &DBDictionary::InvestmentIncomeFootnote  );
    ascribe("IrrDbFootnote"             , &DBDictionary::IrrDbFootnote             );
    ascribe("IrrCsvFootnote"            , &DBDictionary::IrrCsvFootnote            );
    ascribe("MortalityChargesFootnote"  , &DBDictionary::MortalityChargesFootnote  );
    ascribe("LoanAndWithdrawalFootnote" , &DBDictionary::LoanAndWithdrawalFootnote );
    ascribe("LoanFootnote"              , &DBDictionary::LoanFootnote              );
    ascribe("ImprimaturPresale"         , &DBDictionary::ImprimaturPresale         );
    ascribe("ImprimaturPresaleComposite", &DBDictionary::ImprimaturPresaleComposite);
    ascribe("ImprimaturInforce"         , &DBDictionary::ImprimaturInforce         );
    ascribe("ImprimaturInforceComposite", &DBDictionary::ImprimaturInforceComposite);
    ascribe("StateMarketingImprimatur"  , &DBDictionary::StateMarketingImprimatur  );
    ascribe("NonGuaranteedFootnote"     , &DBDictionary::NonGuaranteedFootnote     );
    ascribe("NonGuaranteedFootnote1"    , &DBDictionary::NonGuaranteedFootnote1    );
    ascribe("NonGuaranteedFootnote1Tx"  , &DBDictionary::NonGuaranteedFootnote1Tx  );
    ascribe("FnMonthlyDeductions"       , &DBDictionary::FnMonthlyDeductions       );
    ascribe("SurrenderFootnote"         , &DBDictionary::SurrenderFootnote         );
    ascribe("PortabilityFootnote"       , &DBDictionary::PortabilityFootnote       );
    ascribe("FundRateFootnote"          , &DBDictionary::FundRateFootnote          );
    ascribe("IssuingCompanyFootnote"    , &DBDictionary::IssuingCompanyFootnote    );
    ascribe("SubsidiaryFootnote"        , &DBDictionary::SubsidiaryFootnote        );
    ascribe("PlacementAgentFootnote"    , &DBDictionary::PlacementAgentFootnote    );
    ascribe("MarketingNameFootnote"     , &DBDictionary::MarketingNameFootnote     );
    ascribe("GuarIssueDisclaimerNcSc"   , &DBDictionary::GuarIssueDisclaimerNcSc   );
    ascribe("GuarIssueDisclaimerMd"     , &DBDictionary::GuarIssueDisclaimerMd     );
    ascribe("GuarIssueDisclaimerTx"     , &DBDictionary::GuarIssueDisclaimerTx     );
    ascribe("IllRegCertAgent"           , &DBDictionary::IllRegCertAgent           );
    ascribe("IllRegCertAgentIl"         , &DBDictionary::IllRegCertAgentIl         );
    ascribe("IllRegCertAgentTx"         , &DBDictionary::IllRegCertAgentTx         );
    ascribe("IllRegCertClient"          , &DBDictionary::IllRegCertClient          );
    ascribe("IllRegCertClientIl"        , &DBDictionary::IllRegCertClientIl        );
    ascribe("IllRegCertClientTx"        , &DBDictionary::IllRegCertClientTx        );
    ascribe("FnMaturityAge"             , &DBDictionary::FnMaturityAge             );
    ascribe("FnPartialMortality"        , &DBDictionary::FnPartialMortality        );
    ascribe("FnProspectus"              , &DBDictionary::FnProspectus              );
    ascribe("FnInitialSpecAmt"          , &DBDictionary::FnInitialSpecAmt          );
    ascribe("FnInforceAcctVal"          , &DBDictionary::FnInforceAcctVal          );
    ascribe("FnInforceTaxBasis"         , &DBDictionary::FnInforceTaxBasis         );
    ascribe("Fn1035Charge"              , &DBDictionary::Fn1035Charge              );
    ascribe("FnMecExtraWarning"         , &DBDictionary::FnMecExtraWarning         );
    ascribe("FnNotTaxAdvice"            , &DBDictionary::FnNotTaxAdvice            );
    ascribe("FnNotTaxAdvice2"           , &DBDictionary::FnNotTaxAdvice2           );
    ascribe("FnImf"                     , &DBDictionary::FnImf                     );
    ascribe("FnCensus"                  , &DBDictionary::FnCensus                  );
    ascribe("FnDacTax"                  , &DBDictionary::FnDacTax                  );
    ascribe("FnDefnLifeIns"             , &DBDictionary::FnDefnLifeIns             );
    ascribe("FnBoyEoy"                  , &DBDictionary::FnBoyEoy                  );
    ascribe("FnGeneralAccount"          , &DBDictionary::FnGeneralAccount          );
    ascribe("FnPpMemorandum"            , &DBDictionary::FnPpMemorandum            );
    ascribe("FnPpAccreditedInvestor"    , &DBDictionary::FnPpAccreditedInvestor    );
    ascribe("FnPpLoads"                 , &DBDictionary::FnPpLoads                 );
    ascribe("FnProposalUnderwriting"    , &DBDictionary::FnProposalUnderwriting    );
    ascribe("FnGuaranteedPremium"       , &DBDictionary::FnGuaranteedPremium       );
    ascribe("FnOmnibusDisclaimer"       , &DBDictionary::FnOmnibusDisclaimer       );
    ascribe("FnInitialDbo"              , &DBDictionary::FnInitialDbo              );
    ascribe("DefnGuarGenAcctRate"       , &DBDictionary::DefnGuarGenAcctRate       );
    ascribe("DefnAV"                    , &DBDictionary::DefnAV                    );
    ascribe("DefnCSV"                   , &DBDictionary::DefnCSV                   );
    ascribe("DefnMec"                   , &DBDictionary::DefnMec                   );
    ascribe("DefnOutlay"                , &DBDictionary::DefnOutlay                );
    ascribe("DefnSpecAmt"               , &DBDictionary::DefnSpecAmt               );
    ascribe("WeightClass"               , &DBDictionary::WeightClass               );
    ascribe("WeightGender"              , &DBDictionary::WeightGender              );
    ascribe("WeightSmoking"             , &DBDictionary::WeightSmoking             );
    ascribe("WeightAge"                 , &DBDictionary::WeightAge                 );
    ascribe("WeightSpecAmt"             , &DBDictionary::WeightSpecAmt             );
    ascribe("WeightState"               , &DBDictionary::WeightState               );
    ascribe("FullExpPol"                , &DBDictionary::FullExpPol                );
    ascribe("FullExpPrem"               , &DBDictionary::FullExpPrem               );
    ascribe("FullExpDumpin"             , &DBDictionary::FullExpDumpin             );
    ascribe("FullExpSpecAmt"            , &DBDictionary::FullExpSpecAmt            );
    ascribe("VarExpPol"                 , &DBDictionary::VarExpPol                 );
    ascribe("VarExpPrem"                , &DBDictionary::VarExpPrem                );
    ascribe("VarExpDumpin"              , &DBDictionary::VarExpDumpin              );
    ascribe("VarExpSpecAmt"             , &DBDictionary::VarExpSpecAmt             );
    ascribe("ExpSpecAmtLimit"           , &DBDictionary::ExpSpecAmtLimit           );
    ascribe("MedicalProportion"         , &DBDictionary::MedicalProportion         );
    ascribe("UwTestCost"                , &DBDictionary::UwTestCost                );
    ascribe("VxBasicQTable"             , &DBDictionary::VxBasicQTable             );
    ascribe("VxDeficQTable"             , &DBDictionary::VxDeficQTable             );
    ascribe("VxTaxQTable"               , &DBDictionary::VxTaxQTable               );
    ascribe("StatVxInt"                 , &DBDictionary::StatVxInt                 );
    ascribe("TaxVxInt"                  , &DBDictionary::TaxVxInt                  );
    ascribe("StatVxQ"                   , &DBDictionary::StatVxQ                   );
    ascribe("TaxVxQ"                    , &DBDictionary::TaxVxQ                    );
    ascribe("DeficVxQ"                  , &DBDictionary::DeficVxQ                  );
    ascribe("SnflQ"                     , &DBDictionary::SnflQ                     );
    ascribe("LapseRate"                 , &DBDictionary::LapseRate                 );
    ascribe("ReqSurpNaar"               , &DBDictionary::ReqSurpNaar               );
    ascribe("ReqSurpVx"                 , &DBDictionary::ReqSurpVx                 );
    ascribe("LicFitRate"                , &DBDictionary::LicFitRate                );
    ascribe("LicDacTaxRate"             , &DBDictionary::LicDacTaxRate             );
    ascribe("GdbVxMethod"               , &DBDictionary::GdbVxMethod               );
    ascribe("PrimaryHurdle"             , &DBDictionary::PrimaryHurdle             );
    ascribe("SecondaryHurdle"           , &DBDictionary::SecondaryHurdle           );
}

/// Read a database file.

void DBDictionary::Init(std::string const& filename)
{
    LMI_ASSERT(!filename.empty());

    try
        {
        load(filename);
        }
    catch(...)
        {
        report_exception();
        }
}

/// Save a database file.

void DBDictionary::WriteDB(std::string const& filename) const
{
    save(filename);
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20100608T1241Z

int DBDictionary::class_version() const
{
    return 0;
}

std::string const& DBDictionary::xml_root_name() const
{
    static std::string const s("database");
    return s;
}

/// This override doesn't call redintegrate_ex_ante(); that wouldn't
/// make sense, at least not for now.

void DBDictionary::read_element
    (xml::element const& e
    ,std::string const&  name
    ,int                 // file_version
    )
{
    xml_serialize::from_xml(e, datum(name));
}

void DBDictionary::write_element
    (xml::element&       parent
    ,std::string const&  name
    ) const
{
    xml_serialize::set_element(parent, name, datum(name));
}

void DBDictionary::write_proem
    (xml_lmi::xml_document& document
    ,std::string const&     file_leaf_name
    ) const
{
    ::write_proem(document, file_leaf_name);
}

/// Set a value. (The historical name "Add" is now misleading.)

void DBDictionary::Add(database_entity const& e)
{
    datum(db_name_from_key(e.key())) = e;
}

/// Initialize all database entities to not-necessarily-plausible values.

void DBDictionary::InitDB()
{
    static double const dbl_inf = infinity<double>();

    for(auto const& i : member_names())
        {
        Add({db_key_from_name(i), 0.0});
        }

    // Most enumerative types have a plausible-default enumerator that
    // happens to equal zero. They're explicitly initialized here with
    // appropriate enumerators, to make the implicit types clear.
    //
    // Some, like DB_IntSpreadMode, have no enumerator equal to zero,
    // and must be initialized explicitly with some actual enumerator.
    Add({DB_SmokeOrTobacco      , oe_smoker_nonsmoker});
    Add({DB_CorridorWhence      , oe_7702_corr_first_principles});
    Add({DB_Irc7702NspWhence    , oe_7702_nsp_first_principles});
    Add({DB_SevenPayWhence      , oe_7702_7pp_first_principles});
    Add({DB_Irc7702QWhence      , oe_7702_q_builtin});
    Add({DB_CvatMatChangeDefn   , mce_unnecessary_premium});
    Add({DB_Effective7702DboRop , mce_option1_for_7702});
    Add({DB_TermIsQABOrDb7702   , oe_7702_term_is_db});
    Add({DB_TermIsQABOrDb7702A  , oe_7702_term_is_db});
    Add({DB_CsoEra              , mce_2017cso});
    Add({DB_CsoMisprint         , oe_orthodox});
    Add({DB_GuarCoiWhence       , oe_guar_coi_external_table});
    Add({DB_CoiInforceReentry   , e_reenter_never});
    Add({DB_SepAcctSpreadMethod , mce_spread_is_effective_annual});
    Add({DB_IntSpreadMode       , mce_spread_daily});
    Add({DB_AssetChargeType     , oe_asset_charge_spread});
    Add({DB_PremTaxState        , oe_ee_state});
    Add({DB_WpChargeMethod      , oe_waiver_times_deductions});
    Add({DB_MaxWdDed            , mce_twelve_times_last});
    Add({DB_MinPremType         , oe_monthly_deduction});
    Add({DB_TgtPremType         , oe_monthly_deduction});
    Add({DB_IsSinglePremium     , oe_flexible_premium});
    Add({DB_DeductionMethod     , oe_proportional});
    Add({DB_DeductionAcct       , oe_prefer_general_account});
    Add({DB_DistributionMethod  , oe_proportional});
    Add({DB_DistributionAcct    , oe_prefer_general_account});
    Add({DB_EePremMethod        , oe_input_allocation});
    Add({DB_EePremAcct          , oe_prefer_general_account});
    Add({DB_ErPremMethod        , oe_input_allocation});
    Add({DB_ErPremAcct          , oe_prefer_general_account});
    Add({DB_LedgerType          , mce_ill_reg});
    Add({DB_AgeLastOrNearest    , oe_age_last_birthday});

    // It would be dangerous to set these multipliers to zero.
    Add({DB_GuarCoiMultiplier   , 1.0});
    Add({DB_MinInputCoiMult     , 1.0});
    Add({DB_CurrCoiMultiplier   , 1.0});
    Add({DB_SubstdTableMult     , 1.0});
    Add({DB_MaxWdGenAcctValMult , 1.0});
    Add({DB_MaxWdSepAcctValMult , 1.0});

    // Usually the maximum is a reciprocal, e.g., 1/11 or 1/12; for
    // greatest precision, store the reciprocal of that reciprocal,
    // e.g., 11 or 12.
    Add({DB_MaxMonthlyCoiRate   , 12.0});

    // These are the same as class date_trammel's nominal limits.
    Add({DB_CoiResetMinDate     , calendar_date::gregorian_epoch_jdn});
    Add({DB_CoiResetMaxDate     , calendar_date::last_yyyy_date_jdn });

    Add({DB_CurrCoiTable0Limit  , dbl_inf});
    Add({DB_CurrCoiTable1Limit  , dbl_inf});
    Add({DB_GuarIntSpread       , dbl_inf});
    Add({DB_SpecAmtLoadLimit    , dbl_inf});
    Add({DB_DynSepAcctLoadLimit , dbl_inf});
    Add({DB_PremTaxRetalLimit   , dbl_inf});

    // This is determined by law and regulation, and should be the
    // same for all life-insurance products.
    int ptd[e_number_of_axes] = {1, 1, 1, 1, 1, e_max_dim_state, 1};
    std::vector<int> premium_tax_dimensions(ptd, ptd + e_number_of_axes);
    Add({DB_PremTaxRate, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});

    // This is determined by law, in a way that varies by product;
    // all pre-2021 products use this default.
    Add({DB_AnnInterestRate7702 , 0.04});
    // Should these be specified here?
    //   DB_IgnoreLoanRateFor7702
    //   DB_MlyDiscountRate7702Whence
    // This probably should correspond to the annual 7702 rate.
    Add({DB_MlyDiscountRate7702 , 0.0032737});

    Add({DB_MaxIssSpecAmt       , dbl_inf});
    Add({DB_MaxRenlSpecAmt      , dbl_inf});
    Add({DB_WpLimit             , dbl_inf});
    Add({DB_AdbLimit            , dbl_inf});
    Add({DB_SpouseRiderMaxAmt   , dbl_inf});
    Add({DB_SpouseRiderMaxIssAge  , 99});   // age_trammel's nominal upper limit
    Add({DB_ChildRiderMaxAmt    , dbl_inf});

    // This must not be zero in TX, which specifically requires a
    // "guaranteed" rate of not more than fifteen percent--see:
    //   http://texinfo.library.unt.edu/Texasregister/html/1998/sep-25/adopted/insurance.html
    //   "staff added this subsection which requires that 'if' policy
    //   loans are illustrated on a guaranteed basis, interest charged
    //   must be calculated at the highest numeric rate permitted
    //   under the terms of the contract."
    int mvd[e_number_of_axes] = {1, 1, 1, 1, 1, e_max_dim_state, 1};
    std::vector<int> max_vlr_dimensions(mvd, mvd + e_number_of_axes);
    std::vector<double> max_vlr(e_max_dim_state);
    max_vlr[mce_s_TX] = 0.15;
    Add({DB_MaxVlrRate, max_vlr_dimensions, max_vlr});

    Add({DB_FirstPrefLoanYear   , 100});
    Add({DB_ExpSpecAmtLimit     , dbl_inf});
}

namespace
{
class sample : public DBDictionary {public: sample();};

class sample2 : public sample {public: sample2();};

class sample2naic  : public sample2 {public: sample2naic ();};
class sample2finra : public sample2 {public: sample2finra();};
class sample2prosp : public sample2 {public: sample2prosp();};
class sample2gpp   : public sample2 {public: sample2gpp  ();};
class sample2ipp   : public sample2 {public: sample2ipp  ();};
class sample2xyz   : public sample2 {public: sample2xyz  ();};

sample::sample()
{
    Add({DB_MinIssAge           , 15});
    Add({DB_MaxIssAge           , 70});
    Add({DB_MaxIncrAge          , 99});
    Add({DB_AllowFullUw         , true});
    Add({DB_AllowParamedUw      , true});
    Add({DB_AllowNonmedUw       , true});
    Add({DB_AllowSimpUw         , true});
    Add({DB_AllowGuarUw         , true});
    Add({DB_SmokeOrTobacco      , oe_tobacco_nontobacco});
    Add({DB_AllowPreferredClass , true});
    Add({DB_AllowUltraPrefClass , false});

    // Forbid substandard table ratings with simplified or guaranteed issue.
    int uw_dims[e_number_of_axes] = {1, 1, 1, 1, 5, 1, 1};
    //                              med  para nonmed   SI     GI
    double allow_substd_table[] = {true, true, true, false, false};
    Add({DB_AllowSubstdTable, e_number_of_axes, uw_dims, allow_substd_table});

    Add({DB_AllowFlatExtras     , true});
    Add({DB_AllowRatedWp        , false});
    Add({DB_AllowRatedAdb       , false});
    Add({DB_AllowRatedTerm      , true});
    Add({DB_AllowRetirees       , true});
    Add({DB_AllowUnisex         , true});
    Add({DB_AllowSexDistinct    , true});
    Add({DB_AllowUnismoke       , true});
    Add({DB_AllowSmokeDistinct  , true});
    Add({DB_StateApproved       , true});
    Add({DB_AllowStateXX        , true});
    Add({DB_AllowForeign        , true});
    Add({DB_GroupIndivSelection , false});
    Add({DB_Allowable           , true});
    Add({DB_AllowCvat           , true});
    Add({DB_AllowGpt            , true});
    Add({DB_AllowNo7702         , false});
    Add({DB_CorridorWhence      , oe_7702_corr_from_table});
    Add({DB_Irc7702NspWhence    , oe_7702_nsp_reciprocal_cvat_corridor});
    Add({DB_SevenPayWhence      , oe_7702_7pp_from_table});
    Add({DB_Irc7702QWhence      , oe_7702_q_external_table});

    // This is just a sample product, so make do with plausible
    // all-male seven-pay premiums, and use GPT corridor factors for
    // CVAT. 'Irc7702NspWhence' specifies that NSP is calculated as
    // the reciprocal of CVAT corridor, so no NSP table is needed.
    Add({DB_CorridorTable       , 7});
    Add({DB_Irc7702NspTable     , 0});
    Add({DB_SevenPayTable       , 10});

    Add({DB_CsoEra              , mce_1980cso});
    // Following IRS Notice 88-128, use only the male and female
    // tables with no smoker distinction, and a unisex table where
    // required by state law.
    //
    // US 1980 CSO age last, not smoker distinct. Unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    int dims311[e_number_of_axes] = {3, 1, 1, 1, 1, 1, 1}; // gender
    double T7702q[3] = {35, 41, 107,}; // Female, male, unisex.
    Add({DB_Irc7702QTable, e_number_of_axes, dims311, T7702q});
    Add({DB_Irc7702QAxisGender  , true});
    Add({DB_Irc7702QAxisSmoking , false});

    Add({DB_CvatMatChangeDefn   , mce_earlier_of_increase_or_unnecessary_premium});
    Add({DB_GptMatChangeDefn    , 0});
    Add({DB_Irc7702BftIsSpecAmt , 0});

    // US 1980 CSO age last; unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    int dims313[e_number_of_axes] = {3, 1, 3, 1, 1, 1, 1}; // gender, smoker
    double TgCOI[9] =
        {
         39,  37,  35, // female: sm ns us
         45,  57,  41, // male:   sm ns us
        111, 109, 107, // unisex: sm ns us
        };
    Add({DB_GuarCoiTable, e_number_of_axes, dims313, TgCOI});

    Add({DB_GuarCoiIsAnnual     , true});

    // For now at least, just use (a multiple of) guaranteed COI rates
    // as current.
    Add({DB_CurrCoiTable, e_number_of_axes, dims313, TgCOI});

    Add({DB_CurrCoiIsAnnual     , true});

    double coimult[9] =
        {
        0.40, 0.30, 0.35, // female: sm ns us
        0.60, 0.50, 0.55, // male:   sm ns us
        0.50, 0.40, 0.45, // unisex: sm ns us
        };
    Add({DB_CurrCoiMultiplier, e_number_of_axes, dims313, coimult});

    Add({DB_MdptCoiIsAnnual     , true});
    Add({DB_UseNyCoiFloor       , 0.0});
    Add({DB_GuarCoiCeiling      , false});
    Add({DB_CoiGuarIsMin        , false});
    Add({DB_AllowMortBlendSex   , true});
    Add({DB_AllowMortBlendSmoke , true});
    Add({DB_GuarInt             , 0.03});
    Add({DB_NaarDiscount        , 0.00246627});
    Add({DB_GuarMandE           , 0.009});
    Add({DB_CurrIntSpread       , 0.01});
    Add({DB_CurrMandE           , 0.009});
    Add({DB_GenAcctIntBonus     , 0.0});
    Add({DB_IntFloor            , 0.0});
    Add({DB_AllowGenAcct        , true});
    Add({DB_AllowSepAcct        , true});
    Add({DB_AllowGenAcctEarnRate, true});
    Add({DB_AllowSepAcctNetRate , true});
    Add({DB_MaxGenAcctRate      , 0.06});
    Add({DB_MaxSepAcctRate      , 0.12});
    Add({DB_SepAcctSpreadMethod , mce_spread_is_effective_annual});
    Add({DB_IntSpreadMode       , mce_spread_daily});
    Add({DB_DynamicMandE        , false});
    Add({DB_AllowAmortPremLoad  , false});
    Add({DB_LoadAmortFundCharge , 0.0030});
    Add({DB_AllowImfOverride    , false});
    Add({DB_AssetChargeType     , oe_asset_charge_spread});
    Add({DB_GuarFundAdminChg    , 0.0});
    Add({DB_CurrFundAdminChg    , 0.0});
    Add({DB_FundCharge          , 0.0});
    Add({DB_GuarMonthlyPolFee   , 8.00});
    Add({DB_GuarAnnualPolFee    , 0.0});
    Add({DB_GuarPremLoadTgt     , 0.07});
    Add({DB_GuarPremLoadExc     , 0.04});
    Add({DB_GuarPremLoadTgtRfd  , 0.00});
    Add({DB_GuarPremLoadExcRfd  , 0.00});
    Add({DB_GuarSpecAmtLoad     , 0.0});
    Add({DB_GuarAcctValLoad     , 0.0});
    Add({DB_CurrMonthlyPolFee   , 5.00});
    Add({DB_CurrAnnualPolFee    , 0.0});
    Add({DB_CurrPremLoadTgt     , 0.05});
    Add({DB_CurrPremLoadExc     , 0.02});
    Add({DB_CurrPremLoadTgtRfd  , 0.00});
    Add({DB_CurrPremLoadExcRfd  , 0.00});
    Add({DB_CurrSpecAmtLoad     , 0.0});
    Add({DB_CurrAcctValLoad     , 0.0});
    Add({DB_TgtPremMonthlyPolFee, 0.0});
    Add({DB_LoadRfdProportion   , 0.0});
    Add({DB_SpecAmtLoadLimit    , 10000000.0});
    Add({DB_DynamicSepAcctLoad  , false});
    Add({DB_DacTaxFundCharge    , 0.0});
    Add({DB_DacTaxPremLoad      , 0.01});
    Add({DB_PremTaxFundCharge   , 0.0});

    // Pass through premium tax.
    int ptd[e_number_of_axes] = {1, 1, 1, 1, 1, e_max_dim_state, 1};
    std::vector<int> premium_tax_dimensions(ptd, ptd + e_number_of_axes);
    Add({DB_PremTaxLoad, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});

    Add({DB_WaivePremTaxInt1035 , true});
    Add({DB_PremTaxAmortPeriod  , 0});
    Add({DB_PremTaxAmortIntRate , 0.0});

    Add({DB_PremTaxRate, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});

    Add({DB_PremTaxState        , oe_ee_state});
    Add({DB_AllowSpecAmtIncr    , true});
    Add({DB_MinSpecAmtIncr      , 0.0});
    Add({DB_EnforceNaarLimit    , true});
    Add({DB_MinSpecAmt          , 100000.0});
    Add({DB_MinIssSpecAmt       , 50000.0});
    Add({DB_MinIssBaseSpecAmt   , 50000.0});
    Add({DB_MinRenlSpecAmt      , 50000.0});
    Add({DB_MinRenlBaseSpecAmt  , 50000.0});
    Add({DB_AllowDboLvl         , true});
    Add({DB_AllowDboInc         , true});
    Add({DB_AllowDboRop         , true});
    Add({DB_AllowDboMdb         , true});
    Add({DB_DboLvlChangeToWhat  , 0b1111});
    Add({DB_DboLvlChangeMethod  , 0b1111});
    Add({DB_DboIncChangeToWhat  , 0b1111});
    Add({DB_DboIncChangeMethod  , 0b1111});
    Add({DB_DboRopChangeToWhat  , 0b1111});
    Add({DB_DboRopChangeMethod  , 0b1111});
    Add({DB_DboMdbChangeToWhat  , 0b1111});
    Add({DB_DboMdbChangeMethod  , 0b1111});
    Add({DB_AllowChangeToDbo2   , true});
    Add({DB_DboChgCanIncrSpecAmt, true});
    Add({DB_DboChgCanDecrSpecAmt, true});
    Add({DB_AllowExtEndt        , true});
    Add({DB_AllowTerm           , true});

    int dims143[e_number_of_axes] = {1, 4, 3, 1, 1, 1, 1}; // uw_class, smoker
    double TtCOI[12] =
        {
        3, 2, 1, // pref:  sm ns us
        6, 5, 4, // std:   sm ns us
        6, 5, 4, // rated: sm ns us [same as std]
        0, 0, 0, // ultra: sm ns us [zero: error message--no ultrapref class]
        };
    Add({DB_GuarTermTable, e_number_of_axes, dims143, TtCOI});
    Add({DB_TermTable    , e_number_of_axes, dims143, TtCOI});

    Add({DB_TermMinIssAge       , 15});
    Add({DB_TermMaxIssAge       , 65});
    Add({DB_TermForcedConvAge   , 70});
    Add({DB_TermForcedConvDur   , 10});
    Add({DB_MaxTermProportion   , 0.0});
    Add({DB_AllowWp             , true});
    Add({DB_WpTable             , 8});
    Add({DB_WpMinIssAge         , 18});
    Add({DB_WpMaxIssAge         , 64});
    Add({DB_AllowAdb            , true});
    Add({DB_AdbTable            , 708});   // 70-75 US ADB experience
    Add({DB_AdbMinIssAge        , 15});
    Add({DB_AdbMaxIssAge        , 70});
    Add({DB_AdbLimit            , 1000000.0});
    Add({DB_AllowSpouseRider    , true});
    Add({DB_SpouseRiderMinAmt   , 10000.0});
    Add({DB_SpouseRiderMaxAmt   , 1000000.0});
    Add({DB_SpouseRiderMinIssAge, 20});
    Add({DB_SpouseRiderMaxIssAge, 65});
    Add({DB_SpouseRiderGuarTable, 305});   // arbitrarily use 1960 CSG
    Add({DB_SpouseRiderTable    , 305});   // arbitrarily use 1960 CSG
    Add({DB_AllowChildRider     , true});
    Add({DB_ChildRiderMinAmt    , 25000}); // for testing, min==max
    Add({DB_ChildRiderMaxAmt    , 25000}); // for testing, min==max
    Add({DB_ChildRiderTable     , 305});   // arbitrarily use 1960 CSG
    Add({DB_AllowWd             , true});
    Add({DB_WdFee               , 25.0});
    Add({DB_WdFeeRate           , 0.02});
    Add({DB_MinWd               , 100.0});
    Add({DB_MaxWdDed            , mce_to_next_anniversary});
    Add({DB_WdDecrSpecAmtDboLvl , true});
    Add({DB_WdDecrSpecAmtDboInc , true});
    Add({DB_WdDecrSpecAmtDboRop , true});
    Add({DB_FirstWdMonth        , 0.0});
    Add({DB_AllowLoan           , true});
    Add({DB_AllowPrefLoan       , false});
    Add({DB_AllowFixedLoan      , true});
    Add({DB_AllowVlr            , true});
    Add({DB_FixedLoanRate       , 0.06});
    Add({DB_MinVlrRate          , 0.04});
    Add({DB_MaxLoanAcctValMult  , 1.0});
    Add({DB_MaxLoanDed          , mce_to_next_anniversary});
    Add({DB_GuarPrefLoanSpread  , 0.0});
    Add({DB_GuarRegLoanSpread   , 0.04});
    Add({DB_CurrPrefLoanSpread  , 0.0});
    Add({DB_CurrRegLoanSpread   , 0.02});
    Add({DB_FirstLoanMonth      , 0.0});
    Add({DB_MinPremType         , oe_monthly_deduction});
    Add({DB_TgtPremType         , oe_modal_nonmec});
    Add({DB_TgtPremTable        , 10});    // use seven-pay as target
    Add({DB_TgtPremFixedAtIssue , false});
    Add({DB_TgtPremIgnoreSubstd , true});
    Add({DB_MinPmt              , 0.0});
    Add({DB_NoLapseMinDur       , 0.0});
    Add({DB_NoLapseMinAge       , 0.0});
    Add({DB_NoLapseUnratedOnly  , false});
    Add({DB_NoLapseDboLvlOnly   , false});
    Add({DB_NoLapseAlwaysActive , false});
    Add({DB_AllowHoneymoon      , true});
    Add({DB_AllowExtraAssetComp , true});
    Add({DB_AllowExtraPremComp  , true});
    Add({DB_LedgerType          , mce_ill_reg});
    Add({DB_AgeLastOrNearest    , oe_age_last_birthday});
    Add({DB_MaturityAge         , 100});
    Add({DB_GroupProxyRateTable , 305});   // 1960 CSG (gender-indistinct)

    // 1983 GAM; unisex=male because no unisex table was published.
    double T83Gam[3] = {825, 826, 826,}; // f, m, u
    Add({DB_PartialMortTable, e_number_of_axes, dims311, T83Gam});

    Add({DB_AllowGroupQuote     , true});

    // Policy form differs in states beginning with "K".
    std::vector<double> policy_form(e_max_dim_state, superior::policy_form);
    policy_form[mce_s_KS] = superior::policy_form_KS_KY;
    policy_form[mce_s_KY] = superior::policy_form_KS_KY;
    Add({DB_PolicyForm, premium_tax_dimensions, policy_form});
    Add({DB_PolicyMktgName             , superior::PolicyMktgName});
    Add({DB_PolicyLegalName            , superior::PolicyLegalName});
    Add({DB_InsCoShortName             , superior::InsCoShortName});
    Add({DB_InsCoName                  , superior::InsCoName});
    Add({DB_InsCoAddr                  , superior::InsCoAddr});
    Add({DB_InsCoStreet                , superior::InsCoStreet});
    Add({DB_InsCoPhone                 , superior::InsCoPhone});
    Add({DB_MainUnderwriter            , superior::MainUnderwriter});
    Add({DB_MainUnderwriterAddress     , superior::MainUnderwriterAddress});
    Add({DB_CoUnderwriter              , superior::CoUnderwriter});
    Add({DB_CoUnderwriterAddress       , superior::CoUnderwriterAddress});
    Add({DB_AvName                     , superior::AvName});
    Add({DB_CsvName                    , superior::CsvName});
    Add({DB_CsvHeaderName              , superior::CsvHeaderName});
    Add({DB_NoLapseProvisionName       , superior::NoLapseProvisionName});
    Add({DB_ContractName               , superior::ContractName});
    Add({DB_DboName                    , superior::DboName});
    Add({DB_DboNameLevel               , superior::DboNameLevel});
    Add({DB_DboNameIncreasing          , superior::DboNameIncreasing});
    Add({DB_DboNameReturnOfPremium     , superior::DboNameReturnOfPremium});
    Add({DB_DboNameMinDeathBenefit     , superior::DboNameMinDeathBenefit});
    Add({DB_GenAcctName                , superior::GenAcctName});
    Add({DB_GenAcctNameElaborated      , superior::GenAcctNameElaborated});
    Add({DB_SepAcctName                , superior::SepAcctName});
    Add({DB_SpecAmtName                , superior::SpecAmtName});
    Add({DB_SpecAmtNameElaborated      , superior::SpecAmtNameElaborated});
    Add({DB_UwBasisMedical             , superior::UwBasisMedical});
    Add({DB_UwBasisParamedical         , superior::UwBasisParamedical});
    Add({DB_UwBasisNonmedical          , superior::UwBasisNonmedical});
    Add({DB_UwBasisSimplified          , superior::UwBasisSimplified});
    Add({DB_UwBasisGuaranteed          , superior::UwBasisGuaranteed});
    Add({DB_UwClassPreferred           , superior::UwClassPreferred});
    Add({DB_UwClassStandard            , superior::UwClassStandard});
    Add({DB_UwClassRated               , superior::UwClassRated});
    Add({DB_UwClassUltra               , superior::UwClassUltra});
    Add({DB_AccountValueFootnote       , superior::AccountValueFootnote});
    Add({DB_AttainedAgeFootnote        , superior::AttainedAgeFootnote});
    Add({DB_CashSurrValueFootnote      , superior::CashSurrValueFootnote});
    Add({DB_DeathBenefitFootnote       , superior::DeathBenefitFootnote});
    Add({DB_InitialPremiumFootnote     , superior::InitialPremiumFootnote});
    Add({DB_NetPremiumFootnote         , superior::NetPremiumFootnote});
    Add({DB_GrossPremiumFootnote       , superior::GrossPremiumFootnote});
    Add({DB_OutlayFootnote             , superior::OutlayFootnote});
    Add({DB_PolicyYearFootnote         , superior::PolicyYearFootnote});
    Add({DB_ADDTerseName               , superior::ADDTerseName});
    Add({DB_InsurabilityTerseName      , superior::InsurabilityTerseName});
    Add({DB_ChildTerseName             , superior::ChildTerseName});
    Add({DB_SpouseTerseName            , superior::SpouseTerseName});
    Add({DB_TermTerseName              , superior::TermTerseName});
    Add({DB_WaiverTerseName            , superior::WaiverTerseName});
    Add({DB_AccelBftRiderTerseName     , superior::AccelBftRiderTerseName});
    Add({DB_OverloanRiderTerseName     , superior::OverloanRiderTerseName});
    Add({DB_ADDFootnote                , superior::ADDFootnote});
    Add({DB_ChildFootnote              , superior::ChildFootnote});
    Add({DB_SpouseFootnote             , superior::SpouseFootnote});
    Add({DB_TermFootnote               , superior::TermFootnote});
    Add({DB_WaiverFootnote             , superior::WaiverFootnote});
    Add({DB_AccelBftRiderFootnote      , superior::AccelBftRiderFootnote});
    Add({DB_OverloanRiderFootnote      , superior::OverloanRiderFootnote});
    Add({DB_GroupQuoteShortProductName , superior::GroupQuoteShortProductName});
    Add({DB_GroupQuoteIsNotAnOffer     , superior::GroupQuoteIsNotAnOffer});
    Add({DB_GroupQuoteRidersFooter     , superior::GroupQuoteRidersFooter});
    Add({DB_GroupQuotePolicyFormId     , superior::GroupQuotePolicyFormId});
    Add({DB_GroupQuoteStateVariations  , superior::GroupQuoteStateVariations});
    Add({DB_GroupQuoteProspectus       , superior::GroupQuoteProspectus});
    Add({DB_GroupQuoteUnderwriter      , superior::GroupQuoteUnderwriter});
    Add({DB_GroupQuoteBrokerDealer     , superior::GroupQuoteBrokerDealer});
    Add({DB_GroupQuoteRubricMandatory  , superior::GroupQuoteRubricMandatory});
    Add({DB_GroupQuoteRubricVoluntary  , superior::GroupQuoteRubricVoluntary});
    Add({DB_GroupQuoteRubricFusion     , superior::GroupQuoteRubricFusion});
    Add({DB_GroupQuoteFooterMandatory  , superior::GroupQuoteFooterMandatory});
    Add({DB_GroupQuoteFooterVoluntary  , superior::GroupQuoteFooterVoluntary});
    Add({DB_GroupQuoteFooterFusion     , superior::GroupQuoteFooterFusion});
    Add({DB_MinimumPremiumFootnote     , superior::MinimumPremiumFootnote});
    Add({DB_PremAllocationFootnote     , superior::PremAllocationFootnote});
    Add({DB_InterestDisclaimer         , superior::InterestDisclaimer});
    Add({DB_GuarMortalityFootnote      , superior::GuarMortalityFootnote});
    Add({DB_ProductDescription         , superior::ProductDescription});
    Add({DB_StableValueFootnote        , superior::StableValueFootnote});
    Add({DB_NoVanishPremiumFootnote    , superior::NoVanishPremiumFootnote});
    Add({DB_RejectPremiumFootnote      , superior::RejectPremiumFootnote});
    Add({DB_ExpRatingFootnote          , superior::ExpRatingFootnote});
    Add({DB_MortalityBlendFootnote     , superior::MortalityBlendFootnote});
    Add({DB_HypotheticalRatesFootnote  , superior::HypotheticalRatesFootnote});
    Add({DB_SalesLoadRefundFootnote    , superior::SalesLoadRefundFootnote});
    Add({DB_NoLapseEverFootnote        , superior::NoLapseEverFootnote});
    Add({DB_NoLapseFootnote            , superior::NoLapseFootnote});
    Add({DB_CurrentValuesFootnote      , superior::CurrentValuesFootnote});
    Add({DB_DBOption1Footnote          , superior::DBOption1Footnote});
    Add({DB_DBOption2Footnote          , superior::DBOption2Footnote});
    Add({DB_DBOption3Footnote          , superior::DBOption3Footnote});
    Add({DB_MinDeathBenefitFootnote    , superior::MinDeathBenefitFootnote});
    Add({DB_ExpRatRiskChargeFootnote   , superior::ExpRatRiskChargeFootnote});
    Add({DB_ExchangeChargeFootnote1    , superior::ExchangeChargeFootnote1});
    Add({DB_FlexiblePremiumFootnote    , superior::FlexiblePremiumFootnote});
    Add({DB_GuaranteedValuesFootnote   , superior::GuaranteedValuesFootnote});
    Add({DB_CreditingRateFootnote      , superior::CreditingRateFootnote});
    Add({DB_GrossRateFootnote          , superior::GrossRateFootnote});
    Add({DB_NetRateFootnote            , superior::NetRateFootnote});
    Add({DB_MecFootnote                , superior::MecFootnote});
    Add({DB_GptFootnote                , superior::GptFootnote});
    Add({DB_MidpointValuesFootnote     , superior::MidpointValuesFootnote});
    Add({DB_SinglePremiumFootnote      , superior::SinglePremiumFootnote});
    Add({DB_MonthlyChargesFootnote     , superior::MonthlyChargesFootnote});
    Add({DB_UltCreditingRateFootnote   , superior::UltCreditingRateFootnote});
    Add({DB_UltCreditingRateHeader     , superior::UltCreditingRateHeader});
    Add({DB_MaxNaarFootnote            , superior::MaxNaarFootnote});
    Add({DB_PremTaxSurrChgFootnote     , superior::PremTaxSurrChgFootnote});
    Add({DB_PolicyFeeFootnote          , superior::PolicyFeeFootnote});
    Add({DB_AssetChargeFootnote        , superior::AssetChargeFootnote});
    Add({DB_InvestmentIncomeFootnote   , superior::InvestmentIncomeFootnote});
    Add({DB_IrrDbFootnote              , superior::IrrDbFootnote});
    Add({DB_IrrCsvFootnote             , superior::IrrCsvFootnote});
    Add({DB_MortalityChargesFootnote   , superior::MortalityChargesFootnote});
    Add({DB_LoanAndWithdrawalFootnote  , superior::LoanAndWithdrawalFootnote});
    Add({DB_LoanFootnote               , superior::LoanFootnote});
    Add({DB_ImprimaturPresale          , superior::ImprimaturPresale});
    Add({DB_ImprimaturPresaleComposite , superior::ImprimaturPresaleComposite});
    Add({DB_ImprimaturInforce          , superior::ImprimaturInforce});
    Add({DB_ImprimaturInforceComposite , superior::ImprimaturInforceComposite});
    Add({DB_StateMarketingImprimatur   , superior::StateMarketingImprimatur});
    Add({DB_NonGuaranteedFootnote      , superior::NonGuaranteedFootnote});
    Add({DB_NonGuaranteedFootnote1     , superior::NonGuaranteedFootnote1});
    Add({DB_NonGuaranteedFootnote1Tx   , superior::NonGuaranteedFootnote1Tx});
    Add({DB_FnMonthlyDeductions        , superior::FnMonthlyDeductions});
    Add({DB_SurrenderFootnote          , superior::SurrenderFootnote});
    Add({DB_PortabilityFootnote        , superior::PortabilityFootnote});
    Add({DB_FundRateFootnote           , superior::FundRateFootnote});
    Add({DB_IssuingCompanyFootnote     , superior::IssuingCompanyFootnote});
    Add({DB_SubsidiaryFootnote         , superior::SubsidiaryFootnote});
    Add({DB_PlacementAgentFootnote     , superior::PlacementAgentFootnote});
    Add({DB_MarketingNameFootnote      , superior::MarketingNameFootnote});
    Add({DB_GuarIssueDisclaimerNcSc    , superior::GuarIssueDisclaimerNcSc});
    Add({DB_GuarIssueDisclaimerMd      , superior::GuarIssueDisclaimerMd});
    Add({DB_GuarIssueDisclaimerTx      , superior::GuarIssueDisclaimerTx});
    Add({DB_IllRegCertAgent            , superior::IllRegCertAgent});
    Add({DB_IllRegCertAgentIl          , superior::IllRegCertAgentIl});
    Add({DB_IllRegCertAgentTx          , superior::IllRegCertAgentTx});
    Add({DB_IllRegCertClient           , superior::IllRegCertClient});
    Add({DB_IllRegCertClientIl         , superior::IllRegCertClientIl});
    Add({DB_IllRegCertClientTx         , superior::IllRegCertClientTx});
    Add({DB_FnMaturityAge              , superior::FnMaturityAge});
    Add({DB_FnPartialMortality         , superior::FnPartialMortality});
    Add({DB_FnProspectus               , superior::FnProspectus});
    Add({DB_FnInitialSpecAmt           , superior::FnInitialSpecAmt});
    Add({DB_FnInforceAcctVal           , superior::FnInforceAcctVal});
    Add({DB_FnInforceTaxBasis          , superior::FnInforceTaxBasis});
    Add({DB_Fn1035Charge               , superior::Fn1035Charge});
    Add({DB_FnMecExtraWarning          , superior::FnMecExtraWarning});
    Add({DB_FnNotTaxAdvice             , superior::FnNotTaxAdvice});
    Add({DB_FnNotTaxAdvice2            , superior::FnNotTaxAdvice2});
    Add({DB_FnImf                      , superior::FnImf});
    Add({DB_FnCensus                   , superior::FnCensus});
    Add({DB_FnDacTax                   , superior::FnDacTax});
    Add({DB_FnDefnLifeIns              , superior::FnDefnLifeIns});
    Add({DB_FnBoyEoy                   , superior::FnBoyEoy});
    Add({DB_FnGeneralAccount           , superior::FnGeneralAccount});
    Add({DB_FnPpMemorandum             , superior::FnPpMemorandum});
    Add({DB_FnPpAccreditedInvestor     , superior::FnPpAccreditedInvestor});
    Add({DB_FnPpLoads                  , superior::FnPpLoads});
    Add({DB_FnProposalUnderwriting     , superior::FnProposalUnderwriting});
    Add({DB_FnGuaranteedPremium        , superior::FnGuaranteedPremium});
    Add({DB_FnOmnibusDisclaimer        , superior::FnOmnibusDisclaimer});
    Add({DB_FnInitialDbo               , superior::FnInitialDbo});
    Add({DB_DefnGuarGenAcctRate        , superior::DefnGuarGenAcctRate});
    Add({DB_DefnAV                     , superior::DefnAV});
    Add({DB_DefnCSV                    , superior::DefnCSV});
    Add({DB_DefnMec                    , superior::DefnMec});
    Add({DB_DefnOutlay                 , superior::DefnOutlay});
    Add({DB_DefnSpecAmt                , superior::DefnSpecAmt});
}

sample2::sample2()
{
    // LINGO !! Eventually, perhaps these all can be initialized
    // to a corresponding "_term" value using some terse syntax.
    Add({DB_PolicyForm                 , superior::policy_form_term});
    Add({DB_PolicyMktgName             , superior::PolicyMktgName_term});
    Add({DB_PolicyLegalName            , superior::PolicyLegalName_term});
    Add({DB_InsCoShortName             , superior::InsCoShortName_term});
    Add({DB_InsCoName                  , superior::InsCoName_term});
    Add({DB_InsCoAddr                  , superior::InsCoAddr_term});
    Add({DB_InsCoStreet                , superior::InsCoStreet_term});
    Add({DB_InsCoPhone                 , superior::InsCoPhone_term});
    Add({DB_MainUnderwriter            , superior::MainUnderwriter_term});
    Add({DB_MainUnderwriterAddress     , superior::MainUnderwriterAddress_term});
    Add({DB_CoUnderwriter              , superior::CoUnderwriter_term});
    Add({DB_CoUnderwriterAddress       , superior::CoUnderwriterAddress_term});
    Add({DB_AvName                     , superior::AvName_term});
    Add({DB_CsvName                    , superior::CsvName_term});
    Add({DB_CsvHeaderName              , superior::CsvHeaderName_term});
    Add({DB_NoLapseProvisionName       , superior::NoLapseProvisionName_term});
    Add({DB_ContractName               , superior::ContractName_term});
    Add({DB_DboName                    , superior::DboName_term});
    Add({DB_DboNameLevel               , superior::DboNameLevel_term});
    Add({DB_DboNameIncreasing          , superior::DboNameIncreasing_term});
    Add({DB_DboNameReturnOfPremium     , superior::DboNameReturnOfPremium_term});
    Add({DB_DboNameMinDeathBenefit     , superior::DboNameMinDeathBenefit_term});
    Add({DB_GenAcctName                , superior::GenAcctName_term});
    Add({DB_GenAcctNameElaborated      , superior::GenAcctNameElaborated_term});
    Add({DB_SepAcctName                , superior::SepAcctName_term});
    Add({DB_SpecAmtName                , superior::SpecAmtName_term});
    Add({DB_SpecAmtNameElaborated      , superior::SpecAmtNameElaborated_term});
    Add({DB_UwBasisMedical             , superior::UwBasisMedical_term});
    Add({DB_UwBasisParamedical         , superior::UwBasisParamedical_term});
    Add({DB_UwBasisNonmedical          , superior::UwBasisNonmedical_term});
    Add({DB_UwBasisSimplified          , superior::UwBasisSimplified_term});
    Add({DB_UwBasisGuaranteed          , superior::UwBasisGuaranteed_term});
    Add({DB_UwClassPreferred           , superior::UwClassPreferred_term});
    Add({DB_UwClassStandard            , superior::UwClassStandard_term});
    Add({DB_UwClassRated               , superior::UwClassRated_term});
    Add({DB_UwClassUltra               , superior::UwClassUltra_term});
    Add({DB_AccountValueFootnote       , superior::AccountValueFootnote_term});
    Add({DB_AttainedAgeFootnote        , superior::AttainedAgeFootnote_term});
    Add({DB_CashSurrValueFootnote      , superior::CashSurrValueFootnote_term});
    Add({DB_DeathBenefitFootnote       , superior::DeathBenefitFootnote_term});
    Add({DB_InitialPremiumFootnote     , superior::InitialPremiumFootnote_term});
    Add({DB_NetPremiumFootnote         , superior::NetPremiumFootnote_term});
    Add({DB_GrossPremiumFootnote       , superior::GrossPremiumFootnote_term});
    Add({DB_OutlayFootnote             , superior::OutlayFootnote_term});
    Add({DB_PolicyYearFootnote         , superior::PolicyYearFootnote_term});
    Add({DB_ADDTerseName               , superior::ADDTerseName_term});
    Add({DB_InsurabilityTerseName      , superior::InsurabilityTerseName_term});
    Add({DB_ChildTerseName             , superior::ChildTerseName_term});
    Add({DB_SpouseTerseName            , superior::SpouseTerseName_term});
    Add({DB_TermTerseName              , superior::TermTerseName_term});
    Add({DB_WaiverTerseName            , superior::WaiverTerseName_term});
    Add({DB_AccelBftRiderTerseName     , superior::AccelBftRiderTerseName_term});
    Add({DB_OverloanRiderTerseName     , superior::OverloanRiderTerseName_term});
    Add({DB_ADDFootnote                , superior::ADDFootnote_term});
    Add({DB_ChildFootnote              , superior::ChildFootnote_term});
    Add({DB_SpouseFootnote             , superior::SpouseFootnote_term});
    Add({DB_TermFootnote               , superior::TermFootnote_term});
    Add({DB_WaiverFootnote             , superior::WaiverFootnote_term});
    Add({DB_AccelBftRiderFootnote      , superior::AccelBftRiderFootnote_term});
    Add({DB_OverloanRiderFootnote      , superior::OverloanRiderFootnote_term});
    Add({DB_GroupQuoteShortProductName , superior::GroupQuoteShortProductName_term});
    Add({DB_GroupQuoteIsNotAnOffer     , superior::GroupQuoteIsNotAnOffer_term});
    Add({DB_GroupQuoteRidersFooter     , superior::GroupQuoteRidersFooter_term});
    Add({DB_GroupQuotePolicyFormId     , superior::GroupQuotePolicyFormId_term});
    Add({DB_GroupQuoteStateVariations  , superior::GroupQuoteStateVariations_term});
    Add({DB_GroupQuoteProspectus       , superior::GroupQuoteProspectus_term});
    Add({DB_GroupQuoteUnderwriter      , superior::GroupQuoteUnderwriter_term});
    Add({DB_GroupQuoteBrokerDealer     , superior::GroupQuoteBrokerDealer_term});
    Add({DB_GroupQuoteRubricMandatory  , superior::GroupQuoteRubricMandatory_term});
    Add({DB_GroupQuoteRubricVoluntary  , superior::GroupQuoteRubricVoluntary_term});
    Add({DB_GroupQuoteRubricFusion     , superior::GroupQuoteRubricFusion_term});
    Add({DB_GroupQuoteFooterMandatory  , superior::GroupQuoteFooterMandatory_term});
    Add({DB_GroupQuoteFooterVoluntary  , superior::GroupQuoteFooterVoluntary_term});
    Add({DB_GroupQuoteFooterFusion     , superior::GroupQuoteFooterFusion_term});
    Add({DB_MinimumPremiumFootnote     , superior::MinimumPremiumFootnote_term});
    Add({DB_PremAllocationFootnote     , superior::PremAllocationFootnote_term});
    Add({DB_InterestDisclaimer         , superior::InterestDisclaimer_term});
    Add({DB_GuarMortalityFootnote      , superior::GuarMortalityFootnote_term});
    Add({DB_ProductDescription         , superior::ProductDescription_term});
    Add({DB_StableValueFootnote        , superior::StableValueFootnote_term});
    Add({DB_NoVanishPremiumFootnote    , superior::NoVanishPremiumFootnote_term});
    Add({DB_RejectPremiumFootnote      , superior::RejectPremiumFootnote_term});
    Add({DB_ExpRatingFootnote          , superior::ExpRatingFootnote_term});
    Add({DB_MortalityBlendFootnote     , superior::MortalityBlendFootnote_term});
    Add({DB_HypotheticalRatesFootnote  , superior::HypotheticalRatesFootnote_term});
    Add({DB_SalesLoadRefundFootnote    , superior::SalesLoadRefundFootnote_term});
    Add({DB_NoLapseEverFootnote        , superior::NoLapseEverFootnote_term});
    Add({DB_NoLapseFootnote            , superior::NoLapseFootnote_term});
    Add({DB_CurrentValuesFootnote      , superior::CurrentValuesFootnote_term});
    Add({DB_DBOption1Footnote          , superior::DBOption1Footnote_term});
    Add({DB_DBOption2Footnote          , superior::DBOption2Footnote_term});
    Add({DB_DBOption3Footnote          , superior::DBOption3Footnote_term});
    Add({DB_MinDeathBenefitFootnote    , superior::MinDeathBenefitFootnote_term});
    Add({DB_ExpRatRiskChargeFootnote   , superior::ExpRatRiskChargeFootnote_term});
    Add({DB_ExchangeChargeFootnote1    , superior::ExchangeChargeFootnote1_term});
    Add({DB_FlexiblePremiumFootnote    , superior::FlexiblePremiumFootnote_term});
    Add({DB_GuaranteedValuesFootnote   , superior::GuaranteedValuesFootnote_term});
    Add({DB_CreditingRateFootnote      , superior::CreditingRateFootnote_term});
    Add({DB_GrossRateFootnote          , superior::GrossRateFootnote_term});
    Add({DB_NetRateFootnote            , superior::NetRateFootnote_term});
    Add({DB_MecFootnote                , superior::MecFootnote_term});
    Add({DB_GptFootnote                , superior::GptFootnote_term});
    Add({DB_MidpointValuesFootnote     , superior::MidpointValuesFootnote_term});
    Add({DB_SinglePremiumFootnote      , superior::SinglePremiumFootnote_term});
    Add({DB_MonthlyChargesFootnote     , superior::MonthlyChargesFootnote_term});
    Add({DB_UltCreditingRateFootnote   , superior::UltCreditingRateFootnote_term});
    Add({DB_UltCreditingRateHeader     , superior::UltCreditingRateHeader_term});
    Add({DB_MaxNaarFootnote            , superior::MaxNaarFootnote_term});
    Add({DB_PremTaxSurrChgFootnote     , superior::PremTaxSurrChgFootnote_term});
    Add({DB_PolicyFeeFootnote          , superior::PolicyFeeFootnote_term});
    Add({DB_AssetChargeFootnote        , superior::AssetChargeFootnote_term});
    Add({DB_InvestmentIncomeFootnote   , superior::InvestmentIncomeFootnote_term});
    Add({DB_IrrDbFootnote              , superior::IrrDbFootnote_term});
    Add({DB_IrrCsvFootnote             , superior::IrrCsvFootnote_term});
    Add({DB_MortalityChargesFootnote   , superior::MortalityChargesFootnote_term});
    Add({DB_LoanAndWithdrawalFootnote  , superior::LoanAndWithdrawalFootnote_term});
    Add({DB_LoanFootnote               , superior::LoanFootnote_term});
    Add({DB_ImprimaturPresale          , superior::ImprimaturPresale_term});
    Add({DB_ImprimaturPresaleComposite , superior::ImprimaturPresaleComposite_term});
    Add({DB_ImprimaturInforce          , superior::ImprimaturInforce_term});
    Add({DB_ImprimaturInforceComposite , superior::ImprimaturInforceComposite_term});
    Add({DB_StateMarketingImprimatur   , superior::StateMarketingImprimatur_term});
    Add({DB_NonGuaranteedFootnote      , superior::NonGuaranteedFootnote_term});
    Add({DB_NonGuaranteedFootnote1     , superior::NonGuaranteedFootnote1_term});
    Add({DB_NonGuaranteedFootnote1Tx   , superior::NonGuaranteedFootnote1Tx_term});
    Add({DB_FnMonthlyDeductions        , superior::FnMonthlyDeductions_term});
    Add({DB_SurrenderFootnote          , superior::SurrenderFootnote_term});
    Add({DB_PortabilityFootnote        , superior::PortabilityFootnote_term});
    Add({DB_FundRateFootnote           , superior::FundRateFootnote_term});
    Add({DB_IssuingCompanyFootnote     , superior::IssuingCompanyFootnote_term});
    Add({DB_SubsidiaryFootnote         , superior::SubsidiaryFootnote_term});
    Add({DB_PlacementAgentFootnote     , superior::PlacementAgentFootnote_term});
    Add({DB_MarketingNameFootnote      , superior::MarketingNameFootnote_term});
    Add({DB_GuarIssueDisclaimerNcSc    , superior::GuarIssueDisclaimerNcSc_term});
    Add({DB_GuarIssueDisclaimerMd      , superior::GuarIssueDisclaimerMd_term});
    Add({DB_GuarIssueDisclaimerTx      , superior::GuarIssueDisclaimerTx_term});
    Add({DB_IllRegCertAgent            , superior::IllRegCertAgent_term});
    Add({DB_IllRegCertAgentIl          , superior::IllRegCertAgentIl_term});
    Add({DB_IllRegCertAgentTx          , superior::IllRegCertAgentTx_term});
    Add({DB_IllRegCertClient           , superior::IllRegCertClient_term});
    Add({DB_IllRegCertClientIl         , superior::IllRegCertClientIl_term});
    Add({DB_IllRegCertClientTx         , superior::IllRegCertClientTx_term});
    Add({DB_FnMaturityAge              , superior::FnMaturityAge_term});
    Add({DB_FnPartialMortality         , superior::FnPartialMortality_term});
    Add({DB_FnProspectus               , superior::FnProspectus_term});
    Add({DB_FnInitialSpecAmt           , superior::FnInitialSpecAmt_term});
    Add({DB_FnInforceAcctVal           , superior::FnInforceAcctVal_term});
    Add({DB_FnInforceTaxBasis          , superior::FnInforceTaxBasis_term});
    Add({DB_Fn1035Charge               , superior::Fn1035Charge_term});
    Add({DB_FnMecExtraWarning          , superior::FnMecExtraWarning_term});
    Add({DB_FnNotTaxAdvice             , superior::FnNotTaxAdvice_term});
    Add({DB_FnNotTaxAdvice2            , superior::FnNotTaxAdvice2_term});
    Add({DB_FnImf                      , superior::FnImf_term});
    Add({DB_FnCensus                   , superior::FnCensus_term});
    Add({DB_FnDacTax                   , superior::FnDacTax_term});
    Add({DB_FnDefnLifeIns              , superior::FnDefnLifeIns_term});
    Add({DB_FnBoyEoy                   , superior::FnBoyEoy_term});
    Add({DB_FnGeneralAccount           , superior::FnGeneralAccount_term});
    Add({DB_FnPpMemorandum             , superior::FnPpMemorandum_term});
    Add({DB_FnPpAccreditedInvestor     , superior::FnPpAccreditedInvestor_term});
    Add({DB_FnPpLoads                  , superior::FnPpLoads_term});
    Add({DB_FnProposalUnderwriting     , superior::FnProposalUnderwriting_term});
    Add({DB_FnGuaranteedPremium        , superior::FnGuaranteedPremium_term});
    Add({DB_FnOmnibusDisclaimer        , superior::FnOmnibusDisclaimer_term});
    Add({DB_FnInitialDbo               , superior::FnInitialDbo_term});
    Add({DB_DefnGuarGenAcctRate        , superior::DefnGuarGenAcctRate_term});
    Add({DB_DefnAV                     , superior::DefnAV_term});
    Add({DB_DefnCSV                    , superior::DefnCSV_term});
    Add({DB_DefnMec                    , superior::DefnMec_term});
    Add({DB_DefnOutlay                 , superior::DefnOutlay_term});
    Add({DB_DefnSpecAmt                , superior::DefnSpecAmt_term});
}

sample2naic::sample2naic()
{
    Add({DB_LedgerType          , mce_ill_reg});
}

sample2finra::sample2finra()
{
    Add({DB_LedgerType          , mce_finra});
}

sample2prosp::sample2prosp()
{
    Add({DB_LedgerType          , mce_prospectus_abeyed});
}

sample2gpp::sample2gpp()
{
    Add({DB_LedgerType          , mce_group_private_placement});
}

sample2ipp::sample2ipp()
{
    Add({DB_LedgerType               , mce_individual_private_placement});
    Add({DB_IrrDbFootnote            , superior::Poe0});
    Add({DB_IrrCsvFootnote           , superior::Poe1});
    Add({DB_MortalityChargesFootnote , superior::Poe2});
    Add({DB_PolicyYearFootnote       , superior::Poe3});
}

sample2xyz::sample2xyz()
{
    // Exotica.
#if 0
    // US 1980 CSO age last, not gender distinct. Unisex = table D.
    // This deviation from the 'sample' family should necessitate
    // different 7pp and corridor tables. Enable this deliberate
    // inconsistency as an optional test of the product verifier.
    int dims113[e_number_of_axes] = {1, 1, 3, 1, 1, 1, 1}; // smoking
    double T7702q[3] = {111, 109, 107,}; // Smoker, nonsmoker, unismoke.
    Add({DB_Irc7702QTable, e_number_of_axes, dims113, T7702q});
    Add({DB_Irc7702QAxisGender  , false});
    Add({DB_Irc7702QAxisSmoking , true});
#endif // 0
    // This is determined by law, in a way that varies by product;
    // 2021 products use this two-percent rate.
    Add({DB_AnnInterestRate7702 , 0.02});
    // Arguably the most complex ledger type.
    Add({DB_LedgerType          , mce_finra});
    // Certain group-quote columns are available only when these two
    // entities are 'true':
    Add({DB_SplitMinPrem        , true});
    Add({DB_TermIsNotRider      , true});
    // Certain illustration columns are controlled by this:
    Add({DB_ErNotionallyPaysTerm, true});
    Add({DB_TxCallsGuarUwSubstd , true});
    // This fixed loan rate varies by duration.
    int dims_1111113[e_number_of_axes] = {1, 1, 1, 1, 1, 1, 3};
    double loanrate[3] = {0.06, 0.05, 0.04};
    Add({DB_FixedLoanRate, e_number_of_axes, dims_1111113, loanrate});
    Add({DB_AllowCashValueEnh   , true});
    double cv_enh[3] = {0.10, 0.05, 0.00};
    Add({DB_CashValueEnhMult, e_number_of_axes, dims_1111113, cv_enh});
    Add({DB_FnMaturityAge              , superior::FnMaturityAge});
    Add({DB_FnPartialMortality         , superior::FnPartialMortality});
}
} // Unnamed namespace.

void DBDictionary::write_database_files()
{
    sample      ().WriteDB(AddDataDir("sample.database"));
    sample2naic ().WriteDB(AddDataDir("sample2naic.database"));
    sample2finra().WriteDB(AddDataDir("sample2finra.database"));
    sample2prosp().WriteDB(AddDataDir("sample2prosp.database"));
    sample2gpp  ().WriteDB(AddDataDir("sample2gpp.database"));
    sample2ipp  ().WriteDB(AddDataDir("sample2ipp.database"));
    sample2xyz  ().WriteDB(AddDataDir("sample2xyz.database"));
}

/// Initialize the built-in database for the antediluvian branch.

void DBDictionary::InitAntediluvian()
{
    // Zero is inappropriate for some entities ("DB_CurrCoiMultiplier",
    // e.g.), but the antediluvian branch doesn't actually use most
    // database entities.
    for(auto const& i : member_names())
        {
        Add({db_key_from_name(i), 0.0});
        }

    Add({DB_SmokeOrTobacco      , oe_smoker_nonsmoker});
    Add({DB_AllowPreferredClass , true});
    Add({DB_AllowFlatExtras     , true});
    Add({DB_CorridorTable       , 7});

    int guar_coi_dims[e_number_of_axes] = {1, 1, 3, 1, 1, 1, 1};
    // smoker, nonsmoker, unismoke
    double guar_coi_tables[3] = {111, 109, 107};
    Add({DB_GuarCoiTable, e_number_of_axes, guar_coi_dims, guar_coi_tables});

    int curr_coi_dims[e_number_of_axes] = {1, 4, 3, 1, 1, 1, 1};
    // preferred, standard, rated, ultrapreferred by smoker, nonsmoker, unismoke
    double curr_coi_tables[] =
        {
        2, 3, 1, // pref  sm ns us
        5, 6, 4, // std   sm ns us
        5, 6, 4, // rated sm ns us
        0, 0, 0, // ultra sm ns us
        };
    Add({DB_CurrCoiTable, e_number_of_axes, curr_coi_dims, curr_coi_tables});

    // These are the same as class date_trammel's nominal limits.
    // They mustn't be zero.
    Add({DB_CoiResetMinDate     , calendar_date::gregorian_epoch_jdn});
    Add({DB_CoiResetMaxDate     , calendar_date::last_yyyy_date_jdn });

    Add({DB_GuarInt             , 0.03});
    Add({DB_AllowGenAcct        , true});
    Add({DB_MaxGenAcctRate      , 0.12});
    Add({DB_MaxSepAcctRate      , 0.12});
    Add({DB_GuarMonthlyPolFee   , 12.00});
    Add({DB_GuarPremLoadTgt     , 0.025});
    Add({DB_GuarPremLoadExc     , 0.025});
    Add({DB_GuarSpecAmtLoad     , 0.0});
    Add({DB_CurrMonthlyPolFee   , 5.00});
    Add({DB_CurrPremLoadTgt     , 0.025});
    Add({DB_CurrPremLoadExc     , 0.025});
    Add({DB_CurrSpecAmtLoad     , 0.0});
    Add({DB_DacTaxFundCharge    , 0.0});
    Add({DB_Has1035ExchCharge   , 0.0});
    Add({DB_MinSpecAmt          , 10000.0});
    Add({DB_AllowDboRop         , true});
    Add({DB_AllowChangeToDbo2   , true});
    Add({DB_AllowWp             , false});
    Add({DB_WpTable             , 8});
    Add({DB_AllowAdb            , false});
    Add({DB_AdbTable            , 9});
    Add({DB_AllowSpouseRider    , false});
    Add({DB_AllowChildRider     , false});
    Add({DB_AllowWd             , true});
    Add({DB_WdFee               , 5.0});
    Add({DB_WdFeeRate           , 0.01});
    Add({DB_MinWd               , 100.0});
    Add({DB_AllowLoan           , true});
    Add({DB_FixedLoanRate       , 0.06});
    Add({DB_GuarPrefLoanSpread  , 0.0});
    Add({DB_GuarRegLoanSpread   , 0.0});
    Add({DB_CurrPrefLoanSpread  , 0.0});
    Add({DB_CurrRegLoanSpread   , 0.0});
    Add({DB_NoLapseMinDur       , 0.0});
    Add({DB_NoLapseMinAge       , 0.0});
    Add({DB_NoLapseAlwaysActive , 0.0});
    Add({DB_LedgerType          , mce_ill_reg});
    Add({DB_AgeLastOrNearest    , oe_age_nearest_birthday_ties_older});
    Add({DB_MaturityAge         , 100});
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
        try
            {
            DBDictionary const z(i->string());

            fs::path out_file = fs::change_extension(*i, ".dbt");
            fs::ofstream os(out_file, ios_out_trunc_binary());
            for(auto const& j : z.member_names())
                {
                z.datum(j).write(os);
                }
            }
        catch(...)
            {
            report_exception();
            continue;
            }
        }
}
