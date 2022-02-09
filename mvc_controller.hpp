// MVC Controller.
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef mvc_controller_hpp
#define mvc_controller_hpp

#include "config.hpp"

#include <wx/bookctrl.h>                // wxBookCtrlBase, wxBookCtrlBaseEvent
#include <wx/dialog.h>
#include <wx/stattext.h>

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

class MvcModel;
class MvcView;

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
///  - enumerative UDTs: wxRadioBox or a wxItemContainer derivative
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
/// [TODO ?? to be written...]
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
/// Furthermore, automated unit tests are not yet fully implemented.
///
/// A future version should factor out wxBookCtrlBase dependencies.
/// This class is derived only from wxDialog, so clearly it can work
/// as well with a wxDialog or anything similar.

namespace model_view_controller{} // doxygen workaround.

/// Design notes for class MvcController.
///
/// Public member functions.
///
/// TestModelViewConsistency(): Diagnose inconsistencies between the
/// Model and the View. This function is designed for developers
/// rather than end users, and makes little attempt to avoid false
/// positives.
///
/// Private member functions.
///
/// Assimilate(): Enforce relationships among entities in the Model,
/// and update the View to reflect consequent changes.
///
/// Bind(): Associate a string key (shared with the Model) with a
/// string in data member transfer_data_, the latter being passed by
/// non-const reference because its identity is important as well as
/// its value.
///
/// BookControl(): Return a reference to the book control.
///
/// ConditionallyEnable(): Enforce Model enablement rules.
///
/// ConditionallyEnableControl(): Enforce Model control-enablement
/// rules.
///
/// ConditionallyEnableItems(): Enforce Model item-enablement rules
/// for controls with subitems--wxItemContainerImmutable derivatives
/// (i.e., wxRadioBox, and wxItemContainer derivatives).
///
/// CurrentPage(): Return the current notebook page. Throws an
/// exception if none is selected, or if the selected page is
/// invalid.
///
/// DiagnosticsWindow(): Return a window suitable for displaying
/// diagnostic messages. Returns a wxStaticText& where a wxWindow&
/// might seem more general. Rationale: the implementation uses
/// wxStaticText, whose contents can be written only with GetLabel();
/// other controls that might be used instead may implement GetLabel()
/// differently.
///
/// INELEGANT !! It would be better to write diagnostics to a custom
/// stream.
///
/// EnsureOptimalFocus(): If this MVC framework disables the focused
/// control, move focus to a window that can accept it--ideally, to a
/// control that accepts keyboard input.
///
/// Initialize(): Ensure that enumerative controls have appropriate
/// items when the dialog is initialized.
///
/// ModelAndViewValuesEquivalent(): Ascertain equivalence of an
/// entity's Model and View values.
///
/// ModelPointer(): Pointer to a Model entity, looked up by string
/// name, and cast to a particular type; null if the cast fails.
/// Throws if the entity doesn't exist.
///
/// ModelReference(): Result of ModelPointer(), dereferenced if not
/// null; throws if null.
///
/// NameOfControlToDeferEvaluating(): Name of control, if any, whose
/// value is to be ignored temporarily, because it is of a type that
/// accepts incremental input and it is the last_focused_window_;
/// else, an empty string. A wxTextCtrl, for example, accepts input
/// one character at a time, but if "1e-1" is entered, then it would
/// be premature to consider the contents to constitute an invalid
/// floating-point number after the second character is typed.
///
/// RefocusLastFocusedWindow(): Move focus to last_focused_window_.
///
/// UpdateCircumscription(): Update numeric range limits for controls
/// that have them.
///
/// UponChildFocus(): Trigger validation of a text control when it
/// loses focus. This event handler is needed because UponUpdateUI()
/// doesn't handle focus changes.
///
/// UponInitDialog(): Doesn't do much yet....
///
/// UponPageChanged(): Enable controls and control items appropriately
/// when the notebook page changes. Enablement matters only on the
/// current page, and performing it on other pages has been observed
/// to make the system noticeably slower.
///
/// UponPageChanging(): Trigger validation of a text control when the
/// notebook page is about to change; veto the page change if
/// validation fails. This event handler is needed because
/// UponUpdateUI() doesn't handle focus changes.
///
/// UponRefocusInvalidControl(): Call RefocusLastFocusedWindow() when
/// a custom wxEVT_REFOCUS_INVALID_CONTROL event occurs.
///
/// UponUpdateUI(): Update the Model and the View as data is entered,
/// dynamically enforcing Model rules.
///
/// Validate(): Enforce numeric range limits when edit controls that
/// represent numbers lose focus. Limits are not enforced as each
/// keystroke is typed, because, e.g., it is valid to type '-' in an
/// edit control constrained to [-100, 100], provided that a digit is
/// typed after it before moving focus.
///
/// FindWindow(): [This const member function merely forwards to the
/// wx function it hides, which is regrettably not const in older wx
/// versions. It may someday be expunged.]
///
/// template<typename T> T& WindowFromXrcName(): Obtain a reference to
/// an object of class T from its XRC name, throwing an exception if
/// unobtainable.
///
/// Data members.
///
/// Use of reference members precludes copying and assignment, which
/// aren't useful anyway.
///
/// model_: Reference to Model.
///
/// view_: Reference to View.
///
/// itemboxes_cache_: Cached items of wxItemContainer instances,
/// identified by their names, in the View.
///
/// last_focused_window_: Points to the last window, other than a
/// 'Cancel' pushbutton, that had gained focus without losing it
/// immediately due to UponRefocusInvalidControl(). This is the one
/// and only window that potentially requires validation.
///
/// lineage_: Flat vector of all children, grandchildren, etc.
///
/// transfer_data_: Maps each name shared by Model and View to a
/// string that buffers the contents of controls in the View.
///
/// cached_transfer_data_: Holds the prior contents of transfer_data_
/// when the latter is refreshed. This caching makes it possible to
/// determine which controls have changed.
///
/// unit_test_idle_processing_completed_: True iff the Controller has
/// determined that it has no more work to do, absent any user input
/// changes, because the Model and the View are consistent. Useful
/// only for unit testing; in any other context, the possibility of
/// user input changes cannot be ruled out.
///
/// unit_test_refocus_event_pending_: True iff the Controller has
/// posted a custom wxEVT_REFOCUS_INVALID_CONTROL event and focus has
/// not yet been restored to last_focused_window_. Useful only for
/// unit testing, which explicitly addresses this circumstance.
///
/// unit_test_under_way_: True iff a unit test is in charge of the
/// Controller.

