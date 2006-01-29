// Input 'notebook' (tabbed dialog) driven by xml resources.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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

// $Id: xml_notebook.cpp,v 1.16 2006-01-29 13:52:01 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_notebook.hpp"
#include "xml_notebook.tpp"

#include "alert.hpp"
#include "input.hpp"
#include "map_lookup.hpp"
#include "transferor.hpp"
#include "wx_workarounds.hpp"

#include <wx/checkbox.h>
#include <wx/ctrlsub.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

#include <exception>
#include <fstream> // TODO ?? Temporary--diagnostics only.

// TODO ?? Development plans:
//
// Eventually, input will come from a document class.
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
//
// Rename this file and class to make it plain that it's an MVC
// Controller.
//
// Idea: if no 'diagnostics' window found, use a messagebox e.g.

/// Custom event to trigger a call to SetFocus(). This action requires
/// a custom event because wxFocusEvent does not change focus--it only
/// notifies the affected windows that focus changes have occurred.
///
/// On the msw platform, ms cautions against changing focus in a
/// WM_?ETFOCUS handler, which can have dire results; and focus
/// messages are just informational and can't be vetoed, probably to
/// prevent rogue applications from refusing to yield focus.
///
/// Usage: where focus has undesirably been lost, do:
///   wxCommandEvent event0(wxEVT_REFOCUS_INVALID_CONTROL);
///   wxPostEvent(pointer_to_window_to_get_focus, event0);
/// and add a handler in the event table that invokes a function that
/// calls SetFocus().

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
        ,0 \
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
        fatal_error() << "Unable to load dialog." << LMI_FLUSH;
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
        Bind(*i, transfer_data_[*i] = input_[*i].str());
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
void XmlNotebook::Bind(std::string const& name, std::string& data) const
{
    wxWindow* window = FindWindow(XRCID(name.c_str()));
    // TODO ?? Don't throw--this isn't actually failure. Rather,
    // it's how we can get a complete list of controls that we have
    // input items for.
    if(!window)
        {
// This is not an error.
//        warning() << "No control named '" << name << "'." << LMI_FLUSH;;
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

    wxWindowList wl = CurrentPage().GetChildren();
    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
        {
        wxWindow* pw = *i;
        LMI_ASSERT(0 != pw);
        Transferor* t = dynamic_cast<Transferor*>(pw->GetValidator());
        if(t)
            {
            // INELEGANT !! If this is too slow in practice (because
            // refreshing item lists is expensive), then perhaps the
            // enablement state could be cached.
            //
            // TODO ?? The order of these two statements is probably
            // critical.
            ConditionallyEnableControl(t->name(), *pw);
            ConditionallyEnableItems  (t->name(), *pw);
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
        fatal_error() << "Input data must be enumerative." << LMI_FLUSH;
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
        fatal_error() << "Unanticipated case." << LMI_FLUSH;
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
        fatal_error() << "Input data must be enumerative." << LMI_FLUSH;
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
            fatal_error()
                << "Radiobox '"
                << input_name
                << "' has "
                << radiobox->GetCount()
                << " items, but datatype expects "
                << base_datum->cardinality()
                << "."
                << LMI_FLUSH
                ;
            }
        for(std::size_t j = 0; j < base_datum->cardinality(); ++j)
            {
            if(base_datum->str(j) != radiobox->GetString(j))
                {
                fatal_error()
                    << "Radiobox '"
                    << input_name
                    << "' button ["
                    << j
                    << "] must be '"
                    << base_datum->str(j)
                    << "', but instead it is '"
                    << radiobox->GetString(j)
                    << "'."
                    << LMI_FLUSH
                    ;
                }
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
        fatal_error() << "Unanticipated case." << LMI_FLUSH;
        }
}

wxNotebookPage& XmlNotebook::CurrentPage() const
{
    // INELEGANT !! This window could be held elsewhere, e.g. as a reference.
    wxNotebook& notebook = WindowFromXrcName<wxNotebook>("input_notebook");
    wxNotebookPage* page = notebook.GetPage(notebook.GetSelection());
    if(!page)
        {
        fatal_error() << "No page selected in notebook." << LMI_FLUSH;
        }
    return *page;
}

wxStaticText& XmlNotebook::DiagnosticsWindow() const
{
    return WindowFromXrcName<wxStaticText>("diagnostics");
}

/// Ideally, focus is on an enabled window that ought to accept focus.
/// But sometimes that condition isn't ensured by wx, e.g., when a
/// control that appropriately had focus becomes disabled. And it's
/// possible for all controls on a notebook page to be disabled. This
/// function either achieves the ideal, or does the best that can be
/// done.
///
///  - If the ideal is already achieved, then exit immediately.
///
///  - Else, change focus to the best window possible, ideally not
///    changing it to the notebook tab (the tab meets all the initial
///    conditions, and would be perfect had the user selected it; but
///    it is preferable to find a control that accepts input: an extra
///    ideal condition that pertains only to focus changes made under
///    program control):
///
///    - first, focus the notebook: this is always possible, and it's
///      better than letting a disabled window keep the focus;
///
///    - then, focus the first child window that meets the ideal
///      conditions, if any can be found;
///
///    - then, assert that at least subideal conditions have been
///      achieved, viz., that an enabled window has focus: this
///      outcome should always be feasible.

void XmlNotebook::EnsureOptimalFocus()
{
    wxWindow* f = FindFocus();
    if(f && f->IsEnabled() && f->AcceptsFocus())
        {
        return;
        }

    SetFocus();
    wxWindowList wl = CurrentPage().GetChildren();
    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
        {
        wxWindow* w = *i;
        if(w && w->IsEnabled() && w->AcceptsFocus())
            {
            w->SetFocus();
            break;
            }
        }

    f = FindFocus();
    LMI_ASSERT(f && f->IsEnabled());
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
    // SetFocus() here would not work: as soon as the present function
    // returns, focus would shift to the other window that's about to
    // gain focus. Posting an event to refocus the invalid window,
    // after this function has returned and after the the pending
    // focus change has occurred, solves that problem.
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
    Setup(GetChildren());

    TransferDataToWindow();

    updates_blocked_ = false;
    UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

// TODO ?? Remove this function when the kludge it contains becomes
// needless.
void XmlNotebook::OnOK(wxCommandEvent& event)
{
    wxDialog::OnOK(event);
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

// TODO ?? Expunge? Aren't transfer and focus already managed elsewhere?
void XmlNotebook::OnPageChanged(wxNotebookEvent& event)
{
    ConditionallyEnable();
    CurrentPage().TransferDataToWindow();

    // WX !! Normally, the first interactive control would have focus.
    // But if that control was just disabled, then no window has
    // focus--so focus might need to be reset now. See:
    //   http://lists.nongnu.org/archive/html/lmi/2005-11/msg00040.html

    EnsureOptimalFocus();
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

    if(hold_focus_window_)
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
    LMI_ASSERT(hold_focus_window_);
    hold_focus_window_->SetFocus();
}

void XmlNotebook::OnUpdateGUI(wxUpdateUIEvent& event)
{
    // Do nothing until the notebook has been fully created.
    if(updates_blocked_)
        {
        return;
        }

    // A disabled window might have focus:
    //   http://lists.nongnu.org/archive/html/lmi/2005-11/msg00040.html
    // so make sure focus is valid now.
    EnsureOptimalFocus();

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

    DiagnosticsWindow().SetLabel("");
    std::vector<std::string>::const_iterator i;
    std::vector<std::string> names_of_changed_controls;
    for(i = input_.member_names().begin(); i != input_.member_names().end(); ++i)
        {
        try
            {
            if
                (   CurrentPage().FindWindow(XRCID(i->c_str()))
                &&  input_[*i].str() != map_lookup(transfer_data_, *i)
                )
                {
                names_of_changed_controls.push_back(*i);
                input_[*i] = map_lookup(transfer_data_, *i);
                }
            }
        catch(std::exception const& event)
            {
            DiagnosticsWindow().SetLabel(*i + ": " + event.what());
            }
        }

/*
    // Experimental. It is hypothesized that EVT_UPDATE_UI events
    // occur frequently enough that two control changes cannot be
    // simultaneous.
TODO ?? ...except for changes the framework itself makes, which
ought to be forced through somehow.
    if(1 < names_of_changed_controls.size())
        {
        warning() << "Contents of more than one control changed, namely\n";
        std::vector<std::string>::const_iterator i;
        for
            (i = names_of_changed_controls.begin()
            ;i != names_of_changed_controls.end()
            ;++i
            )
            {
            warning() << *i << " changed" << '\n';
            }
        warning() << LMI_FLUSH;
        }
*/

    input_.Harmonize();

    // TODO ?? Experimental. A general solution is wanted instead.
    transfer_data_["GeneralAccountRate"] = input_["GeneralAccountRate"].str();

    ConditionallyEnable();
}

void XmlNotebook::Setup(wxWindowList const& wl)
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

    for(wxWindowList::const_iterator i = wl.begin(); i != wl.end(); ++i)
        {
        wxWindow* pw = *i;
        LMI_ASSERT(0 != pw);

/*
        if
            (   "-1" != pw->GetName()
            &&  input_.member_names().end() == std::find
                (input_.member_names().begin()
                ,input_.member_names().end()
                ,pw->GetName()
                )
            )
            {
            warning()
                << (pw->GetName()
                << " not in member name list."
                << LMI_FLUSH
                ;
            }
*/
        Transferor* t = dynamic_cast<Transferor*>(pw->GetValidator());
        if(t)
            {
            // INELEGANT !! Assert this once, upon construction e.g.,
            // or perhaps when page changes.
            if(pw != &WindowFromXrcName<wxWindow>(t->name()))
                {
                fatal_error()
                    << "Input name '"
                    << t->name()
                    << "': window "
                    << pw
                    << " being traversed doesn't match window "
                    << &WindowFromXrcName<wxWindow>(t->name())
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
            SetupControlItems(t->name(), *pw);
            }
        else
            {
            // Do nothing. Some windows don't have validators--for
            // example, most static controls.
            ;
            }
        Setup(pw->GetChildren());
        }
}

// TODO ?? expunge?
bool XmlNotebook::TransferDataToWindow()
{
return CurrentPage().wxWindow::TransferDataToWindow();
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
//        fatal_error() << "No transferor associated with control." << LMI_FLUSH;

// TODO ?? This happens only on page losing focus, probably because
// of an IsShown() conditional. Test on page gaining focus instead.

/*
        fatal_error()
            << "No transferror associated with control:\n"
            << textctrl->GetId() << " = GetId()\n"
            << textctrl->GetId() << " = GetId()\n"
            << textctrl->GetName() << " = GetName()\n"
            << textctrl->GetTitle() << " = GetTitle()\n"
            << textctrl->GetValue() << " = GetValue()\n"
            << LMI_FLUSH
            ;
*/
        return;
        }

    // Assume that OnUpdateGUI() has already been called.
    if
        (input_[t->name()].cast_blithely<datum_base>()->is_valid
            (map_lookup(transfer_data_, t->name())
            )
        )
        {
        hold_focus_window_ = 0;
        DiagnosticsWindow().SetLabel("");
        }
    else
        {
        hold_focus_window_ = textctrl;
//        DiagnosticsWindow().SetLabel("Invalid.");
        }
}

#if !wxCHECK_VERSION(2,5,4)
wxWindow* XmlNotebook::FindWindow(long int window_id) const
{
    return const_cast<XmlNotebook*>(this)->wxDialog::FindWindow(window_id);
}
#endif // !wxCHECK_VERSION(2,5,4)

