// Sole top-level facility for illustration generation.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: illustrator.cpp,v 1.5 2007-06-01 01:24:18 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "illustrator.hpp"

#include "alert.hpp"
#include "group_values.hpp"
#include "multiple_cell_document.hpp"
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>

#include <iostream>

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
    bool return_code = false;
    std::string const extension = fs::extension(file_path);
    if(".cns" == extension)
        {
        Timer timer;
        multiple_cell_document doc(file_path.string());
        run_census::assert_consistency(doc.case_parms()[0], doc.cell_parms()[0]);
        usec_for_input_ = timer.stop().elapsed_usec();
        if(mce_emit_timings & emission_)
            {
            std::cerr
                << "    Input:        "
                << Timer::elapsed_msec_str(usec_for_input_)
                << '\n'
                ;
            }
        return_code = run_census()(file_path, emission_, doc.cell_parms());
        }
// TODO ?? Implement these, too.
#if 0
    else if(".ill" == extension)
        {
// Eventual replacement for RegressionTestOneCensusFile().
        }
    else if(".ini" == extension)
        {
// Eventual replacement for RegressionTestOneIniFile().
        }
#endif // 0
    else
        {
        warning()
            << "File '"
            << file_path.string()
            << "': extension '"
            << extension
            << "' not supported."
            << LMI_FLUSH
            ;
        }
    return return_code;
}

