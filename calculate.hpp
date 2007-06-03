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

// $Id: calculate.hpp,v 1.24 2007-06-03 14:32:30 chicares Exp $

#ifndef calculate_hpp
#define calculate_hpp

#include "config.hpp"

#include "assert_lmi.hpp"
#include "inputillus.hpp"
#include "inputs.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_text_formats.hpp" // PrintLedgerFlatText()
#include "ledger_variant.hpp"
#include "ledgervalues.hpp"
#include "miscellany.hpp"
#include "multiple_cell_document.hpp"
#include "single_cell_document.hpp"
#include "timer.hpp"

// TODO ?? Apparently the original reason for using smart pointers
// was to minimize stack usage in a 16-bit environment; clearly that
// doesn't matter anymore.
#include <boost/scoped_ptr.hpp>

#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

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

struct RunCensus
    :public std::unary_function<std::vector<IllusInputParms>, void>
{
    explicit RunCensus(std::ostream& aOutputDest)
        :OutputDest           (aOutputDest)
// TODO ?? Hardcoded ledger type must be changed later, but is it?
        ,XXXComposite         ()
        ,time_for_calculations(0.0)
        ,time_for_output      (0.0)
        {}

    void operator()(std::vector<IllusInputParms> a_input)
        {
        if(a_input.empty())
            {
            return;
            }

        Timer timer;
        for
            (std::vector<IllusInputParms>::iterator lives_it = a_input.begin()
            ;lives_it != a_input.end()
            ;++lives_it
            )
            {
            boost::scoped_ptr<IllusVal> IV(new IllusVal());
            IV->Run(&*lives_it);
// TODO ?? Pick one:
//            Composite.operator+=(IV->ledger());
            XXXComposite.PlusEq(IV->ledger());
            }
        time_for_calculations += timer.stop().elapsed_usec();

        timer.restart();
        PrintLedgerFlatText(XXXComposite, OutputDest);
        time_for_output       += timer.stop().elapsed_usec();
        }

    std::ostream& OutputDest;

    Ledger XXXComposite;

    double time_for_calculations;
    double time_for_output;
};

#endif // calculate_hpp

