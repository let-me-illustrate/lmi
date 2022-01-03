// Transfer data between wx standard controls and std::strings.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

// wxWindows-2.4.1 file 'src/common/valgen.cpp', copyright 1999 by its
// author Kevin Smith and released under the wxWindows license, solves
// a similar problem. The present file draws some ideas from that work
// but is implemented differently and has different goals, so any
// problems introduced here don't reflect on Kevin Smith's reputation.

#include "pchfile_wx.hpp"

#include "transferor.hpp"

#include "alert.hpp"
#include "calendar_date.hpp"
#include "input_sequence_entry.hpp"
#include "numeric_io_cast.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/datectrl.h>
#include <wx/dynarray.h>
#include <wx/filepicker.h>
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
    bool Transfer(transfer_direction, std::string&, wxDirPickerCtrl&  );
    bool Transfer(transfer_direction, std::string&, wxFilePickerCtrl& );
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
    // Custom controls.
    bool Transfer(transfer_direction, std::string&, InputSequenceEntry& );
} // Unnamed namespace.

Transferor::Transferor(std::string& data, std::string const& name)
    :wxValidator {}
    ,data_       {data}
    ,name_       {name}
{
}

wxObject* Transferor::Clone() const
{
    return new(wx) Transferor(data_, name_);
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
    if(!GetWindow())
        {
        alarum()
            << "Validator for '"
            << name()
            << "' not bound to any control."
            << LMI_FLUSH
            ;
        }
    wxWindowBase* control = GetWindow();

    wxButton         * button       ;
    wxCheckBox       * checkbox     ;
    wxCheckListBox   * checklistbox ;
    wxComboBox       * combobox     ;
    wxChoice         * choice       ;
    wxDatePickerCtrl * datepicker   ;
    wxDirPickerCtrl  * dirpicker    ;
    wxFilePickerCtrl * filepicker   ;
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
    // Custom controls.
    InputSequenceEntry * sequence   ;

    if     (nullptr != (button       = dynamic_cast<wxButton         *>(control)))
        return Transfer(td, data_,             *button      );
    else if(nullptr != (checkbox     = dynamic_cast<wxCheckBox       *>(control)))
        return Transfer(td, data_,             *checkbox    );
    else if(nullptr != (checklistbox = dynamic_cast<wxCheckListBox   *>(control)))
        return Transfer(td, data_,             *checklistbox);
    else if(nullptr != (combobox     = dynamic_cast<wxComboBox       *>(control)))
        return Transfer(td, data_,             *combobox    );
    else if(nullptr != (choice       = dynamic_cast<wxChoice         *>(control)))
        return Transfer(td, data_,             *choice      );
    else if(nullptr != (datepicker   = dynamic_cast<wxDatePickerCtrl *>(control)))
        return Transfer(td, data_,             *datepicker  );
    else if(nullptr != (dirpicker    = dynamic_cast<wxDirPickerCtrl *>(control)))
        return Transfer(td, data_,             *dirpicker   );
    else if(nullptr != (filepicker   = dynamic_cast<wxFilePickerCtrl *>(control)))
        return Transfer(td, data_,             *filepicker  );
    else if(nullptr != (gauge        = dynamic_cast<wxGauge          *>(control)))
        return Transfer(td, data_,             *gauge       );
    else if(nullptr != (listbox      = dynamic_cast<wxListBox        *>(control)))
        return Transfer(td, data_,             *listbox     );
    else if(nullptr != (radiobox     = dynamic_cast<wxRadioBox       *>(control)))
        return Transfer(td, data_,             *radiobox    );
    else if(nullptr != (radiobutton  = dynamic_cast<wxRadioButton    *>(control)))
        return Transfer(td, data_,             *radiobutton );
    else if(nullptr != (scrollbar    = dynamic_cast<wxScrollBar      *>(control)))
        return Transfer(td, data_,             *scrollbar   );
    else if(nullptr != (slider       = dynamic_cast<wxSlider         *>(control)))
        return Transfer(td, data_,             *slider      );
    else if(nullptr != (spinbutton   = dynamic_cast<wxSpinButton     *>(control)))
        return Transfer(td, data_,             *spinbutton  );
    else if(nullptr != (spinctrl     = dynamic_cast<wxSpinCtrl       *>(control)))
        return Transfer(td, data_,             *spinctrl    );
    else if(nullptr != (statictext   = dynamic_cast<wxStaticText     *>(control)))
        return Transfer(td, data_,             *statictext  );
    else if(nullptr != (textctrl     = dynamic_cast<wxTextCtrl       *>(control)))
        return Transfer(td, data_,             *textctrl    );
    // Custom controls.
    else if(nullptr != (sequence     = dynamic_cast<InputSequenceEntry *>(control)))
        return Transfer(td, data_,             *sequence    );
    else
        {
        alarum()
            << "Unrecognized control '"
            << name()
            << "'."
            << LMI_FLUSH
            ;
        throw "Unreachable--silences a compiler diagnostic.";
        }
}

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
            data = control.GetLabel().ToStdString(wxConvUTF8);
            }
        return true;
    }

    // This uses "Yes" and "No" instead of true and false or 1 and 0,
    // because the capitalized English words seem clearer for humans
    // who read the xml files that store the data edited here.
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
                    << "Control '" << control.GetName() << "':"
                    << " expected 'Yes' or 'No', but got '" << data << "'."
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
                    << "Control '" << control.GetName() << "':"
                    << " invalid entry '" << data << "'."
                    << LMI_FLUSH
                    ;
                return false;
                }
            }
        else
            {
            data = control.GetStringSelection().ToStdString(wxConvUTF8);
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
            alarum()
                << "CheckListBox '" << control.GetName() << "':"
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
            alarum()
                << "ComboBox '" << control.GetName() << "':"
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

    bool Transfer(transfer_direction td, std::string& data, wxDirPickerCtrl& control)
    {
        if(td == from_string_to_control)
            {
            control.SetPath(data);
            }
        else
            {
            data = control.GetPath().ToStdString();
            }
        return true;
    }

    bool Transfer(transfer_direction td, std::string& data, wxFilePickerCtrl& control)
    {
        if(td == from_string_to_control)
            {
            control.SetPath(data);
            }
        else
            {
            data = control.GetPath().ToStdString();
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
            alarum()
                << "ListBox '" << control.GetName() << "':"
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
            data = control.GetValue().ToStdString(wxConvUTF8);
            }
        return true;
    }

    bool Transfer(transfer_direction td, std::string& data, InputSequenceEntry& control)
    {
        return Transfer(td, data, control.text_ctrl());
    }
} // Unnamed namespace.
