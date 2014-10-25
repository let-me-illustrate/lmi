// Helper for creating new documents in unattended GUI tests.
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

#ifndef wx_test_new_hpp
#define wx_test_new_hpp

#include "config.hpp"

#include "mvc_controller.hpp"
#include "uncopyable_lmi.hpp"

#include <wx/log.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

/// Helper base class for classes testing creation of specific new documents.
///
/// This class provides methods for closing the current document, optionally
/// discarding the changes done to it.
///
/// Unfortunately it is impossible to close the document automatically from
/// this class dtor as doing this may result in an exception and throwing from
/// dtors is too dangerous, generally speaking (and not allowed at all by
/// default since C++11), to prefer it to an approach involving explicit calls
/// to close().

class wx_test_new_document_base
    :private lmi::uncopyable<wx_test_new_document_base>
{
  public:
    wx_test_new_document_base()
        :opened_(false)
    {
    }

    ~wx_test_new_document_base()
    {
        // As we don't want to throw an exception from the dtor, all we can do
        // is to complain to the user directly.
        if(opened_)
            {
            wxSafeShowMessage
                ("Programming error"
                ,"A document created during unattended test hasn't been closed, "
                 "please report this."
                );
            }
    }

    // Close the document window, the document must not be modified.
    void close()
    {
        do_close();

        wxYield();
    }

    // Close the document window, the document must have been modified and the
    // changes to it will be discarded.
    void close_discard_changes()
    {
        do_close();

        wxTEST_DIALOG(wxYield(), wxExpectModal<wxMessageDialog>(wxNO));
    }

  protected:
    // This method should be called by the derived classes when the document
    // window is really opened.
    void set_opened() { opened_ = true; }

  private:
    // Common part of different close() methods.
    void do_close()
    {
        // If we started closing the document, we should reset the flag: even
        // if closing it fails, we shouldn't complain about forgetting to close
        // it as we clearly didn't forget to do it.
        opened_ = false;

        wxUIActionSimulator ui;
        ui.Char('l', wxMOD_CONTROL);    // "File|Close"
    }

    bool opened_;
};

/// Represents a new illustration document.
///
/// Instantiating an object of this class simulates creating a new illustration.
/// Its close() method must be called before destroying an object of this class
/// to ensure that it doesn't stay open.

class wx_test_new_illustration
    :public wx_test_new_document_base
{
  public:
    // Default constructor creates an illustration with the default parameters.
    wx_test_new_illustration()
    {
        do_new_illustration(wxExpectDismissableModal<MvcController>(wxID_OK));
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
/// document. As with illustrations, close() method must be called before
/// destroying it.

class wx_test_new_census
    :public wx_test_new_document_base
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
