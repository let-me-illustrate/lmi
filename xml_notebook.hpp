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

// $Id: xml_notebook.hpp,v 1.14 2006-03-09 13:30:15 chicares Exp $

#ifndef xml_notebook_hpp
#define xml_notebook_hpp

#include "config.hpp"

#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <wx/dialog.h>
#include <wx/notebook.h> // wxNotebookEvent, wxNotebookPage
#include <wx/stattext.h>

#include <map>
#include <string>

class Input;
class mc_enum_base;
class WXDLLEXPORT wxControlWithItems;

/// A Model-View-Controller (MVC) framework for validated wx input.
///
/// What it is.
///
/// This framework handles enablement rules and value constraints for
/// GUI input in a dialog, or in a set of interrelated dialogs in a
/// wx 'notebook'.
///
/// What it isn't.
///
/// MVC may suggest Doc-View to wx programmers. This is different. It
/// has nothing to do with wxDocument and wxView, whose purpose is to
/// mediate between files and applications. Here, the model needn't
/// even be a document, and the purpose is to mediate between dialogs
/// and data constrained by rules.
///
/// Event-driven programming: not.
///
/// Many frameworks encourage an event-driven implementation, which wx
/// can of course support:
///
///   EVT_CHANGE_DATE(BirthDate,      Dialog::UponChangeBirthDate)
///   EVT_CHANGE_DATE(MarriageDate,   Dialog::UponChangeMarriageDate)
///   EVT_CHANGE_DATE(RetirementDate, Dialog::UponChangeRetirementDate)
///
///  - Dialog knows what controls it contains.
///  - Dialog::UponChangeXXXDate() knows the semantics of its
///      particular 'XXX' date instance.
///  - Dialog::UponChangeXXXDate() knows the semantics of other
///      controls that are affected by it: one date's value may affect
///      another date's range, e.g., one may marry after retirement,
///      but not before birth.
///
/// This is the "RAD" or "visual" approach. The programmer places a
/// control on a dialog, then sets its "properties" or writes event
/// handlers. It's easy to create a prototype this way, but it's easy
/// to make mistakes, too. The example above has three separate event
/// handlers that must do similar things, creating a temptation to use
/// copy-and-paste programming; after a few years of changes, they may
/// grow to look very different--especially if the controls are on
/// different dialogs, which are generally implemented in different
/// classes within different source files.
///
/// Each event handler probably resets the range of the other two
/// controls. If that forces another control's out-of-range value to
/// change, then that other control may emit events that trigger their
/// own handlers, which can require semaphores to prevent circularity.
///
/// This approach may be okay for dialogs with no interdependency
/// among controls, such as {name, address, city}. But if dependencies
/// are present, where are they coded? In the event-driven paradigm:
///  - input is managed by dialogs;
///  - dialogs are in distinct translation units;
///  - dialogs have controls;
///  - controls have properties or event handlers;
/// and it seems natural to code dependencies in event handlers. Logic
/// to validate input is then scattered across many functions in many
/// GUI-dependent translation units. If it later becomes necessary to
/// validate input from another source such as a web server, or if the
/// application is migrated to a different GUI framework, then all the
/// validation code must be factored out of the GUI event handlers,
/// which is no simple task. MVC can prevent these problems.
///
/// Separation of concerns.
///
/// In this MVC implementation, everything is factored into three
/// distinct classes with minimal interdependencies. Ideally,
///  - only the Model knows problem-domain rules;
///  - only the View knows which data a particular user sees, or
///      what controls represent them;
///  - the controller knows nothing except how to work with Models and
///      Views generically;
/// and this stringent separation of concerns makes the implementation
/// smaller, more likely to be correct, and easier to understand and
/// maintain, yet flexible. This ideal is not completely achieved
/// here: see the "Shortcomings" section below.
///
/// Model: A class that contains all input parameters as data members,
/// and implements all problem-domain rules. Parameters are UDTs that
/// know what values are valid (date within some range, e.g.). One
/// control's valid range or enablement may depend on another's value;
/// member functions work with the parameter UDTs to handle that. One
/// could wish for a prolog inference engine integrated with C++,
/// because this is all about backward chaining over Horn clauses.
///
/// Each UDT embodies the desired enablement state of the associated
/// control. Each numeric UDT embodies a range of allowable values.
/// Each enumerative UDT embodies a subset of allowable enumerators.
/// Collectively, these properties embody the problem-domain rules.
///
/// As implemented here, the Model uses a symbolic-member-name library
/// so that, e.g.,
///   Model["DateOfBirth"] maps to Model::DateOfBirth
/// and a member function returns a list of all data members, which
/// can be traversed with iterators. Similarly, its enumerative UDTs
/// provide string names for each enumerator, e.g., so that
///   Model["Gender"] = "Female";
/// may be written without worrying about particular enum values.
///
/// View: An '.xrc' file that specifies control layout. Control names
/// in the '.xrc' file must match the Model's data-member names: e.g.,
///   <object class="wxDatePickerCtrl" name="DateOfBirth">
/// requires
///   wxDateTime Model::DateOfBirth; // Same name.
/// which may be accessed by the same string:
///   Model["DateOfBirth"] // As above.
/// Furthermore, enumerative controls such as wxRadioBox or wxComboBox
/// must use the exact strings provided by the Model; otherwise, it
/// would be necessary either to depend on underlying enum values:
///   // This breaks if the enum value changes.
///   Model["Gender"] = 2;
/// or to translate strings somehow:
///   // Where would the translation table reside?
///   Model["Gender"] = translate("Woman");
/// either of which forces an undesirable dependency between the View
/// and the Model.
///
/// TODO ?? At least for now, control names must not be duplicated:
/// only one control can be bound to a data member in the Model. Is
/// this too restrictive?
///
/// TODO ?? It appears that the order of enumerators in a wxRadioBox
/// must also match the Model's ordering, which seems too restrictive.
///
/// Control types must be compatible with data types. Some data types
/// are compatible with more than one control type:
///  - numeric     UDTs: wxTextCtrl or perhaps wxSpinCtrl
///  - enumerative UDTs: wxRadioBox or a wxControlWithItems derivative
/// while others naturally map to a single control type:
///  - string      UDTs: wxTextCtrl
///  - date        UDTs: wxDatePickerCtrl
///
/// The View is a 'skin'. It can be customized for any set of users by
/// changing the '.xrc' file, without changing the program. Each skin
/// chooses among the compatible controls listed above, for each data
/// member. Compatibility does not imply good taste: wxSpinCtrl may be
/// a poor choice for an integer with a range [-10000, 10000], but the
/// framework doesn't forbid it.
///
/// Skins may ignore data members, provided of course that the Model
/// provides reasonable defaults.
///
/// The tabs in a 'notebook' are determined by the skin, as are the
/// selection, order, and layout of controls on each tab.
///
/// At least for now, wxRadioBox is a special case: unlike other
/// controls, the View must know what enumerative choices the Model
/// offers. This makes layout predictable, because the area required
/// to display a wxRadioBox depends on the number of enumerators and
/// the length of each one's string name. And it does prevent nasty
/// surprises, e.g., if one skin presents the names of all chemical
/// elements in a wxComboBox, and another tries to use a wxRadioBox
/// that requires astonishingly more space on the screen.
///
/// TODO ?? However, treating wxRadioBox as a special case creates a
/// regrettable dependency between the Model and the View, so perhaps
/// this design decision should be reconsidered.
///
/// This data-driven design precludes the use of wxButton in the View.
/// Other controls are stateful substantives, but pushbuttons are
/// stateless predicates. The framework could work around this, e.g.,
/// by binding a pushbutton to a semaphore that's raised when the
/// button is pressed and lowered when the Model has performed the
/// appropriate action. In practice, lmi hasn't needed pushbuttons
/// other than the usual {OK, Cancel, Help}, which are hardwired.
///
/// [TODO ?? Here, I'd like to say:
/// Controller: A template class with a Model parameter
///   Controller<ConcreteModel> : public wxDialog
/// but can't yet; meanwhile...]
///
/// Controller: A class derived from wxDialog that mediates between
/// the Model and the View.
///
/// The Controller's ctor maps Model and View entities: it gets the
/// Model's list of data members and pairs them with IDs in the View's
/// '.xrc' file. This relation need not be onto in either direction:
/// a particular View may well exclude some Model entities, and may
/// also have entities (such as static-text labels) that are not
/// present in the Model. However, for the intersection of the sets of
/// Model and View entities, the relation is one-to-one and onto.
/// Class Transferor, implemented and documented elsewhere, performs
/// this pairing and implements bidirectional data transfer.
///
/// The Controller essentially polls controls and reacts to changes in
/// their values. The polling is performed within wx, which generates
/// wxUpdateUIEvent pseudoevents. The resemblance to an event-driven
/// style is accidental: a fundamentally event-driven Controller would
/// need to anticipate all the value-change events that the View might
/// beget, and would suffer from the problems described above. Along
/// with wxUpdateUIEvent pseudoevents, the Controller also handles
/// focus and notebook-page-change events, merely so that it can force
/// a wxTextCtrl with invalid data to retain focus--a capability that
/// wx does not natively provide.
///
/// When control values change, the Controller passes them to the
/// Model. The Model validates the change (described separately below)
/// and updates its UDT members to reflect enablement and constraints
/// on numeric ranges or enumerator allowability. Then the Controller
/// updates the View to correspond to the updated Model, potentially
/// changing indicia of enablement or of constraints (such as the
/// choices available in a wxComboBox) as well as values in the View.
///
/// Validation.
///
/// [TODO ?? to be written--not well implemented yet]
///
/// Shortcomings.
///
/// These shortcomings are explained in detail above:
///  - only one control may be mapped to a Model data member;
///  - wxButton is not supported;
///  - for wxRadioBox, the View must know the Model's enumerators;
///  - for wxRadioBox, enumerators may not be reordered;
///  - the Model dictates enumeration strings: Views can't vary them;
///  - numeric validation is not yet completely implemented;
///  - the Controller should take the Model as a template parameter.
/// Furthermore, no automated unit tests are implemented yet.
///
/// A future version should factor out wxNotebook dependencies. This
/// class is derived only from wxDialog, so clearly it can work as
/// well with a wxDialog or anything similar.

