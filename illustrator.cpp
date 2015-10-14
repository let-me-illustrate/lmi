// Sole top-level facility for illustration generation.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.
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

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "illustrator.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "custom_io_1.hpp"
#include "emit_ledger.hpp"
#include "group_values.hpp"
#include "handle_exceptions.hpp"
#include "input.hpp"
#include "ledgervalues.hpp"
#include "multiple_cell_document.hpp"
#include "path_utility.hpp"             // fs::path inserter
#include "platform_dependent.hpp"       // access()
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>

#include <cstddef>                      // std::size_t
#include <cstdio>                       // std::remove()
#include <iostream>
#include <string>

illustrator::illustrator(mcenum_emission emission)
    :emission_                 (emission)
    ,seconds_for_input_        (0.0)
    ,seconds_for_calculations_ (0.0)
    ,seconds_for_output_       (0.0)
{
}

illustrator::~illustrator()
{
}

bool illustrator::operator()(fs::path const& file_path)
{
    std::string const extension = fs::extension(file_path);
    if(".cns" == extension)
        {
        Timer timer;
        multiple_cell_document doc(file_path.string());
        assert_consistency_in_context(emission_, doc.case_parms()[0], doc.cell_parms());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        return operator()(file_path, doc.cell_parms());
        }
    else if(".ill" == extension)
        {
        Timer timer;
        single_cell_document doc(file_path.string());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        return operator()(file_path, doc.input_data());
        }
    else if(".ini" == extension)
        {
        Timer timer;
        Input input;
        bool close_when_done = custom_io_0_read(input, file_path.string());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        timer.restart();
        IllusVal z(file_path.string());
        z.run(input);
        principal_ledger_ = z.ledger();
        seconds_for_calculations_ = timer.stop().elapsed_seconds();
        seconds_for_output_ = emit_ledger(file_path, *z.ledger(), emission_);
        conditionally_show_timings_on_stdout();
        return close_when_done;
        }
    else if(".inix" == extension)
        {
        Timer timer;
        Input input;
        bool emit_pdf_too = custom_io_1_read(input, file_path.string());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        timer.restart();
        IllusVal z(file_path.string());
        z.run(input);
        principal_ledger_ = z.ledger();
        seconds_for_calculations_ = timer.stop().elapsed_seconds();
        mcenum_emission x = emit_pdf_too ? mce_emit_pdf_file : mce_emit_nothing;
        mcenum_emission y = static_cast<mcenum_emission>(x | emission_);
        seconds_for_output_ = emit_ledger(file_path, *z.ledger(), y);
        conditionally_show_timings_on_stdout();
        return true;
        }
    else
        {
        fatal_error()
            << "File '"
            << file_path
            << "': extension '"
            << extension
            << "' not supported."
            << LMI_FLUSH
            ;
        return false;
        }
}

bool illustrator::operator()(fs::path const& file_path, Input const& z)
{
    Timer timer;
    IllusVal IV(file_path.string());
    IV.run(z);
    principal_ledger_ = IV.ledger();
    seconds_for_calculations_ = timer.stop().elapsed_seconds();
    seconds_for_output_ = emit_ledger(file_path, *IV.ledger(), emission_);
    conditionally_show_timings_on_stdout();
    return true;
}

bool illustrator::operator()(fs::path const& file_path, std::vector<Input> const& z)
{
    census_run_result result;
    run_census runner;
    result = runner(file_path, emission_, z);
    principal_ledger_ = runner.composite();
    seconds_for_calculations_ = result.seconds_for_calculations_;
    seconds_for_output_       = result.seconds_for_output_      ;
    conditionally_show_timings_on_stdout();
    return result.completed_normally_;
}

void illustrator::conditionally_show_timings_on_stdout() const
{
    if(mce_emit_timings & emission_)
        {
        std::cout
            << "\n    Input:        "
            << Timer::elapsed_msec_str(seconds_for_input_)
            << "\n    Calculations: "
            << Timer::elapsed_msec_str(seconds_for_calculations_)
            << "\n    Output:       "
            << Timer::elapsed_msec_str(seconds_for_output_)
            << '\n'
            ;
        }
}