class MvcController final
    :public wxDialog
{
    friend class MvcTest;

  public:
    MvcController(wxWindow* parent, MvcModel&, MvcView const&);
    ~MvcController() override = default;

    void TestModelViewConsistency() const;

    MvcModel const& Model() const;

  private:
    MvcController(MvcController const&) = delete;
    MvcController& operator=(MvcController const&) = delete;

    void Assimilate(std::string const& name_to_ignore);
    void Bind(std::string const& name, std::string& data) const;
    wxBookCtrlBase      & BookControl()      ;
    wxBookCtrlBase const& BookControl() const;
    void ConditionallyEnable();
    void ConditionallyEnableControl(std::string const&, wxWindow&);
    void ConditionallyEnableItems  (std::string const&, wxWindow&);
    wxWindow& CurrentPage() const;
    wxStaticText& DiagnosticsWindow() const;
    void EnsureOptimalFocus();
    void Initialize();
    bool ModelAndViewValuesEquivalent(std::string const& name) const;

    template<typename T>
    T const* ModelPointer(std::string const& name) const;

    template<typename T>
    T const& ModelReference(std::string const& name) const;

    std::string NameOfControlToDeferEvaluating() const;
    void RefocusLastFocusedWindow();
    void UpdateCircumscription(wxWindow&, std::string const&);

    void UponChildFocus            (wxChildFocusEvent&   );
    void UponInitDialog            (wxInitDialogEvent&   );
    void UponPageChanged           (wxBookCtrlBaseEvent& );
    void UponPageChanging          (wxBookCtrlBaseEvent& );
    void UponRefocusInvalidControl (wxCommandEvent&      );
    void UponUpdateUI              (wxUpdateUIEvent&     );

    // wxWindow overrides.
    bool Validate() override;

    // Auxiliary functions to extend wxWindow::FindWindow().

    template<typename T>
    T& WindowFromXrcName(char const*) const;

    template<typename T>
    T& WindowFromXrcName(std::string const&) const;

    MvcModel& model_;
    MvcView const& view_;

    std::unordered_map<std::string, std::vector<wxString>> itemboxes_cache_;

    wxWindow* last_focused_window_;

    std::vector<wxWindow*> lineage_;

    std::map<std::string,std::string> transfer_data_;
    std::map<std::string,std::string> cached_transfer_data_;

    bool unit_test_idle_processing_completed_;
    bool unit_test_refocus_event_pending_;
    bool unit_test_under_way_;
};

#endif // mvc_controller_hpp
