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

// $Id: xml_notebook.cpp,v 1.7 2005-11-23 03:59:16 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_notebook.hpp"

#include "alert.hpp"
#include "input.hpp"
#include "transferor.hpp"
#include "wx_workarounds.hpp"

#include <wx/checkbox.h>
#include <wx/ctrlsub.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include <fstream> // TODO ?? Temporary--diagnostics only.
#include <sstream>
#include <stdexcept>

// TODO ?? Development plans:
//
// The eventual document class represents the whole input class.
// The input class expresses all available data that can potentially
// be used. Several alternative .xrc files may present different
// subsets of the input class's data in different ways.
//
// Ascertain here what data the active .xrc file requires, during
// construction, and store their names. Verify that collection of
// names and bind each automatically to its corresponding data
// member in the input class.
//
// Traverse the controls dynamically at run time: they must be
// elements of the stored collection.
//
// Perhaps the dependency on the collection of potentially-available
// data cannot be removed.

// Custom event to trigger a call to SetFocus(). This action requires
// a custom event because wxFocusEvent does not change focus--it only
// notifies the affected windows that focus changes have occurred.
//
// On the msw platform, ms cautions against changing focus in a
// WM_?ETFOCUS handler, which can have dire results. Instead, use
//   wxCommandEvent event0(wxEVT_REFOCUS_INVALID_CONTROL);
//   wxPostEvent(pointer_to_window_to_get_focus, event0);
//
wxEventType const wxEVT_REFOCUS_INVALID_CONTROL = wxNewEventType();

#define EVT_REFOCUS_INVALID_CONTROL(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY \
        (wxEVT_REFOCUS_INVALID_CONTROL \
        ,id \
        ,-1 \
        ,static_cast<wxObjectEventFunction> \
            (reinterpret_cast<wxEventFunction> \
                (wxStaticCastEvent(wxCommandEventFunction, &fn) \
                ) \
            ) \
        ,(wxObject *) NULL \
        ),

// Entries alphabetized by function name.
BEGIN_EVENT_TABLE(XmlNotebook, wxDialog)
    EVT_BUTTON(wxID_OK, XmlNotebook::OnOK)
    EVT_CHILD_FOCUS(XmlNotebook::OnChildFocus)
    EVT_INIT_DIALOG(XmlNotebook::OnInitDialog)
    EVT_NOTEBOOK_PAGE_CHANGED(XRCID("input_notebook"), XmlNotebook::OnPageChanged)
    EVT_NOTEBOOK_PAGE_CHANGING(XRCID("input_notebook"), XmlNotebook::OnPageChanging)
    EVT_REFOCUS_INVALID_CONTROL(-1, XmlNotebook::OnRefocusInvalidControl)
    EVT_UPDATE_UI(XRCID("dialog_containing_notebook"), XmlNotebook::OnUpdateGUI)
END_EVENT_TABLE()

// WX !! wxDIALOG_EX_CONTEXTHELP is unknown to wxxrc.
// 'Extra' styles (such as wxWS_EX_VALIDATE_RECURSIVELY) that wxxrc
// recognizes can be specified as an <exstyle> attribute, which is
// tidier than setting them dynamically as in this message:
//   http://lists.wxwindows.org/archive/wxPython-users/msg15676.html
// But that technique is needed for wxDIALOG_EX_CONTEXTHELP, which
// wxxrc does not recognize; it must be specified here, before
// LoadDialog() is called.
//
XmlNotebook::XmlNotebook(wxWindow* parent, Input& input)
    :input_              (input)
    ,hold_focus_window_  (0)
    ,old_focused_window_ (0)
    ,updates_blocked_    (true)
{
    SetExtraStyle(GetExtraStyle() | wxDIALOG_EX_CONTEXTHELP);
    if
        (!wxXmlResource::Get()->LoadDialog
            (this
            ,parent
            ,"dialog_containing_notebook"
            )
        )
        {
        // WX !! When wx handles exceptions more gracefully, throw here:
        //   throw std::runtime_error("Unable to load dialog.");
        wxLogError("Unable to load dialog.");
        }

    // Iterate over the complete set of input class member names.
    //
    // The names of interactive (transfer-enabled) controls form a
    // subset of the input class's member names. Some other controls,
    // such as static labels, don't need to be bound to any member of
    // the input class. Some input-class members may not be bound to
    // any control for a particular xml resource dialog.
    std::vector<std::string>::const_iterator i;
    for(i = input_.member_names().begin(); i != input_.member_names().end(); ++i)
        {
        transfer_data_[*i] = input_[*i].str();
        Bind(*i, transfer_data_[*i]);
        }
}

