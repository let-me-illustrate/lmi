// Helper for testing MvcController dialogs.
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

#ifndef wx_test_mvc_dialog_hpp
#define wx_test_mvc_dialog_hpp

#include "config.hpp"

#include "mvc_controller.hpp"

#include <wx/scopeguard.h>
#include <wx/testing.h>

/// Abstract base class for the concrete expectations defining the actions to
/// perform when a given MvcController-derived dialog is shown.
///
/// The main reason for this class existence is the unusual reliance of
/// MvcController on wxEVT_UPDATE_UI events for its functionality. As these
/// events are not sent from inside wxYield(), which is used throughout the
/// automatic tests, the dialog is not updated (i.e. the controls inside it
/// are not enabled when they should be, the corresponding program variables
/// are not updated when GUI controls change, and so on) unless we send these
/// events ourselves and this class helps with doing it.

class ExpectMvcDialog
    :public wxExpectModalBase<MvcController>
{
  public:
    virtual int OnInvoked(MvcController* dialog) const
    {
        // Bring the dialog up.
        dialog->Show();
        wxYield();

        // Perform whichever actions are needed.
        DoRunDialog(dialog);

        // And ensure that the model data is updated at the end.
        DoUpdateDialogUI(dialog);

        return wxID_OK;
    }

  protected:
    // The method to be implemented in the derived classes for simulating the
    // user actions that need to be performed in this dialog.
    //
    // DoUpdateDialogUI() should be used after simulating any action updating
    // the state of the dialog.
    virtual void DoRunDialog(MvcController* dialog) const = 0;

    // Ensure that the dialog state takes into account all the events simulated
    // so far by explicitly letting it process a wxUpdateUIEvent.
    void DoUpdateDialogUI(MvcController* dialog) const
    {
        wxUpdateUIEvent event(dialog->GetId());
        event.SetEventObject(dialog);
        dialog->ProcessWindowEvent(event);
    }
};

#endif // wx_test_mvc_dialog_hpp
