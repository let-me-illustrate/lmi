// Compare two regression-test '.crc' files.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_crc_comp.cpp,v 1.5 2005-05-27 10:37:06 chicares Exp $

// Sample command line to compile:
// /gcc-2.95.2-1/bin/g++ -Iming29521 ihs_crc_comp.cpp

#include "miscellany.hpp"
#include "value_cast.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// IHS's regression test facility emits files with extension .crc
// in a prescribed format. This program compares two such files
// and emits a report to standard output. The report quantifies
// any differences in floating-point values as
//   (observed - expected) / expected
// with a modification if expected is zero.
//
// .CRC FILE LAYOUT
//
// Regression test input census files have extension .cns .
// Regression testing is a 1-1 and onto map of .cns to .crc .
// A .crc file contains four types of lines:
//
// [1] "crc": zero-based index of cell, whitespace, 32-bit CRC
//   the composite is defined to have index -1
// regexp: ^[-0-9][0-9]*[ \t][-0-9][0-9]*$ TODO ?? Right?
// sed -e'/^[-0-9][0-9]*[[:blank:]][-0-9][0-9]*$/!d'
// examples:
//   0  2753575139
//   -1 1560388799
//
// [2] "name": name of a composite yearly variable
// regexp: ^[A-Za-z][0-9A-Za-z]*$ TODO ?? Right?
// example:
//   EeGrossPmt
// except that this regexp would also get the following values:
std::vector<std::string> const& get_special_type_3_not_2()
{
    int const n = 7;
    static std::string const c[n] =
        {"A"
        ,"B"
        ,"ROP"
        ,"Annual"
        ,"Semiannual"
        ,"Quarterly"
        ,"Monthly"
        };
    static std::vector<std::string> const s(c, c + n);
    return s;
}
// which are yearly elements instead. No other yearly quantity has
// a non-numeric value at this time.
//
// TODO ?? It would be more maintainable to include enums.hpp here, but
// that would force us to compile other .cpp files and link their
// object code. For now at least, I've chosen the straightforward
// approach.
//
// [3] "yearly": element of a vector of yearly values
// (generally floating point, but see get_special_type_3_not_2())
// regexp: ^[-0-9][.0-9]*$
// examples:
//   0
//   100000000.820000008
//
// [4] scalar: name and value of a scalar, separated by "=="
// regexp: ^[A-Za-z][0-9A-Za-z]*==[.0-9A-Za-z]$
// examples:
//   Age==0
//   GuarMaxMandE==0.0064999999999999997
//   PartMortTableName==1983 GAM
//
// ALGORITHM
//
// This program reads through a pair of .crc files, line by line.
// Define the "state" of the program in terms of the last line
// read to be any of [1-4] above, or state [0] "initial" if no
// line has yet been read, or [5] "final" if EOF is reached. It
// is a fatal error if the two input files have different states;
// otherwise, the state of the program is the (equal) state of
// both input files.
enum line_type
    {initial    = 0
    ,crc        = 1
    ,name       = 2
    ,yearly     = 3
    ,scalar     = 4
    ,final      = 5
    };
// States must follow this transition matrix mapping row -> col:
bool const transition_matrix[6][6] =
    {
    /*        to: 0  1  2  3  4  5 */
    /* from 0 */ {0, 1, 1, 0, 0, 0,},
    /* from 1 */ {0, 1, 1, 0, 0, 0,},
    /* from 2 */ {0, 0, 1, 1, 1, 1,},   // TODO ?? Think about 2 -> 2,4 some more.
    /* from 3 */ {0, 0, 1, 1, 1, 0,},
    /* from 4 */ {0, 0, 1, 0, 1, 0,},
    /* from 5 */ {0, 0, 0, 0, 0, 0,},
    };
// where (0) means prohibited and (1) means allowed. Any prohibited
// (0) transition is a fatal error. A function is defined to handle
// each allowable transition; its name is formed as
//   "f" + from-state + to-state
// so that a transition from state [1] to state [2] is named "f12".
//
// Functions are similarly defined to perform processing that is
// always required upon allowable entry into each non-initial
// state, independent of the from-state; their names are formed as
//   "f" + "_" + to-state

