// Ledger xml input and output.
//
// Copyright (C) 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: ledger_xml_io.cpp,v 1.48.2.8 2006-10-20 17:46:02 etarassov Exp $

#include "ledger.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
#include "global_settings.hpp"
#include "ledger_base.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "security.hpp"
#include "value_cast.hpp"
#include "version.hpp"
#include "xml_lmi.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>
#include <libxml++/libxml++.h>

#include <fstream>
#include <iomanip>
#include <ios>
#include <locale>
#include <sstream>
#include <utility>

void Ledger::read(xml_lmi::Element const&)
{
    // TODO ?? Not yet implemented.
}

namespace
{

typedef std::vector< double      > double_vector_t;
typedef std::vector< std::string > string_vector_t;

int const n = 7;

// TODO ?? consider using shortcuts such as 'curr', 'guar', etc.
char const* char_p_suffixes[n] =
    {"run_curr_basis"         // e_run_curr_basis
    ,"run_guar_basis"         // e_run_guar_basis
    ,"run_mdpt_basis"         // e_run_mdpt_basis
    ,"run_curr_basis_sa_zero" // e_run_curr_basis_sa_zero
    ,"run_guar_basis_sa_zero" // e_run_guar_basis_sa_zero
    ,"run_curr_basis_sa_half" // e_run_curr_basis_sa_half
    ,"run_guar_basis_sa_half" // e_run_guar_basis_sa_half
    };

string_vector_t const suffixes
    (char_p_suffixes
    ,char_p_suffixes + n
    );

template<typename EnumType, int N>
std::vector<std::string> enum_vector_to_string_vector
    (std::vector<xenum<EnumType, N> > const& ve
    )
{
    string_vector_t vs;
    typename std::vector<xenum<EnumType, N> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        vs.push_back(ve_i->str());
        }
    return vs;
}

/// double_formatter_t implements the double number formatting into strings
///
/// Internally define a set of allowed formats (f1, f2, f3, f4, bp?).
/// Make sure this list corresponds to formats described in 'schema.xsd' file.
/// At initialisation read column formats from 'format.xml'.
///
/// if SHOW_MISSING_FORMATS is defined, log all missing format requests
/// into 'missing_formats' file

class double_formatter_t
{
  public:
    double_formatter_t();

    bool            has_format(std::string const & s) const;
    std::string     format(std::string const & s, double d) const;
    string_vector_t format(std::string const & s, double_vector_t const & dv) const;

  private:
    // Double conversion units
    // It is a <double coefficient, string suffix>.
    typedef std::pair<double,std::string> unit_t;
    // Double to string conversion format.
    // It is notionally <int precision, unit_t units>.
    typedef std::pair<int,unit_t> format_t;

    // Map value name to the corresponding format
    typedef std::map<std::string, format_t> format_map_t;

    format_map_t format_map;

    format_t get_format(std::string const & s) const;
    std::string do_format(double d, format_t const & f) const;

#if SHOW_MISSING_FORMATS
    static std::string missing_formats_filename()
    {
        return "missing_formats";
    }
#endif // SHOW_MISSING_FORMATS
};

