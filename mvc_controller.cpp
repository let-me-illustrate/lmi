// MVC Controller.
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

// $Id: mvc_controller.cpp,v 1.6 2006-08-12 17:16:33 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "mvc_controller.hpp"
#include "mvc_controller.tpp"

#include "alert.hpp"
#include "any_entity.hpp"
#include "map_lookup.hpp"
#include "mc_enum.hpp"
#include "mvc_model.hpp"
#include "mvc_view.hpp"
#include "rtti_lmi.hpp"
#include "tn_range.hpp"
#include "transferor.hpp"
#include "value_cast.hpp"
#include "wx_utility.hpp"
#include "wx_workarounds.hpp"

#include <wx/app.h> // wxTheApp
#include <wx/checkbox.h>
#include <wx/ctrlsub.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/xrc/xmlres.h>

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

namespace
{
wxEventType const wxEVT_REFOCUS_INVALID_CONTROL = wxNewEventType();
} // Unnamed namespace.

// WX !! wxDIALOG_EX_CONTEXTHELP is unknown to wxxrc.
// 'Extra' styles (such as wxWS_EX_VALIDATE_RECURSIVELY) that wxxrc
// recognizes can be specified as an <exstyle> attribute, which is
// tidier than setting them dynamically as in this message:
//   http://lists.wxwindows.org/archive/wxPython-users/msg15676.html
// But that technique is needed for wxDIALOG_EX_CONTEXTHELP, which
// wxxrc does not recognize; it must be specified here, before
// LoadDialog() is called.

MvcController::MvcController
    (wxWindow*      parent
    ,MvcModel&      model
    ,MvcView const& view
    )
    :model_                               (model)
    ,view_                                (view)
    ,last_focused_window_                 (parent)
    ,unit_test_idle_processing_completed_ (false)
    ,unit_test_refocus_event_pending_     (false)
    ,unit_test_under_way_                 (false)
{
    SetExtraStyle(GetExtraStyle() | wxDIALOG_EX_CONTEXTHELP);
    if(!wxXmlResource::Get()->LoadDialog(this, parent, view_.MainDialogName()))
        {
        fatal_error() << "Unable to load dialog." << LMI_FLUSH;
        }

    // This assignment must follow the call to LoadDialog().
    // Initialization to 'parent' in the ctor-initializer-list
    // ensures that 'last_focused_window_' always points to a
    // window upon which SetFocus() can be called. It's better
    // to use the dialog window, though, once that exists.

    last_focused_window_ = this;
    LMI_ASSERT(last_focused_window_);

    // Bind each pair of identically-named Model and View entities.
    //
    // The names of interactive (transfer-enabled) controls form a
    // subset of the Model class's member names. Some other controls,
    // such as static labels, don't need to be bound to any member of
    // the Model class. Some Model-class members may not be bound to
    // any control for a particular xml resource dialog.
    //
    // Call ModelReference() to ensure that each Model entity is of a
    // type derived from class datum_base.

    std::vector<std::string> const& mn = model_.Names();
    typedef std::vector<std::string>::const_iterator svci;
    for(svci i = mn.begin(); i != mn.end(); ++i)
        {
        ModelReference<datum_base>(*i);
        if(FindWindow(XRCID(i->c_str())))
            {
            Bind(*i, transfer_data_[*i] = model_.Entity(*i).str());
            }
        }

    lineage_ = Lineage(this);

    ::Connect
        (this
        ,wxEVT_INIT_DIALOG
        ,&MvcController::UponInitDialog
        );
}

MvcController::~MvcController()
{
}

/// Make the Model consistent, and change the View to comport with it.
///
/// Argument 'name_to_ignore' allows callers to specify one entity to
/// skip, so that NameOfTextControlRequiringValidation() can be
/// ignored unless it has just been validated successfully.

