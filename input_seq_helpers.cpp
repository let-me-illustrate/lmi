// Interpret sequence strings.
//
// Copyright (C) 2002, 2005, 2006, 2007, 2008, 2009 Gregory W. Chicares.
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

// $Id: input_seq_helpers.cpp,v 1.6 2008-12-27 02:56:45 chicares Exp $

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "input_seq_helpers.hpp"

namespace detail
{
    stringmap invert_map(stringmap const& sm)
    {
        stringmap inverse;
        stringmap_iterator i;
        for(i = sm.begin(); i != sm.end(); ++i)
            {
            inverse[(*i).second] = (*i).first;
            }
        return inverse;
    }

    std::vector<std::string> extract_keys_from_string_map
        (stringmap const& keyword_dictionary
        )
    {
        std::vector<std::string> keywords;
        stringmap_iterator i_keyword_dictionary = keyword_dictionary.begin();
        for
            (
            ;i_keyword_dictionary != keyword_dictionary.end()
            ;++i_keyword_dictionary
            )
            {
            keywords.push_back((*i_keyword_dictionary).first);
            }
        return keywords;
    }
} // namespace detail

// Input-sequence support.
//
// Input-sequence fields use their own keywords, which might differ from
// xenum strings.
//
// TODO ?? Perhaps we should just use the same strings.
//
// TODO ?? Perhaps we should move this elsewhere.
//
// We return a map by value so that we can modify it, e.g. by removing
// a death benefit option not available with a certain policy form.

std::map<std::string, std::string, std::less<std::string> > dbo_map()
{
// Available death benefit options:
//   multiple-from-through    mce_dbopt
//       a                    "A"
//       b                    "B"
//       rop                  "ROP"

// TODO ?? We could use the 'stringmap' typedef. Wait and see how
// this code evolves.
    std::map<std::string, std::string, std::less<std::string> > m;
    m["a"  ] = "A"  ;
    m["b"  ] = "B"  ;
    m["rop"] = "ROP";
    return m;
}

std::map<std::string, std::string, std::less<std::string> > mode_map()
{
// Available modes:
//   multiple-from-through    mce_mode
//       annual               "Annual"
//       semiannual           "Semiannual"
//       quarterly            "Quarterly"
//       monthly              "Monthly"

// TODO ?? We could use the 'stringmap' typedef. Wait and see how
// this code evolves.
    std::map<std::string, std::string, std::less<std::string> > m;
    m["annual"    ] = "Annual";
    m["semiannual"] = "Semiannual";
    m["quarterly" ] = "Quarterly";
    m["monthly"   ] = "Monthly";
    return m;
}

std::map<std::string, std::string, std::less<std::string> > sastrategy_map()
{
// Available strategies:
//   multiple-from-through    mce_sa_strategy
//       [input amount]       "SAInputScalar"
//       vector               "SAInputVector"
//       maximum              "SAMaximum"
//       target               "SATarget"
//       sevenpay             "SAMEP"
//       glp                  "SAGLP"
//       gsp                  "SAGSP"
//       corridor             "SACorridor"
//       salary               "SASalary"

// TODO ?? We could use the 'stringmap' typedef. Wait and see how
// this code evolves.
    std::map<std::string, std::string, std::less<std::string> > m;
    m["maximum" ] = "SAMaximum"    ;
    m["target"  ] = "SATarget"     ;
    m["sevenpay"] = "SAMEP"        ;
    m["glp"     ] = "SAGLP"        ;
    m["gsp"     ] = "SAGSP"        ;
    m["corridor"] = "SACorridor"   ;
    m["salary"  ] = "SASalary"     ;
    m["none"    ] = "SAInputScalar";

    return m;
}

std::map<std::string, std::string, std::less<std::string> > pmtstrategy_map()
{
// Available strategies:
//   multiple-from-through    mce_pmt_strategy
//       [input amount]       "PmtInputScalar"
//       vector               "PmtInputVector"
//       minimum              "PmtMinimum"
//       target               "PmtTarget"
//       sevenpay             "PmtMEP"
//       glp                  "PmtGLP"
//       gsp                  "PmtGSP"
//       corridor             "PmtCorridor"
//       table                "PmtTable"

// TODO ?? We could use the 'stringmap' typedef. Wait and see how
// this code evolves.
    std::map<std::string, std::string, std::less<std::string> > m;
    m["minimum" ] = "PmtMinimum"    ;
    m["target"  ] = "PmtTarget"     ;
    m["sevenpay"] = "PmtMEP"        ;
    m["glp"     ] = "PmtGLP"        ;
    m["gsp"     ] = "PmtGSP"        ;
    m["corridor"] = "PmtCorridor"   ;
    m["table"   ] = "PmtTable"      ;
    m["none"    ] = "PmtInputScalar";

    return m;
}

