// Run life insurance illustrations and composites.
//
// Copyright (C) 2001, 2002, 2004, 2005, 2006, 2007 Gregory W. Chicares.
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

// $Id: calculate.hpp,v 1.26 2007-06-04 01:57:30 chicares Exp $

#ifndef calculate_hpp
#define calculate_hpp

#include "config.hpp"

#include "inputillus.hpp"
#include "ledgervalues.hpp"
#include "miscellany.hpp"
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <fstream>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <string>

struct RunIllustration
    :public std::unary_function<IllusInputParms, void>
{
    explicit RunIllustration(std::ostream& aOutputDest)
        :OutputDest           (aOutputDest)
        ,time_for_calculations(0.0)
        ,time_for_output      (0.0)
        {}

    void operator()(IllusInputParms a_input)
        {
        Timer timer;
        IllusVal IV;
        IV.Run(&a_input);
        time_for_calculations += timer.stop().elapsed_usec();

        timer.restart();
        IV.Print(OutputDest);
        time_for_output       += timer.stop().elapsed_usec();
        }

    std::ostream& OutputDest;

    double time_for_calculations;
    double time_for_output;
};

struct RunIllustrationFromFile
    :public RunIllustration
{
    explicit RunIllustrationFromFile(std::ostream& aOutputDest)
        :RunIllustration(aOutputDest)
        ,time_for_input (0.0)
        {}
    void operator()(std::string a_filename)
        {
        Timer timer;
        std::ifstream is(a_filename.c_str(), ios_in_binary());
/* TODO ?? expunge
        IllusInputParms input;
        is >> input;
*/
        if(!is)
            {
            throw std::runtime_error("Unable to open file " + a_filename);
            }
        single_cell_document doc;
        is >> doc;
        IllusInputParms input(doc.input_data());
        time_for_input += timer.stop().elapsed_usec();
        RunIllustration::operator()(input);
        }
    double time_for_input;
};

#endif // calculate_hpp