// Look at file 'missing_formats'. It's important. You want
// it to be empty; once it is, you can suppress the code that creates
// and writes to it.
//
// Here's what it means. For all numbers (so-called 'scalars' and
// 'vectors', but not 'strings') grabbed from all ledgers, we look
// for a format. If we find one, we use it to turn the number into
// a string. If not, we ignore it.
//
// Some things you probably want are so ignored.
//
// Why did I think this reasonble? Because no other reasonable
// behavior occurs to me, for one thing: silently falling back on
// some 'default' format can't be right, because it masks defects
// that we want to fix. For another thing, this gives you a handy
// way to do the 'subsetting' we'd spoken of. If you want a (numeric)
// field, then give it a format; if you don't, then don't.
//
// Speaking of masked defects--now I'm really glad I did this.
// Look at that 'missing_formats' file. Not only does it list
// everything you consciously decided you didn't want, like
//   EffDateJdn
// it also shows stuff that I think we never had, but need, like
//   AllowDbo3
// which I think is used for some purpose that was important to
// Compliance.
//
// I've designed this to be maintained, except for the ugly parts
// we aren't talking about here, so I think you'll be able to fix
// this stuff easily. Where your specs said, e.g.
//
// > Format as a number with thousand separators and no decimal places (#,###,##0)
// >
// > AcctVal
// > SepAcctLoad
// > AccumulatedPremium
//
// I translated that into
//
//    format_map["AcctVal"                           ] = f1;
//    format_map["SepAcctLoad"                       ] = f1;
//    format_map["AccumulatedPremium"                ] = f1;
//
// where 'f1' is one of several formats I abstracted from your specs
// at the top level. For names formed as
//   basename
//
// This translation is just text transformations on your specs.
// I imported your specs and then did regex search-and-replace
// to write this code.
//
// To make a missing (numeric) variable appear in the xml,
// just add a line like those.
//
// Searching for the first occurrence of, say, 'f1' will take you
// to the section where I analyze your formats. It's marked with
// your name in caps so that you can find it easily.

double_formatter_t::double_formatter_t()
{
#ifdef SHOW_MISSING_FORMATS
    {
    std::ofstream ofs(missing_formats_filename().c_str(), std::ios_base::out | std::ios_base::trunc);
    ofs << "No format found for the following numeric data.\n";
    ofs << "These data were therefore not written to xml.\n";
    }
#endif // defined SHOW_MISSING_FORMATS

// Here's my top-level analysis of the formatting specification.
//
// Formats
//
// f0: zero decimals
// f1: zero decimals, commas
// f2: two decimals, commas
// f3: scaled by 100, zero decimals, with '%' at end:
// f4: scaled by 100, two decimals, with '%' at end:
// bp: scaled by 10000, two decimals, with 'bp' at end:
//
// Presumably all use commas as thousands-separators, so that
// an IRR of 12345.67% would be formatted as "12,345.67%".
//
// So the differences are:
//   'precision'      (number of decimal places)
//   'scaling factor' (1 by default, 100 for percents, 1000 for 'bp')
//   'units'          (empty by default, '%' for percents, 'bp' for bp)
// and therefore F0 is equivalent to F1

    unit_t units_default  (1.    , ""  );
    unit_t units_percents (100.  , "%" );
    unit_t units_bp       (10000., "bp");

    format_t f1(0, units_default );
    format_t f2(2, units_default );
    format_t f3(0, units_percents);
    format_t f4(2, units_percents);
    format_t bp(2, units_bp      );

    std::map<std::string, format_t> known_formats;

    known_formats["f0"] = f1;
    known_formats["f1"] = f1;
    known_formats["f2"] = f2;
    known_formats["f3"] = f3;
    known_formats["f4"] = f4;
    known_formats["bp"] = bp;

    // read all the formatting information from "format.xml" file
    std::string format_path;
    try
    {
        boost::filesystem::path xslt_directory = configurable_settings::instance().xslt_directory();
        format_path  =
            (xslt_directory / configurable_settings::instance().xslt_format_xml_filename()
            ).string();
    }
    catch(boost::filesystem::filesystem_error const & e)
    {
        hobsons_choice()
            << "Invalid directory '"
            << configurable_settings::instance().xslt_directory()
            << "' or filename '"
            << configurable_settings::instance().xslt_format_xml_filename()
            << "' specified."
            << e.what()
            << LMI_FLUSH
            ;
    }

    try
        {
        xml_lmi::dom_parser parser(format_path);

        // We will not check 'format.xml' for validity here. It should be done during tests.
        xml_lmi::Element const& root_node = parser.root_node("columns");

        xml_lmi::NodeContainer const columns = root_node.get_children("column");
        for
            (xml_lmi::NodeContainer::const_iterator it = columns.begin()
            ,end = columns.end()
            ;it != end
            ;++it
            )
            {
            xml_lmi::Element const* column_element
                = dynamic_cast<xml_lmi::Element const*>(*it);
            // a 'column' node is not an element node, skip it
            if(!column_element)
                {
                continue;
                }

            xml_lmi::Attribute const* name_attribute = column_element->get_attribute("name");
            // a 'column' node has to have @name attribute
            if(!name_attribute)
                {
                continue;
                }
            std::string const name = name_attribute->get_value();

            xml_lmi::NodeContainer const formats = (*it)->get_children("format");
            // skip nodes without format information
            if(formats.empty())
                {
                continue;
                }

            xml_lmi::Element const* format_element
                = dynamic_cast<xml_lmi::Element const*>(*formats.begin());
            // a 'column/format' node is not an element node, skip it
            if(!format_element)
                {
                continue;
                }

            // format has already been specified. show a warning and continue
            if(format_map.find(name) != format_map.end())
                {
                warning()
                    << "Formats file '"
                    << format_path
                    << "' contains more than one format definition for '"
                    << name
                    << "' on line "
                    << format_element->get_line()
                    << "."
                    << LMI_FLUSH;
                }

            std::string const format_name = xml_lmi::get_content(*format_element);

            // unknown format specified
            if(known_formats.find(format_name) == known_formats.end())
                {
                warning()
                    << "Unknown format '"
                    << format_name
                    << "' specified in '"
                    << format_path
                    << "'."
                    << LMI_FLUSH;
                }
            format_map[name] = known_formats[format_name];
            }

        if(format_map.empty())
            {
            std::ostringstream oss;
            oss
                << "Could not read no format definitions from '"
                << format_path
                << "'. File is empty or has invalid format."
                ;
            throw std::runtime_error(oss.str());
            }
        }
    catch(std::exception const & e)
        {
        warning()
            << "Error reading format information from '"
            << format_path
            << "'. Error: "
            << e.what()
            << LMI_FLUSH
            ;
        }
}

