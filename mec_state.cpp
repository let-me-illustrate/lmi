// Transient state of MEC testing.
//
// Copyright (C) 2009, 2010 Gregory W. Chicares.
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

#include "mec_state.hpp"

#include "alert.hpp"
#include "miscellany.hpp" // htmlize(), lmi_array_size()
#include "value_cast.hpp"
#include "xml_lmi.hpp"

#include <boost/filesystem/fstream.hpp>

#include <xmlwrapp/nodes_view.h>

#include <algorithm>      // std::find()
#include <iterator>       // std::back_inserter
#include <limits>
#include <sstream>
#include <vector>

namespace
{
/// Entities that were present in older versions and then removed
/// are recognized and ignored. If they're resurrected in a later
/// version, then they aren't ignored.

bool is_detritus(std::string const& s)
{
    static std::string const a[] =
        {"Remove this string when adding the first removed entity."
        };
    static std::vector<std::string> const v(a, a + lmi_array_size(a));
    return v.end() != std::find(v.begin(), v.end(), s);
}
} // Unnamed namespace.

/// Initialize all ascribed members to zero.

mec_state::mec_state()
{
    AscribeMembers();
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = "0";
        }
}

mec_state::mec_state(mec_state const& z)
    :obstruct_slicing<mec_state>()
    ,streamable()
    ,MemberSymbolTable<mec_state>()
{
    AscribeMembers();
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        // This would be wrong:
        //   operator[](*i) = z[*i];
        // because it would swap in a copy of z's *members*.
        operator[](*i) = z[*i].str();
        }
}

mec_state::~mec_state()
{
}

mec_state& mec_state::operator=(mec_state const& z)
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        operator[](*i) = z[*i].str();
        }
    return *this;
}

bool mec_state::operator==(mec_state const& z) const
{
    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string const s0 = operator[](*i).str();
        std::string const s1 = z[*i].str();
        if(s0 != s1)
            {
            return false;
            }
        }
    return true;
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

std::string mec_state::format_as_html(std::string const& heading) const
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

    oss
        << "</body>\n"
        << "</html>\n"
        ;

    return oss.str();
}

void mec_state::save(fs::path const& filepath) const
{
    fs::ofstream ofs(filepath, ios_out_trunc_binary());
    xml_lmi::xml_document document(xml_root_name() + "_document");
    xml::element& root = document.root_node();
    root << *this;
    ofs << document;
    if(!ofs)
        {
        warning() << "Unable to save '" << filepath.string() << "'." << LMI_FLUSH;
        }
}

void mec_state::AscribeMembers()
{
    ascribe("B0_deduced_policy_year"   , &mec_state::B0_deduced_policy_year   );
    ascribe("B1_deduced_contract_year" , &mec_state::B1_deduced_contract_year );
    ascribe("B2_deduced_px7_rate"      , &mec_state::B2_deduced_px7_rate      );
    ascribe("B3_deduced_nsp_rate"      , &mec_state::B3_deduced_nsp_rate      );
    ascribe("B4_deduced_target_premium", &mec_state::B4_deduced_target_premium);
    ascribe("B5_deduced_target_load"   , &mec_state::B5_deduced_target_load   );
    ascribe("B6_deduced_excess_load"   , &mec_state::B6_deduced_excess_load   );

    ascribe("C0_init_bft"              , &mec_state::C0_init_bft              );
    ascribe("C1_init_ldb"              , &mec_state::C1_init_ldb              );
    ascribe("C2_init_amt_pd"           , &mec_state::C2_init_amt_pd           );
    ascribe("C3_init_is_mc"            , &mec_state::C3_init_is_mc            );
    ascribe("C4_init_dcv"              , &mec_state::C4_init_dcv              );
    ascribe("C5_init_px7"              , &mec_state::C5_init_px7              );
    ascribe("C6_init_mec"              , &mec_state::C6_init_mec              );

    ascribe("D0_incr_bft"              , &mec_state::D0_incr_bft              );
    ascribe("D1_incr_ldb"              , &mec_state::D1_incr_ldb              );
    ascribe("D2_incr_amt_pd"           , &mec_state::D2_incr_amt_pd           );
    ascribe("D3_incr_is_mc"            , &mec_state::D3_incr_is_mc            );
    ascribe("D4_incr_dcv"              , &mec_state::D4_incr_dcv              );
    ascribe("D5_incr_px7"              , &mec_state::D5_incr_px7              );
    ascribe("D6_incr_mec"              , &mec_state::D6_incr_mec              );

    ascribe("E0_decr_bft"              , &mec_state::E0_decr_bft              );
    ascribe("E1_decr_ldb"              , &mec_state::E1_decr_ldb              );
    ascribe("E2_decr_amt_pd"           , &mec_state::E2_decr_amt_pd           );
    ascribe("E3_decr_is_mc"            , &mec_state::E3_decr_is_mc            );
    ascribe("E4_decr_dcv"              , &mec_state::E4_decr_dcv              );
    ascribe("E5_decr_px7"              , &mec_state::E5_decr_px7              );
    ascribe("E6_decr_mec"              , &mec_state::E6_decr_mec              );

    ascribe("F0_nec_pm_bft"            , &mec_state::F0_nec_pm_bft            );
    ascribe("F1_nec_pm_ldb"            , &mec_state::F1_nec_pm_ldb            );
    ascribe("F2_nec_pm_amt_pd"         , &mec_state::F2_nec_pm_amt_pd         );
    ascribe("F3_nec_pm_is_mc"          , &mec_state::F3_nec_pm_is_mc          );
    ascribe("F4_nec_pm_dcv"            , &mec_state::F4_nec_pm_dcv            );
    ascribe("F5_nec_pm_px7"            , &mec_state::F5_nec_pm_px7            );
    ascribe("F6_nec_pm_mec"            , &mec_state::F6_nec_pm_mec            );

    ascribe("G0_do_mc_bft"             , &mec_state::G0_do_mc_bft             );
    ascribe("G1_do_mc_ldb"             , &mec_state::G1_do_mc_ldb             );
    ascribe("G2_do_mc_amt_pd"          , &mec_state::G2_do_mc_amt_pd          );
    ascribe("G3_do_mc_is_mc"           , &mec_state::G3_do_mc_is_mc           );
    ascribe("G4_do_mc_dcv"             , &mec_state::G4_do_mc_dcv             );
    ascribe("G5_do_mc_px7"             , &mec_state::G5_do_mc_px7             );
    ascribe("G6_do_mc_mec"             , &mec_state::G6_do_mc_mec             );

    ascribe("H0_unnec_pm_bft"          , &mec_state::H0_unnec_pm_bft          );
    ascribe("H1_unnec_pm_ldb"          , &mec_state::H1_unnec_pm_ldb          );
    ascribe("H2_unnec_pm_amt_pd"       , &mec_state::H2_unnec_pm_amt_pd       );
    ascribe("H3_unnec_pm_is_mc"        , &mec_state::H3_unnec_pm_is_mc        );
    ascribe("H4_unnec_pm_dcv"          , &mec_state::H4_unnec_pm_dcv          );
    ascribe("H5_unnec_pm_px7"          , &mec_state::H5_unnec_pm_px7          );
    ascribe("H6_unnec_pm_mec"          , &mec_state::H6_unnec_pm_mec          );

    ascribe("Q0_net_1035"              , &mec_state::Q0_net_1035              );
    ascribe("Q1_max_nec_prem_net"      , &mec_state::Q1_max_nec_prem_net      );
    ascribe("Q2_max_nec_prem_gross"    , &mec_state::Q2_max_nec_prem_gross    );
    ascribe("Q3_cv_before_last_mc"     , &mec_state::Q3_cv_before_last_mc     );
    ascribe("Q4_cum_px7"               , &mec_state::Q4_cum_px7               );
    ascribe("Q5_cum_amt_pd"            , &mec_state::Q5_cum_amt_pd            );
    ascribe("Q6_max_non_mec_prem"      , &mec_state::Q6_max_non_mec_prem      );
}