void MvcController::Assimilate(std::string const& name_to_ignore)
{
    model_.Reconcile();

    typedef std::map<std::string,std::string>::const_iterator smci;
    for(smci i = transfer_data_.begin(); i != transfer_data_.end(); ++i)
        {
        std::string const& name        = i->first;
        std::string const& model_value = model_.Entity(name).str();
        if(name == name_to_ignore || ModelAndViewValuesEquivalent(name))
            {
            continue;
            }
        transfer_data_       [name] = model_value;
        cached_transfer_data_[name] = model_value;
        wxWindow& w = WindowFromXrcName<wxWindow>(name);
        w.GetValidator()->TransferToWindow();
        }

    ConditionallyEnable();
}

void MvcController::Bind(std::string const& name, std::string& data) const
{
    Transferor transferor(data, name);
    WindowFromXrcName<wxWindow>(name).SetValidator(transferor);
}

void MvcController::ConditionallyEnable()
{
    wxWindowList const& wl = CurrentPage().GetChildren();
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
            ConditionallyEnableControl(t->name(), *pw);
            ConditionallyEnableItems  (t->name(), *pw);
            }
        else
            {
            // Do nothing. Some windows don't have validators--for
            // example, most static controls.
            }
        }
}

void MvcController::ConditionallyEnableControl
    (std::string const& name
    ,wxWindow&          control
    )
{
    control.Enable(ModelReference<datum_base>(name).is_enabled());
}

void MvcController::ConditionallyEnableItems
    (std::string const& name
    ,wxWindow&          control
    )
{
    mc_enum_base const* datum = ModelPointer<mc_enum_base>(name);

    wxCheckBox*         checkbox = dynamic_cast<wxCheckBox        *>(&control);
    wxRadioBox*         radiobox = dynamic_cast<wxRadioBox        *>(&control);
    wxControlWithItems* itembox  = dynamic_cast<wxControlWithItems*>(&control);
    bool const control_is_enumerative = radiobox || itembox;

    // Type 'mce_yes_or_no' is enumerative, but is often used with
    // checkboxes.
    if(checkbox || !control_is_enumerative && !datum)
        {
        return;
        }

    if(control_is_enumerative && !datum)
        {
        fatal_error()
            << "View control '"
            << name
            << "' is of enumerative type '"
            << lmi::TypeInfo(typeid(control))
            << "', but the corresponding Model datum, of type '"
            << lmi::TypeInfo(model_.Entity(name).type())
            << "', is not."
            << LMI_FLUSH
            ;
        }

    // TODO ?? Above, type() describes a pointer to member. It would
    // be better to indicate the member type only. Template class
    // any_member should provide member_type() and class_type()
    // functions.

    if(!control_is_enumerative && datum)
        {
        fatal_error()
            << "Model datum '"
            << name
            << "' is of enumerative type '"
            << lmi::TypeInfo(model_.Entity(name).type())
            << "', but the corresponding View control, of type '"
            << lmi::TypeInfo(typeid(control))
            << "', is not."
            << LMI_FLUSH
            ;
        }

    LMI_ASSERT(datum);

    if(radiobox)
        {
        // If a radiobox is disabled, then all its radiobuttons should
        // also be disabled. Even so, the framework might force a
        // different radiobutton to be selected.
        bool radiobox_enabled = radiobox->IsEnabled();
        for(std::size_t j = 0; j < datum->cardinality(); ++j)
            {
            radiobox->Enable(j, radiobox_enabled && datum->is_allowed(j));
            }
        radiobox->SetSelection(datum->ordinal());
        }
    else if(itembox)
        {
// WX !! Broken in wx-2.5.1:
// http://cvs.wxwidgets.org/viewcvs.cgi/wxWidgets/include/wx/ctrlsub.h?annotate=1.20
//                itembox->SetStringSelection("A");
// WX !! Broken in wx-2.5.1:
//                itembox->SetSelection(itembox->FindString("A"));
        // WX !! Freeze() doesn't seem to help much.
        itembox->Freeze();
        itembox->Clear();
        // WX !! Append(wxArrayString const&) "may be much faster"
        // according to wx online help, but that seems untrue: its
        // implementation just uses a loop.
        for(std::size_t j = 0; j < datum->cardinality(); ++j)
            {
            if(datum->is_allowed(j))
                {
                itembox->Append(datum->str(j));
                }
            }
        itembox->Select(datum->ordinal());
        itembox->Thaw();
        }
    else
        {
        fatal_error() << "Unexpected case." << LMI_FLUSH;
        }
}