XmlNotebook::~XmlNotebook()
{
}

// The wxxrc sample program suggests something like this:
//    XRCCTRL(*this, "comments", wxTextCtrl)->SetValidator
//        (wxTextValidator(wxFILTER_ALPHA, &comments)
//        );
// which requires the application to know the type of the control
// (wxTextCtrl in this example); but normal controls are derived from
// class wxWindow, on which wxWindow::SetValidator() can be called
// without knowing the control type.
//
void XmlNotebook::Bind(std::string const& name, std::string& data)
{
    wxWindow* window = FindWindow(XRCID(name.c_str()));
    // TODO ?? Don't throw--this isn't actually failure. Rather,
    // it's how we can get a complete list of controls that we have
    // input items for.
    if(!window)
        {
        // WX !! When wx handles exceptions more gracefully, throw here:
        //   throw std::runtime_error("("No control named '" + name + "'.").c_str());
// This is not an error.
//        wxLogError(("No control named '" + name + "'.").c_str());
        return;
        }
    Transferor t(data, name);
    window->SetValidator(t);
}

void XmlNotebook::ConditionallyEnable()
{
    // TODO ?? This operates only on the current page's controls:
    // a speed optimization. Perhaps a change on this page forces
    // a change to an enumerative control on another page. That
    // consequent change is ignored at first (it's on another page),
    // and not handled later (no control changed in UpdateUI) until
    // some control on the other page is manually changed.
    //
    // Solution: disable the optimization here, or have page changes
    // trigger a nonignorable UpdateUI event.

    wxWindowList::compatibility_iterator node;
    for
        (node = CurrentPage().GetChildren().GetFirst()
        ;node
        ;node = node->GetNext()
        )
        {
        // INELEGANT !! If this is too slow in practice, the enablement
        // state could be cached, to indicate whether the GUI needs to
        // be updated.
        wxWindow* w = node->GetData();
        Transferor* t = dynamic_cast<Transferor*>(w->GetValidator());
        if(t)
            {
            // TODO ?? The order of these two statements is probably
            // critical.
            ConditionallyEnableControl(t->name(), *w);
            ConditionallyEnableItems  (t->name(), *w);
            }
        else
            {
            // Do nothing. Some windows don't have validators--for
            // example, most static controls.
            ;
            }
        }
}

void XmlNotebook::ConditionallyEnableControl
    (std::string const& input_name
    ,wxWindow&          control
    )
{
    control.Enable
        (input_[input_name].cast_blithely<datum_base>()->is_enabled()
        );
}