namespace model_view_controller{} // doxygen workaround.

/// Design notes for class XmlNotebook.
///
/// TODO ?? Document every member here.
///
/// Bind() associates a string key (shared with the Model) with a
/// string in data member transfer_data_, the latter being passed by
/// non-const reference because its identity is important as well as
/// its value.
///
/// DiagnosticsWindow() returns a wxStaticText& where a wxWindow&
/// might seem more general. Rationale: the implementation uses
/// wxStaticText, whose contents can be written only with GetLabel();
/// other controls that might be used instead may implement GetLabel()
/// differently.
///
/// INELEGANT !! It would be better to write diagnostics to a custom
/// stream.
///
/// Data members that relate entities shared by Model and View--those
/// for which a Transferor exists.
///
/// transfer_data_ maps each name shared by Model and View to a string
/// that buffers the contents of controls in the View.
///
/// cached_transfer_data_ holds the prior contents of transfer_data_
/// when the latter is refreshed. This caching makes it possible to
/// determine which controls have changed.
///
/// Data members for managing focus.
///
/// Text controls are validated when they lose focus. For at least one
/// (and perhaps all) of the platforms wx supports, it is not possible
/// to block another control from gaining focus when the control
/// losing focus is determined to contain invalid input, so it is
/// necessary to track focus here in data members.
///
/// Data member hold_focus_window_ points to a control that should
/// retain focus because its contents are invalid.
///
/// Data member old_focused_window_ points to the window that last lost
/// focus--unless it's a 'Cancel' button, which gains focus without
/// triggering validation of the control that lost focus.

