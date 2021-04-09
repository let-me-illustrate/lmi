// Transfer data between wx standard controls and std::strings.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// Acknowledgment

// wxWindows-2.4.1 file 'include/wx/valgen.h', copyright 1999 by its
// author Kevin Smith and released under the wxWindows license, solves
// a similar problem. The present file draws some ideas from that work
// but is implemented differently and has different goals, so any
// problems introduced here don't reflect on Kevin Smith's reputation.

#ifndef transferor_hpp
#define transferor_hpp

#include "config.hpp"

#include <wx/validate.h>

#include <string>

enum transfer_direction
    {from_control_to_string
    ,from_string_to_control
    };

/// Design notes
///
/// Class Transferor transfers data between standard controls and
/// std::strings, whereas class wxGenericValidator transfers data
/// between standard controls and control-specific datatypes:
///
///   wxGenericValidator datatypes
/// bool*       wxCheckBox, wxRadioButton
/// wxString*   wxButton, wxComboBox, wxStaticText, wxTextCtrl
/// int*        wxChoice, wxGauge, wxRadioBox, wxSpinButton, wxSpinCtrl
///               and, oddly enough, wxScrollBar
/// wxArrayInt* wxCheckListBox, wxListBox
///
/// The wx 'validator' classes are thus designed to bind directly to
/// variables of multifarious wx datatypes, but this class is designed
/// to bind to std::strings, which can it turn be bound to any other
/// type that may be desired using only standard C++. This class's
/// implementation would be trivial if wx provided member functions
///   wxControl::StringValue(std::string const&);
///   std::string wxControl::GetStringValue() const;
/// but the library's interface is less simple, e.g.
///   wxButton       * str = GetLabel()             SetLabel(*str)
///   wxCheckBox     * bool = GetValue()            SetValue(*bool)
///   wxCheckListBox * str = GetStringSelection()   SetStringSelection(*str)
///   wxChoice       * str = GetStringSelection()   SetStringSelection(*str)
///   wxComboBox     * int = GetSelection()         SetSelection(*int)
///                  * str = GetValue()             SetStringSelection(*str)
///                                                 SetValue(*str)
///   wxGauge        * int = GetValue()             SetValue(*int)
///   wxListBox      * str = GetStringSelection()   SetStringSelection(*str)
///                  if(Selected(i)) array->Add(i)  SetSelection(array->Item(i))
///   wxRadioBox     * int = GetSelection()         SetSelection(*int)
///                  * str = GetStringSelection()   SetStringSelection(*str)
///   wxRadioButton  * int = GetValue()             SetValue(*int)
///   wxScrollBar    * int = GetThumbPosition       SetThumbPosition(*int)
///   wxSlider       * int = GetValue()             SetValue(*int)
///   wxSpinButton   * int = GetValue()             SetValue(*int)
///   wxSpinCtrl     * int = GetValue()             SetValue(*int)
///   wxStaticText   * str = GetLabel()             SetLabel(*str)
///   wxTextCtrl     * str = GetValue()             SetValue(*str)
///
/// The base class's name is confusing: it only transfers data, and
/// validates nothing. Its Validate() member function simply returns
/// 'false' in all cases, apparently to force users to derive from it.
/// This class overrides Validate() to return 'true' in all cases,
/// because wx uses 'false' to signal failure. Even though this
/// overridden function may be called, it doesn't actually validate
/// anything--this design contemplates performing validation on the
/// strings that controls are transferred to.
///
/// This class is less general than wx 'validators' in that it permits
/// only single selections with controls that might allow multiple
/// selections. Multiple-selection controls are not used in the
/// application this class was designed for, whose users find such
/// controls confusing.
///
/// Technical notes
///
/// The wx documentation says "All validator classes must implement the
/// Clone function, which returns an identical copy of itself." The
/// base class is uncopyable, although it has a nonvirtual Copy()
/// function that it is apparently safe to ignore here.
///
/// Boost coding guideline 15.5 warns that reference members preclude
/// assignment, but this class is Nonassignable, and nonreseatable
/// semantics are exactly appropriate for the string bound to.
///
/// Base-class function Validate() returns false by default. It is
/// overridden here to return true instead.

class Transferor
    :public wxValidator
{
  public:
    Transferor(std::string& data, std::string const& name);
    ~Transferor() override = default;

    // wxValidator overrides.
    wxObject* Clone() const override;
    bool TransferFromWindow() override;
    bool TransferToWindow() override;
    bool Validate(wxWindow*) override;

    std::string const& name() const;

  private:
    Transferor(Transferor const&) = delete;
    Transferor& operator=(Transferor const&) = delete;

    bool PerformTransfer(transfer_direction);

    std::string& data_;
    std::string const name_;

    DECLARE_CLASS(Transferor)
};

#endif // transferor_hpp
