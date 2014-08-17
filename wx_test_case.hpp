// Supporting framework for wx interface test cases.
//
// Copyright (C) 2014 Gregory W. Chicares.
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

#ifndef wx_test_case_hpp
#define wx_test_case_hpp

/// Function responsible for registering test cases with the main tester object.
///
/// The boolean return type is a convenience allowing this function to be used
/// inside LMI_WX_TEST_CASE macro and is not otherwise used.
bool add_wx_test_case(void (*test_func)(), char const* test_name);

/// Define a test function and register it with the application tester.
///
/// Usage is:
///
///     LMI_WX_TEST_CASE(my_test)
///     {
///         ... code of the test ...
///     }
#define LMI_WX_TEST_CASE(name) \
static void wx_test_case_##name(); \
volatile bool register_wx_test_case_##name = add_wx_test_case \
    (wx_test_case_##name \
    ,#name \
    ); \
static void wx_test_case_##name()

#endif // wx_test_case_hpp