/// The "principal" ledger is the one most likely to be retained for
/// other uses, such as displaying in a GUI. For a single-cell
/// illustration, it's the one and only ledger. For a multiple-cell
/// illustration, it's the composite ledger.

boost::shared_ptr<Ledger const> illustrator::principal_ledger() const
{
    LMI_ASSERT(principal_ledger_.get());
    return principal_ledger_;
}

double illustrator::seconds_for_input() const
{
    return seconds_for_input_;
}

double illustrator::seconds_for_calculations() const
{
    return seconds_for_calculations_;
}

double illustrator::seconds_for_output() const
{
    return seconds_for_output_;
}

Input const& default_cell()
{
    static Input const builtin_default;
    static Input       user_default;

    configurable_settings const& c = configurable_settings::instance();
    std::string const& default_input_file = c.default_input_filename();
    if(0 != access(default_input_file.c_str(), F_OK))
        {
        user_default = builtin_default;
        }
    else
        {
        try
            {
            user_default = single_cell_document(default_input_file).input_data();
            }
        catch(...)
            {
            report_exception();
            if(0 == std::remove(default_input_file.c_str()))
                {
                warning()
                    << "Removed defective default input file '"
                    << default_input_file
                    << "'."
                    << LMI_FLUSH
                    ;
                }
            else
                {
                warning()
                    << "Unable to remove defective default input file '"
                    << default_input_file
                    << "'. Make sure it is not write protected."
                    << LMI_FLUSH
                    ;
                }
            }
        }

    return user_default;
}

/// Throw if run order for any cell does not match case default.
///
/// If lmi had case-only input fields, run order would be one of them.
///
/// If emission includes mce_emit_group_quote bit, also throw if any of the
/// fields used in the group quotes header or footer are not constant across
/// all cells and the case default and also throw if the company name is empty
/// as it is a required field for the group quotes.

void assert_consistency_in_context
    (mcenum_emission           emission
    ,Input              const& case_default
    ,std::vector<Input> const& cells
    )
{
    typedef std::vector<Input>::size_type svst;
    for(svst i = 0; i != cells.size(); ++i)
        {
        Input const& cell = cells[i];

        if(case_default["RunOrder"] != cell["RunOrder"])
            {
            fatal_error()
                << "Case-default run order '"
                << case_default["RunOrder"]
                << "' differs from run order '"
                << cell["RunOrder"]
                << "' of cell number "
                << 1 + i
                << ". Make this consistent before running illustrations."
                << LMI_FLUSH
                ;
            }

        if(emission & mce_emit_group_quote)
            {
            // Notice that the fields checked here correspond to the members of
            // LedgerInvariant class used in fill_global_report_data() function
            // in the group quote generation code and must be updated whenever
            // that code changes. The product name field is a proxy for all the
            // product-specific fields used as asserting that it is the same
            // for all cells is sufficient to ensure that the values of all
            // those fields are the same as well.
            static char const* const fields[] =
                {"ProductName"
                ,"CorporationName"
                ,"AgentName"
                ,"CorporationPaymentMode"
                ,"StateOfJurisdiction"
                ,"EffectiveDate"
                ,"Comments"
                };
            for(std::size_t j = 0; j != sizeof fields / sizeof(fields[0]); ++j)
                {
                char const* const field = fields[j];
                if(case_default[field] != cell[field])
                    {
                    fatal_error()
                        << "Value of the field '"
                        << field
                        << "' is '"
                        << case_default[field]
                        << "' in the case defaults but '"
                        << cell[field]
                        << "' for the cell #"
                        << 1 + i
                        << ". Make them identical before creating a group quote report."
                        << LMI_FLUSH
                        ;
                    }
                }
            }
        }

    if(emission & mce_emit_group_quote)
        {
        if(case_default["CorporationName"].str().empty())
            {
            fatal_error()
                << "Corporation name field is required in a group quote report."
                << "Ensure the name is non-empty before generating this report."
                << LMI_FLUSH
                ;
            }
        }
}
