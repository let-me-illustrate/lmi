// Helper for creating documents in unattended GUI tests.
//
// Copyright (C) 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

#ifndef wx_test_document_hpp
#define wx_test_document_hpp

#include "config.hpp"

#include "mvc_controller.hpp"

#include <wx/log.h>
#include <wx/testing.h>
#include <wx/uiaction.h>

#include <exception>                    // uncaught_exceptions()

/// Helper function for finding and focusing a control with the specified name
/// inside MvcController (actually it could be any top level window containing
/// a book control).
///
/// Returns the never null pointer to the window.
///
/// Throws if the control couldn't be found.
wxWindow* wx_test_focus_controller_child(MvcController& dialog, char const* name);

/// Helper base class for classes creating or opening documents.
///
/// This class provides functions for closing the current document, optionally
/// discarding the changes done to it.
///
/// Unfortunately it is impossible to close the document automatically from
/// this class dtor as doing this may result in an exception and throwing from
/// dtors is too dangerous, generally speaking (and not allowed at all by
/// default since C++11), to prefer it to an approach involving explicit calls
/// to close().

class wx_test_document_base
{
  public:
    wx_test_document_base()
        :opened_ {false}
    {
    }

    ~wx_test_document_base()
    {
        // Normally either close() or close_discard_changes() should be called,
        // so complain about forgetting to do this if neither was. Except that
        // we shouldn't do this if we're unwinding due to an exception from a
        // test failure, as this is not a bug in the test code then.
        if(opened_)
            {
            if(std::uncaught_exceptions())
                {
                // Moreover, in case of exception, try to close the window to
                // avoid showing message boxes asking the user if it should be
                // saved: this is undesirable in an unattended test.
                do_close();

                wxTEST_DIALOG
                    (wxYield()
                    ,wxExpectModal<wxMessageDialog>(wxNO).Optional()
                    );
                }
            else
                {
                wxSafeShowMessage
                    ("Programming error"
                    ,"A document created during unattended test hasn't been closed, "
                     "please report this."
                    );
                }
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

        wxTEST_DIALOG(wxYield()
                     ,wxExpectModal<wxMessageDialog>(wxNO).
                        Describe("message box confirming closing modified file")
                     );
    }

  protected:
    // This function should be called by the derived classes when the document
    // window is really opened.
    void set_opened() { opened_ = true; }

  private:
    wx_test_document_base(wx_test_document_base const&) = delete;
    wx_test_document_base& operator=(wx_test_document_base const&) = delete;

    // Common part of different close() functions.
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

/// Represents an existing illustration document.
///
/// Instantiating an object of this class simulates opening the specified
/// illustration. Its close() function must be called before destroying an object
/// of this class to ensure that it doesn't stay open.

class wx_test_existing_illustration
    :public wx_test_document_base
{
  public:
    // Default constructor opens an illustration without changing the parameters.
    explicit wx_test_existing_illustration(std::string const& file_ill)
    {
        wxUIActionSimulator ui;
        ui.Char('o', wxMOD_CONTROL);    // "File|Open"

        wxTEST_DIALOG
            (wxYield()
            ,wxExpectModal<wxFileDialog>(file_ill)
            ,wxExpectDismissableModal<MvcController>(wxID_OK)
                .Describe("illustration properties for " + file_ill)
            );

        set_opened();
    }
};

#endif // wx_test_document_hpp
