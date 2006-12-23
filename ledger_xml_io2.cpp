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

// $Id: ledger_xml_io2.cpp,v 1.17 2006-12-23 17:47:20 chicares Exp $

#include "ledger.hpp"

#include "alert.hpp"
#include "authenticity.hpp"
#include "calendar_date.hpp"
#include "comma_punct.hpp"
#include "configurable_settings.hpp"
#include "data_directory.hpp"
#include "global_settings.hpp"
#include "ledger_base.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "value_cast.hpp"
#include "version.hpp"
#include "xml_lmi.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <ios>
#include <locale>
#include <map>
#include <set>
#include <sstream>
#include <utility>

#if defined LMI_USE_NEW_REPORTS
void Ledger::read(xml::element const&)
{
    // TODO ?? Not yet implemented.
}
#endif // defined LMI_USE_NEW_REPORTS

namespace
{

typedef std::vector< double      > double_vector_t;
typedef std::vector< std::string > string_vector_t;

int const n = 7;

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

/// A ledger value id.
///
/// A ledger value is identified by a (name, basis) pair, where
/// name is required and basis is optional.
/// Name is stored by value and basis is stored as a pointer to an element
/// of the static vector of strings 'suffixes' which corresponds to
/// enum_run_basis.

class value_id
{
  public:
    value_id();
    value_id(value_id const& rhs);
    // assignment opr is ok

    static value_id empty_value();
    static value_id from_name(std::string const&);
    static value_id from_name_basis(std::string const&, std::string const*);
    static value_id from_name_basis(std::string const&, enum_run_basis);
    static value_id from_report_column_title(std::string const&);
    static value_id from_xml_element(xml::element const&);

    std::string const& name() const { return name_; }
    std::string const* basis() const { return basis_; }

    bool empty() const;

    void set_to_xml_element(xml::element&) const;
    void get_from_xml_element(xml::element const&);

    bool operator<(value_id const&) const;
  private:
    std::string name_;
    std::string const* basis_;

    value_id(std::string const& name, std::string const* basis = NULL);
};

value_id value_id::empty_value()
{
    return value_id("", NULL);
}

value_id value_id::from_name(std::string const& name)
{
    return value_id(name, NULL);
}

value_id value_id::from_name_basis
    (std::string const& name
    ,std::string const* basis
    )
{
    return value_id(name, basis);
}

value_id value_id::from_name_basis
    (std::string const& name
    ,enum_run_basis basis
    )
{
    return value_id(name, &suffixes[basis]);
}

value_id::value_id()
    :basis_(NULL)
{
}

value_id::value_id(std::string const& name, std::string const* basis)
    :name_(name)
    ,basis_(basis)
{
}

value_id::value_id(value_id const& rhs)
    :name_(rhs.name_)
    ,basis_(rhs.basis_)
{
}

bool value_id::empty() const
{
    return name_.empty();
}

bool string_ends_with(std::string const& name, std::string const& suffix)
{
    if(name.length() < suffix.length())
        {
        return false;
        }

    return (0 == name.compare(name.length() - suffix.length(), suffix.length(), suffix));
}

/// Convert an old value identifier from report_column_NAMES
/// (in mc_enum_types.xpp) into a value_id.
///
/// This function looks for a suffix it knows and, if it is found,
/// strips it and adds the corresponding basis value.
///
/// The special value "[none]" is converted into an empty name.

value_id value_id::from_report_column_title(std::string const& title)
{
    typedef std::map<std::string, e_run_basis> suffix_map_t;
    static suffix_map_t suffix_map;
    if(suffix_map.empty())
    {
        suffix_map["_Current"       ] = e_run_curr_basis;
        suffix_map["_Guaranteed"    ] = e_run_guar_basis;
        suffix_map["_Midpoint"      ] = e_run_mdpt_basis;
        suffix_map["_CurrentZero"   ] = e_run_curr_basis_sa_zero;
        suffix_map["_GuaranteedZero"] = e_run_guar_basis_sa_zero;
        suffix_map["_CurrentHalf"   ] = e_run_curr_basis_sa_half;
        suffix_map["_GuaranteedHalf"] = e_run_guar_basis_sa_half;
    }

    for
        (suffix_map_t::const_iterator it = suffix_map.begin()
        ;it != suffix_map.end()
        ;++it
        )
        {
        if(string_ends_with(title, it->first))
            {
            return value_id::from_name_basis
                    (title.substr
                        (0
                        ,title.length() - it->first.length()
                        )
                    ,it->second
                    );
            }
        }

    // the magic "[none]" means no value at all
    if(title == "[none]")
        {
        return value_id::empty_value();
        }

    return value_id::from_name(title);
}

value_id value_id::from_xml_element(xml::element const& element)
{
    value_id id;
    id.get_from_xml_element(element);
    return id;
}

void value_id::set_to_xml_element(xml::element& element) const
{
    if(!empty())
        {
        xml_lmi::set_attr(element, "name", name());
        if(basis())
            {
            xml_lmi::set_attr(element, "basis", *basis());
            }
        }
}

void value_id::get_from_xml_element(xml::element const& element)
{
    name_ = "";
    basis_ = NULL;

    if(!xml_lmi::get_attr(element, "name", name_))
        {
        return;
        }

    std::string basis_value;
    if(!xml_lmi::get_attr(element, "basis", basis_value))
        {
        return;
        }

    string_vector_t::const_iterator it = std::find
        (suffixes.begin()
        ,suffixes.end()
        ,basis_value
        );
    if(it != suffixes.end())
        {
        basis_ = &*it;
        }
}

bool value_id::operator<(value_id const& rhs) const
{
    int names_compared = name_.compare(rhs.name_);

    if(names_compared < 0)
        {
        return true;
        }
    if(0 == names_compared)
        {
        return basis_ < rhs.basis_;
        }

    return false;
}

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
/// At initialization read column formats from an external xml file.
///
/// TODO ?? CALCULATION_SUMMARY Get rid of hardcoded formats: instead,
/// specify precision and scaling separately.
///
/// If SHOW_MISSING_FORMATS is defined, write all missing formats
/// to a 'missing_formats' file.
///
/// TODO ?? Why not do something like SHOW_MISSING_FORMATS does by
/// default, but make it a runtime warning instead, or validate it in
/// 'make cvs_ready'?
///
/// TODO ?? CALCULATION_SUMMARY If the external xml file should be
/// validated, then do that automatically, e.g. in 'make cvs_ready'.

class double_formatter_t
{
  public:
    double_formatter_t();