void XmlNotebook::ConditionallyEnableItems
    (std::string const& input_name
    ,wxWindow&          control
    )
{
    // WX !! Enabling a radiobutton enables its radiobox; it would
    // seem better not to do that. To defeat that feature, store
    // the whole control's enablement state, and restore it on exit.
    //
    bool control_should_be_enabled = control.IsEnabled();

    mc_enum_base* base_datum = input_[input_name].cast_blithely<mc_enum_base>();
    // TODO ?? Apparently this can never throw. Keep it in case we
    // find a way to use a dynamic cast.
    if(!base_datum)
        {
        // WX !! When wx handles exceptions more gracefully, throw here:
        //   throw std::runtime_error("Input data must be enumerative.");
        wxLogError("Input data must be enumerative.");
        return;
        }

    wxRadioBox* radiobox = dynamic_cast<wxRadioBox*>(&control);
    wxControlWithItems* itembox = dynamic_cast<wxControlWithItems*>(&control);
    if(!radiobox && !itembox)
        {
        return;
        }
    if(radiobox)
        {
        for(std::size_t j = 0; j < base_datum->cardinality(); ++j)
            {
            radiobox->Enable(j, base_datum->is_allowed(j));
            }
        // WX !! This looks like a library problem.
        // TODO ?? Defeats smooth keyboard navigation:
        radiobox->SetSelection(base_datum->allowed_ordinal());
        // TODO ?? ...but this (alone) lets a disabled radiobutton be selected:
        // base_datum->force_to_allowable_value();

        // TODO ?? Need to do something like this:
        // input_["FundChoiceType"].cast_blithely<mc_enum_base>()->force_to_allowable_value();
        // transfer_data_["FundChoiceType"] = input_["FundChoiceType"].str();
        }
    else if(itembox)
        {
        if(ItemBoxNeedsRefreshing(base_datum, *itembox))
            {
            RefreshItemBox(base_datum, *itembox);
            }
        itembox->Select(base_datum->allowed_ordinal());
        }
    else
        {
        // TODO ?? Improve this message. Really, it should be unreachable.
        wxLogError("Unanticipated case.");
        }
    control.Enable(control_should_be_enabled);
}

// TODO ?? Alphabetize.
// TODO ?? Doesn't need to be a member?
// TODO ?? This can be further improved.
bool XmlNotebook::ItemBoxNeedsRefreshing
    (mc_enum_base*       base_datum
    ,wxControlWithItems& itembox
    )
{
    if(FindFocus() != &itembox)
        {
// TODO ?? But now we have to handle this on drop down--or on focus?
// And test the validity of the selected item here and now.
// No--just do it.
//        return false;
        }
    if(static_cast<unsigned int>(itembox.GetCount()) != base_datum->cardinality())
        {
//        return false; // wrong, no?
        return true;
        }
    for(std::size_t j = 0; j < base_datum->cardinality(); ++j)
        {
        if(!base_datum->is_allowed(j))
            {
            return true;
            }

//        if(base_datum->is_allowed(j))
            {
            // Isn't this condition too restrictive?
            // They won't align if any item is disabled.
            if(itembox.GetString(j) != base_datum->str(j))
                {
//                return false; // wrong, no?
                return true;
                }
            }
        }
//    return true; // wrong, no?
    return false;
}

// TODO ?? Alphabetize.
void XmlNotebook::RefreshItemBox
    (mc_enum_base*       base_datum
    ,wxControlWithItems& itembox
    )
{
// WX !! Broken in wx-2.5.1:
// http://cvs.wxwidgets.org/viewcvs.cgi/wxWidgets/include/wx/ctrlsub.h?annotate=1.20
//                itembox->SetStringSelection("A");
// WX !! Broken in wx-2.5.1:
//                itembox->SetSelection(itembox->FindString("A"));

    // WX !! Freeze() doesn't seem to help much.
    bool updates_were_blocked = updates_blocked_;
    updates_blocked_ = true;
    itembox.Freeze();
    itembox.Clear();
    // WX !! Append(wxArrayString const&) "may be much faster"
    // according to wx online help, but that seems untrue: its
    // implementation just uses a loop.
    for(std::size_t j = 0; j < base_datum->cardinality(); ++j)
        {
        if(base_datum->is_allowed(j))
            {
            itembox.Append(base_datum->str(j));
            }
        }

    // Always leave at least one item in the itembox, and make it the
    // default item that would get chosen when all are impermissible.
    if(0 == itembox.GetCount())
        {
        itembox.Append(base_datum->str(base_datum->allowed_ordinal()));
        }
    itembox.Select
        (itembox.FindString(base_datum->str(base_datum->allowed_ordinal()))
        );
    itembox.Thaw();
    updates_blocked_ = updates_were_blocked;
}