class XmlNotebook
    :public wxDialog
    ,private boost::noncopyable
    ,virtual private obstruct_slicing<XmlNotebook>
{
    friend class mvc_test;

  public:
    XmlNotebook(wxWindow* parent, Input& input);
    ~XmlNotebook();

  private:
    void Bind(std::string const& name, std::string& data) const;

    void ConditionallyEnable();
    void ConditionallyEnableControl(std::string const&, wxWindow&);
    void ConditionallyEnableItems  (std::string const&, wxWindow&);

    wxNotebookPage& CurrentPage() const;
    wxStaticText& DiagnosticsWindow() const;

    void EnsureOptimalFocus();

    bool ItemBoxNeedsRefreshing(mc_enum_base*, wxControlWithItems&);

    void UponChildFocus            (wxChildFocusEvent&);
    void UponInitDialog            (wxInitDialogEvent&);
    void UponOK                    (wxCommandEvent&   );
    void UponPageChanged           (wxNotebookEvent&  );
    void UponPageChanging          (wxNotebookEvent&  );
    void UponRefocusInvalidControl (wxCommandEvent&   );
    void UponUpdateGUI             (wxUpdateUIEvent&  );

    void RefreshItemBox(mc_enum_base*, wxControlWithItems&);

    void Setup(wxWindowList const&);

    void SetupControlItems(std::string const&, wxWindow& control);

    // wxDialog overrides.
    virtual bool TransferDataToWindow();

    void ValidateTextControl(wxWindow*);

    // Auxiliary functions to extend wxWindow::FindWindow().

#if !wxCHECK_VERSION(2,5,4)
    // Workaround: this function was not const until wx-2.5.4 .
    wxWindow* FindWindow(long int window_id) const;
#endif // !wxCHECK_VERSION(2,5,4)

    template<typename T>
    T& WindowFromXrcName(char const*) const;

    template<typename T>
    T& WindowFromXrcName(std::string const&) const;

    // Data members.

    Input& input_;

    wxWindow* hold_focus_window_;
    wxWindow* old_focused_window_;

    bool updates_blocked_;

    std::map<std::string,std::string> transfer_data_;
    std::map<std::string,std::string> cached_transfer_data_;

    DECLARE_EVENT_TABLE()
};

#endif // xml_notebook_hpp