bool double_formatter_t::has_format(std::string const & s) const
{
    format_map_t::const_iterator it = format_map.find(s);
    if (it == format_map.end())
    {
#ifdef SHOW_MISSING_FORMATS
        std::ofstream ofs("missing_formats", std::ios_base::out | std::ios_base::ate | std::ios_base::app);
        ofs << s << "\n";
#endif // defined SHOW_MISSING_FORMATS
        return false;
    }
    return true;
}

double_formatter_t::format_t
double_formatter_t::get_format(std::string const & s) const
{
    format_map_t::const_iterator it = format_map.find(s);
    if (it == format_map.end())
        {
        hobsons_choice()
            << "Unknown column name '"
            << s
            << "' encountered."
            << LMI_FLUSH
            ;
        // use default format
        return format_t(2, unit_t(1., ""));
        }
    return it->second;
}

std::string double_formatter_t::format(std::string const & name, double d) const
{
    format_t f = get_format(name);

    return do_format(d, f);
}

string_vector_t double_formatter_t::format(std::string const & s, double_vector_t const & dv) const
{
    format_t f = get_format(s);
    string_vector_t sv;
    for(double_vector_t::const_iterator it = dv.begin(),
                                       end = dv.end();
                                       it != end; ++it)
        {
        sv.push_back( do_format(*it, f) );
        }
    return sv;
}

std::string double_formatter_t::do_format(double d, format_t const & f) const
{
    std::stringstream interpreter;
    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    interpreter.imbue(new_loc);
    interpreter.setf(std::ios_base::fixed, std::ios_base::floatfield);
    interpreter.precision(f.first);
    std::string s;
    unit_t const & units = f.second;
    if(units.first != 1.) d *= units.first;
    interpreter << d;
    interpreter >> s;
    if(!units.second.empty()) s += units.second;
    if(!interpreter.eof())
        {
        fatal_error() << "Format error" << LMI_FLUSH;
        }
    return s;
}