    bool            has_format(value_id const&) const;
    std::string     format
        (value_id const&
        ,double d
        ,enum_xml_version
        ) const;
    string_vector_t format
        (value_id const&
        ,double_vector_t const& dv
        ,enum_xml_version
        ) const;

    void add_columns_to_format(std::vector<value_id> const& columns);
    bool wants_column(value_id const& id) const;

  private:
    // Double conversion units
    // It is a <double coefficient, string suffix>.
    typedef std::pair<double,std::string> unit_t;
    // Double to string conversion format.
    // It is notionally <int precision, unit_t units>.
    typedef std::pair<int,unit_t> format_t;

    // Map value name to the corresponding format
    typedef std::map<std::string, format_t> format_map_t;

    // Set of calculation summary names
    typedef std::set<std::string> calculation_summary_fields_t;

    format_map_t format_map;
    calculation_summary_fields_t cs_set;

    format_t get_format(value_id const&) const;
    std::string do_format(double d, format_t const& f) const;

#if defined SHOW_MISSING_FORMATS
    static std::string missing_formats_filename()
    {
        return "missing_formats";
    }
#endif // defined SHOW_MISSING_FORMATS
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
    std::ofstream ofs(missing_formats_filename().c_str(), ios_out_trunc_binary());
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

    // Read formatting information from an external xml file.
    std::string format_path;
    try
    {
        format_path = AddDataDir
            (configurable_settings::instance().xslt_format_xml_filename()
            );
    }
    catch(fs::filesystem_error const& e)
    {
        fatal_error()
            << "Invalid xml file '"
            << configurable_settings::instance().xslt_format_xml_filename()
            << "' specified. "
            << e.what()
            << LMI_FLUSH
            ;
    }

