// Helper for creating new documents in unattended GUI tests.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023 Gregory W. Chicares.
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
// https://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

#ifndef wx_test_new_hpp
#define wx_test_new_hpp

#include "config.hpp"

#include "wx_test_document.hpp"

/// Represents a new illustration document.
///
/// Instantiating an object of this class simulates creating a new illustration.
/// Its close() function must be called before destroying an object of this class
/// to ensure that it doesn't stay open.

class wx_test_new_illustration
    :public wx_test_document_base
{
  public:
    // Default constructor creates an illustration with the default parameters.
    wx_test_new_illustration()
    {
        do_new_illustration(wxExpectDismissableModal<MvcController>(wxID_OK).
                                Describe("new illustration properties")
                           );
    }

    // This constructor takes a class responsible for handling the illustration
    // parameters dialog and may modify it in any desired way before accepting.
    wx_test_new_illustration(wxModalExpectation const& e)
    {
        do_new_illustration(e);
    }

  private:
    // Common part of both constructors.
    void do_new_illustration(wxModalExpectation const& e)
    {
        wxUIActionSimulator ui;
        ui.Char('n', wxMOD_CONTROL);    // "File|New"
        ui.Char('i');                   // "Illustration"

        wxTEST_DIALOG(wxYield(), e);

        set_opened();
    }
};

/// Represents a new census document.
///
/// Instantiating an object of this class simulates creating a new census
/// document. As with illustrations, close() function must be called before
/// destroying it.

class wx_test_new_census
    :public wx_test_document_base
{
  public:
    wx_test_new_census()
    {
        wxUIActionSimulator ui;
        ui.Char('n', wxMOD_CONTROL);    // "File|New"
        ui.Char('c');                   // "Census"
        wxYield();

        set_opened();
    }
};

#endif // wx_test_new_hpp