wxNotebookPage& MvcController::CurrentPage() const
{
    // INELEGANT !! This window could be held elsewhere, e.g. as a reference.
    wxNotebook& book = WindowFromXrcName<wxNotebook>(view_.BookControlName());
    if(wxNOT_FOUND == book.GetSelection())
        {
        fatal_error() << "No page selected in notebook." << LMI_FLUSH;
        }
    wxNotebookPage* page = book.GetPage(book.GetSelection());
    if(!page)
        {
        fatal_error() << "Selected notebook page is invalid." << LMI_FLUSH;
        }
    return *page;
}

wxStaticText& MvcController::DiagnosticsWindow() const
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

void MvcController::EnsureOptimalFocus()
{
    wxWindow* f = FindFocus();
    if(f && f->IsEnabled() && f->AcceptsFocus())
        {
        return;
        }

    SetFocus();
    wxWindowList const& wl = CurrentPage().GetChildren();
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

void MvcController::Initialize()
{
    typedef std::vector<wxWindow*>::const_iterator wvci;
    for(wvci i = lineage_.begin(); i != lineage_.end(); ++i)
        {
        wxWindow* pw = *i;
        LMI_ASSERT(0 != pw);
        Transferor* t = dynamic_cast<Transferor*>(pw->GetValidator());
        if(t)
            {
            LMI_ASSERT(pw == &WindowFromXrcName<wxWindow>(t->name()));
            ConditionallyEnableControl(t->name(), *pw);
            ConditionallyEnableItems  (t->name(), *pw);
            }
        }
}

/// Ascertain equivalence of an entity's Model and View values.
///
/// Verbatim equality implies equivalence. String representations of
/// numbers, even if unequal, are yet equivalent if they evaluate to
/// the same number; only data of types derived from tn_range_base are
/// evaluated.
///
/// Type long double would be a better choice than type double for the
/// value_cast() comparison, but the former is not yet supported by
/// value_cast().

bool MvcController::ModelAndViewValuesEquivalent(std::string const& name) const
{
    std::string const& view_value = map_lookup(transfer_data_, name);
    std::string const& model_value = model_.Entity(name).str();
    bool equal = view_value == model_value;
    bool equivalent = equal;
    if(!equal && 0 != ModelPointer<tn_range_base>(name))
        {
        equivalent =
                value_cast<double>(view_value)
            ==  value_cast<double>(model_value)
            ;
        }
    return equivalent;
}

std::string MvcController::NameOfTextControlRequiringValidation() const
{
    wxWindow         * w = dynamic_cast<wxTextCtrl*>(last_focused_window_);
    wxValidator const* v = w ? w->GetValidator() : 0;
    Transferor  const* t = dynamic_cast<Transferor const*>(v);
    return (t && 0 != ModelPointer<tn_range_base>(t->name())) ? t->name() : "";
}

void MvcController::RefocusLastFocusedWindow()
{
    LMI_ASSERT(last_focused_window_ && last_focused_window_->IsEnabled());
    last_focused_window_->SetFocus();
}

void MvcController::TestModelViewConsistency() const
{
    std::vector<std::string> const& mn = model_.Names();
    typedef std::vector<std::string>::const_iterator svci;
    for(svci i = mn.begin(); i != mn.end(); ++i)
        {
        if(!FindWindow(XRCID(i->c_str())))
            {
            warning() << "No View entity matches '" << *i << "'\n";
            }
        }

    typedef std::vector<wxWindow*>::const_iterator wvci;
    for(wvci i = lineage_.begin(); i != lineage_.end(); ++i)
        {
        wxWindow* pw = *i;
        if
            (   pw->AcceptsFocus()
            &&  !dynamic_cast<Transferor*>(pw->GetValidator())
            &&  pw->GetId() != wxID_OK
            &&  pw->GetId() != wxID_CANCEL
            &&  pw->GetId() != wxID_HELP
            )
            {
            warning() << "No Model entity matches " << NameLabelId(pw) << ".\n";
            }
        }

    warning() << std::flush;
}

/// Cause a text control to be validated upon losing focus.
///
/// UponUpdateUI() doesn't handle focus changes, so this function is
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
/// (wxEVT_REFOCUS_INVALID_CONTROL) is posted: it reseizes focus for
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

void MvcController::UponChildFocus(wxChildFocusEvent& event)
{
    event.Skip();

    wxWindow* new_focused_window = FindFocus();

    // Do nothing if focus hasn't changed. This case arises when
    // another application is activated, and then this application
    // is reactivated.
    if(last_focused_window_ == new_focused_window)
        {
        return;
        }

    if(FindWindow(wxID_CANCEL) == new_focused_window)
        {
        return;
        }

    if(Validate())
        {
        if(new_focused_window)
            {
            last_focused_window_ = new_focused_window;
            }
        else
            {
            warning() << "Keyboard focus was lost." << LMI_FLUSH;
            RefocusLastFocusedWindow();
            }
        }
    else
        {
        LMI_ASSERT(!unit_test_refocus_event_pending_);
        if(!unit_test_under_way_)
            {
            wxCommandEvent event0(wxEVT_REFOCUS_INVALID_CONTROL);
            wxPostEvent(this, event0);
            }
        unit_test_refocus_event_pending_ = true;
        }
}

void MvcController::UponInitDialog(wxInitDialogEvent& event)
{
    event.Skip();

    Initialize();

    ::Connect
        (this
        ,wxEVT_REFOCUS_INVALID_CONTROL
        ,&MvcController::UponRefocusInvalidControl
        );
    ::Connect
        (this
        ,wxEVT_COMMAND_BUTTON_CLICKED
        ,&MvcController::UponOK
        ,wxID_OK
        );
    ::Connect
        (this
        ,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGING
        ,&MvcController::UponPageChanging
        ,XRCID(view_.BookControlName())
        );
    ::Connect
        (this
        ,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED
        ,&MvcController::UponPageChanged
        ,XRCID(view_.BookControlName())
        );
    ::Connect
        (this
        ,wxEVT_CHILD_FOCUS
        ,&MvcController::UponChildFocus
        );
    ::Connect
        (this
        ,wxEVT_UPDATE_UI
        ,&MvcController::UponUpdateUI
        ,XRCID(view_.MainDialogName())
        );
}

/// Validate a text control if necessary, then display all Model data.
/// Validation is handled by the base-class function.
///
/// This augments wxDialog::OnOK(), but isn't a complete replacement.
/// It must call that base-class function explicitly (or do everything
/// it would do, for each platform) in order to perform a conditional
/// action (none for the nonce) depending on the base-class function's
/// result. Skip(false) is written at the top to make it clear that
/// not calling Skip with its default 'true' value is no oversight.

void MvcController::UponOK(wxCommandEvent& event)
{
    event.Skip(false);

    wxDialog::OnOK(event);
    if(wxID_OK != GetReturnCode())
        {
        return;
        }

    // Perform any needed postprocessing here.
}

void MvcController::UponPageChanged(wxNotebookEvent& event)
{
    event.Skip();

    ConditionallyEnable();
}

/// Veto a page change if Validate() fails--but never veto the very
/// first page-change event, engendered by notebook creation, as doing
/// so leads to a segfault, at least with wxmsw-2.5.4: see
///   http://lists.gnu.org/archive/html/lmi/2006-03/msg00000.html
///
/// Is is imperative to unset the 'skip' flag before vetoing: see
///   http://lists.gnu.org/archive/html/lmi/2006-04/msg00008.html

void MvcController::UponPageChanging(wxNotebookEvent& event)
{
    event.Skip();

    UpdateWindowUI();

    if(!Validate())
        {
        event.Skip(false);
        event.Veto();
        RefocusLastFocusedWindow();
        }
}

void MvcController::UponRefocusInvalidControl(wxCommandEvent&)
{
    LMI_ASSERT(unit_test_refocus_event_pending_);
    RefocusLastFocusedWindow();
    unit_test_refocus_event_pending_ = false;
}

void MvcController::UponUpdateUI(wxUpdateUIEvent& event)
{
    event.Skip();

    unit_test_idle_processing_completed_ = false;

    // Explicitly disregard any wxUpdateUIEvent received while this
    // application is not active: see
    //   http://lists.gnu.org/archive/html/lmi/2006-03/msg00006.html
    // for a discussion.
    if(!wxTheApp->IsActive())
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
    TransferDataFromWindow();
    if(cached_transfer_data_ == transfer_data_)
        {
        unit_test_idle_processing_completed_ = true;
        return;
        }
    cached_transfer_data_ = transfer_data_;

    DiagnosticsWindow().SetLabel("");
    std::vector<std::string> names_of_changed_controls;
    std::string const name_to_ignore = NameOfTextControlRequiringValidation();
    typedef std::map<std::string,std::string>::const_iterator smci;
    for(smci i = transfer_data_.begin(); i != transfer_data_.end(); ++i)
        {
        std::string const& name       = i->first;
        std::string const& view_value = i->second;
        if(name == name_to_ignore || ModelAndViewValuesEquivalent(name))
            {
            continue;
            }
        try
            {
            names_of_changed_controls.push_back(name);
            model_.Entity(name) = view_value;
            }
        catch(std::exception const& e)
            {
            DiagnosticsWindow().SetLabel(name + ": " + e.what());
            }
        }

    // wxEVT_UPDATE_UI events should occur frequently enough that two
    // control changes cannot be simultaneous.
    if(1 < names_of_changed_controls.size())
        {
        warning() << "Contents of more than one control changed, namely\n";
        typedef std::vector<std::string>::const_iterator svci;
        for
            (svci i = names_of_changed_controls.begin()
            ;i     != names_of_changed_controls.end()
            ;++i
            )
            {
            warning() << *i << " changed" << '\n';
            }
        warning() << LMI_FLUSH;
        }

    Assimilate(name_to_ignore);
}

/// Validate a numeric control.
///
/// In this MVC framework, only controls representing numeric ranges
/// can contain invalid input. All other controls constrain input to
/// values that are known to be valid. Furthermore, because only the
/// focused control can be changed, and controls are validated upon
/// losing focus (and retain focus if validation fails), at most one
/// control can contain invalid input at any time.
///
/// Moreover, only controls of type wxTextCtrl can actually contain
/// invalid input. These other controls can contain numeric input:
///   wxDatePickerCtrl
///   wxGauge
///   wxScrollBar
///   wxSlider
///   wxSpinButton
///   wxSpinCtrl
/// but can't actually assume an invalid state because they are all
/// intrinsically range limited. At least in wx-2.7, the text-control
/// portion of a wxSpinButton silently discards ill-formed input, too.
///
/// Only controls with Transferors need be validated. A text control
/// in the View may legitimately correspond to no entity in the Model,
/// but is then presumptively invalid: knowing nothing about it, the
/// Model could never determine it to be invalid.

bool MvcController::Validate()
{
    std::string const name = NameOfTextControlRequiringValidation();
    if(name.empty())
        {
        return true;
        }

    UpdateWindowUI();
    tn_range_base const& datum = ModelReference<tn_range_base>(name);
    std::string const& view_value = map_lookup(transfer_data_, name);
    std::string diagnosis(datum.diagnose_invalidity(view_value));
    DiagnosticsWindow().SetLabel(diagnosis);
    if(diagnosis.empty())
        {
        model_.Entity(name) = view_value;
        Assimilate("");
        }
    return diagnosis.empty();
}

#if !wxCHECK_VERSION(2,5,4)
wxWindow* MvcController::FindWindow(long int window_id) const
{
    return const_cast<MvcController*>(this)->wxDialog::FindWindow(window_id);
}
#endif // !wxCHECK_VERSION(2,5,4)

