// Sole top-level facility for illustration generation.
//
// Copyright (C) 2007, 2008 Gregory W. Chicares.
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

// $Id: illustrator.cpp,v 1.26 2008-08-03 13:05:23 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "illustrator.hpp"

#include "alert.hpp"
#include "configurable_settings.hpp"
#include "custom_io_0.hpp"
#include "emit_ledger.hpp"
#include "group_values.hpp"
#include "handle_exceptions.hpp"
#include "inputillus.hpp"
#include "ledgervalues.hpp"
#include "multiple_cell_document.hpp"
#include "platform_dependent.hpp" // access()
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>

#include <cstdio>                 // std::remove()
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

illustrator::illustrator(mcenum_emission emission)
    :emission_              (emission)
    ,usec_for_input_        (0.0)
    ,usec_for_calculations_ (0.0)
    ,usec_for_output_       (0.0)
{
}

illustrator::~illustrator()
{
}

bool illustrator::operator()(fs::path const& file_path)
{
    bool completed_normally = true;
    std::string const extension = fs::extension(file_path);
    if(".cns" == extension)
        {
        Timer timer;
        multiple_cell_document doc(file_path.string());
        run_census::assert_consistency(doc.case_parms()[0], doc.cell_parms()[0]);
        usec_for_input_ = timer.stop().elapsed_usec();
        census_run_result result;
        result = run_census()(file_path, emission_, doc.cell_parms());
        completed_normally     = result.completed_normally_   ;
        usec_for_calculations_ = result.usec_for_calculations_;
        usec_for_output_       = result.usec_for_output_      ;
        }
    else if(".ill" == extension)
        {
        Timer timer;
        single_cell_document doc(file_path.string());
        usec_for_input_        = timer.stop().elapsed_usec();
        timer.restart();
        IllusVal IV;
        IV.run(doc.input_data());
        usec_for_calculations_ = timer.stop().elapsed_usec();
        usec_for_output_ = emit_ledger(file_path, 0, IV.ledger(), emission_);
        }
    else if(".ini" == extension)
        {
        Timer timer;
        IllusInputParms input;
        custom_io_0_read(input, file_path.string());
        usec_for_input_ = timer.stop().elapsed_usec();
        timer.restart();
        IllusVal z;
        z.run(input);
        usec_for_calculations_ = timer.stop().elapsed_usec();
        fs::path out_file = file_path;
        if(!file_path.string().empty())
            {
            out_file = fs::change_extension(file_path, ".test0");
            }
        usec_for_output_ = emit_ledger(out_file, 0, z.ledger(), emission_);
        }
    else
        {
        fatal_error()
            << "File '"
            << file_path.string()
            << "': extension '"
            << extension
            << "' not supported."
            << LMI_FLUSH
            ;
        }

    if(mce_emit_timings & emission_)
        {
        std::cout
            << "\n    Input:        "
            << Timer::elapsed_msec_str(usec_for_input_)
            << "\n    Calculations: "
            << Timer::elapsed_msec_str(usec_for_calculations_)
            << "\n    Output:       "
            << Timer::elapsed_msec_str(usec_for_output_)
            << '\n'
            ;
        }

    return completed_normally;
}

double illustrator::usec_for_input() const
{
    return usec_for_input_;
}

double illustrator::usec_for_calculations() const
{
    return usec_for_calculations_;
}

double illustrator::usec_for_output() const
{
    return usec_for_output_;
}

IllusInputParms const& default_cell()
{
    static IllusInputParms const builtin_default;
    static IllusInputParms       user_default;

    std::string const default_input_file =
        configurable_settings::instance().default_input_filename()
        ;
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

template<> void temporary_file_kludge(std::vector<IllusInputParms> const& z)
{
    multiple_cell_document document;
    typedef std::vector<IllusInputParms> T;
    T& cells = const_cast<T&>(document.cell_parms());
    cells = z;
    std::ofstream ofs("eraseme.cns");
    document.write(ofs);
}

template<> void temporary_file_kludge(IllusInputParms const& z)
{
    single_cell_document document;
    typedef IllusInputParms T;
    T& cell = const_cast<T&>(document.input_data());
    cell = z;
    std::ofstream ofs("eraseme.ill");
    document.write(ofs);
}

