// Transfer data between wx standard controls and std::strings.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

// Acknowledgment

// wxWindows-2.4.1 file 'src/common/valgen.cpp', copyright 1999 by its
// author Kevin Smith and released under the wxWindows license, solves
// a similar problem. The present file draws some ideas from that work
// but is implemented differently and has different goals, so any
// problems introduced here don't reflect on Kevin Smith's reputation.

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "transferor.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "numeric_io_cast.hpp"
#include "wx_utility.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/datectrl.h>
#include <wx/dynarray.h>
#include <wx/gauge.h>
#include <wx/intl.h>
#include <wx/listbox.h>
#include <wx/radiobox.h>
#include <wx/radiobut.h>
#include <wx/scrolbar.h>
#include <wx/slider.h>
#include <wx/spinbutt.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>

IMPLEMENT_CLASS(Transferor, wxValidator)

namespace
{
    bool Transfer(transfer_direction, std::string&, wxButton&         );
    bool Transfer(transfer_direction, std::string&, wxCheckBox&       );
    bool Transfer(transfer_direction, std::string&, wxCheckListBox&   );
    bool Transfer(transfer_direction, std::string&, wxChoice&         );
    bool Transfer(transfer_direction, std::string&, wxComboBox&       );
    bool Transfer(transfer_direction, std::string&, wxDatePickerCtrl& );
    bool Transfer(transfer_direction, std::string&, wxGauge&          );
    bool Transfer(transfer_direction, std::string&, wxListBox&        );
    bool Transfer(transfer_direction, std::string&, wxRadioBox&       );
    bool Transfer(transfer_direction, std::string&, wxRadioButton&    );
    bool Transfer(transfer_direction, std::string&, wxScrollBar&      );
    bool Transfer(transfer_direction, std::string&, wxSlider&         );
    bool Transfer(transfer_direction, std::string&, wxSpinButton&     );
    bool Transfer(transfer_direction, std::string&, wxSpinCtrl&       );
    bool Transfer(transfer_direction, std::string&, wxStaticText&     );
    bool Transfer(transfer_direction, std::string&, wxTextCtrl&       );
} // Unnamed namespace.

Transferor::Transferor(std::string& data, std::string const& name)
    :wxValidator()
    ,data_(data)
    ,name_(name)
{
}

Transferor::~Transferor()
{
}

wxObject* Transferor::Clone() const
{
    return new Transferor(data_, name_);
}

std::string const& Transferor::name() const
{
    return name_;
}

bool Transferor::TransferToWindow()
{
    return PerformTransfer(from_string_to_control);
}

bool Transferor::TransferFromWindow()
{
    return PerformTransfer(from_control_to_string);
}

bool Transferor::Validate(wxWindow*)
{
    return true;
}

// Class wxGenericValidator uses switch on type in its similar
// facility; wx doesn't seem to support any alternative easily.
// The ordering of the switch must recognize that
//   wxCheckListBox is derived from wxListBox
//   wxComboBox     is derived from wxChoice
// and test derived classes first.

// INELEGANT !! Avoid this switch on type.

