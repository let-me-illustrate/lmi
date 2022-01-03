// Transient state of guideline premium test.
//
// Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "gpt_state.hpp"
#include "xml_serializable.tpp"

#include "alert.hpp"
#include "contains.hpp"
#include "miscellany.hpp"               // htmlize()
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <limits>
#include <sstream>
#include <vector>

template class xml_serializable<gpt_state>;

/// Initialize all ascribed members to zero.

gpt_state::gpt_state()
{
    AscribeMembers();
    for(auto const& i : member_names())
        {
        operator[](i) = "0";
        }
}

gpt_state::gpt_state(gpt_state const& z)
    :xml_serializable  <gpt_state>()
    ,MemberSymbolTable <gpt_state>()
{
    AscribeMembers();
    MemberSymbolTable<gpt_state>::assign(z);
}

gpt_state& gpt_state::operator=(gpt_state const& z)
{
    MemberSymbolTable<gpt_state>::assign(z);
    return *this;
}

bool gpt_state::operator==(gpt_state const& z) const
{
    return MemberSymbolTable<gpt_state>::equals(z);
}

namespace
{
template<typename T>
std::string f(T t)
{
    return "&nbsp;&nbsp;&nbsp;" + value_cast<std::string>(t);
}

template<>
std::string f<>(double t)
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

std::string gpt_state::format_as_html(std::string const& heading) const
{
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

    oss << "<p>" << htmlize(heading) << "</p>\n";
#if 0
    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "policy year"                     << "</td>\n"
        << "<td nowrap>" << f(B0_deduced_policy_year   ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "contract year"                   << "</td>\n"
        << "<td nowrap>" << f(B1_deduced_contract_year ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "seven-pay rate"                  << "</td>\n"
        << "<td nowrap>" << f(B2_deduced_px7_rate      ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "nsp rate"                        << "</td>\n"
        << "<td nowrap>" << f(B3_deduced_nsp_rate      ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "target premium"                  << "</td>\n"
        << "<td nowrap>" << f(B4_deduced_target_premium) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "target load"                     << "</td>\n"
        << "<td nowrap>" << f(B5_deduced_target_load   ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "excess load"                     << "</td>\n"
        << "<td nowrap>" << f(B6_deduced_excess_load   ) << "</td>\n"
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
        << "<td nowrap>" << f(C0_init_bft       ) << "</td>\n"
        << "<td nowrap>" << f(D0_incr_bft       ) << "</td>\n"
        << "<td nowrap>" << f(E0_decr_bft       ) << "</td>\n"
        << "<td nowrap>" << f(F0_nec_pm_bft     ) << "</td>\n"
        << "<td nowrap>" << f(G0_do_mc_bft      ) << "</td>\n"
        << "<td nowrap>" << f(H0_unnec_pm_bft   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "LDB"                       << "</td>\n"
        << "<td nowrap>" << f(C1_init_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(D1_incr_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(E1_decr_ldb       ) << "</td>\n"
        << "<td nowrap>" << f(F1_nec_pm_ldb     ) << "</td>\n"
        << "<td nowrap>" << f(G1_do_mc_ldb      ) << "</td>\n"
        << "<td nowrap>" << f(H1_unnec_pm_ldb   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "amts pd"                   << "</td>\n"
        << "<td nowrap>" << f(C2_init_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(D2_incr_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(E2_decr_amt_pd    ) << "</td>\n"
        << "<td nowrap>" << f(F2_nec_pm_amt_pd  ) << "</td>\n"
        << "<td nowrap>" << f(G2_do_mc_amt_pd   ) << "</td>\n"
        << "<td nowrap>" << f(H2_unnec_pm_amt_pd) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "MC"                        << "</td>\n"
        << "<td nowrap>" << f(C3_init_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(D3_incr_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(E3_decr_is_mc     ) << "</td>\n"
        << "<td nowrap>" << f(F3_nec_pm_is_mc   ) << "</td>\n"
        << "<td nowrap>" << f(G3_do_mc_is_mc    ) << "</td>\n"
        << "<td nowrap>" << f(H3_unnec_pm_is_mc ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "DCV"                       << "</td>\n"
        << "<td nowrap>" << f(C4_init_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(D4_incr_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(E4_decr_dcv       ) << "</td>\n"
        << "<td nowrap>" << f(F4_nec_pm_dcv     ) << "</td>\n"
        << "<td nowrap>" << f(G4_do_mc_dcv      ) << "</td>\n"
        << "<td nowrap>" << f(H4_unnec_pm_dcv   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "7PP"                       << "</td>\n"
        << "<td nowrap>" << f(C5_init_px7       ) << "</td>\n"
        << "<td nowrap>" << f(D5_incr_px7       ) << "</td>\n"
        << "<td nowrap>" << f(E5_decr_px7       ) << "</td>\n"
        << "<td nowrap>" << f(F5_nec_pm_px7     ) << "</td>\n"
        << "<td nowrap>" << f(G5_do_mc_px7      ) << "</td>\n"
        << "<td nowrap>" << f(H5_unnec_pm_px7   ) << "</td>\n"
        << "</tr>\n"

        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "MEC"                       << "</td>\n"
        << "<td nowrap>" << f(C6_init_mec       ) << "</td>\n"
        << "<td nowrap>" << f(D6_incr_mec       ) << "</td>\n"
        << "<td nowrap>" << f(E6_decr_mec       ) << "</td>\n"
        << "<td nowrap>" << f(F6_nec_pm_mec     ) << "</td>\n"
        << "<td nowrap>" << f(G6_do_mc_mec      ) << "</td>\n"
        << "<td nowrap>" << f(H6_unnec_pm_mec   ) << "</td>\n"
        << "</tr>\n"

        << "</table>\n"
        ;

    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "net 1035 amount"              << "</td>\n"
        << "<td nowrap>" << f(Q0_net_1035          ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "net max necessary premium"    << "</td>\n"
        << "<td nowrap>" << f(Q1_max_nec_prem_net  ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gross max necessary premium"  << "</td>\n"
        << "<td nowrap>" << f(Q2_max_nec_prem_gross) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "CV before last MC"            << "</td>\n"
        << "<td nowrap>" << f(Q3_cv_before_last_mc ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "cumulative seven-pay premium" << "</td>\n"
        << "<td nowrap>" << f(Q4_cum_px7           ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "cumulative amounts paid"      << "</td>\n"
        << "<td nowrap>" << f(Q5_cum_amt_pd        ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "max non-MEC premium"          << "</td>\n"
        << "<td nowrap>" << f(Q6_max_non_mec_prem  ) << "</td>\n"
        << "</tr>\n"
        << "</table>\n"
        ;
#endif // 0
    oss
        << "<hr>\n"
        << "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\">\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "glp"                          << "</td>\n"
        << "<td nowrap>" << f(X0_glp               ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gsp"                          << "</td>\n"
        << "<td nowrap>" << f(X1_gsp               ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "glp A"                        << "</td>\n"
        << "<td nowrap>" << f(X2_glp_a             ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gsp A"                        << "</td>\n"
        << "<td nowrap>" << f(X3_gsp_a             ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "glp B"                        << "</td>\n"
        << "<td nowrap>" << f(X4_glp_b             ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gsp B"                        << "</td>\n"
        << "<td nowrap>" << f(X5_gsp_b             ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "glp C"                        << "</td>\n"
        << "<td nowrap>" << f(X6_glp_c             ) << "</td>\n"
        << "</tr>\n"
        << "<tr align=\"right\">\n"
        << "<td nowrap>" << "gsp C"                        << "</td>\n"
        << "<td nowrap>" << f(X7_gsp_c             ) << "</td>\n"
        << "</tr>\n"
        << "</table>\n"
        ;

    oss
        << "</body>\n"
        << "</html>\n"
        ;

    return oss.str();
}

void gpt_state::AscribeMembers()
{
#if 0
    ascribe("B0_deduced_policy_year"   , &gpt_state::B0_deduced_policy_year   );
    ascribe("B1_deduced_contract_year" , &gpt_state::B1_deduced_contract_year );
    ascribe("B2_deduced_px7_rate"      , &gpt_state::B2_deduced_px7_rate      );
    ascribe("B3_deduced_nsp_rate"      , &gpt_state::B3_deduced_nsp_rate      );
    ascribe("B4_deduced_target_premium", &gpt_state::B4_deduced_target_premium);
    ascribe("B5_deduced_target_load"   , &gpt_state::B5_deduced_target_load   );
    ascribe("B6_deduced_excess_load"   , &gpt_state::B6_deduced_excess_load   );

    ascribe("C0_init_bft"              , &gpt_state::C0_init_bft              );
    ascribe("C1_init_ldb"              , &gpt_state::C1_init_ldb              );
    ascribe("C2_init_amt_pd"           , &gpt_state::C2_init_amt_pd           );
    ascribe("C3_init_is_mc"            , &gpt_state::C3_init_is_mc            );
    ascribe("C4_init_dcv"              , &gpt_state::C4_init_dcv              );
    ascribe("C5_init_px7"              , &gpt_state::C5_init_px7              );
    ascribe("C6_init_mec"              , &gpt_state::C6_init_mec              );

    ascribe("D0_incr_bft"              , &gpt_state::D0_incr_bft              );
    ascribe("D1_incr_ldb"              , &gpt_state::D1_incr_ldb              );
    ascribe("D2_incr_amt_pd"           , &gpt_state::D2_incr_amt_pd           );
    ascribe("D3_incr_is_mc"            , &gpt_state::D3_incr_is_mc            );
    ascribe("D4_incr_dcv"              , &gpt_state::D4_incr_dcv              );
    ascribe("D5_incr_px7"              , &gpt_state::D5_incr_px7              );
    ascribe("D6_incr_mec"              , &gpt_state::D6_incr_mec              );

    ascribe("E0_decr_bft"              , &gpt_state::E0_decr_bft              );
    ascribe("E1_decr_ldb"              , &gpt_state::E1_decr_ldb              );
    ascribe("E2_decr_amt_pd"           , &gpt_state::E2_decr_amt_pd           );
    ascribe("E3_decr_is_mc"            , &gpt_state::E3_decr_is_mc            );
    ascribe("E4_decr_dcv"              , &gpt_state::E4_decr_dcv              );
    ascribe("E5_decr_px7"              , &gpt_state::E5_decr_px7              );
    ascribe("E6_decr_mec"              , &gpt_state::E6_decr_mec              );

    ascribe("F0_nec_pm_bft"            , &gpt_state::F0_nec_pm_bft            );
    ascribe("F1_nec_pm_ldb"            , &gpt_state::F1_nec_pm_ldb            );
    ascribe("F2_nec_pm_amt_pd"         , &gpt_state::F2_nec_pm_amt_pd         );
    ascribe("F3_nec_pm_is_mc"          , &gpt_state::F3_nec_pm_is_mc          );
    ascribe("F4_nec_pm_dcv"            , &gpt_state::F4_nec_pm_dcv            );
    ascribe("F5_nec_pm_px7"            , &gpt_state::F5_nec_pm_px7            );
    ascribe("F6_nec_pm_mec"            , &gpt_state::F6_nec_pm_mec            );

    ascribe("G0_do_mc_bft"             , &gpt_state::G0_do_mc_bft             );
    ascribe("G1_do_mc_ldb"             , &gpt_state::G1_do_mc_ldb             );
    ascribe("G2_do_mc_amt_pd"          , &gpt_state::G2_do_mc_amt_pd          );
    ascribe("G3_do_mc_is_mc"           , &gpt_state::G3_do_mc_is_mc           );
    ascribe("G4_do_mc_dcv"             , &gpt_state::G4_do_mc_dcv             );
    ascribe("G5_do_mc_px7"             , &gpt_state::G5_do_mc_px7             );
    ascribe("G6_do_mc_mec"             , &gpt_state::G6_do_mc_mec             );

    ascribe("H0_unnec_pm_bft"          , &gpt_state::H0_unnec_pm_bft          );
    ascribe("H1_unnec_pm_ldb"          , &gpt_state::H1_unnec_pm_ldb          );
    ascribe("H2_unnec_pm_amt_pd"       , &gpt_state::H2_unnec_pm_amt_pd       );
    ascribe("H3_unnec_pm_is_mc"        , &gpt_state::H3_unnec_pm_is_mc        );
    ascribe("H4_unnec_pm_dcv"          , &gpt_state::H4_unnec_pm_dcv          );
    ascribe("H5_unnec_pm_px7"          , &gpt_state::H5_unnec_pm_px7          );
    ascribe("H6_unnec_pm_mec"          , &gpt_state::H6_unnec_pm_mec          );

    ascribe("Q0_net_1035"              , &gpt_state::Q0_net_1035              );
    ascribe("Q1_max_nec_prem_net"      , &gpt_state::Q1_max_nec_prem_net      );
    ascribe("Q2_max_nec_prem_gross"    , &gpt_state::Q2_max_nec_prem_gross    );
    ascribe("Q3_cv_before_last_mc"     , &gpt_state::Q3_cv_before_last_mc     );
    ascribe("Q4_cum_px7"               , &gpt_state::Q4_cum_px7               );
    ascribe("Q5_cum_amt_pd"            , &gpt_state::Q5_cum_amt_pd            );
    ascribe("Q6_max_non_mec_prem"      , &gpt_state::Q6_max_non_mec_prem      );
#endif // 0
    ascribe("X0_glp"                   , &gpt_state::X0_glp                   );
    ascribe("X1_gsp"                   , &gpt_state::X1_gsp                   );
    ascribe("X2_glp_a"                 , &gpt_state::X2_glp_a                 );
    ascribe("X3_gsp_a"                 , &gpt_state::X3_gsp_a                 );
    ascribe("X4_glp_b"                 , &gpt_state::X4_glp_b                 );
    ascribe("X5_gsp_b"                 , &gpt_state::X5_gsp_b                 );
    ascribe("X6_glp_c"                 , &gpt_state::X6_glp_c                 );
    ascribe("X7_gsp_c"                 , &gpt_state::X7_gsp_c                 );
}

/// Backward-compatibility serial number of this class's xml version.
///
/// version 0: 20140615T1717Z

int gpt_state::class_version() const
{
    return 0;
}

std::string const& gpt_state::xml_root_name() const
{
    static std::string const s("gpt_state");
    return s;
}

bool gpt_state::is_detritus(std::string const& s) const
{
    static std::vector<std::string> const v
        {"Remove this string when adding the first removed entity."
        };
    return contains(v, s);
}