// --------------------------------------------------------------
// a column is identified by its name and an optional basis value
// --------------------------------------------------------------
typedef std::pair<std::string, std::string const*> value_id;

value_id make_value_id(std::string const & name)
{
    return value_id(name, NULL);
}

value_id make_value_id(std::string const & name, e_run_basis const & basis)
{
    return value_id(name, &suffixes[basis]);
}

} // Unnamed namespace.

// ---------------------
// Ledger implementation
// ---------------------
void Ledger::write(xml_lmi::Element& illustration) const
{
    // by default generate a full version (not a light_version)
    do_write(illustration, false);
}

void Ledger::do_write(xml_lmi::Element& illustration, bool light_version) const
{
    // This is a little tricky. We have some stuff that
    // isn't in the maps inside the ledger classes. We're going to
    // stuff it into a copy of the invariant-ledger class's data.
    // To avoid copying, we'll use pointers to the data. Most of
    // this stuff is invariant anyway, so that's a reasonable
    // place to put it.
    //
    // First we make a copy of the invariant ledger:

    double_vector_map vectors = ledger_invariant_->AllVectors;
    scalar_map        scalars = ledger_invariant_->AllScalars;
    string_map        strings = ledger_invariant_->Strings;

    // Now we add the stuff that wasn't in the invariant
    // ledger's class's maps (indexable by name). Because we're
    // working with maps of pointers, we need pointers here.
    //
    // The IRRs are the worst of all. Only calculate it if e_xml_variant_heavy.
    if(!light_version)
        {
        if(!ledger_invariant_->IsInforce)
            {
            ledger_invariant_->CalculateIrrs(*this);
            }
        vectors["IrrCsv_GuaranteedZero" ] = &ledger_invariant_->IrrCsvGuar0    ;
        vectors["IrrDb_GuaranteedZero"  ] = &ledger_invariant_->IrrDbGuar0     ;
        vectors["IrrCsv_CurrentZero"    ] = &ledger_invariant_->IrrCsvCurr0    ;
        vectors["IrrDb_CurrentZero"     ] = &ledger_invariant_->IrrDbCurr0     ;
        vectors["IrrCsv_Guaranteed"     ] = &ledger_invariant_->IrrCsvGuarInput;
        vectors["IrrDb_Guaranteed"      ] = &ledger_invariant_->IrrDbGuarInput ;
        vectors["IrrCsv_Current"        ] = &ledger_invariant_->IrrCsvCurrInput;
        vectors["IrrDb_Current"         ] = &ledger_invariant_->IrrDbCurrInput ;
        }

// GetMaxLength() is max *composite* length.
//    int max_length = GetMaxLength();
    double MaxDuration = ledger_invariant_->EndtAge - ledger_invariant_->Age;
    scalars["MaxDuration"] = &MaxDuration;
    int max_duration = static_cast<int>(MaxDuration);

    std::vector<double> PolicyYear;
    std::vector<double> AttainedAge;

    PolicyYear .resize(max_duration);
    AttainedAge.resize(max_duration);

    int issue_age = static_cast<int>(ledger_invariant_->Age);
    for(int j = 0; j < max_duration; ++j)
        {
        PolicyYear[j]  = 1 + j;
        AttainedAge[j] = 1 + j + issue_age;
        }

// STEVEN What about the composite? I think you want to avoid using
// an attained-age column there, because it'd be meaningless.
    vectors["AttainedAge"] = &AttainedAge;
    vectors["PolicyYear" ] = &PolicyYear ;

    vectors["InforceLives"] = &ledger_invariant_->InforceLives;

    vectors["FundNumbers"    ] = &ledger_invariant_->FundNumbers    ;
    vectors["FundAllocations"] = &ledger_invariant_->FundAllocations;

    // The Ledger object should contain a basic minimal set of columns
    // from which others may be derived. It must be kept small because
    // its size imposes a practical limit on the number of lives that
    // can be run as part of a single census.
    //
    // TODO ?? A really good design would give users the power to
    // define and store their own derived-column definitions. For now,
    // however, code changes are required, and this is as appropriate
    // a place as any to make them.
    LedgerVariant const& Curr_ = GetCurrFull();
    // ET !! Easier to write as
    //   std::vector<double> NetDeathBenefit =
    //     Curr_.EOYDeathBft - Curr_.TotalLoanBalance;
    std::vector<double> NetDeathBenefit(Curr_.EOYDeathBft);
    std::transform
        (NetDeathBenefit.begin()
        ,NetDeathBenefit.end()
        ,Curr_.TotalLoanBalance.begin()
        ,NetDeathBenefit.begin()
        ,std::minus<double>()
        );
    vectors["NetDeathBenefit"] = &NetDeathBenefit ;
    // [End of derived columns.]
    double Composite = GetIsComposite();
    scalars["Composite"] = &Composite;

    double NoLapse =
            0 != ledger_invariant_->NoLapseMinDur
        ||  0 != ledger_invariant_->NoLapseMinAge
        ;
    scalars["NoLapse"] = &NoLapse;

    calendar_date prep_date;

    // Skip security validation for non-interactive regression testing.
    if(!global_settings::instance().regression_testing())
        {
        // Skip security validation for the most privileged password.
        validate_security(!global_settings::instance().ash_nazg());
        }
    else
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        prep_date.julian_day_number(static_cast<int>(ledger_invariant_->EffDateJdn));
        }

    std::string PrepYear  = value_cast<std::string>(prep_date.year());
    std::string PrepMonth = month_name(prep_date.month());
    std::string PrepDay   = value_cast<std::string>(prep_date.day());

    strings["PrepYear" ] = &PrepYear;
    strings["PrepMonth"] = &PrepMonth;
    strings["PrepDay"  ] = &PrepDay;

    double SalesLoadRefund =
        !each_equal
            (ledger_invariant_->RefundableSalesLoad.begin()
            ,ledger_invariant_->RefundableSalesLoad.end()
            ,0.0
            );
    double SalesLoadRefundRate0 = ledger_invariant_->RefundableSalesLoad[0];
    double SalesLoadRefundRate1 = ledger_invariant_->RefundableSalesLoad[1];

    scalars["SalesLoadRefund"     ] = &SalesLoadRefund     ;
    scalars["SalesLoadRefundRate0"] = &SalesLoadRefundRate0;
    scalars["SalesLoadRefundRate1"] = &SalesLoadRefundRate1;

    // IsSubjectToIllustrationReg
    double IsSubjectToIllustrationReg = is_subject_to_ill_reg(GetLedgerType());
    scalars["IsSubjectToIllustrationReg"] = &IsSubjectToIllustrationReg;

    std::string ScaleUnit = ledger_invariant_->ScaleUnit();
    strings["ScaleUnit"] = &ScaleUnit;

    // TODO ?? Presumably this is translated to a string in xsl;
    // why not use the first element of <DbOpt>, which is already
    // so formatted? Wouldn't that rule out any possibility of
    // inconsistency between xsl's and the program's translations?
    double DBOptInitInteger = ledger_invariant_->DBOpt[0].value();
    scalars["DBOptInitInteger"] = &DBOptInitInteger;

    double InitTotalSA =
            ledger_invariant_->InitBaseSpecAmt
        +   ledger_invariant_->InitTermSpecAmt
        ;
    scalars["InitTotalSA"] = &InitTotalSA;

    std::string LmiVersion(LMI_VERSION);
    strings["LmiVersion"] = &LmiVersion;

    // Maps to hold the numeric data.

    typedef std::pair<std::string, std::string const*> value_id;

    typedef std::map<value_id, double>            double_scalar_map_t;
    typedef std::map<value_id, double_vector_t >  double_vector_map_t;
    typedef std::map<value_id, std::string>       string_scalar_map_t;
    typedef std::map<value_id, string_vector_t >  string_vector_map_t;

    double_scalar_map_t double_scalars;
    double_vector_map_t double_vectors;
    string_scalar_map_t string_scalars;
    string_vector_map_t string_vectors;

    string_vectors[ make_value_id("FundNames") ] = ledger_invariant_->FundNames;

    // Map the data, formatting it as necessary.

    // initialize number formatting facility
    double_formatter_t formatter;

    // First we'll get the invariant stuff--the copy we made,
    // along with all the stuff we plugged into it above.

    for
        (scalar_map::const_iterator j = scalars.begin()
        ;j != scalars.end()
        ;++j
        )
        {
        double_scalars[ make_value_id( j->first ) ] = *j->second;
        }
    for
        (string_map::const_iterator j = strings.begin()
        ;j != strings.end()
        ;++j
        )
        {
        string_scalars[ make_value_id( j->first ) ] = *j->second;
        }
    for
        (double_vector_map::const_iterator j = vectors.begin()
        ;j != vectors.end()
        ;++j
        )
        {
        double_vectors[ make_value_id( j->first ) ] = *j->second;
        }

