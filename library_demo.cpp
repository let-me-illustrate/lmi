// Demonstration of life insurance calculations.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: library_demo.cpp,v 1.1 2005-01-14 19:47:45 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "alert.hpp"
#include "calendar_date.hpp"
#include "ihs_acctval.hpp"
#include "ihs_ledger.hpp"
#include "inputillus.hpp"
#include "single_cell_document.hpp"
#include "timer.hpp"

#include <fstream>
#include <ios>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <string>

class IllustrationDocument
{
  public:
    explicit IllustrationDocument(std::string const&);
    IllusInputParms* input_parameters() const;

  private:
    single_cell_document doc;
};

IllustrationDocument::IllustrationDocument(std::string const& filename)
{
    std::ifstream ifs(filename.c_str());
//    ifs >> doc;
// TODO ?? Why not offer doc.read(filename)?
// TODO ?? Would a vector<char> help?
    doc.read(ifs);
/*
    ifs >> std::noskipws;
    std::string s
        ((std::istreambuf_iterator<char>(ifs))
        , std::istreambuf_iterator<char>()
        );
std::cout << s.size() << std::endl;
    ifs >> s;
std::cout << s.size() << std::endl;
    if(!ifs.eof())
        {
        throw std::runtime_error("Unable to read input file into string.");
        }
*/
}

IllusInputParms* IllustrationDocument::input_parameters() const
{
    return doc.input_data.get();
}

void test()
{
    warning() << "Warning from main()." << LMI_FLUSH;

    std::cout
        << "Should be 0 1 0 1:\n"
        << calendar_date(1900,  1,  1).is_leap_year() << '\n'
        << calendar_date(2000,  1,  1).is_leap_year() << '\n'
        << calendar_date(2003,  1,  1).is_leap_year() << '\n'
        << calendar_date(2004,  1,  1).is_leap_year() << '\n'
        << std::endl
        ;

    Timer timer;

    IllustrationDocument document("foo.ill");

    warning() << "Read input file: " << timer.Stop().Report() << std::flush;
    timer.Reset().Start();

    IllusInputParms input = *document.input_parameters();
    AccountValue av(&input);
    av.SetDebugFilename("foo.debug");
    av.RunAV();

    warning() << "Calculate: " << timer.Stop().Report() << std::flush;
    timer.Reset().Start();

    TLedger ledger = av.GetLedgerValues();
    std::ofstream ofs("eraseme.xml", std::ios_base::out | std::ios_base::trunc);
    ledger.write(ofs);
    warning() << "Generate and write xml output: " << timer.Stop().Report() << std::flush;

    ledger.Spew(std::cerr);
}

int main()
{
    std::cout << "Testing shared library." << std::endl;
    try
        {
// TODO ?? This catches an exception that's not what was intended.
//        fatal_error() << "Fatal error from main()." << LMI_FLUSH;
        test();
        }
    catch(std::exception const& e)
        {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    catch(...)
        {
        std::cerr << "Uncaught exception...terminating." << std::endl;
        }
}

