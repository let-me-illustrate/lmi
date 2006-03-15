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

// $Id: xml_notebook.cpp,v 1.24 2006-03-15 03:10:10 chicares Exp $

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

#include <wx/app.h> // wxTheApp
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

// Entries alphabetized by event name rather than function name.
BEGIN_EVENT_TABLE(XmlNotebook, wxDialog)
    EVT_BUTTON(wxID_OK, XmlNotebook::UponOK)
    EVT_CHILD_FOCUS(XmlNotebook::UponChildFocus)
    EVT_INIT_DIALOG(XmlNotebook::UponInitDialog)
    EVT_NOTEBOOK_PAGE_CHANGED(XRCID("input_notebook"), XmlNotebook::UponPageChanged)
    EVT_NOTEBOOK_PAGE_CHANGING(XRCID("input_notebook"), XmlNotebook::UponPageChanging)
    EVT_REFOCUS_INVALID_CONTROL(-1, XmlNotebook::UponRefocusInvalidControl)
    EVT_UPDATE_UI(XRCID("dialog_containing_notebook"), XmlNotebook::UponUpdateGUI)
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
    for
        (i = input_.member_names().begin()
        ;i != input_.member_names().end()
        ;++i
        )
        {
        if(FindWindow(XRCID(i->c_str())))
            {
            Bind(*i, transfer_data_[*i] = input_[*i].str());
            }
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
    Transferor transferor(data, name);
    WindowFromXrcName<wxWindow>(name).SetValidator(transferor);
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
    if(wxNOT_FOUND == notebook.GetSelection())
        {
        fatal_error() << "No page selected in notebook." << LMI_FLUSH;
        }
    wxNotebookPage* page = notebook.GetPage(notebook.GetSelection());
    if(!page)
        {
        fatal_error() << "Selected notebook page is invalid." << LMI_FLUSH;
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
///    - first, focus the dialog: this is always possible, and it's
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
    if(!(f && f->IsEnabled()))
        {
        // This is a fatal error because a warning would repeat itself
        // ad infinitum.
        fatal_error() << "No enabled window to focus." << LMI_FLUSH;
        }
}

/// Cause a text control to be validated upon losing focus.
///
/// UponUpdateGUI() doesn't handle focus changes, so this function is
/// needed for text-control validation.
///
/// At least for msw, it is not possible to veto focus loss before
/// another control irrevocably begins to gain focus, and wx has no
/// means to do that for any platform. Therefore, when wx calls this
/// function, the control that must be validated has already lost
/// focus, and another window is about to gain focus, but hasn't quite
/// gained it yet. It is a design requirement of this MVC framework
/// that focus be retained in the offending control if validation
/// fails, but simply calling SetFocus() here would not work: as soon
/// as the present function returns, focus would shift to the other
/// window that's about to gain focus. Instead, a custom event
/// (EVT_REFOCUS_INVALID_CONTROL) is posted: it reseizes focus for
/// the window for which validation failed, after the present function
/// has returned and after the pending focus change has occurred.
///
/// The 'Cancel' button is a special case. At least for msw, clicking
/// 'Cancel' first focuses it upon depressing the mouse button down,
/// then cancels the dialog upon releasing the mouse button. Because
/// 'Cancel' must always be permitted even for invalid input, it must
/// always be allowed to gain focus. However, focusing 'Cancel' does
/// not trigger validation. Furthermore, 'Cancel' can acquire focus
/// without cancelling the dialog, because the mouse may be moved
/// before the mouse button is released. However, in that case,
/// last_focused_window_ is not changed: to do so would serve no end,
/// and to avoid doing so preserves the useful invariant that
/// last_focused_window_ is the only window that may need validation.
///
/// WX !! It seems surprising that calling GetWindow() on the
/// wxChildFocusEvent argument doesn't return the same thing as
/// FindFocus(): instead, it returns a pointer to the notebook tab.

void XmlNotebook::UponChildFocus(wxChildFocusEvent&)
{
    // Do nothing until the notebook has been fully created.
    if(updates_blocked_)
        {
        return;
        }

    wxWindow* new_focused_window = FindFocus();

    // Do nothing if focus hasn't changed. This case arises when
    // another application is activated, and then this application
    // is reactivated.
    if(old_focused_window_ == new_focused_window)
        {
        return;
        }

    if(FindWindow(wxID_CANCEL) == new_focused_window)
        {
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
    // after this function has returned and after the pending
    // focus change has occurred, solves that problem.
    if(hold_focus_window_)
        {
        wxCommandEvent event0(wxEVT_REFOCUS_INVALID_CONTROL);
        wxPostEvent(this, event0);
        }
}

// TODO ?? As this is written, it can't call Skip() because it
// changes the value of updates_blocked_ between the two actions
// that the base-class handler would perform. Rethink this.

void XmlNotebook::UponInitDialog(wxInitDialogEvent&)
{
    Setup(GetChildren());

    TransferDataToWindow();

    updates_blocked_ = false;
    UpdateWindowUI(wxUPDATE_UI_RECURSE);
}

/// This augments wxDialog::UponOK(), but isn't a complete replacement.
/// It calls that base-class function explicitly because Skip()
/// wouldn't work here.

void XmlNotebook::UponOK(wxCommandEvent& event)
{
    wxDialog::OnOK(event);
    if(wxID_OK != GetReturnCode())
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
void XmlNotebook::UponPageChanged(wxNotebookEvent& event)
{
    ConditionallyEnable();
    CurrentPage().TransferDataToWindow();

    // WX !! Normally, the first interactive control would have focus.
    // But if that control was just disabled, then no window has
    // focus--so focus might need to be reset now. See:
    //   http://lists.nongnu.org/archive/html/lmi/2005-11/msg00040.html

    EnsureOptimalFocus();
}

/// Veto a page change if Validate() fails--but never veto the very
/// first page-change event, engendered by notebook creation, as doing
/// so leads to a segfault, at least with wxmsw-2.5.4 . See
///   http://lists.gnu.org/archive/html/lmi/2006-03/msg00000.html
///
/// Called when page is about to change, but hasn't yet.

void XmlNotebook::UponPageChanging(wxNotebookEvent& event)
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

void XmlNotebook::UponRefocusInvalidControl(wxCommandEvent&)
{
    LMI_ASSERT(hold_focus_window_);
    hold_focus_window_->SetFocus();
}

void XmlNotebook::UponUpdateGUI(wxUpdateUIEvent& event)
{
    // Do nothing if
    //  - the notebook has not been fully created, or
    //  - updates are temporarily blocked, or
    //  - the application is not active.
    if(updates_blocked_ || !wxTheApp->IsActive())
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
    std::vector<std::string> names_of_changed_controls;
    std::map<std::string,std::string>::const_iterator i;
    for(i = transfer_data_.begin(); i != transfer_data_.end(); ++i)
        {
        std::string const& name = i->first;
        try
            {
            if
                (   CurrentPage().FindWindow(XRCID(name.c_str()))
                &&  input_[name].str() != i->second
                )
                {
                names_of_changed_controls.push_back(name);
                input_[name] = i->second;
                }
            }
        catch(std::exception const& e)
            {
            DiagnosticsWindow().SetLabel(name + ": " + e.what());
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
        // Only text controls are validated on losing focus.
        return;
        }

    Transferor* t = dynamic_cast<Transferor*>(textctrl->GetValidator());
    if(!t)
        {
//        fatal_error() << "No transferor associated with control." << LMI_FLUSH;

// TODO ?? This happens only on page losing focus, probably because of
// an updates_blocked_ conditional. Test on page gaining focus instead.

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

    // Assume that UponUpdateGUI() has already been called.
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