//    doublescalars[make_value_id("GuarMaxMandE")] = *scalars[make_value_id("GuarMaxMandE")];
//    stringvectors[make_value_id("CorridorFactor")] = double_vector_to_string_vector(*vectors[make_value_id("CorridorFactor")]);
//    doublescalars[make_value_id("InitAnnGenAcctInt", e_run_curr_basis)] = *scalars[make_value_id("InitAnnGenAcctInt", e_run_curr_basis)];

    // That was the tricky part. Now it's all downhill.

    ledger_map const& l_map_rep = ledger_map_->held();
    ledger_map::const_iterator i = l_map_rep.begin();
    for(;i != l_map_rep.end(); i++)
        {
        for
            (scalar_map::const_iterator j = i->second.AllScalars.begin()
            ;j != i->second.AllScalars.end()
            ;++j
            )
            {
            double_scalars[ make_value_id( j->first, i->first ) ] = *j->second;
            }
        // TODO ?? 'strings' variable is not going to be referenced by the rest
        // of the code, why do we want to modify it here?
        for
            (string_map::const_iterator j = i->second.Strings.begin()
            ;j != i->second.Strings.end()
            ;++j
            )
            {
            strings[j->first] = j->second;
            }
        for
            (double_vector_map::const_iterator j = i->second.AllVectors.begin()
            ;j != i->second.AllVectors.end()
            ;++j
            )
            {
            double_vectors[ make_value_id( j->first, i->first ) ] = *j->second;
            }
        }

    string_vectors[make_value_id("EeMode")] = enum_vector_to_string_vector( ledger_invariant_->EeMode );
    string_vectors[make_value_id("ErMode")] = enum_vector_to_string_vector( ledger_invariant_->ErMode );
    string_vectors[make_value_id("DBOpt")]  = enum_vector_to_string_vector( ledger_invariant_->DBOpt  );