bool Transferor::PerformTransfer(transfer_direction td)
{
    if(!m_validatorWindow)
        {
        fatal_error()
            << "Validator for '"
            << name()
            << "' not bound to any control."
            << LMI_FLUSH
            ;
        }
    wxWindowBase* control = m_validatorWindow;

    wxButton         * button       ;
    wxCheckBox       * checkbox     ;
    wxCheckListBox   * checklistbox ;
    wxComboBox       * combobox     ;
    wxChoice         * choice       ;
    wxDatePickerCtrl * datepicker   ;
    wxGauge          * gauge        ;
    wxListBox        * listbox      ;
    wxRadioBox       * radiobox     ;
    wxRadioButton    * radiobutton  ;
    wxScrollBar      * scrollbar    ;
    wxSlider         * slider       ;
    wxSpinButton     * spinbutton   ;
    wxSpinCtrl       * spinctrl     ;
    wxStaticText     * statictext   ;
    wxTextCtrl       * textctrl     ;

    if     (0 != (button       = dynamic_cast<wxButton         *>(control)))
        return Transfer(td, data_,             *button      );
    else if(0 != (checkbox     = dynamic_cast<wxCheckBox       *>(control)))
        return Transfer(td, data_,             *checkbox    );
    else if(0 != (checklistbox = dynamic_cast<wxCheckListBox   *>(control)))
        return Transfer(td, data_,             *checklistbox);
    else if(0 != (combobox     = dynamic_cast<wxComboBox       *>(control)))
        return Transfer(td, data_,             *combobox    );
    else if(0 != (choice       = dynamic_cast<wxChoice         *>(control)))
        return Transfer(td, data_,             *choice      );
    else if(0 != (datepicker   = dynamic_cast<wxDatePickerCtrl *>(control)))
        return Transfer(td, data_,             *datepicker  );
    else if(0 != (gauge        = dynamic_cast<wxGauge          *>(control)))
        return Transfer(td, data_,             *gauge       );
    else if(0 != (listbox      = dynamic_cast<wxListBox        *>(control)))
        return Transfer(td, data_,             *listbox     );
    else if(0 != (radiobox     = dynamic_cast<wxRadioBox       *>(control)))
        return Transfer(td, data_,             *radiobox    );
    else if(0 != (radiobutton  = dynamic_cast<wxRadioButton    *>(control)))
        return Transfer(td, data_,             *radiobutton );
    else if(0 != (scrollbar    = dynamic_cast<wxScrollBar      *>(control)))
        return Transfer(td, data_,             *scrollbar   );
    else if(0 != (slider       = dynamic_cast<wxSlider         *>(control)))
        return Transfer(td, data_,             *slider      );
    else if(0 != (spinbutton   = dynamic_cast<wxSpinButton     *>(control)))
        return Transfer(td, data_,             *spinbutton  );
    else if(0 != (spinctrl     = dynamic_cast<wxSpinCtrl       *>(control)))
        return Transfer(td, data_,             *spinctrl    );
    else if(0 != (statictext   = dynamic_cast<wxStaticText     *>(control)))
        return Transfer(td, data_,             *statictext  );
    else if(0 != (textctrl     = dynamic_cast<wxTextCtrl       *>(control)))
        return Transfer(td, data_,             *textctrl    );
    else
        {
        fatal_error()
            << "Unrecognized control '"
            << name()
            << "'."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

// TODO ?? Use TransferString() with class wxControlWithItems instead
// of its derived classes, now that wx has been changed to implement
// wxControlWithItems::SetSelection(). But consider whether class
// wxItemContainerImmutable can be used for all those classes and
// wxRadioBox as well.

namespace
{
    bool TransferLabel(transfer_direction td, std::string& data, wxControl& control)
    {
        if(td == from_string_to_control)
            {
            control.SetLabel(data);
            }
        else
            {
            data = control.GetLabel();
            }
        return true;
    }

    // TODO ?? Explain why this uses "Yes" and "No" instead of
    // true and false, or change it to support bool, e.g.
    //   control.SetValue(numeric_io_cast<bool>(data));
    //   data = numeric_io_cast<std::string>(control.GetValue());
    //
    template<typename T>
    bool TransferBool(transfer_direction td, std::string& data, T& control)
    {
        if(td == from_string_to_control)
            {
            if("Yes" == data)
                {
                control.SetValue(true);
                }
            else if("No" == data)
                {
                control.SetValue(false);
                }
            else
                {
                warning()
                    << "Boolean data is unexpectedly numeric"
// TODO ?? Show the offending control's name (unavailable for now).
//                    << " for control '" << name() << "'"
                    << "."
                    << LMI_FLUSH
                    ;
                return false;
                }
            }
        else
            {
            data = control.GetValue() ? "Yes" : "No";
            }
        return true;
    }

    template<typename T>
    bool TransferInt(transfer_direction td, std::string& data, T& control)
    {
        if(td == from_string_to_control)
            {
            control.SetValue(numeric_io_cast<int>(data));
            }
        else
            {
            data = numeric_io_cast<std::string>(control.GetValue());
            }
        return true;
    }

    template<typename T>
    bool TransferString(transfer_direction td, std::string& data, T& control)
    {
        if(td == from_string_to_control)
            {
            if(wxNOT_FOUND != control.FindString(data))
                {
                control.SetStringSelection(data);
                }
            else
                {
                warning()
                    << "String '"
                    << data
                    << "' not found"
// TODO ?? Show the offending control's name (unavailable for now).
//                    << " for control '" << name() << "'"
                    << "."
                    << LMI_FLUSH
                    ;
                return false;
                }
            }
        else
            {
#if !wxCHECK_VERSION(2,6,2)
            // For dropdown comboboxes, scrolling through the list
            // selects the highlighted item immediately, with wx
            // versions prior to 2.6.2 . This error directive may be
            // suppressed if living with that problem is acceptable.
#   error Outdated library: wx-2.6.2 or greater is required.
#endif // !wxCHECK_VERSION(2,6,2)
            data = control.GetStringSelection();
            }
        return true;
    }

    bool Transfer(transfer_direction td, std::string& data, wxButton& control)
    {
        return TransferLabel(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxCheckBox& control)
    {
        return TransferBool(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxCheckListBox& control)
    {
        if(!(wxLB_SINGLE & control.GetWindowStyle()))
            {
            fatal_error()
                << "CheckListBox"
// TODO ?? Show the offending control's name (unavailable for now).
//                << "Control '" << name() << "'"
                << " must be constrained to a single selection."
                << LMI_FLUSH
                ;
            }
        return TransferString(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxChoice& control)
    {
        return TransferString(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxComboBox& control)
    {
        if(!(wxCB_READONLY & control.GetWindowStyle()))
            {
            fatal_error()
                << "ComboBox"
// TODO ?? Show the offending control's name (unavailable for now).
//                << "Control '" << name() << "'"
                << " must be read only."
                << LMI_FLUSH
                ;
            }
        return TransferString(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxDatePickerCtrl& control)
    {
        if(td == from_string_to_control)
            {
            calendar_date lmi_date;
            lmi_date.julian_day_number(numeric_io_cast<int>(data));
            wxDateTime wx_date = ConvertDateToWx(lmi_date);
            control.SetValue(wx_date);
            }
        else
            {
            wxDateTime wx_date = control.GetValue();
            calendar_date lmi_date = ConvertDateFromWx(wx_date);
            data = numeric_io_cast<std::string>(lmi_date.julian_day_number());
            }
        return true;
    }

    bool Transfer(transfer_direction td, std::string& data, wxGauge& control)
    {
        return TransferInt(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxListBox& control)
    {
        if(!(wxLB_SINGLE & control.GetWindowStyle()))
            {
            fatal_error()
                << "ListBox"
// TODO ?? Show the offending control's name (unavailable for now).
//                << "Control '" << name() << "'"
                << " must be constrained to a single selection."
                << LMI_FLUSH
                ;
            }
        return TransferString(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxRadioBox& control)
    {
        return TransferString(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxRadioButton& control)
    {
        return TransferBool(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxScrollBar& control)
    {
        if(td == from_string_to_control)
            {
            control.SetThumbPosition(numeric_io_cast<int>(data));
            }
        else
            {
            data = numeric_io_cast<std::string>(control.GetThumbPosition());
            }
        return true;
    }

    bool Transfer(transfer_direction td, std::string& data, wxSlider& control)
    {
        return TransferInt(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxSpinButton& control)
    {
        return TransferInt(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxSpinCtrl& control)
    {
        return TransferInt(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxStaticText& control)
    {
        return TransferLabel(td, data, control);
    }

    bool Transfer(transfer_direction td, std::string& data, wxTextCtrl& control)
    {
        if(td == from_string_to_control)
            {
            control.SetValue(data);
            }
        else
            {
            data = control.GetValue();
            }
        return true;
    }
} // Unnamed namespace.