std::string current_name;
double max_abs_diff;
double max_rel_err;

//============================================================================
void show_error_context(std::string const& line1, std::string const& line2)
{
    std::cerr << "line1: " << line1 << "\nline2: " << line2 << '\n';
}


//============================================================================
line_type get_type(std::string const& line)
{
    if(line == "")
        {
        return final;
        }
    else if(std::string::npos != line.find("\t"))
        {
        return crc;
        }
    else if(std::string::npos != line.find("=="))
        {
        return scalar;
        }
    else if
        (
           line[0] == '-'
        || (is_ok_for_cctype(line[0]) && std::isdigit(line[0]))
        )
        {
        return yearly;
        }
    else if
        (get_special_type_3_not_2().end() != std::find
            (get_special_type_3_not_2().begin()
            ,get_special_type_3_not_2().end()
            ,line
            )
        )
        {
        return yearly;
        }
    else if(is_ok_for_cctype(line[0]) && std::isalpha(line[0]))
        {
        return name;
        }
    else
        {
        std::cerr << "Logic error in get_type()\n";
        std::cerr << "line: " << line << '\n';
        throw std::logic_error("Logic error in get_type().");
        }
}

//============================================================================
void f_1(std::string const& line1, std::string const& line2)
{
    if(line1 == line2)
        {
        return;
        }

    int i1;
    int c1;
    std::istringstream stream1(line1);
    stream1 >> i1 >> c1;

    int i2;
    int c2;
    std::istringstream stream2(line2);
    stream2 >> i2 >> c2;

    std::cout << "file 1: index " << i1 << " crc " << c1 << '\n';
    std::cout << "file 2: index " << i2 << " crc " << c2 << '\n';
}

//============================================================================
void f01(std::string const& line1, std::string const& line2)
{
    f_1(line1, line2);
}

//============================================================================
void f11(std::string const& line1, std::string const& line2)
{
    f_1(line1, line2);
}

//============================================================================
void f_2(std::string const& line1, std::string const& line2)
{
    if
        (   line1 != line2
// TODO ?? Fix this kludge, which strives to ignore fund names.
        &&  std::string::npos != line1.find(" ")
        )
        {
        return;
        }

    current_name = line1;
    if(line1 != line2)
        {
        std::cerr << "Logic error in f_2()\n";
        show_error_context(line1, line2);
        std::exit(EXIT_FAILURE);
        }
// error if different
// hold; print if >0 numbers differ
}

//============================================================================
void f02(std::string const& line1, std::string const& line2)
{
    f_2(line1, line2);
}

//============================================================================
void f12(std::string const& line1, std::string const& line2)
{
    f_2(line1, line2);
}

//============================================================================
// TODO ?? Think about this one some more.
void f22(std::string const& line1, std::string const& line2)
{
    f_2(line1, line2);
}

//============================================================================
void f32(std::string const& line1, std::string const& line2)
{
    f_2(line1, line2);
}

//============================================================================
void f42(std::string const& line1, std::string const& line2)
{
    f_2(line1, line2);
}

//============================================================================
void f_3(std::string const& line1, std::string const& line2)
{
// single floating-point number
// compare--chi square, heeding 0
// hold max chi sq

    if(line1 == line2)
        {
        return;
        }

    long double d1 = value_cast<long double>(line1);
    long double d2 = value_cast<long double>(line2);
    if(d1 == d2)
        {
        return;
        }

    double abs_diff = std::fabs(d1 - d2);
    max_abs_diff = std::max(max_abs_diff, abs_diff);

    double rel_err =
        std::fabs(
                (d1 - d2)
            /   ((0.0 == d1) ? d2 : d1)
            );
    max_rel_err = std::max(max_rel_err, rel_err);

    if(rel_err < 1.0E-11)
        {
        return;
        }

    std::cout
        << current_name
        << '\n';
    std::cout
        << std::setprecision(20)
        << rel_err
        << "  " << d1
        << " vs. " << d2
        << '\n';
}

//============================================================================
void f23(std::string const& line1, std::string const& line2)
{
    f_3(line1, line2);
}

//============================================================================
void f33(std::string const& line1, std::string const& line2)
{
    f_3(line1, line2);
}

