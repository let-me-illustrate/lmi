// Input 'notebook' (tabbed dialog) driven by xml resources.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: xml_notebook.hpp,v 1.1 2005-03-12 03:01:08 chicares Exp $

#ifndef xml_notebook_hpp
#define xml_notebook_hpp

#include "config.hpp"

#include "expimp.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <wx/dialog.h>
#include <wx/stattext.h>

#include <map>
#include <string>

class mc_enum_base;
class WXDLLEXPORT wxControlWithItems;

// Unlike other event classes used here, class wxNotebookEvent is not
// defined by including class wxDialog's header.
class WXDLLEXPORT wxNotebookEvent;

// TODO ?? Explain--it's documented as a typedef by wx help.
typedef wxWindow wxNotebookPage;

class Input;

// Member functions
//   Bind()
//   DiagnosticsWindow()
//   Validate()
// cannot be const because they use wxWindowBase::FindWindow(), which
// is not const. And
//   Validate()
// cannot be const anyway because wxWindowBase::Validate() is a non-
// const virtual, as perhaps it must be so that wx 'validators' can
// change control contents, even though that capability is not useful
// here.
//
// WX !! Shouldn't wxWindowBase::FindWindow() be const?

// Text controls are validated when they lose focus. For at least one
// (and perhaps all) of the platforms wx supports, it is not possible
// to block another control from gaining focus when the control losing
// focus is determined to be invalid, so it is necessary to track
// focus here in data members.
//
// Data member hold_focus_window_ points to a control that should
// retain focus because its contents are invalid.
//
// Data member old_focused_window_ points to the window that last lost
// focus--unless it's a 'Cancel' button, which gains focus without
// triggering validation of the control that lost focus.

// DiagnosticsWindow() returns a wxStaticText* where a wxWindow* might
// seem more general. Rationale: the implementation uses wxStaticText,
// whose contents can be written only with GetLabel(); other controls
// that might be used instead may implement GetLabel() differently.
//
// INELEGANT !! It would be better to write diagnostics to a custom
// stream.

class XmlNotebook
    :public wxDialog
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<XmlNotebook>
{
  public:
    XmlNotebook(wxWindow* parent, Input& input);
    ~XmlNotebook();

  private:
    void Bind(std::string const& name, std::string& data);

    void ConditionallyEnable();

    void ConditionallyEnableControl
        (std::string const& input_name
        ,wxWindow&          control
        );

    void ConditionallyEnableItems
        (std::string const& input_name
        ,wxWindow&          control
        );

    // TODO ?? Here and elsewhere, isn't FindWindow now const?
    wxNotebookPage& CurrentPage(); // Uses FindWindow--can't be const.
    wxStaticText* DiagnosticsWindow(); // Uses FindWindow--can't be const.

    bool ItemBoxNeedsRefreshing
        (mc_enum_base*       base_datum
        ,wxControlWithItems& itembox
        );

    void OnChildFocus            (wxChildFocusEvent&);

    // TODO ?? This hides wxWindow::OnInitDialog. Shouldn't that
    // function be virtual in the base class? Should a different
    // name be used here?
    void OnInitDialog            (wxInitDialogEvent&);
    void OnPageChanged           (wxNotebookEvent  &);
    void OnPageChanging          (wxNotebookEvent  &);
    void OnRefocusInvalidControl (wxCommandEvent   &);
    void OnUpdateGUI             (wxUpdateUIEvent  &);

    void RefreshItemBox
        (mc_enum_base*       base_datum
        ,wxControlWithItems& itembox
        );

    void Setup(wxWindowList::compatibility_iterator);

    void SetupControlItems
        (std::string const& input_name
        ,wxWindow&          control
        );

    // wxDialog overrides.
    virtual bool TransferDataToWindow();
    virtual bool Validate(); // TODO ?? expunge?

    void ValidateTextControl(wxWindow*);

    Input& input_;

    wxWindow* hold_focus_window_;
    wxWindow* old_focused_window_;

    bool updates_blocked_;

    std::map<std::string, std::string> transfer_data_;
    std::map<std::string, std::string> cached_transfer_data_;

void foo(); // TODO ?? expunge

    DECLARE_EVENT_TABLE()
};

#endif // xml_notebook_hpp

