// Product-database map.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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

#include "dbdict.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "calendar_date.hpp"            // gregorian_epoch(), last_yyyy_date()
#include "data_directory.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "handle_exceptions.hpp"
#include "ieee754.hpp"                  // infinity<>()
#include "mc_enum_type_enums.hpp"
#include "miscellany.hpp"
#include "my_proem.hpp"                 // ::write_proem()
#include "oecumenic_enumerations.hpp"
#include "premium_tax.hpp"              // premium_tax_rates_for_life_insurance()
#include "xml_lmi.hpp"
#include "xml_serialize.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <limits>
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

DBDictionary::DBDictionary(std::string const& filename)
{
    ascribe_members();
    InitDB();
    Init(filename);
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
    ascribe("MinIssAge"           , &DBDictionary::MinIssAge           );
    ascribe("MaxIssAge"           , &DBDictionary::MaxIssAge           );
    ascribe("MaxIncrAge"          , &DBDictionary::MaxIncrAge          );
    ascribe("AllowFullUw"         , &DBDictionary::AllowFullUw         );
    ascribe("AllowParamedUw"      , &DBDictionary::AllowParamedUw      );
    ascribe("AllowNonmedUw"       , &DBDictionary::AllowNonmedUw       );
    ascribe("AllowSimpUw"         , &DBDictionary::AllowSimpUw         );
    ascribe("AllowGuarUw"         , &DBDictionary::AllowGuarUw         );
    ascribe("SmokeOrTobacco"      , &DBDictionary::SmokeOrTobacco      );
    ascribe("AllowPreferredClass" , &DBDictionary::AllowPreferredClass );
    ascribe("AllowUltraPrefClass" , &DBDictionary::AllowUltraPrefClass );
    ascribe("AllowSubstdTable"    , &DBDictionary::AllowSubstdTable    );
    ascribe("AllowFlatExtras"     , &DBDictionary::AllowFlatExtras     );
    ascribe("AllowRatedWp"        , &DBDictionary::AllowRatedWp        );
    ascribe("AllowRatedAdb"       , &DBDictionary::AllowRatedAdb       );
    ascribe("AllowRatedTerm"      , &DBDictionary::AllowRatedTerm      );
    ascribe("AllowRetirees"       , &DBDictionary::AllowRetirees       );
    ascribe("AllowUnisex"         , &DBDictionary::AllowUnisex         );
    ascribe("AllowSexDistinct"    , &DBDictionary::AllowSexDistinct    );
    ascribe("AllowUnismoke"       , &DBDictionary::AllowUnismoke       );
    ascribe("AllowSmokeDistinct"  , &DBDictionary::AllowSmokeDistinct  );
    ascribe("StateApproved"       , &DBDictionary::StateApproved       );
    ascribe("AllowStateXX"        , &DBDictionary::AllowStateXX        );
    ascribe("AllowForeign"        , &DBDictionary::AllowForeign        );
    ascribe("GroupIndivSelection" , &DBDictionary::GroupIndivSelection );
    ascribe("Allowable"           , &DBDictionary::Allowable           );
    ascribe("AllowCvat"           , &DBDictionary::AllowCvat           );
    ascribe("AllowGpt"            , &DBDictionary::AllowGpt            );
    ascribe("AllowNo7702"         , &DBDictionary::AllowNo7702         );
    ascribe("CorridorWhence"      , &DBDictionary::CorridorWhence      );
    ascribe("Irc7702NspWhence"    , &DBDictionary::Irc7702NspWhence    );
    ascribe("SevenPayWhence"      , &DBDictionary::SevenPayWhence      );
    ascribe("CorridorTable"       , &DBDictionary::CorridorTable       );
    ascribe("Irc7702NspTable"     , &DBDictionary::Irc7702NspTable     );
    ascribe("SevenPayTable"       , &DBDictionary::SevenPayTable       );
    ascribe("Irc7702QTable"       , &DBDictionary::Irc7702QTable       );
    ascribe("RatingsAffect7702"   , &DBDictionary::RatingsAffect7702   );
    ascribe("CvatMatChangeDefn"   , &DBDictionary::CvatMatChangeDefn   );
    ascribe("GptMatChangeDefn"    , &DBDictionary::GptMatChangeDefn    );
    ascribe("Irc7702BftIsSpecAmt" , &DBDictionary::Irc7702BftIsSpecAmt );
    ascribe("RiskyInitial7702Db"  , &DBDictionary::RiskyInitial7702Db  );
    ascribe("Irc7702Endowment"    , &DBDictionary::Irc7702Endowment    );
    ascribe("Effective7702DboRop" , &DBDictionary::Effective7702DboRop );
    ascribe("TermIsQABOrDb7702"   , &DBDictionary::TermIsQABOrDb7702   );
    ascribe("TermIsQABOrDb7702A"  , &DBDictionary::TermIsQABOrDb7702A  );
    ascribe("GioIsQAB"            , &DBDictionary::GioIsQAB            );
    ascribe("AdbIsQAB"            , &DBDictionary::AdbIsQAB            );
    ascribe("SpouseRiderIsQAB"    , &DBDictionary::SpouseRiderIsQAB    );
    ascribe("ChildRiderIsQAB"     , &DBDictionary::ChildRiderIsQAB     );
    ascribe("WpIsQAB"             , &DBDictionary::WpIsQAB             );
    ascribe("GuarCoiTable"        , &DBDictionary::GuarCoiTable        );
    ascribe("GuarCoiIsAnnual"     , &DBDictionary::GuarCoiIsAnnual     );
    ascribe("GuarCoiMultiplier"   , &DBDictionary::GuarCoiMultiplier   );
    ascribe("CurrCoiTable"        , &DBDictionary::CurrCoiTable        );
    ascribe("CurrCoiIsAnnual"     , &DBDictionary::CurrCoiIsAnnual     );
    ascribe("MinInputCoiMult"     , &DBDictionary::MinInputCoiMult     );
    ascribe("CurrCoiMultiplier"   , &DBDictionary::CurrCoiMultiplier   );
    ascribe("UnusualCoiBanding"   , &DBDictionary::UnusualCoiBanding   );
    ascribe("CurrCoiTable0Limit"  , &DBDictionary::CurrCoiTable0Limit  );
    ascribe("CurrCoiTable1"       , &DBDictionary::CurrCoiTable1       );
    ascribe("CurrCoiTable1Limit"  , &DBDictionary::CurrCoiTable1Limit  );
    ascribe("CurrCoiTable2"       , &DBDictionary::CurrCoiTable2       );
    ascribe("MdptCoiTable"        , &DBDictionary::MdptCoiTable        );
    ascribe("MdptCoiIsAnnual"     , &DBDictionary::MdptCoiIsAnnual     );
    ascribe("CoiNyMinTable"       , &DBDictionary::CoiNyMinTable       );
    ascribe("UseNyCoiFloor"       , &DBDictionary::UseNyCoiFloor       );
    ascribe("MaxMonthlyCoiRate"   , &DBDictionary::MaxMonthlyCoiRate   );
    ascribe("GuarCoiCeiling"      , &DBDictionary::GuarCoiCeiling      );
    ascribe("CoiGuarIsMin"        , &DBDictionary::CoiGuarIsMin        );
    ascribe("SubstdTableMult"     , &DBDictionary::SubstdTableMult     );
    ascribe("SubstdTableMultTable", &DBDictionary::SubstdTableMultTable);
    ascribe("CoiUpper12Method"    , &DBDictionary::CoiUpper12Method    );
    ascribe("CoiInforceReentry"   , &DBDictionary::CoiInforceReentry   );
    ascribe("CoiResetMinDate"     , &DBDictionary::CoiResetMinDate     );
    ascribe("CoiResetMaxDate"     , &DBDictionary::CoiResetMaxDate     );
    ascribe("AllowMortBlendSex"   , &DBDictionary::AllowMortBlendSex   );
    ascribe("AllowMortBlendSmoke" , &DBDictionary::AllowMortBlendSmoke );
    ascribe("GuarInt"             , &DBDictionary::GuarInt             );
    ascribe("NaarDiscount"        , &DBDictionary::NaarDiscount        );
    ascribe("GuarIntSpread"       , &DBDictionary::GuarIntSpread       );
    ascribe("GuarMandE"           , &DBDictionary::GuarMandE           );
    ascribe("CurrIntSpread"       , &DBDictionary::CurrIntSpread       );
    ascribe("CurrMandE"           , &DBDictionary::CurrMandE           );
    ascribe("GenAcctIntBonus"     , &DBDictionary::GenAcctIntBonus     );
    ascribe("BonusInt"            , &DBDictionary::BonusInt            );
    ascribe("IntFloor"            , &DBDictionary::IntFloor            );
    ascribe("AllowGenAcct"        , &DBDictionary::AllowGenAcct        );
    ascribe("AllowSepAcct"        , &DBDictionary::AllowSepAcct        );
    ascribe("AllowGenAcctEarnRate", &DBDictionary::AllowGenAcctEarnRate);
    ascribe("AllowSepAcctNetRate" , &DBDictionary::AllowSepAcctNetRate );
    ascribe("MaxGenAcctRate"      , &DBDictionary::MaxGenAcctRate      );
    ascribe("MaxSepAcctRate"      , &DBDictionary::MaxSepAcctRate      );
    ascribe("SepAcctSpreadMethod" , &DBDictionary::SepAcctSpreadMethod );
    ascribe("IntSpreadMode"       , &DBDictionary::IntSpreadMode       );
    ascribe("DynamicMandE"        , &DBDictionary::DynamicMandE        );
    ascribe("AllowAmortPremLoad"  , &DBDictionary::AllowAmortPremLoad  );
    ascribe("LoadAmortFundCharge" , &DBDictionary::LoadAmortFundCharge );
    ascribe("AllowImfOverride"    , &DBDictionary::AllowImfOverride    );
    ascribe("AssetChargeType"     , &DBDictionary::AssetChargeType     );
    ascribe("StableValFundCharge" , &DBDictionary::StableValFundCharge );
    ascribe("GuarFundAdminChg"    , &DBDictionary::GuarFundAdminChg    );
    ascribe("CurrFundAdminChg"    , &DBDictionary::CurrFundAdminChg    );
    ascribe("FundCharge"          , &DBDictionary::FundCharge          );
    ascribe("GuarMonthlyPolFee"   , &DBDictionary::GuarMonthlyPolFee   );
    ascribe("GuarAnnualPolFee"    , &DBDictionary::GuarAnnualPolFee    );
    ascribe("GuarPremLoadTgt"     , &DBDictionary::GuarPremLoadTgt     );
    ascribe("GuarPremLoadExc"     , &DBDictionary::GuarPremLoadExc     );
    ascribe("GuarPremLoadTgtRfd"  , &DBDictionary::GuarPremLoadTgtRfd  );
    ascribe("GuarPremLoadExcRfd"  , &DBDictionary::GuarPremLoadExcRfd  );
    ascribe("GuarSpecAmtLoad"     , &DBDictionary::GuarSpecAmtLoad     );
    ascribe("GuarSpecAmtLoadTable", &DBDictionary::GuarSpecAmtLoadTable);
    ascribe("GuarAcctValLoad"     , &DBDictionary::GuarAcctValLoad     );
    ascribe("CurrMonthlyPolFee"   , &DBDictionary::CurrMonthlyPolFee   );
    ascribe("CurrAnnualPolFee"    , &DBDictionary::CurrAnnualPolFee    );
    ascribe("CurrPremLoadTgt"     , &DBDictionary::CurrPremLoadTgt     );
    ascribe("CurrPremLoadExc"     , &DBDictionary::CurrPremLoadExc     );
    ascribe("CurrPremLoadTgtRfd"  , &DBDictionary::CurrPremLoadTgtRfd  );
    ascribe("CurrPremLoadExcRfd"  , &DBDictionary::CurrPremLoadExcRfd  );
    ascribe("CurrSpecAmtLoad"     , &DBDictionary::CurrSpecAmtLoad     );
    ascribe("CurrSpecAmtLoadTable", &DBDictionary::CurrSpecAmtLoadTable);
    ascribe("CurrAcctValLoad"     , &DBDictionary::CurrAcctValLoad     );
    ascribe("TgtPremMonthlyPolFee", &DBDictionary::TgtPremMonthlyPolFee);
    ascribe("LoadRfdProportion"   , &DBDictionary::LoadRfdProportion   );
    ascribe("SpecAmtLoadLimit"    , &DBDictionary::SpecAmtLoadLimit    );
    ascribe("DynamicSepAcctLoad"  , &DBDictionary::DynamicSepAcctLoad  );
    ascribe("DynSepAcctLoadLimit" , &DBDictionary::DynSepAcctLoadLimit );
    ascribe("DacTaxFundCharge"    , &DBDictionary::DacTaxFundCharge    );
    ascribe("DacTaxPremLoad"      , &DBDictionary::DacTaxPremLoad      );
    ascribe("PremTaxFundCharge"   , &DBDictionary::PremTaxFundCharge   );
    ascribe("PremTaxLoad"         , &DBDictionary::PremTaxLoad         );
    ascribe("WaivePremTaxInt1035" , &DBDictionary::WaivePremTaxInt1035 );
    ascribe("PremTaxRetalLimit"   , &DBDictionary::PremTaxRetalLimit   );
    ascribe("PremTaxTierGroup"    , &DBDictionary::PremTaxTierGroup    );
    ascribe("PremTaxTierPeriod"   , &DBDictionary::PremTaxTierPeriod   );
    ascribe("PremTaxTierNonDecr"  , &DBDictionary::PremTaxTierNonDecr  );
    ascribe("PremTaxAmortPeriod"  , &DBDictionary::PremTaxAmortPeriod  );
    ascribe("PremTaxAmortIntRate" , &DBDictionary::PremTaxAmortIntRate );
    ascribe("PremTaxRate"         , &DBDictionary::PremTaxRate         );
    ascribe("PremTaxState"        , &DBDictionary::PremTaxState        );
    ascribe("SurrChgOnIncr"       , &DBDictionary::SurrChgOnIncr       );
    ascribe("SurrChgOnDecr"       , &DBDictionary::SurrChgOnDecr       );
    ascribe("Has1035ExchCharge"   , &DBDictionary::Has1035ExchCharge   );
    ascribe("SnflQTable"          , &DBDictionary::SnflQTable          );
    ascribe("CoiSnflIsGuar"       , &DBDictionary::CoiSnflIsGuar       );
    ascribe("SurrChgByFormula"    , &DBDictionary::SurrChgByFormula    );
    ascribe("SurrChgPeriod"       , &DBDictionary::SurrChgPeriod       );
    ascribe("SurrChgZeroDur"      , &DBDictionary::SurrChgZeroDur      );
    ascribe("SurrChgNlpMult"      , &DBDictionary::SurrChgNlpMult      );
    ascribe("SurrChgNlpMax"       , &DBDictionary::SurrChgNlpMax       );
    ascribe("SurrChgEaMax"        , &DBDictionary::SurrChgEaMax        );
    ascribe("SurrChgAmort"        , &DBDictionary::SurrChgAmort        );
    ascribe("AllowSpecAmtIncr"    , &DBDictionary::AllowSpecAmtIncr    );
    ascribe("MinSpecAmtIncr"      , &DBDictionary::MinSpecAmtIncr      );
    ascribe("EnforceNaarLimit"    , &DBDictionary::EnforceNaarLimit    );
    ascribe("MinSpecAmt"          , &DBDictionary::MinSpecAmt          );
    ascribe("MinIssSpecAmt"       , &DBDictionary::MinIssSpecAmt       );
    ascribe("MinIssBaseSpecAmt"   , &DBDictionary::MinIssBaseSpecAmt   );
    ascribe("MinRenlSpecAmt"      , &DBDictionary::MinRenlSpecAmt      );
    ascribe("MinRenlBaseSpecAmt"  , &DBDictionary::MinRenlBaseSpecAmt  );
    ascribe("MaxIssSpecAmt"       , &DBDictionary::MaxIssSpecAmt       );
    ascribe("MaxRenlSpecAmt"      , &DBDictionary::MaxRenlSpecAmt      );
    ascribe("AllowDboLvl"         , &DBDictionary::AllowDboLvl         );
    ascribe("AllowDboInc"         , &DBDictionary::AllowDboInc         );
    ascribe("AllowDboRop"         , &DBDictionary::AllowDboRop         );
    ascribe("AllowDboMdb"         , &DBDictionary::AllowDboMdb         );
    ascribe("DboLvlChangeToWhat"  , &DBDictionary::DboLvlChangeToWhat  );
    ascribe("DboLvlChangeMethod"  , &DBDictionary::DboLvlChangeMethod  );
    ascribe("DboIncChangeToWhat"  , &DBDictionary::DboIncChangeToWhat  );
    ascribe("DboIncChangeMethod"  , &DBDictionary::DboIncChangeMethod  );
    ascribe("DboRopChangeToWhat"  , &DBDictionary::DboRopChangeToWhat  );
    ascribe("DboRopChangeMethod"  , &DBDictionary::DboRopChangeMethod  );
    ascribe("DboMdbChangeToWhat"  , &DBDictionary::DboMdbChangeToWhat  );
    ascribe("DboMdbChangeMethod"  , &DBDictionary::DboMdbChangeMethod  );
    ascribe("AllowChangeToDbo2"   , &DBDictionary::AllowChangeToDbo2   );
    ascribe("DboChgCanIncrSpecAmt", &DBDictionary::DboChgCanIncrSpecAmt);
    ascribe("DboChgCanDecrSpecAmt", &DBDictionary::DboChgCanDecrSpecAmt);
    ascribe("AllowExtEndt"        , &DBDictionary::AllowExtEndt        );
    ascribe("AllowTerm"           , &DBDictionary::AllowTerm           );
    ascribe("GuarTermTable"       , &DBDictionary::GuarTermTable       );
    ascribe("TermTable"           , &DBDictionary::TermTable           );
    ascribe("TermMinIssAge"       , &DBDictionary::TermMinIssAge       );
    ascribe("TermMaxIssAge"       , &DBDictionary::TermMaxIssAge       );
    ascribe("TermIsNotRider"      , &DBDictionary::TermIsNotRider      );
    ascribe("TermCanLapse"        , &DBDictionary::TermCanLapse        );
    ascribe("TermForcedConvAge"   , &DBDictionary::TermForcedConvAge   );
    ascribe("TermForcedConvDur"   , &DBDictionary::TermForcedConvDur   );
    ascribe("MaxTermProportion"   , &DBDictionary::MaxTermProportion   );
    ascribe("TermCoiRate"         , &DBDictionary::TermCoiRate         );
    ascribe("TermPremRate"        , &DBDictionary::TermPremRate        );
    ascribe("AllowWp"             , &DBDictionary::AllowWp             );
    ascribe("WpTable"             , &DBDictionary::WpTable             );
    ascribe("WpMinIssAge"         , &DBDictionary::WpMinIssAge         );
    ascribe("WpMaxIssAge"         , &DBDictionary::WpMaxIssAge         );
    ascribe("WpLimit"             , &DBDictionary::WpLimit             );
    ascribe("WpCoiRate"           , &DBDictionary::WpCoiRate           );
    ascribe("WpPremRate"          , &DBDictionary::WpPremRate          );
    ascribe("WpChargeMethod"      , &DBDictionary::WpChargeMethod      );
    ascribe("AllowAdb"            , &DBDictionary::AllowAdb            );
    ascribe("AdbTable"            , &DBDictionary::AdbTable            );
    ascribe("AdbMinIssAge"        , &DBDictionary::AdbMinIssAge        );
    ascribe("AdbMaxIssAge"        , &DBDictionary::AdbMaxIssAge        );
    ascribe("AdbLimit"            , &DBDictionary::AdbLimit            );
    ascribe("AdbCoiRate"          , &DBDictionary::AdbCoiRate          );
    ascribe("AdbPremRate"         , &DBDictionary::AdbPremRate         );
    ascribe("AllowSpouseRider"    , &DBDictionary::AllowSpouseRider    );
    ascribe("SpouseRiderMinAmt"   , &DBDictionary::SpouseRiderMinAmt   );
    ascribe("SpouseRiderMaxAmt"   , &DBDictionary::SpouseRiderMaxAmt   );
    ascribe("SpouseRiderMinIssAge", &DBDictionary::SpouseRiderMinIssAge);
    ascribe("SpouseRiderMaxIssAge", &DBDictionary::SpouseRiderMaxIssAge);
    ascribe("SpouseRiderGuarTable", &DBDictionary::SpouseRiderGuarTable);
    ascribe("SpouseRiderTable"    , &DBDictionary::SpouseRiderTable    );
    ascribe("AllowChildRider"     , &DBDictionary::AllowChildRider     );
    ascribe("ChildRiderMinAmt"    , &DBDictionary::ChildRiderMinAmt    );
    ascribe("ChildRiderMaxAmt"    , &DBDictionary::ChildRiderMaxAmt    );
    ascribe("ChildRiderTable"     , &DBDictionary::ChildRiderTable     );
    ascribe("AllowWd"             , &DBDictionary::AllowWd             );
    ascribe("WdFee"               , &DBDictionary::WdFee               );
    ascribe("WdFeeRate"           , &DBDictionary::WdFeeRate           );
    ascribe("FreeWdProportion"    , &DBDictionary::FreeWdProportion    );
    ascribe("MinWd"               , &DBDictionary::MinWd               );
    ascribe("MaxWdGenAcctValMult" , &DBDictionary::MaxWdGenAcctValMult );
    ascribe("MaxWdSepAcctValMult" , &DBDictionary::MaxWdSepAcctValMult );
    ascribe("MaxWdDed"            , &DBDictionary::MaxWdDed            );
    ascribe("WdDecrSpecAmtDboLvl" , &DBDictionary::WdDecrSpecAmtDboLvl );
    ascribe("WdDecrSpecAmtDboInc" , &DBDictionary::WdDecrSpecAmtDboInc );
    ascribe("WdDecrSpecAmtDboRop" , &DBDictionary::WdDecrSpecAmtDboRop );
    ascribe("FirstWdMonth"        , &DBDictionary::FirstWdMonth        );
    ascribe("AllowLoan"           , &DBDictionary::AllowLoan           );
    ascribe("AllowPrefLoan"       , &DBDictionary::AllowPrefLoan       );
    ascribe("AllowFixedLoan"      , &DBDictionary::AllowFixedLoan      );
    ascribe("AllowVlr"            , &DBDictionary::AllowVlr            );
    ascribe("FixedLoanRate"       , &DBDictionary::FixedLoanRate       );
    ascribe("MaxVlrRate"          , &DBDictionary::MaxVlrRate          );
    ascribe("MinVlrRate"          , &DBDictionary::MinVlrRate          );
    ascribe("MaxLoanAcctValMult"  , &DBDictionary::MaxLoanAcctValMult  );
    ascribe("MaxLoanDed"          , &DBDictionary::MaxLoanDed          );
    ascribe("FirstPrefLoanYear"   , &DBDictionary::FirstPrefLoanYear   );
    ascribe("PrefLoanRateDecr"    , &DBDictionary::PrefLoanRateDecr    );
    ascribe("GuarPrefLoanSpread"  , &DBDictionary::GuarPrefLoanSpread  );
    ascribe("GuarRegLoanSpread"   , &DBDictionary::GuarRegLoanSpread   );
    ascribe("CurrPrefLoanSpread"  , &DBDictionary::CurrPrefLoanSpread  );
    ascribe("CurrRegLoanSpread"   , &DBDictionary::CurrRegLoanSpread   );
    ascribe("FirstLoanMonth"      , &DBDictionary::FirstLoanMonth      );
    ascribe("MinPremType"         , &DBDictionary::MinPremType         );
    ascribe("MinPremIntSpread"    , &DBDictionary::MinPremIntSpread    );
    ascribe("SplitMinPrem"        , &DBDictionary::SplitMinPrem        );
    ascribe("UnsplitSplitMinPrem" , &DBDictionary::UnsplitSplitMinPrem );
    ascribe("TgtPremType"         , &DBDictionary::TgtPremType         );
    ascribe("TgtPremTable"        , &DBDictionary::TgtPremTable        );
    ascribe("TgtPremFixedAtIssue" , &DBDictionary::TgtPremFixedAtIssue );
    ascribe("TgtPremIgnoreSubstd" , &DBDictionary::TgtPremIgnoreSubstd );
    ascribe("MinPmt"              , &DBDictionary::MinPmt              );
    ascribe("NoLapseMinDur"       , &DBDictionary::NoLapseMinDur       );
    ascribe("NoLapseMinAge"       , &DBDictionary::NoLapseMinAge       );
    ascribe("NoLapseUnratedOnly"  , &DBDictionary::NoLapseUnratedOnly  );
    ascribe("NoLapseDboLvlOnly"   , &DBDictionary::NoLapseDboLvlOnly   );
    ascribe("NoLapseAlwaysActive" , &DBDictionary::NoLapseAlwaysActive );
    ascribe("AllowHoneymoon"      , &DBDictionary::AllowHoneymoon      );
    ascribe("DeductionMethod"     , &DBDictionary::DeductionMethod     );
    ascribe("DeductionAcct"       , &DBDictionary::DeductionAcct       );
    ascribe("DistributionMethod"  , &DBDictionary::DistributionMethod  );
    ascribe("DistributionAcct"    , &DBDictionary::DistributionAcct    );
    ascribe("EePremMethod"        , &DBDictionary::EePremMethod        );
    ascribe("EePremAcct"          , &DBDictionary::EePremAcct          );
    ascribe("ErPremMethod"        , &DBDictionary::ErPremMethod        );
    ascribe("ErPremAcct"          , &DBDictionary::ErPremAcct          );
    ascribe("CalculateComp"       , &DBDictionary::CalculateComp       );
    ascribe("CompTarget"          , &DBDictionary::CompTarget          );
    ascribe("CompExcess"          , &DBDictionary::CompExcess          );
    ascribe("CompChargeback"      , &DBDictionary::CompChargeback      );
    ascribe("AssetComp"           , &DBDictionary::AssetComp           );
    ascribe("AllowExtraAssetComp" , &DBDictionary::AllowExtraAssetComp );
    ascribe("AllowExtraPremComp"  , &DBDictionary::AllowExtraPremComp  );
    ascribe("AllowExpRating"      , &DBDictionary::AllowExpRating      );
    ascribe("ExpRatStdDevMult"    , &DBDictionary::ExpRatStdDevMult    );
    ascribe("ExpRatIbnrMult"      , &DBDictionary::ExpRatIbnrMult      );
    ascribe("ExpRatCoiRetention"  , &DBDictionary::ExpRatCoiRetention  );
    ascribe("ExpRatRiskCoiMult"   , &DBDictionary::ExpRatRiskCoiMult   );
    ascribe("ExpRatAmortPeriod"   , &DBDictionary::ExpRatAmortPeriod   );
    ascribe("LedgerType"          , &DBDictionary::LedgerType          );
    ascribe("Nonillustrated"      , &DBDictionary::Nonillustrated      );
    ascribe("NoLongerIssued"      , &DBDictionary::NoLongerIssued      );
    ascribe("AgeLastOrNearest"    , &DBDictionary::AgeLastOrNearest    );
    ascribe("MaturityAge"         , &DBDictionary::MaturityAge         );
    ascribe("CashValueEnhMult"    , &DBDictionary::CashValueEnhMult    );
    ascribe("LapseIgnoresSurrChg" , &DBDictionary::LapseIgnoresSurrChg );
    ascribe("DefaultProcessOrder" , &DBDictionary::DefaultProcessOrder );
    ascribe("GroupProxyRateTable" , &DBDictionary::GroupProxyRateTable );
    ascribe("PartialMortTable"    , &DBDictionary::PartialMortTable    );
    ascribe("UsePolicyFormAlt"    , &DBDictionary::UsePolicyFormAlt    );
    ascribe("WeightClass"         , &DBDictionary::WeightClass         );
    ascribe("WeightGender"        , &DBDictionary::WeightGender        );
    ascribe("WeightSmoking"       , &DBDictionary::WeightSmoking       );
    ascribe("WeightAge"           , &DBDictionary::WeightAge           );
    ascribe("WeightSpecAmt"       , &DBDictionary::WeightSpecAmt       );
    ascribe("WeightState"         , &DBDictionary::WeightState         );
    ascribe("FullExpPol"          , &DBDictionary::FullExpPol          );
    ascribe("FullExpPrem"         , &DBDictionary::FullExpPrem         );
    ascribe("FullExpDumpin"       , &DBDictionary::FullExpDumpin       );
    ascribe("FullExpSpecAmt"      , &DBDictionary::FullExpSpecAmt      );
    ascribe("VarExpPol"           , &DBDictionary::VarExpPol           );
    ascribe("VarExpPrem"          , &DBDictionary::VarExpPrem          );
    ascribe("VarExpDumpin"        , &DBDictionary::VarExpDumpin        );
    ascribe("VarExpSpecAmt"       , &DBDictionary::VarExpSpecAmt       );
    ascribe("ExpSpecAmtLimit"     , &DBDictionary::ExpSpecAmtLimit     );
    ascribe("MedicalProportion"   , &DBDictionary::MedicalProportion   );
    ascribe("UwTestCost"          , &DBDictionary::UwTestCost          );
    ascribe("VxBasicQTable"       , &DBDictionary::VxBasicQTable       );
    ascribe("VxDeficQTable"       , &DBDictionary::VxDeficQTable       );
    ascribe("VxTaxQTable"         , &DBDictionary::VxTaxQTable         );
    ascribe("StatVxInt"           , &DBDictionary::StatVxInt           );
    ascribe("TaxVxInt"            , &DBDictionary::TaxVxInt            );
    ascribe("StatVxQ"             , &DBDictionary::StatVxQ             );
    ascribe("TaxVxQ"              , &DBDictionary::TaxVxQ              );
    ascribe("DeficVxQ"            , &DBDictionary::DeficVxQ            );
    ascribe("SnflQ"               , &DBDictionary::SnflQ               );
    ascribe("LapseRate"           , &DBDictionary::LapseRate           );
    ascribe("ReqSurpNaar"         , &DBDictionary::ReqSurpNaar         );
    ascribe("ReqSurpVx"           , &DBDictionary::ReqSurpVx           );
    ascribe("LicFitRate"          , &DBDictionary::LicFitRate          );
    ascribe("LicDacTaxRate"       , &DBDictionary::LicDacTaxRate       );
    ascribe("GdbVxMethod"         , &DBDictionary::GdbVxMethod         );
    ascribe("PrimaryHurdle"       , &DBDictionary::PrimaryHurdle       );
    ascribe("SecondaryHurdle"     , &DBDictionary::SecondaryHurdle     );
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
    static double const bignum = std::numeric_limits<double>::max();

    for(auto const& i : member_names())
        {
        Add({db_key_from_name(i), 0.0});
        }

    // It would be dangerous to set these to zero.
    Add({DB_MinInputCoiMult       , 1.0});
    Add({DB_CurrCoiMultiplier     , 1.0});
    Add({DB_GuarCoiMultiplier     , 1.0});
    Add({DB_SubstdTableMult       , 1.0});
    Add({DB_MaxWdGenAcctValMult   , 1.0});
    Add({DB_MaxWdSepAcctValMult   , 1.0});

    // This is determined by law and regulation, and should be the
    // same for all life-insurance products.
    int ptd[e_number_of_axes] = {1, 1, 1, 1, 1, e_max_dim_state, 1};
    std::vector<int> premium_tax_dimensions(ptd, ptd + e_number_of_axes);
    Add({DB_PremTaxRate, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});

    // These are the same as class date_trammel's nominal limits.
    Add({DB_CoiResetMinDate       , calendar_date::gregorian_epoch_jdn});
    Add({DB_CoiResetMaxDate       , calendar_date::last_yyyy_date_jdn });

    // Usually the maximum is a reciprocal, e.g., 1/11 or 1/12; for
    // greatest precision, store the reciprocal of that reciprocal,
    // e.g., 11 or 12.
    Add({DB_MaxMonthlyCoiRate     , 12.0});

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

    Add({DB_FirstPrefLoanYear     , 100});

    Add({DB_GuarIntSpread         , dbl_inf});

    Add({DB_CurrCoiTable0Limit    , dbl_inf});
    Add({DB_CurrCoiTable1         , 999});
    Add({DB_CurrCoiTable1Limit    , dbl_inf});
    Add({DB_CurrCoiTable2         , 999});

    Add({DB_SpecAmtLoadLimit      , dbl_inf});
    Add({DB_DynSepAcctLoadLimit   , dbl_inf});
    Add({DB_PremTaxRetalLimit     , dbl_inf});
    Add({DB_WpLimit               , dbl_inf});
    Add({DB_AdbLimit              , dbl_inf});
    Add({DB_ExpSpecAmtLimit       , dbl_inf});

    // These are the same as the nominal limits of the associated
    // members of class Input.
    Add({DB_SpouseRiderMaxAmt     , bignum});
    Add({DB_SpouseRiderMaxIssAge  , 99});
    Add({DB_ChildRiderMaxAmt      , bignum});
}