// TODO ?? Alphabetize.
void XmlNotebook::SetupControlItems
    (std::string const& input_name
    ,wxWindow&          control
    )
{
    mc_enum_base* base_datum = input_[input_name].cast_blithely<mc_enum_base>();
    // TODO ?? Apparently this can never throw. Keep it in case we
    // find a way to use a dynamic cast.
    if(!base_datum)
        {
        // WX !! When wx handles exceptions more gracefully, throw here:
        //   throw std::runtime_error("Input data must be enumerative.");
        wxLogError("Input data must be enumerative.");
        return;
        }

    wxRadioBox* radiobox = dynamic_cast<wxRadioBox*>(&control);
    wxControlWithItems* itembox = dynamic_cast<wxControlWithItems*>(&control);
    if(!radiobox && !itembox)
        {
        return;
        }
    if(radiobox)
        {
        if(static_cast<int>(base_datum->cardinality()) != radiobox->GetCount())
            {
            std::ostringstream oss;
            oss
                << "Radiobox '"
                << input_name
                << "' has "
                << radiobox->GetCount()
                << " items, but datatype expects "
                << base_datum->cardinality()
                << "."
                ;
            wxLogMessage(oss.str().c_str());
            wxLog::FlushActive();
            return;
            }
        for(std::size_t j = 0; j < base_datum->cardinality(); ++j)
            {
            if(base_datum->str(j) != radiobox->GetString(j))
                {
                std::ostringstream oss;
                oss
                    << "Radiobox '"
                    << input_name
                    << "' button ["
                    << j
                    << "] must be '"
                    << base_datum->str(j)
                    << "', but instead it is '"
                    << radiobox->GetString(j)
                    << "'."
                    ;
                wxLogMessage(oss.str().c_str());
                }
            wxLog::FlushActive();
            radiobox->Enable(j, base_datum->is_allowed(j));
            }
        }
    else if(itembox)
        {
        RefreshItemBox(base_datum, *itembox);
        }
    else
        {
        // TODO ?? Improve this message. Really, it should be unreachable.
        wxLogError("Unanticipated case.");
        }
}

wxNotebookPage& XmlNotebook::CurrentPage()
{
    // INELEGANT !! This window could be held elsewhere, e.g. as a reference.
    int id = XRCID("input_notebook");
    wxNotebook* notebook = dynamic_cast<wxNotebook*>(FindWindow(id));
    if(!notebook)
        {
        fatal_error() << "No notebook window." << LMI_FLUSH;
        }
    wxNotebookPage* page = notebook->GetPage(notebook->GetSelection());
    if(!page)
        {
        fatal_error() << "No page selected in notebook." << LMI_FLUSH;
        }
    return *page;
}

// This might be cached, after verifying up front that the xml dialog
// resource provides a static-text 'diagnostics' control on every tab.
//
wxStaticText* XmlNotebook::DiagnosticsWindow()
{
    // Confirm that the xml dialog resource provides a static-text
    // 'diagnostics' control.
    wxStaticText* diagnostics_window = dynamic_cast<wxStaticText*>
        (FindWindow(XRCID("diagnostics"))
        );
    if(!diagnostics_window)
        {
        fatal_error() << "No 'diagnostics' window." << LMI_FLUSH;
        }
    return diagnostics_window;
}

