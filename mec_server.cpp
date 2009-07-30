// MEC-testing server.
//
// Copyright (C) 2009 Gregory W. Chicares.
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

// $Id: mec_server.cpp,v 1.1 2009-07-30 21:46:13 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mec_server.hpp"

#include "alert.hpp"
#include "mec_input.hpp"
#include "mec_xml_document.hpp"
#include "path_utility.hpp"       // fs::path inserter
#include "timer.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>

#include <iostream>
#include <string>

namespace
{
mec_state test_one_days_7702A_transactions
    (fs::path  const& // file_path
    ,mec_input const& // input
    )
{
    // Real implementation coming soon....
    return mec_state();
}
} // Unnamed namespace.

mec_server::mec_server(mcenum_emission emission)
    :emission_              (emission)
    ,usec_for_input_        (0.0)
    ,usec_for_calculations_ (0.0)
    ,usec_for_output_       (0.0)
{
}

mec_server::~mec_server()
{
}

bool mec_server::operator()(fs::path const& file_path)
{
    std::string const extension = fs::extension(file_path);
    if(".mec" == extension)
        {
        Timer timer;
        fs::ifstream ifs(file_path);
        if(!ifs)
            {
            fatal_error()
                << "Unable to read file '"
                << file_path
                << "'."
                << LMI_FLUSH
                ;
            }
        mec_xml_document doc;
        doc.read(ifs);
        usec_for_input_ = timer.stop().elapsed_usec();
        return operator()(file_path, doc.input_data());
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

bool mec_server::operator()(fs::path const& file_path, mec_input const& z)
{
    Timer timer;
    state_ = test_one_days_7702A_transactions(file_path, z);
    usec_for_calculations_ = timer.stop().elapsed_usec();
    usec_for_output_ = 0.0;
    conditionally_show_timings_on_stdout();
    return true;
}

void mec_server::conditionally_show_timings_on_stdout() const
{
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
}

mec_state mec_server::state() const
{
    return state_;
}

double mec_server::usec_for_input() const
{
    return usec_for_input_;
}

double mec_server::usec_for_calculations() const
{
    return usec_for_calculations_;
}

double mec_server::usec_for_output() const
{
    return usec_for_output_;
}