    try
        {
        xml_lmi::dom_parser parser(format_path);

        xml::element const& root_node = parser.root_node("columns");
        xml_lmi::ElementContainer const columns
            (xml_lmi::child_elements(root_node, "column")
            );
        typedef xml_lmi::ElementContainer::const_iterator eci;
        for(eci it = columns.begin(); it != columns.end(); ++it)
            {
            value_id id = value_id::from_xml_element(**it);

            // a 'column' node has to have @name attribute
            if(id.empty())
                {
                continue;
                }

            xml_lmi::ElementContainer const formats
                (xml_lmi::child_elements(**it, "format")
                );

            // skip nodes without format information
            if(formats.empty())
                {
                continue;
                }

            xml::element const& format_element = *formats[0];

            // format has already been specified. show a warning and continue
            if(format_map.find(id.name()) != format_map.end())
                {
                warning()
                    << "Formats file '"
                    << format_path
                    << "' contains more than one format definition for '"
                    << id.name()
// TODO ?? CALCULATION_SUMMARY Consider adding this to xmlwrapp.
#if 0
                    << "' on line "
                    << format_element.get_line()
#endif // 0
                    << "."
                    << LMI_FLUSH
                    ;
                }

            std::string const format_name = xml_lmi::get_content(format_element);

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
            format_map[id.name()] = known_formats[format_name];
            }

        if(format_map.empty())
            {
            std::ostringstream oss;
            oss
                << "Could not read any format definitions from '"
                << format_path
                << "'. File is empty or has invalid format."
                ;
            throw std::runtime_error(oss.str());
            }
        }
    catch(std::exception const& e)
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

void double_formatter_t::add_columns_to_format
    (std::vector<value_id> const& columns
    )
{
    for
        (std::vector<value_id>::const_iterator it = columns.begin()
        ;it != columns.end()
        ;++it
        )
        {
        cs_set.insert(it->name());
        }
}

bool double_formatter_t::has_format(value_id const& id) const
{
    format_map_t::const_iterator it = format_map.find(id.name());
    if(it == format_map.end())
        {
#ifdef SHOW_MISSING_FORMATS
        std::ofstream ofs("missing_formats", ios_out_app_binary());
        ofs << id.name() << "\n";
#endif // defined SHOW_MISSING_FORMATS
        return false;
        }
    return true;
}

double_formatter_t::format_t
double_formatter_t::get_format(value_id const& id) const
{
    format_map_t::const_iterator it = format_map.find(id.name());
    if(it == format_map.end())
        {
        fatal_error()
            << "Unknown column name '"
            << id.name()
            << "' encountered."
            << LMI_FLUSH
            ;
        // use default format
        return format_t(2, unit_t(1., ""));
        }
    return it->second;
}

std::string double_formatter_t::format
    (value_id const& id
    ,double d
    ,enum_xml_version // not used currently
    ) const
{
    format_t f = get_format(id);

    return do_format(d, f);
}

bool double_formatter_t::wants_column(value_id const& id) const
{
    return cs_set.find(id.name()) != cs_set.end();
}

string_vector_t double_formatter_t::format
    (value_id const& id
    ,double_vector_t const& dv
    ,enum_xml_version xml_version
    ) const
{
    string_vector_t sv;

    // Include column only if it is a full xml version or if the column is in
    // the calculation summary.
    if(xml_version == e_xml_full || wants_column(id))
        {
        format_t f = get_format(id);
        for(double_vector_t::const_iterator it = dv.begin(),
                                           end = dv.end();
                                           it != end; ++it)
            {
            sv.push_back( do_format(*it, f) );
            }
        return sv;
        }

    static std::string const zero = "0";
    sv.resize(dv.size(), zero);
    return sv;
}

std::string double_formatter_t::do_format(double d, format_t const& f) const
{
    std::stringstream interpreter;
    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    interpreter.imbue(new_loc);
    interpreter.setf(std::ios_base::fixed, std::ios_base::floatfield);
    interpreter.precision(f.first);
    std::string s;
    unit_t const& units = f.second;
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

} // Unnamed namespace.

// TODO ?? Consider factoring out everything above into a separate file.

void Ledger::writeXXX(xml::element& illustration) const
{
    // by default generate a complete version of xml data
    write_excerpt(illustration, e_xml_full);
}

void Ledger::write_excerpt
    (xml::element&    illustration
    ,enum_xml_version xml_version
    ) const
{
    // initialize number formatting facility
    double_formatter_t formatter;

    // Generate the supplemental report column list
    std::vector<value_id> supplemental_report_columns;
    if(ledger_invariant_->SupplementalReport)
        {
        // prepare supplement report column list
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn00));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn01));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn02));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn03));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn04));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn05));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn06));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn07));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn08));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn09));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn10));
        supplemental_report_columns.push_back(value_id::from_report_column_title(ledger_invariant_->SupplementalReportColumn11));
        }

    // TODO ?? CALCULATION_SUMMARY EVGENIY--I think it would be better
    // to move this to 'configurable_settings.cpp', perhaps adding it
    // to effective_calculation_summary_columns(); would that work?
    std::vector<value_id> required_columns;
    required_columns.push_back(value_id::from_name("PolicyYear"));
    formatter.add_columns_to_format(required_columns);

    std::vector<value_id> calculation_summary_columns;
    try
        {
        std::istringstream iss(effective_calculation_summary_columns());
        std::vector<std::string> columns;
        // split column names list by spaces and put int columns vector
        std::copy
            (std::istream_iterator<std::string>(iss)
            ,std::istream_iterator<std::string>()
            ,std::back_inserter(columns)
            );
        std::vector<std::string>::const_iterator it = columns.begin();
        for(; it != columns.end(); ++it)
            {
            try
                {
                value_id id = value_id::from_report_column_title(*it);
                calculation_summary_columns.push_back(id);
                }
            catch(std::exception const& ex)
                {
                std::ostringstream oss;
                oss
                    << "Invalid column name '"
                    << *it
                    << "' ("
                    << ex.what()
                    << ")"
                    ;
                throw std::runtime_error(oss.str());
                }
            }

        formatter.add_columns_to_format(calculation_summary_columns);
        }
    catch(std::exception const& e)
        {
        warning()
            << "Cannot read calculation summary columns "
            << "from 'configurable_settings.xml'. Error: "
            << e.what()
            << LMI_FLUSH
            ;
        }

    // If an xml "microcosm" is desired, then use this hardcoded
    // list of columns.
    //
    // TODO ?? Find a better way: e.g., read the list from an
    // appropriate xml file.

    if(xml_version == e_xml_detailed)
        {
        std::vector<value_id> detailed_ids;
        detailed_ids.push_back(value_id::from_name("DBOpt"));
        detailed_ids.push_back(value_id::from_name("EeGrossPmt"));
        detailed_ids.push_back(value_id::from_name("ErGrossPmt"));
        detailed_ids.push_back(value_id::from_name("NetWD"));
        detailed_ids.push_back(value_id::from_name("NewCashLoan"));
        detailed_ids.push_back(value_id::from_name_basis("TotalLoanBalance", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name("Outlay"));

        detailed_ids.push_back(value_id::from_name_basis("NetPmt", e_run_curr_basis));

        detailed_ids.push_back(value_id::from_name_basis("PremTaxLoad", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("DacTaxLoad", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("PolicyFee", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("SpecAmtLoad", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name("MonthlyFlatExtra"));
        detailed_ids.push_back(value_id::from_name_basis("COICharge", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("NetCOICharge", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("SepAcctLoad", e_run_curr_basis));

        detailed_ids.push_back(value_id::from_name_basis("AnnSAIntRate", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("AnnGAIntRate", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("GrossIntCredited", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("NetIntCredited", e_run_curr_basis));

        detailed_ids.push_back(value_id::from_name_basis("AcctVal", e_run_guar_basis));
        detailed_ids.push_back(value_id::from_name_basis("CSVNet", e_run_guar_basis));
        detailed_ids.push_back(value_id::from_name_basis("EOYDeathBft", e_run_guar_basis));
        detailed_ids.push_back(value_id::from_name_basis("AcctVal", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("CSVNet", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("EOYDeathBft", e_run_curr_basis));

        detailed_ids.push_back(value_id::from_name("IrrOnSurrender"));
        detailed_ids.push_back(value_id::from_name("IrrOnDeath"));

        detailed_ids.push_back(value_id::from_name("InforceLives"));

        detailed_ids.push_back(value_id::from_name_basis("ClaimsPaid", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("NetClaims", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("ExperienceReserve", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("ProjectedCoiCharge", e_run_curr_basis));
        detailed_ids.push_back(value_id::from_name_basis("KFactor", e_run_curr_basis));

        detailed_ids.push_back(value_id::from_name_basis("NetCOICharge", e_run_curr_basis_sa_zero));
        detailed_ids.push_back(value_id::from_name_basis("NetClaims", e_run_curr_basis_sa_zero));
        detailed_ids.push_back(value_id::from_name_basis("ExperienceReserve", e_run_curr_basis_sa_zero));
        detailed_ids.push_back(value_id::from_name_basis("ProjectedCoiCharge", e_run_curr_basis_sa_zero));
        detailed_ids.push_back(value_id::from_name_basis("KFactor", e_run_curr_basis_sa_zero));

        detailed_ids.push_back(value_id::from_name("ProducerCompensation"));
        formatter.add_columns_to_format(detailed_ids);
        }

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
    if
        (  xml_version == e_xml_full
        || formatter.wants_column(value_id::from_name("IrrCsv"))
        || formatter.wants_column(value_id::from_name("IrrDb"))
        )
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

// TODO ?? An attained-age column is meaningless in a composite. So
// are several others--notably those affected by partial mortaility.
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

    std::string LmiVersion(LMI_VERSION);
    calendar_date prep_date;

    // Skip authentication for non-interactive regression testing.
    if(!global_settings::instance().regression_testing())
        {
        authenticate_system();
        }
    else
        {
        // For regression tests,
        //   - use an invariant string as version
        //   - use EffDate as date prepared
        // in order to avoid gratuitous failures.
        LmiVersion = "Regression testing";
        prep_date.julian_day_number(static_cast<int>(ledger_invariant_->EffDateJdn));
        }

    strings["LmiVersion"] = &LmiVersion;

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

    // Maps to hold the numeric data.

    typedef std::map<value_id, double>            double_scalar_map_t;
    typedef std::map<value_id, double_vector_t >  double_vector_map_t;
    typedef std::map<value_id, std::string>       string_scalar_map_t;
    typedef std::map<value_id, string_vector_t >  string_vector_map_t;

    double_scalar_map_t double_scalars;
    double_vector_map_t double_vectors;
    string_scalar_map_t string_scalars;
    string_vector_map_t string_vectors;

    string_vectors[value_id::from_name("FundNames")]
        = ledger_invariant_->FundNames;

    // Map the data, formatting it as necessary.

    // First we'll get the invariant stuff--the copy we made,
    // along with all the stuff we plugged into it above.

    for
        (scalar_map::const_iterator j = scalars.begin()
        ;j != scalars.end()
        ;++j
        )
        {
        double_scalars[value_id::from_report_column_title(j->first)] = *j->second;
        }
    for
        (string_map::const_iterator j = strings.begin()
        ;j != strings.end()
        ;++j
        )
        {
        string_scalars[value_id::from_report_column_title(j->first)] = *j->second;
        }
    for
        (double_vector_map::const_iterator j = vectors.begin()
        ;j != vectors.end()
        ;++j
        )
        {
        double_vectors[value_id::from_report_column_title(j->first)] = *j->second;
        }

//    doublescalars[value_id::from_name("GuarMaxMandE")] = *scalars[value_id::from_name("GuarMaxMandE")];
//    stringvectors[value_id::from_name("CorridorFactor")] = double_vector_to_string_vector(*vectors[value_id::from_name("CorridorFactor")]);
//    doublescalars[value_id::from_name_basis("InitAnnGenAcctInt", e_run_curr_basis)] = *scalars[value_id::from_name_basis("InitAnnGenAcctInt", e_run_curr_basis)];

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
            double_scalars[value_id::from_name_basis(j->first, i->first)]
                = *j->second;
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
            double_vectors[value_id::from_name_basis(j->first, i->first)]
                = *j->second;
            }
        }

    string_vectors[value_id::from_name("EeMode")]
        = enum_vector_to_string_vector( ledger_invariant_->EeMode );
    string_vectors[value_id::from_name("ErMode")]
        = enum_vector_to_string_vector( ledger_invariant_->ErMode );
    string_vectors[value_id::from_name("DBOpt")]
        = enum_vector_to_string_vector( ledger_invariant_->DBOpt  );

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
        xml::element string_scalar("string_scalar", j->second.c_str());
        j->first.set_to_xml_element(string_scalar);
        illustration.push_back(string_scalar);
        }
    // double scalars
    for
        (double_scalar_map_t::const_iterator j = double_scalars.begin()
        ;j != double_scalars.end()
        ;++j
        )
        {
        value_id const& id = j->first;
        if(formatter.has_format(id))
            {
            std::string f = formatter.format(id, j->second, xml_version);
            xml::element double_scalar("double_scalar", f.c_str());
            id.set_to_xml_element(double_scalar);
            illustration.push_back(double_scalar);
            }
        }
    // vectors of strings
    for
        (string_vector_map_t::const_iterator j = string_vectors.begin()
        ;j != string_vectors.end()
        ;++j
        )
        {
        xml::element svector("string_vector");

        j->first.set_to_xml_element(svector);

        string_vector_t const& v = j->second;
        for
            (string_vector_t::const_iterator k = v.begin()
            ;k != v.end()
            ;++k
            )
            {
            xml_lmi::add_node(svector, "duration", *k);
            }
        illustration.push_back(svector);
        }
    // vectors of doubles
    for
        (double_vector_map_t::const_iterator j = double_vectors.begin()
        ;j != double_vectors.end()
        ;++j
        )
        {
        value_id const& id = j->first;
        if(formatter.has_format(id))
            {
            xml::element dvector("double_vector");

            id.set_to_xml_element(dvector);

            string_vector_t v
                = formatter.format(id, j->second, xml_version);
// TODO ?? InforceLives shows an extra value past the end; should it
// be truncated here?
            for
                (string_vector_t::const_iterator k = v.begin()
                ;k != v.end()
                ;++k
                )
                {
                xml_lmi::add_node(dvector, "duration", *k);
                }
            illustration.push_back(dvector);
            }
        }

    // insert calculation_summary_columns list into xml
    if(!calculation_summary_columns.empty())
        {
        xml::element calculation_summary("calculation_summary_columns");
        std::vector<value_id>::const_iterator j;
        for
            (j = calculation_summary_columns.begin()
            ;j !=  calculation_summary_columns.end()
            ;++j
            )
            {
            xml::element column("column");
            j->set_to_xml_element(column);
            calculation_summary.push_back(column);
            }
        illustration.push_back(calculation_summary);
        }

// EVGENIY You had changed /supplementalreport/supplemental_report/,
// but we have stylesheets that don't expect any such change.
    xml::element supplemental_report("supplementalreport");
    if(ledger_invariant_->SupplementalReport)
        {
        // now pop back trailing empty supplemental report columns
        while
            (!supplemental_report_columns.empty()
            && supplemental_report_columns.back().empty()
            )
            {
            supplemental_report_columns.pop_back();
            }

#if 0 // TODO ?? CALCULATION_SUMMARY Clean this up.
// EVGENIY The original code added a supplemental-report element
// unconditionally, outside this conditional block. Making it
// conditional might break existing stylesheets.
        // now put this information into the xml
        xml::element& supplemental_report = *illustration.add_child
            ("supplemental_report"
            );
#endif // 0

        // Eventually customize the report name.
        xml_lmi::add_node(supplemental_report, "title", "Supplemental Report");

        std::vector<value_id>::const_iterator j;
        for
            (j = supplemental_report_columns.begin()
            ;j != supplemental_report_columns.end()
            ;++j
            )
            {
            if(j->empty())
                {
                xml::element spacer("spacer");
                supplemental_report.push_back(spacer);
                }
            else // *j != "[None]"
                {
                xml::element column("column");
                j->set_to_xml_element(column);
                supplemental_report.push_back(column);
                }
            }
        }
    illustration.push_back(supplemental_report);
}

#if defined LMI_USE_NEW_REPORTS
int Ledger::class_version() const
{
    return 0;
}

std::string Ledger::xml_root_name() const
{
    return "illustration";
}
#endif // defined LMI_USE_NEW_REPORTS

void Ledger::writeXXX(std::ostream& os) const
{
    xml_lmi::xml_document document(xml_root_name());
    xml::element& root = document.root_node();
    root << *this;

    std::string const lmi_xml_ns("http://savannah.nongnu.org/projects/lmi");

    configurable_settings const& z = configurable_settings::instance();
    std::string const xml_schema = lmi_xml_ns + " " + z.xml_schema_filename();

// TODO ?? CALCULATION_SUMMARY XMLWRAPP !! Consider adding namespace
// support to xmlwrapp.
#if 0
    std::string const w3("http://www.w3.org/2001/XMLSchema-instance");

    root.set_namespace_declaration(lmi_xml_ns);
    root.set_namespace_declaration(w3, "xsi");
    xml_lmi::set_attr(root, "noNamespaceSchemaLocation", xml_schema, "xsi");
#endif // 0
    xml_lmi::set_attr(root, "noNamespaceSchemaLocation", xml_schema);

    os << document;
}