// OnUpdateGUI() doesn't handle focus changes, so this function is
// needed for text-control validation. It validates a child control
// that has already lost focus; wx provides no way to perform the
// validation before another control irrevocably begins to gain
// focus.
//
// WX !! It seems surprising that calling GetWindow() on the
// wxChildFocusEvent argument doesn't return the same thing as
// FindFocus(): instead, it returns a pointer to the notebook tab.
//
void XmlNotebook::OnChildFocus(wxChildFocusEvent&)
{
    if(updates_blocked_)
        {
        // The diagnostics window may not yet exist if the notebook is
        // under construction.
        return;
        }

    wxWindow* new_focused_window = FindFocus();
    if(old_focused_window_ == new_focused_window)
        {
        // This do-nothing case arises e.g. when another application
        // is activated and then this application is reactivated.
        return;
        }

    if(FindWindow(wxID_CANCEL) == new_focused_window)
        {
        // Permit a 'Cancel' button to receive focus: otherwise, it
        // couldn't be pressed. But leave the new and old focused-
        // window pointers alone: 'Cancel' is a special case that
        // should not trigger validation; and it is possible to focus
        // the 'Cancel' button by clicking on it and releasing the
        // click event elsewhere, but that causes no harm as long as
        // those pointers are preserved.
        return;
        }

    wxWindow* window_that_lost_focus = old_focused_window_;
    old_focused_window_ = new_focused_window;

    if(hold_focus_window_)
        {
        // Revalidate a control that had been registered as invalid.
        ValidateTextControl(hold_focus_window_);
        }
    else
        {
        // Validate a control that has lost focus. If another control
        // was already registered as invalid, then no other control's
        // contents can be changed by the user, so these really are
        // mutually exclusive cases.
        ValidateTextControl(window_that_lost_focus);
        }

    // When this function is called by the framework, the control that
    // must be validated has already lost focus, and another window is
    // about to gain focus, but hasn't quite yet. Simply calling
    // SetFocus() here would not work because focus would still shift
    // to the other window that's about to gain focus. Posting an
    // event to refocus the invalid window solves that problem.
    if(hold_focus_window_)
        {
        wxCommandEvent event0(wxEVT_REFOCUS_INVALID_CONTROL);
        wxPostEvent(this, event0);
        }
}

