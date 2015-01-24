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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
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
#include "path_utility.hpp"       // fs::path inserter
#include "platform_dependent.hpp" // access()
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>

#include <cstdio>                 // std::remove()
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
        assert_consistency(doc.case_parms()[0], doc.cell_parms()[0]);
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
        configurable_settings const& c = configurable_settings::instance();
        Timer timer;
        Input input;
        bool close_when_done = custom_io_0_read(input, file_path.string());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        timer.restart();
        IllusVal z(file_path.string());
        z.run(input);
        principal_ledger_ = z.ledger();
        seconds_for_calculations_ = timer.stop().elapsed_seconds();
        fs::path out_file =
            file_path.string() == c.custom_input_0_filename()
            ? c.custom_output_0_filename()
            : fs::change_extension(file_path, ".test0")
            ;
        seconds_for_output_ = emit_ledger
            (out_file
            ,out_file
            ,*z.ledger()
            ,emission_
            );
        conditionally_show_timings_on_stdout();
        return close_when_done;
        }
    else if(".inix" == extension)
        {
        configurable_settings const& c = configurable_settings::instance();
        Timer timer;
        Input input;
        bool emit_pdf_too = custom_io_1_read(input, file_path.string());
        seconds_for_input_ = timer.stop().elapsed_seconds();
        timer.restart();
        IllusVal z(file_path.string());
        z.run(input);
        principal_ledger_ = z.ledger();
        seconds_for_calculations_ = timer.stop().elapsed_seconds();
        fs::path out_file =
            file_path.string() == c.custom_input_1_filename()
            ? c.custom_output_1_filename()
            : fs::change_extension(file_path, ".test1")
            ;
        mcenum_emission x = emit_pdf_too ? mce_emit_pdf_file : mce_emit_nothing;
        seconds_for_output_ = emit_ledger
            (out_file
            ,out_file
            ,*z.ledger()
            ,static_cast<mcenum_emission>(x | emission_)
            );
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
    seconds_for_output_ = emit_ledger
        (file_path
        ,file_path
        ,*IV.ledger()
        ,emission_
        );
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
    std::string const default_input_file = c.default_input_filename();
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

/// Throw if an inconsistency is detected between a cell and its
/// corresponding case default.
///
/// The run order depends on the first cell's parameters and ignores
/// any conflicting input for any individual cell. It might be cleaner
/// to offer this field (and certain others) only at the case level.
///
/// TODO ?? Instead, this should be enforced when data is entered.

void assert_consistency
    (Input const& case_default
    ,Input const& cell
    )
{
    if(case_default["RunOrder"] != cell["RunOrder"])
        {
        fatal_error()
            << "Case-default run order '"
            << case_default["RunOrder"]
            << "' differs from first cell's run order '"
            << cell["RunOrder"]
            << "'. Make them consistent before running illustrations."
            << LMI_FLUSH
            ;
        }
}