// TODO ?? Here I copied some stuff from the ledger class files: the
// parts that speak of odd members that aren't in those class's
// maps. This may reveal incomplete or incorrect systems analysis.
// Invariant
//
//    // Special-case vectors (not <double>, or different length than others).
//    EeMode              .reserve(Length);
//    ErMode              .reserve(Length);
//    DBOpt               .reserve(Length);
//
//    std::vector<int>            FundNumbers; [not handled yet]
//    std::vector<std::string>    FundNames;   [not handled yet]
//    std::vector<int>            FundAllocs;  [not handled yet]
//
//    std::vector<double> InforceLives;
//
//    // Special-case strings.
//    std::string     EffDate; [furnished as PrepYear, PrepMonth, PrepDay]
//
// Variant
//
// [None of these are written to xml, and I think none is wanted.]
//
//    // special cases
//    int              Length;
//    e_basis          ExpAndGABasis;
//    e_sep_acct_basis SABasis;
//    bool             FullyInitialized;   // i.e. by Init(BasicValues* b)

// Now we're ready to write the xml.

    // string scalars
    for
        (string_scalar_map_t::const_iterator j = string_scalars.begin()
        ;j != string_scalars.end()
        ;++j
        )
        {
        xml_lmi::Element & string_scalar = *illustration.add_child("string_scalar");
        value_id const & id = j->first;
        string_scalar.set_attribute("name", id.first);
        if (id.second)
            string_scalar.set_attribute("basis", *id.second);
        string_scalar.add_child_text( j->second );
        }
    // double scalars
    for
        (double_scalar_map_t::const_iterator j = double_scalars.begin()
        ;j != double_scalars.end()
        ;++j
        )
        {
        value_id const & id = j->first;
        if ( formatter.has_format( id.first ) )
            {
            xml_lmi::Element & double_scalar = *illustration.add_child("double_scalar");
            double_scalar.set_attribute("name", id.first);
            if (id.second)
                double_scalar.set_attribute("basis", *id.second);
            double_scalar.add_child_text( formatter.format( id.first, j->second ) );
            }
        }
    // vectors of strings
    for
        (string_vector_map_t::const_iterator j = string_vectors.begin()
        ;j != string_vectors.end()
        ;++j
        )
        {
        xml_lmi::Element & svector = *illustration.add_child("string_vector");
        value_id const & id = j->first;
        svector.set_attribute("name", id.first);
        if (id.second)
            svector.set_attribute("basis", *id.second);
        string_vector_t const & v = j->second;
        for
            (string_vector_t::const_iterator k = v.begin()
            ;k != v.end()
            ;++k
            )
            {
            xml_lmi::Element & duration = *svector.add_child("duration");
            duration.add_child_text(*k);
            }
        }
    // vectors of doubles
    for
        (double_vector_map_t::const_iterator j = double_vectors.begin()
        ;j != double_vectors.end()
        ;++j
        )
        {
        value_id const & id = j->first;
        if ( formatter.has_format( id.first ) )
            {
            xml_lmi::Element & dvector = *illustration.add_child("double_vector");
            dvector.set_attribute("name", id.first);
            if (id.second)
                dvector.set_attribute("basis", *id.second);
            string_vector_t v = formatter.format( id.first, j->second );
// TODO ?? InforceLives shows an extra value past the end; should it
// be truncated here?
            for
                (string_vector_t::const_iterator k = v.begin()
                ;k != v.end()
                ;++k
                )
                {
                xml_lmi::Element & duration = *dvector.add_child("duration");
                duration.add_child_text(*k);
                }
            }
        }

    if(ledger_invariant_->SupplementalReport)
        {
        // prepare supplement report column list
        string_vector_t SupplementalReportColumns;

        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn00);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn01);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn02);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn03);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn04);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn05);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn06);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn07);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn08);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn09);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn10);
        SupplementalReportColumns.push_back(ledger_invariant_->SupplementalReportColumn11);

        // now put this information into the xml
        xml_lmi::Element & supplemental_report = *illustration.add_child("supplemental_report");

        // Eventually customize the report name.
        supplemental_report.add_child("title")->add_child_text("Supplemental Report");

        string_vector_t::const_iterator j;
        for
            (j = SupplementalReportColumns.begin()
            ;j != SupplementalReportColumns.end()
            ;++j
            )
            {
            // FIXME find a better way of determining if it is an empty column
            if ( *j == "[None]" )
                {
                supplemental_report.add_child("spacer");
                }
            else // *j != "[None]"
                {
                supplemental_report.add_child("column")->add_child_text( *j );
                }
            }
        }
}

int Ledger::class_version() const
{
    return 0;
}

std::string Ledger::xml_root_name() const
{
    return "illustration";
}

void Ledger::write(std::ostream& os) const
{
    xml_lmi::Document doc;
    xml_lmi::Element& root = *doc.create_root_node(xml_root_name());
    root << *this;

    root.set_namespace_declaration("http://www.w3.org/2001/XMLSchema-instance", "xsi");
    root.set_attribute("noNamespaceSchemaLocation", "file:schema.xsd", "xsi");

    os << doc;
}