// TODO ?? Or overload TransferDataToWindow(); or derive from
// wxNotebook instead of wxDialog.
void XmlNotebook::OnInitDialog(wxInitDialogEvent&)
{
    Setup(GetChildren().GetFirst());

    TransferDataToWindow();

    updates_blocked_ = false;
    UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

// TODO ?? Remove this function when the kludge it contains becomes
// needless.
void XmlNotebook::OnOK(wxCommandEvent& e)
{
    wxDialog::OnOK(e);
    if(0 == GetReturnCode())
        {
        return;
        }
    // TODO ?? WX PORT !! Icky kludge.
    input_["UseAverageOfAllFunds"] =
        ("Average fund"  == input_["FundChoiceType"].str())
        ? "Yes"
        : "No"
        ;
    input_["OverrideFundManagementFee"] =
        ("Override fund" == input_["FundChoiceType"].str())
        ? "Yes"
        : "No"
        ;
}

// TODO ?? expunge?
void XmlNotebook::OnPageChanged(wxNotebookEvent& event)
{
    ConditionallyEnable();
    CurrentPage().TransferDataToWindow();

    // WX !! If first control is disabled, keyboard navigation breaks.
    // This doesn't fix it--makes it crash, actually:
/*
    if(!FindFocus()->IsEnabled())
        {
        FindWindow(wxID_OK)->SetFocus();
        }
*/
}

// Called when page is about to change, but hasn't yet.
//
void XmlNotebook::OnPageChanging(wxNotebookEvent& event)
{
    // Do nothing until the notebook has been fully created.
    if(updates_blocked_)
        {
        return;
        }

    // Veto if an invalid text control has already been detected.
    // Else validate the window that has focus, if it's not a 'Cancel'
    // button; else validate the last window that had focus.

    if(hold_focus_window_)
        {
        ValidateTextControl(hold_focus_window_);
        }
    else
        {
        if(FindWindow(wxID_CANCEL) != FindFocus())
            {
            ValidateTextControl(FindFocus());
            }
        else
            {
            ValidateTextControl(old_focused_window_);
            }
        }

// TODO ?? expunge Validate()?
//    if(hold_focus_window_)
    if(hold_focus_window_ || !Validate())
        {
        event.Veto();
        hold_focus_window_->SetFocus();
        return;
        }
    else
        {
        // Do nothing. Permit the page change. The EVT_CHILD_FOCUS
        // handler cleans up any text left over in the diagnostics
        // window.
        ;
        }

    CurrentPage().TransferDataFromWindow();
}

void XmlNotebook::OnRefocusInvalidControl(wxCommandEvent&)
{
    hold_focus_window_->SetFocus();
}

void XmlNotebook::OnUpdateGUI(wxUpdateUIEvent& event)
{
    // Do nothing until the notebook has been fully created.
    if(updates_blocked_)
        {
        return;
        }

    // Exit immediately if nothing changed. The library calls this
    // function continually in idle time, and it's pointless to fret
    // over inputs that didn't change on this update because they've
    // already been handled. Complex processing of many inputs has
    // been observed to consume excessive CPU time when a malloc
    // debugger is running, so this optimization is significant.
    CurrentPage().TransferDataFromWindow();
    if(cached_transfer_data_ == transfer_data_)
        {
        return;
        }
    cached_transfer_data_ = transfer_data_;

    DiagnosticsWindow()->SetLabel("");
    std::vector<std::string>::const_iterator i;
    std::vector<std::string> names_of_changed_controls;
    for(i = input_.member_names().begin(); i != input_.member_names().end(); ++i)
        {
        try
            {
//            if(input_[*i].str() != transfer_data_[*i])
            if
                (   input_[*i].str() != transfer_data_[*i]
                &&  CurrentPage().FindWindow(XRCID(i->c_str()))
                )
                {
                names_of_changed_controls.push_back(*i);
                input_[*i] = transfer_data_[*i];
                }
            }
        catch(std::exception const& e)
            {
            DiagnosticsWindow()->SetLabel(*i + ": " + e.what());
            }
        }

    // Experimental. It is hypothesized that EVT_UPDATE_UI events
    // occur frequently enough that two control changes cannot be
    // simultaneous.
    if(1 < names_of_changed_controls.size())
        {
        std::ostringstream oss;
        oss << "Contents of more than one control changed, namely\n";
        std::vector<std::string>::const_iterator i;
        for
            (i = names_of_changed_controls.begin()
            ;i != names_of_changed_controls.end()
            ;++i
            )
            {
            oss << *i << " changed" << '\n';
            }
        // This could be verbose, so flush any pending messages first.
// TODO ?? Why does this pop up at all?
/*
        wxLog::FlushActive();
        wxLogMessage(oss.str().c_str());
        wxLog::FlushActive();
*/
        }

    input_.Harmonize();

    // TODO ?? Experimental. A general solution is wanted instead.
    transfer_data_["GeneralAccountRate"] = input_["GeneralAccountRate"].str();

    ConditionallyEnable();
}

void XmlNotebook::Setup(wxWindowList::compatibility_iterator initial_node)
{
/*
// TODO ?? Temporary--diagnostics only.
std::ofstream os
    ("xml_problems"
    ,   std::ios_base::out
      | std::ios_base::ate
      | std::ios_base::app
      | std::ios_base::binary
    );
os << "Only the following controls were processed correctly:" << std::endl;
*/

    wxWindowList::compatibility_iterator node;
    for
        (node = initial_node
        ;node
        ;node = node->GetNext()
        )
        {
        wxWindow* w = node->GetData();
/*
        if
            (   "-1" != w->GetName()
            &&  input_.member_names().end() == std::find
                (input_.member_names().begin()
                ,input_.member_names().end()
                ,w->GetName()
                )
            )
            {
            wxLogMessage(w->GetName() + " not in member name list.");
            }
*/
        Transferor* t = dynamic_cast<Transferor*>(w->GetValidator());
        if(t)
            {
            if(w != FindWindow(XRCID(t->name().c_str())))
                {
                fatal_error()
                    << "Input name '"
                    << t->name()
                    << "': window "
                    << w
                    << " being traversed doesn't match window "
                    << FindWindow(XRCID(t->name().c_str()))
                    << " found from wxxrc ID "
                    << XRCID(t->name().c_str())
                    << " ."
                    << LMI_FLUSH
                    ;
                }
/*
os << t->name() << std::endl;
os.flush();
*/
            SetupControlItems(t->name(), *w);
            }
        else
            {
            // Do nothing. Some windows don't have validators--for
            // example, most static controls.
            ;
            }
            Setup(w->GetChildren().GetFirst());
        }
}

// TODO ?? expunge?
bool XmlNotebook::TransferDataToWindow()
{
return CurrentPage().wxWindow::TransferDataToWindow();
}

// TODO ?? Is this worth rewriting?
//
// WX !! This shows that wxDialog::Validate() isn't generally useful
// for our purposes. The library calls it when the user signals that
// input is complete, e.g. by pressing the 'OK' button. Even though we
// call it here on notebook page changes too, this isn't ideal: users
// shouldn't be able to leave an invalid field. Refocusing the invalid
// field upon later detection of error isn't good enough: the user's
// train of thought has already moved on.
//
bool XmlNotebook::Validate()
{
/*
    // It's not obvious whether wxDialog::Validate() should be called
    // here, or where in relation to other processing. Probably this
    // is the right place; it doesn't matter much because we won't use
    // this approach in production.
    wxDialog::Validate();

    // Don't signal an error when this function is called before the
    // dialog is fully created.
    if(updates_blocked_)
        {
        return true;
        }

    if(transfer_data_["comments"].size() < 7)
        {
        hold_focus_window_ = 0;
        DiagnosticsWindow()->SetLabel("");
        return true;
        }
    else
        {
        hold_focus_window_ = FindWindow(XRCID("comments"));
        DiagnosticsWindow()->SetLabel("Error:\nInput exceeds six characters.");
        return false;
        }
*/
    return true;
}

void XmlNotebook::ValidateTextControl(wxWindow* w)
{
    if(!w)
        {
        // The control that lost focus might be on a different tab.
        return;
        }

    wxTextCtrl* textctrl = dynamic_cast<wxTextCtrl*>(w);
    if(!textctrl)
        {
        // Only text controls are validating on losing focus.
        return;
        }

    Transferor* t = dynamic_cast<Transferor*>(textctrl->GetValidator());
    if(!t)
        {
        // WX !! When wx handles exceptions more gracefully, throw here:
        //   throw std::runtime_error("No transferror associated with control.");
//        wxLogError("No transferror associated with control.");

// TODO ?? This happens only on page losing focus, probably because
// of an IsShown() conditional. Test on page gaining focus instead.

/*
        std::ostringstream oss;
        oss
            << "No transferror associated with control:\n"
            << textctrl->GetId() << " = GetId()\n"
            << textctrl->GetId() << " = GetId()\n"
            << textctrl->GetName() << " = GetName()\n"
            << textctrl->GetTitle() << " = GetTitle()\n"
            << textctrl->GetValue() << " = GetValue()\n"
            ;
        wxLogError(oss.str().c_str());
*/
        return;
        }

    // Assume that OnUpdateGUI() has already been called.
    if
        (input_[t->name()].cast_blithely<datum_base>()->is_valid
            (transfer_data_[t->name()]
            )
        )
        {
        hold_focus_window_ = 0;
        DiagnosticsWindow()->SetLabel("");
        }
    else
        {
        hold_focus_window_ = textctrl;
//        DiagnosticsWindow()->SetLabel("Invalid.");
        }
}