void mec_state::read(xml::element const& x)
{
    if(xml_root_name() != x.get_name())
        {
        fatal_error()
            << "XML node name is '"
            << x.get_name()
            << "' but '"
            << xml_root_name()
            << "' was expected."
            << LMI_FLUSH
            ;
        }

    std::string file_version_string;
    if(!xml_lmi::get_attr(x, "version", file_version_string))
        {
        fatal_error()
            << "XML tag <"
            << xml_root_name()
            << "> lacks required version attribute."
            << LMI_FLUSH
            ;
        }
    int file_version = value_cast<int>(file_version_string);

// COMPILER !! Borland doesn't find operator==() in ns xml.
#ifdef __BORLANDC__
using namespace xml;
#endif // __BORLANDC__

    std::map<std::string, std::string> detritus_map;

    std::list<std::string> residuary_names;
    std::copy
        (member_names().begin()
        ,member_names().end()
        ,std::back_inserter(residuary_names)
        );
    std::list<std::string>::iterator current_member;

    xml::const_nodes_view const elements(x.elements());
    typedef xml::const_nodes_view::const_iterator cnvi;
    for(cnvi child = elements.begin(); child != elements.end(); ++child)
        {
        std::string node_tag(child->get_name());
        current_member = std::find
            (residuary_names.begin()
            ,residuary_names.end()
            ,node_tag
            );
        if(residuary_names.end() != current_member)
            {
            operator[](node_tag) = RedintegrateExAnte
                (file_version
                ,node_tag
                ,xml_lmi::get_content(*child)
                );
            residuary_names.erase(current_member);
            }
        else if(is_detritus(node_tag))
            {
            // Hold certain obsolete entities that must be translated.
            detritus_map[node_tag] = xml_lmi::get_content(*child);
            }
        else
            {
            warning()
                << "XML tag '"
                << node_tag
                << "' not recognized by this version of the program."
                << LMI_FLUSH
                ;
            }
        }

    RedintegrateExPost(file_version, detritus_map, residuary_names);
}

void mec_state::write(xml::element& x) const
{
    xml::element root(xml_root_name().c_str());

// XMLWRAPP !! There's no way to set an integer attribute.
    std::string const version(value_cast<std::string>(class_version()));
    xml_lmi::set_attr(root, "version", version.c_str());

    std::vector<std::string>::const_iterator i;
    for(i = member_names().begin(); i != member_names().end(); ++i)
        {
        std::string node_tag(*i);
        std::string value = operator[](*i).str();
        root.push_back(xml::element(node_tag.c_str(), value.c_str()));
        }

    x.push_back(root);
}

/// Serial number of this class's xml version.
///
/// version 0: 20090728T1324Z

int mec_state::class_version() const
{
    return 0;
}

std::string mec_state::xml_root_name() const
{
    return "mec_state";
}

/// Provide for backward compatibility before assigning values.

std::string mec_state::RedintegrateExAnte
    (int                file_version
    ,std::string const& // name
    ,std::string const& value
    ) const
{
    if(class_version() == file_version)
        {
        return value;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        return value; // Stifle compiler warning.
        }
}

/// Provide for backward compatibility after assigning values.

void mec_state::RedintegrateExPost
    (int                                file_version
    ,std::map<std::string, std::string> // detritus_map
    ,std::list<std::string>             // residuary_names
    )
{
    if(class_version() == file_version)
        {
        return;
        }
    else
        {
        fatal_error() << "Incompatible file version." << LMI_FLUSH;
        }
}