//============================================================================
void f_4(std::string const& line1, std::string const& line2)
{
// initially set global basis = invariant

// first char alpha, has ==
// compare what follows ==; print iff different

// but always copy this
//ExpAndGABasis==Guaranteed_basis etc.
    if(line1 == line2)
        {
        return;
        }

    std::cout << "line1: " << line1 << "\nline2: " << line2 << '\n';
}

//============================================================================
void f24(std::string const& line1, std::string const& line2)
{
    f_4(line1, line2);
}

//============================================================================
void f34(std::string const& line1, std::string const& line2)
{
    f_4(line1, line2);
}

//============================================================================
void f44(std::string const& line1, std::string const& line2)
{
    f_4(line1, line2);
}

//============================================================================
void f_5(std::string const& /* line1 */, std::string const& /* line2 */)
{
}

//============================================================================
void f25(std::string const& line1, std::string const& line2)
{
    f_5(line1, line2);
}

typedef void(*pf)(std::string const& line1, std::string const& line2);
pf const transition_functions[6][6] =
    {
    /*        to: 0     1     2     3     4     5 */
    /* from 0 */ {0, &f01, &f02,    0,    0,    0,},
    /* from 1 */ {0, &f11, &f12,    0,    0,    0,},
    /* from 2 */ {0,    0, &f22, &f23, &f24, &f25,},
    /* from 3 */ {0,    0, &f32, &f33, &f34,    0,},
    /* from 4 */ {0,    0, &f42,    0, &f44,    0,},
    /* from 5 */ {0,    0,    0,    0,    0,    0,},
    };

//============================================================================
int try_main(int argc, char* argv[])
{
    if(3 != argc)
        {
        std::cerr << "Need two filename arguments\n";
        return EXIT_FAILURE;
        }

    std::ifstream is1(argv[1]);
    if(!is1)
        {
        std::cerr << "Cannot open " << argv[1] << '\n';
        return EXIT_FAILURE;
        }
    std::ifstream is2(argv[2]);
    if(!is2)
        {
        std::cerr << "Cannot open " << argv[2] << '\n';
        return EXIT_FAILURE;
        }

    current_name = "";
    max_abs_diff = 0.0;
    max_rel_err  = 0.0;

    // TODO ?? Want different things that match no type.
    std::string line1 = "";
    std::string line2 = "";

    line_type state = initial;
    line_type old_state;

    for(int j = 0; ; j++)
        {
        std::getline(is1, line1);
        std::getline(is2, line2);
        if(!is1 || !is2)
            {
            std::cout << "Processed " << j << " lines\n";
            break;
            }

        old_state = state;
        line_type state_is1 = get_type(line1);
        line_type state_is2 = get_type(line2);
        if(state_is1 != state_is2)
            {
            std::cerr << "Different line types: line " << j << '\n';
            show_error_context(line1, line2);
            return EXIT_FAILURE;
            }
        state = state_is1;

        if(!transition_matrix[old_state][state])
            {
            std::cerr
                << "Forbidden transition"
                << " from state " << old_state
                << " to state "   << state
                << '\n';
            show_error_context(line1, line2);
            return EXIT_FAILURE;
            }
        transition_functions[old_state][state](line1, line2);
        }

    if(!is1.eof())
        {
        std::cerr << "Premature end of file on " << argv[1] << '\n';
        show_error_context(line1, line2);
        return EXIT_FAILURE;
        }
    if(!is2.eof())
        {
        std::cerr << "Premature end of file on " << argv[2] << '\n';
        show_error_context(line1, line2);
        return EXIT_FAILURE;
        }

    std::cout
        << std::setprecision(6) << std::setw(12)
        << "Summary:"
        << " max abs diff: " << max_abs_diff
        << " max rel err:  " << max_rel_err
        << '\n';

    is1.close();
    is2.close();

    return EXIT_SUCCESS;
}

//============================================================================
int main(int argc, char* argv[])
{
    try
        {
        return try_main(argc, argv);
        }
    catch(std::exception& e)
        {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
        }
    catch(...)
        {
        std::cerr << "Fatal exception: [no detail available]" << std::endl;
        return EXIT_FAILURE;
        }
}