void DBDictionary::write_database_files()
{
    DBDictionary z;

    z.InitDB();
    z.Add({DB_GuarMonthlyPolFee   , 8.00});
    z.Add({DB_GuarSpecAmtLoad     , 0.0});
    z.Add({DB_GuarAnnualPolFee    , 0.0});
    z.Add({DB_GuarFundAdminChg    , 0.0});
    z.Add({DB_GuarPremLoadTgt     , 0.07});
    z.Add({DB_GuarPremLoadExc     , 0.04});
    z.Add({DB_GuarPremLoadTgtRfd  , 0.00});
    z.Add({DB_GuarPremLoadExcRfd  , 0.00});
    z.Add({DB_GuarAcctValLoad     , 0.0});
    z.Add({DB_CurrMonthlyPolFee   , 5.00});
    z.Add({DB_CurrSpecAmtLoad     , 0.0});
    z.Add({DB_CurrAnnualPolFee    , 0.0});
    z.Add({DB_CurrFundAdminChg    , 0.0});
    z.Add({DB_CurrPremLoadTgt     , 0.05});
    z.Add({DB_CurrPremLoadExc     , 0.02});
    z.Add({DB_CurrPremLoadTgtRfd  , 0.00});
    z.Add({DB_CurrPremLoadExcRfd  , 0.00});
    z.Add({DB_CurrAcctValLoad     , 0.0});
    z.Add({DB_DacTaxPremLoad      , 0.01});
    z.Add({DB_FundCharge          , 0.0});
    z.Add({DB_PremTaxFundCharge   , 0.0});
    z.Add({DB_DacTaxFundCharge    , 0.0});
    z.Add({DB_WaivePremTaxInt1035 , true});
    z.Add({DB_FirstWdMonth        , 0.0});
    z.Add({DB_MaxWdDed            , mce_to_next_anniversary});
    z.Add({DB_MinWd               , 100.0});
    z.Add({DB_WdFee               , 25.0});
    z.Add({DB_WdFeeRate           , 0.02});
    z.Add({DB_WdDecrSpecAmtDboLvl , true});
    z.Add({DB_WdDecrSpecAmtDboInc , true});
    z.Add({DB_WdDecrSpecAmtDboRop , true});
    z.Add({DB_FirstLoanMonth      , 0.0});
    z.Add({DB_AllowPrefLoan       , false});
    z.Add({DB_AllowFixedLoan      , true});
    z.Add({DB_FixedLoanRate       , 0.06});
    z.Add({DB_AllowVlr            , true});
    z.Add({DB_MaxLoanAcctValMult  , 1.0});
    z.Add({DB_MaxLoanDed          , mce_to_next_anniversary});
    z.Add({DB_GuarPrefLoanSpread  , 0.0});
    z.Add({DB_GuarRegLoanSpread   , 0.04});
    z.Add({DB_CurrPrefLoanSpread  , 0.0});
    z.Add({DB_CurrRegLoanSpread   , 0.02});
    z.Add({DB_GuarInt             , 0.03});
    z.Add({DB_NaarDiscount        , 0.00246627});
    z.Add({DB_GuarIntSpread       , 0.03});
    z.Add({DB_GuarMandE           , 0.009});
    z.Add({DB_CurrIntSpread       , 0.01});
    z.Add({DB_CurrMandE           , 0.009});
    z.Add({DB_BonusInt            , 0.0});
    z.Add({DB_IntFloor            , 0.0});
    z.Add({DB_SepAcctSpreadMethod , mce_spread_is_effective_annual});
    z.Add({DB_DynamicMandE        , false});

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
    z.Add({DB_CurrCoiTable, e_number_of_axes, dims313, TgCOI});
    z.Add({DB_GuarCoiTable, e_number_of_axes, dims313, TgCOI});

    z.Add({DB_CoiNyMinTable       , 0.0});

    double coimult[9] =
        {
        0.40, 0.30, 0.35, // female: sm ns us
        0.60, 0.50, 0.55, // male:   sm ns us
        0.50, 0.40, 0.45, // unisex: sm ns us
        };
    z.Add({DB_CurrCoiMultiplier, e_number_of_axes, dims313, coimult});

    z.Add({DB_UseNyCoiFloor       , 0.0});
    z.Add({DB_GuarCoiCeiling      , 0.0});
    z.Add({DB_CoiGuarIsMin        , 0.0});
    z.Add({DB_CoiSnflIsGuar       , 0.0});
    z.Add({DB_CurrCoiIsAnnual     , true});
    z.Add({DB_GuarCoiIsAnnual     , true});
    z.Add({DB_MdptCoiIsAnnual     , true});
    z.Add({DB_AgeLastOrNearest    , oe_age_last_birthday});
    z.Add({DB_AllowRetirees       , true});
    z.Add({DB_MinSpecAmt          , 100000.0});
    // Forbid substandard table ratings with simplified or guaranteed issue.
    int dim_uw_basis[e_number_of_axes] = {1, 1, 1, 1, 5, 1, 1};
    //                              med  para nonmed   SI     GI
    double allow_substd_table[] = {true, true, true, false, false};
    z.Add({DB_AllowSubstdTable, e_number_of_axes, dim_uw_basis, allow_substd_table});
    z.Add({DB_AllowFlatExtras     , true});
    z.Add({DB_MinIssAge           , 15});
    z.Add({DB_MaxIssAge           , 70});
    z.Add({DB_MinIssSpecAmt       , 50000.0});
    z.Add({DB_MinIssBaseSpecAmt   , 50000.0});
    z.Add({DB_MaxIssSpecAmt       , 10000000.0});
    z.Add({DB_MinRenlSpecAmt      , 50000.0});
    z.Add({DB_MinRenlBaseSpecAmt  , 50000.0});
    z.Add({DB_MaxRenlSpecAmt      , 10000000.0});
    z.Add({DB_MinSpecAmtIncr      , 0.0});
    z.Add({DB_MaxIncrAge          , 99});
    z.Add({DB_MinPmt              , 0.0});
    z.Add({DB_SmokeOrTobacco      , oe_tobacco_nontobacco});
    z.Add({DB_AllowUnisex         , true});
    z.Add({DB_AllowSexDistinct    , true});
    z.Add({DB_AllowUnismoke       , true});
    z.Add({DB_AllowSmokeDistinct  , true});
    z.Add({DB_AllowFullUw         , true});
    z.Add({DB_AllowParamedUw      , true});
    z.Add({DB_AllowNonmedUw       , true});
    z.Add({DB_AllowSimpUw         , true});
    z.Add({DB_AllowGuarUw         , true});
    z.Add({DB_AllowMortBlendSex   , true});
    z.Add({DB_AllowMortBlendSmoke , true});
    z.Add({DB_AllowRatedWp        , false});
    z.Add({DB_AllowRatedAdb       , false});
    z.Add({DB_AllowRatedTerm      , true});
    z.Add({DB_Allowable           , true});
    z.Add({DB_AllowPreferredClass , true});
    z.Add({DB_AllowCvat           , true});
    z.Add({DB_AllowGpt            , true});

    z.Add({DB_CorridorWhence      , 1});
    z.Add({DB_Irc7702NspWhence    , 2});
    z.Add({DB_SevenPayWhence      , 1});
    // This is just a sample product, so make do with plausible
    // all-male seven-pay premiums, and use GPT corridor factors for
    // CVAT. 'Irc7702NspWhence' specifies that NSP is calculated as
    // the reciprocal of corridor, so no NSP table is needed.
    z.Add({DB_CorridorTable       , 7});
    z.Add({DB_Irc7702NspTable     , 0});
    z.Add({DB_SevenPayTable       , 10});

    // Following IRS Notice 88-128, use only the male and female
    // tables with no smoker distinction, and a unisex table where
    // required by state law.
    //
    // US 1980 CSO age last, not smoker distinct. Unisex = table D.
    // Male uses table E, which is correct, as opposed to table F,
    // which contains a numerical error but was adopted by NAIC.
    int dims311[e_number_of_axes] = {3, 1, 1, 1, 1, 1, 1}; // gender
    double T7702q[9] = {35, 41, 107,}; // Female, male, unisex.
    z.Add({DB_Irc7702QTable, e_number_of_axes, dims311, T7702q});

    z.Add({DB_AllowDboLvl         , true});
    z.Add({DB_AllowDboInc         , true});
    z.Add({DB_AllowDboRop         , true});
    z.Add({DB_AllowDboMdb         , true});
    z.Add({DB_DboLvlChangeToWhat  , 0b1111});
    z.Add({DB_DboLvlChangeMethod  , 0b1111});
    z.Add({DB_DboIncChangeToWhat  , 0b1111});
    z.Add({DB_DboIncChangeMethod  , 0b1111});
    z.Add({DB_DboRopChangeToWhat  , 0b1111});
    z.Add({DB_DboRopChangeMethod  , 0b1111});
    z.Add({DB_DboMdbChangeToWhat  , 0b1111});
    z.Add({DB_DboMdbChangeMethod  , 0b1111});
    z.Add({DB_DboChgCanIncrSpecAmt, true});
    z.Add({DB_DboChgCanDecrSpecAmt, true});
    z.Add({DB_SnflQTable          , 0.0});
    z.Add({DB_SurrChgByFormula    , 0.0});
    z.Add({DB_SurrChgPeriod       , 0.0});
    z.Add({DB_SurrChgZeroDur      , 0.0});
    z.Add({DB_SurrChgNlpMult      , 0.0});
    z.Add({DB_SurrChgNlpMax       , 0.0});
    z.Add({DB_SurrChgEaMax        , 0.0});
    z.Add({DB_SurrChgAmort        , 0.0});

    int ptd[e_number_of_axes] = {1, 1, 1, 1, 1, e_max_dim_state, 1};
    std::vector<int> premium_tax_dimensions(ptd, ptd + e_number_of_axes);
    z.Add({DB_PremTaxRate, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});

    // Use alternative policy form name in states beginning with "K".
    std::vector<double> alt_form(e_max_dim_state);
    alt_form[mce_s_KS] = true;
    alt_form[mce_s_KY] = true;
    z.Add({DB_UsePolicyFormAlt, premium_tax_dimensions, alt_form});

    z.Add({DB_PremTaxState        , oe_ee_state});
    z.Add({DB_MaturityAge         , 100});
    z.Add({DB_AllowExtEndt        , true});
    z.Add({DB_MinPremType         , oe_monthly_deduction});
    z.Add({DB_TgtPremType         , oe_modal_nonmec});
    z.Add({DB_TgtPremFixedAtIssue , false});
    z.Add({DB_TgtPremIgnoreSubstd , true});
    z.Add({DB_NoLapseMinDur       , 0.0});
    z.Add({DB_NoLapseMinAge       , 0.0});
    z.Add({DB_NoLapseUnratedOnly  , false});
    z.Add({DB_NoLapseDboLvlOnly   , false});
    z.Add({DB_LoadRfdProportion   , 0.0});

    // uw_class, smoker
    int dims143[e_number_of_axes] = {1, 4, 3, 1, 1, 1, 1};

    double TtCOI[12] =
        {
        3, 2, 1, // pref:  sm ns us
        6, 5, 4, // std:   sm ns us
        6, 5, 4, // rated: sm ns us [same as std]
        0, 0, 0, // ultra: sm ns us [zero: error message--no ultrapref class]
        };

    z.Add({DB_TermTable    , e_number_of_axes, dims143, TtCOI});
    z.Add({DB_GuarTermTable, e_number_of_axes, dims143, TtCOI});
    z.Add({DB_AllowTerm           , true});
    z.Add({DB_TermMinIssAge       , 15});
    z.Add({DB_TermMaxIssAge       , 65});
    z.Add({DB_TermForcedConvAge   , 70});
    z.Add({DB_TermForcedConvDur   , 10});
    z.Add({DB_MaxTermProportion   , 0.0});
    z.Add({DB_TermCoiRate         , 0.0});
    z.Add({DB_TermPremRate        , 0.0});
    z.Add({DB_WpTable             , 8});
    z.Add({DB_AllowWp             , true});
    z.Add({DB_WpMinIssAge         , 18});
    z.Add({DB_WpMaxIssAge         , 64});
    z.Add({DB_WpCoiRate           , 0.0});
    z.Add({DB_WpPremRate          , 0.0});
    // SOA qx_ins table 708 is 70-75 US ADB experience.
    z.Add({DB_AdbTable            , 708});
    z.Add({DB_AllowAdb            , true});
    z.Add({DB_AdbMinIssAge        , 15});
    z.Add({DB_AdbMaxIssAge        , 70});
    z.Add({DB_AdbLimit            , 1000000.0});
    z.Add({DB_AdbCoiRate          , 0.0});
    z.Add({DB_AdbPremRate         , 0.0});
    z.Add({DB_WeightClass         , 0.0});
    z.Add({DB_WeightGender        , 0.0});
    z.Add({DB_WeightSmoking       , 0.0});
    z.Add({DB_WeightAge           , 0.0});
    z.Add({DB_WeightSpecAmt       , 0.0});
    z.Add({DB_WeightState         , 0.0});
    z.Add({DB_FullExpPol          , 0.0});
    z.Add({DB_FullExpPrem         , 0.0});
    z.Add({DB_FullExpDumpin       , 0.0});
    z.Add({DB_FullExpSpecAmt      , 0.0});
    z.Add({DB_VarExpPol           , 0.0});
    z.Add({DB_VarExpPrem          , 0.0});
    z.Add({DB_VarExpDumpin        , 0.0});
    z.Add({DB_VarExpSpecAmt       , 0.0});
    z.Add({DB_MedicalProportion   , 0.0});
    z.Add({DB_UwTestCost          , 0.0});
    z.Add({DB_VxBasicQTable       , 0.0});
    z.Add({DB_VxDeficQTable       , 0.0});
    z.Add({DB_VxTaxQTable         , 0.0});
    z.Add({DB_StatVxInt           , 0.0});
    z.Add({DB_TaxVxInt            , 0.0});
    z.Add({DB_StatVxQ             , 0.0});
    z.Add({DB_TaxVxQ              , 0.0});
    z.Add({DB_DeficVxQ            , 0.0});
    z.Add({DB_SnflQ               , 0.0});
    z.Add({DB_CalculateComp       , false});
    z.Add({DB_CompTarget          , 0.0});
    z.Add({DB_CompExcess          , 0.0});
    z.Add({DB_CompChargeback      , 0.0});
    z.Add({DB_LapseRate           , 0.0});
    z.Add({DB_ReqSurpNaar         , 0.0});
    z.Add({DB_ReqSurpVx           , 0.0});
    z.Add({DB_LicFitRate          , 0.0});
    z.Add({DB_LicDacTaxRate       , 0.0});
    z.Add({DB_GdbVxMethod         , 0.0});
    z.Add({DB_PrimaryHurdle       , 0.0});
    z.Add({DB_SecondaryHurdle     , 0.0});
    z.Add({DB_LedgerType          , mce_ill_reg});
    z.Add({DB_AllowExpRating      , false});

    // 1960 CSG (which does not distinguish gender).
    z.Add({DB_GroupProxyRateTable , 305});

    double T83Gam[3] = {825, 826, 826,};
    z.Add
        ({DB_PartialMortTable
         ,e_number_of_axes
         ,dims311
         ,T83Gam
         ,"1983 GAM, using male rates for unisex because no unisex table was published."
        });

    z.Add({DB_AllowWd             , true});
    z.Add({DB_AllowLoan           , true});
    z.Add({DB_AllowChangeToDbo2   , true});
    z.Add({DB_AllowSpecAmtIncr    , true});
    z.Add({DB_NoLapseAlwaysActive , false});
    z.Add({DB_ExpRatStdDevMult    , 0.0});
    z.Add({DB_ExpRatIbnrMult      , 0.0});
    z.Add({DB_ExpRatCoiRetention  , 0.0});
    z.Add({DB_StableValFundCharge , 0.0});
    z.Add({DB_LoadAmortFundCharge , 0.0030});
    z.Add({DB_AllowAmortPremLoad  , false});
    z.Add({DB_PremTaxAmortPeriod  , 0});
    z.Add({DB_PremTaxAmortIntRate , 0.0});
    // Pass through premium tax.
    z.Add({DB_PremTaxLoad, premium_tax_dimensions, premium_tax_rates_for_life_insurance()});
    z.Add({DB_AllowHoneymoon      , true});
    // Set target equal to seven-pay premium.
    z.Add({DB_TgtPremTable        , 10});
    z.Add({DB_TgtPremMonthlyPolFee, 0.0});
    z.Add({DB_AllowExtraAssetComp , true});
    z.Add({DB_AllowExtraPremComp  , true});
    z.Add({DB_AssetChargeType     , oe_asset_charge_spread});
    z.Add({DB_AllowUltraPrefClass , false});
    z.Add({DB_AllowGenAcct        , true});
    z.Add({DB_AllowSepAcct        , true});
    z.Add({DB_AllowGenAcctEarnRate, true});
    z.Add({DB_AllowSepAcctNetRate , true});
    z.Add({DB_MaxGenAcctRate      , 0.06});
    z.Add({DB_MaxSepAcctRate      , 0.12});
    z.Add({DB_MinVlrRate          , 0.04});
    z.Add({DB_IntSpreadMode       , mce_spread_daily});
    z.Add({DB_StateApproved       , true});
    z.Add({DB_AllowStateXX        , true});
    z.Add({DB_AllowForeign        , true});
    z.Add({DB_GroupIndivSelection , false});
    z.Add({DB_AllowImfOverride    , false});
    z.Add({DB_AllowNo7702         , false});
    z.Add({DB_EnforceNaarLimit    , true});
    z.Add({DB_DynamicSepAcctLoad  , false});
    z.Add({DB_SpecAmtLoadLimit    , 10000000.0});
    z.Add({DB_RatingsAffect7702   , false});
    z.Add({DB_CvatMatChangeDefn   , mce_earlier_of_increase_or_unnecessary_premium});
    z.Add({DB_GptMatChangeDefn    , 0});
    z.Add({DB_Irc7702BftIsSpecAmt , 0});
    z.Add({DB_RiskyInitial7702Db  , false});
    z.Add({DB_Irc7702Endowment    , 0});
    z.Add({DB_Effective7702DboRop , 0});
    z.Add({DB_TermIsQABOrDb7702   , oe_7702_term_is_db});
    z.Add({DB_TermIsQABOrDb7702A  , oe_7702_term_is_db});
    z.Add({DB_GioIsQAB            , false});
    z.Add({DB_AdbIsQAB            , false});
    z.Add({DB_SpouseRiderIsQAB    , false});
    z.Add({DB_ChildRiderIsQAB     , false});
    z.Add({DB_WpIsQAB             , false});
    z.Add({DB_ExpRatRiskCoiMult   , 0});
    z.Add({DB_AllowSpouseRider    , true});
    z.Add({DB_AllowChildRider     , true});

    z.Add({DB_SpouseRiderMinAmt   , 10000});
    z.Add({DB_SpouseRiderMaxAmt   , 1000000});
    z.Add({DB_SpouseRiderMinIssAge, 20});
    z.Add({DB_SpouseRiderMaxIssAge, 65});

    // Min and max deliberately identical for testing.
    z.Add({DB_ChildRiderMinAmt    , 25000});
    z.Add({DB_ChildRiderMaxAmt    , 25000});

    // Spouse and child riders: arbitrarily use 1960 CSG.
    z.Add({DB_SpouseRiderGuarTable, 305});
    z.Add({DB_SpouseRiderTable    , 305});
    z.Add({DB_ChildRiderTable     , 305});

    z.Add({DB_GenAcctIntBonus     , 0.0});

    // Allow experience rating.
    z.Add({DB_AllowExpRating      , true});
    z.Add({DB_ExpRatIbnrMult      , 6.0});
    z.Add({DB_ExpRatAmortPeriod   , 4.0});

    z.WriteDB(AddDataDir("sample.database"));

    z.Add({DB_LedgerType          , mce_finra});
    z.WriteDB(AddDataDir("sample2finra.database"));

    z.Add({DB_LedgerType          , mce_group_private_placement});
    z.WriteDB(AddDataDir("sample2gpp.database"));

    z.Add({DB_LedgerType          , mce_individual_private_placement});
    z.WriteDB(AddDataDir("sample2ipp.database"));

    // Exotica.
    z.Add({DB_LedgerType          , mce_finra});
    // Certain group-quote columns are available only when these two
    // entities are 'true':
    z.Add({DB_SplitMinPrem        , true});
    z.Add({DB_TermIsNotRider      , true});
    // This fixed loan rate varies by duration.
    int dims_1111113[e_number_of_axes] = {1, 1, 1, 1, 1, 1, 3};
    double loanrate[3] = {0.06, 0.05, 0.04};
    z.Add({DB_FixedLoanRate, e_number_of_axes, dims_1111113, loanrate});
    double cv_enh[3] = {0.10, 0.05, 0.00};
    z.Add({DB_CashValueEnhMult, e_number_of_axes, dims_1111113, cv_enh});
    z.WriteDB(AddDataDir("sample2xyz.database"));
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

    // These are the same as class date_trammel's nominal limits.
    // They mustn't be zero.
    Add({DB_CoiResetMinDate       , calendar_date::gregorian_epoch_jdn});
    Add({DB_CoiResetMaxDate       , calendar_date::last_yyyy_date_jdn });

    Add({DB_GuarInt               , 0.03});

    Add({DB_FixedLoanRate         , 0.06});

    Add({DB_GuarRegLoanSpread     , 0.0});
    Add({DB_CurrRegLoanSpread     , 0.0});
    Add({DB_GuarPrefLoanSpread    , 0.0});
    Add({DB_CurrPrefLoanSpread    , 0.0});

    Add({DB_AllowPreferredClass   , true});

    // premium loads

    Add({DB_GuarMonthlyPolFee     , 12.00});
    Add({DB_GuarSpecAmtLoad       , 0.0});
    Add({DB_GuarPremLoadTgt       , 0.025});
    Add({DB_GuarPremLoadExc       , 0.025});
    Add({DB_CurrMonthlyPolFee     , 5.00});
    Add({DB_CurrSpecAmtLoad       , 0.0});
    Add({DB_CurrPremLoadTgt       , 0.025});
    Add({DB_CurrPremLoadExc       , 0.025});

    Add({DB_MinWd                 , 100.0});
    Add({DB_WdFee                 , 5.0});
    Add({DB_WdFeeRate             , 0.01});

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

    Add({DB_CorridorTable         , 7});
    Add({DB_WpTable               , 8});
    Add({DB_AdbTable              , 9});
    Add({DB_MaturityAge           , 100});
    Add({DB_AgeLastOrNearest      , oe_age_nearest_birthday_ties_older});
    Add({DB_MinSpecAmt            , 10000.0});

    Add({DB_AllowGenAcct          , true});
    Add({DB_MaxGenAcctRate        , 0.12});
    Add({DB_MaxSepAcctRate        , 0.12});

    Add({DB_AllowLoan             , true});
    Add({DB_AllowWd               , true});
    Add({DB_AllowFlatExtras       , true});
    Add({DB_AllowChangeToDbo2     , true});
    Add({DB_AllowDboRop           , true});

    Add({DB_LedgerType            , mce_ill_reg});

    Add({DB_NoLapseAlwaysActive   , 0.0});
    Add({DB_NoLapseMinDur         , 0.0});
    Add({DB_NoLapseMinAge         , 0.0});

    Add({DB_Has1035ExchCharge     , 0.0});
    Add({DB_SmokeOrTobacco        , 0.0});
    Add({DB_DacTaxFundCharge      , 0.0});
    Add({DB_AllowWp               , false});
    Add({DB_AllowAdb              , false});
    Add({DB_AllowSpouseRider      , false});
    Add({DB_AllowChildRider       , false});

    Add({DB_ExpRatAmortPeriod     , 4.0});
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
