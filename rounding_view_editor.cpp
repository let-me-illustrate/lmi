// Rounding rules manager helper widget.
//
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "rounding_view_editor.hpp"

#include "alert.hpp"
#include "map_lookup.hpp"
#include "wx_new.hpp"

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/dcclient.h>                // class wxClientDC
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/window.h>

#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <utility>                      // pair

namespace
{
enum enum_rounding_button_id
    {e_rbi_not_at_all = wxID_HIGHEST
    ,e_rbi_to_nearest
    ,e_rbi_upward
    ,e_rbi_downward
    };

/// Toggle-button states we are interested in.

enum enum_bitmap_button_state
    {e_state_normal
    ,e_state_hover
    ,e_state_selected
    };

/// Bitmap storage.

typedef std::map
    <std::pair<rounding_style,enum_bitmap_button_state>
    ,wxBitmap
    > button_bitmaps;

/// Return bitmap storage. Load resources when called for the first time.

button_bitmaps const& all_button_bitmaps()
{
    static button_bitmaps bitmaps;

    if(bitmaps.empty())
        {
        typedef std::map<rounding_style, std::string> style_names;
        style_names styles;
        styles[r_not_at_all] = "not_at_all";
        styles[r_to_nearest] = "to_nearest";
        styles[r_upward]     = "upward";
        styles[r_downward]   = "downward";

        typedef std::map<enum_bitmap_button_state, std::string> state_names;
        state_names states;
        states[e_state_normal]   = "";
        states[e_state_hover]    = "_hover";
        states[e_state_selected] = "_selected";

        wxXmlResource& xml_resources = *wxXmlResource::Get();
        for(auto const& li : styles)
            {
            for(auto const& ti : states)
                {
                std::ostringstream oss;
                oss
                    << "rnd_"
                    << li.second
                    << ti.second
                    ;
                bitmaps[std::make_pair(li.first, ti.first)] =
                    xml_resources.LoadBitmap(oss.str());
                }
            }
        }
    return bitmaps;
}

/// Streaming operator to support map_lookup().

std::ostream& operator<<
    (std::ostream& os
    ,std::pair<rounding_style,enum_bitmap_button_state> const& b
    )
{
    return os << "style " << b.first << " state " << b.second;
}

/// Return the bitmap corresponding to the 'button' in the 'state'.

wxBitmap const& get_button_bitmap
    (rounding_style button
    ,enum_bitmap_button_state state
    )
{
    return map_lookup(all_button_bitmaps(), std::make_pair(button, state));
}

void style_button(wxBitmapButton& button, rounding_style style, bool selected)
{
    button.SetBitmapLabel(get_button_bitmap(style, selected
        ? e_state_selected
        : e_state_normal
        ));

    button.SetBitmapSelected(get_button_bitmap(style, selected
        ? e_state_selected
        : e_state_hover
        ));

    button.SetBitmapHover(get_button_bitmap(style, selected
        ? e_state_selected
        : e_state_hover
        ));
}
} // Unnamed namespace.

BEGIN_EVENT_TABLE(RoundingButtons, wxPanel)
    EVT_BUTTON(e_rbi_not_at_all, RoundingButtons::UponButtonClick)
    EVT_BUTTON(e_rbi_to_nearest, RoundingButtons::UponButtonClick)
    EVT_BUTTON(e_rbi_upward    , RoundingButtons::UponButtonClick)
    EVT_BUTTON(e_rbi_downward  , RoundingButtons::UponButtonClick)
END_EVENT_TABLE()

RoundingButtons::RoundingButtons()
    :wxPanel         {}
    ,previous_value_ (0, r_not_at_all)
{
}

RoundingButtons::RoundingButtons
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxPoint const&     pos
    ,wxSize const&      size
    ,long int           style
    ,std::string const& name
    )
    :wxPanel         {}
    ,previous_value_ (0, r_not_at_all)
{
    Create(parent, id, pos, size, style, name);
}

void RoundingButtons::Create
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxPoint const&     pos
    ,wxSize const&      size
    ,long int           style
    ,wxString const&    name
    )
{
    wxPanel::Create(parent, id, pos, size, style, name);

    wxBitmap empty;
    button_not_at_all_ = new(wx) wxBitmapButton(this, e_rbi_not_at_all, empty);
    button_to_nearest_ = new(wx) wxBitmapButton(this, e_rbi_to_nearest, empty);
    button_upward_     = new(wx) wxBitmapButton(this, e_rbi_upward    , empty);
    button_downward_   = new(wx) wxBitmapButton(this, e_rbi_downward  , empty);

    button_not_at_all().SetToolTip("do not round at all");
    button_to_nearest().SetToolTip("round to nearest");
    button_upward    ().SetToolTip("round upward");
    button_downward  ().SetToolTip("round downward");
    spin_ = new(wx) wxSpinCtrl(this);

    // Set a reasonable range. Negatives are actually useful, e.g.,
    // for rounding to thousands.
    int const digits = std::numeric_limits<double>::digits10;
    spin().SetRange(-digits, digits);

    // Adjust widget size to be as small as possible: 6 represents the
    // digit and its margins, plus the same for spin arrows.
    wxSize spin_size = CalculateMinimumTextControlSize(spin_, 6);
    spin_size.SetHeight(spin().GetSize().GetHeight());
    spin().SetMinSize(spin_size);

    wxBoxSizer* sizer_ = new(wx) wxBoxSizer(wxHORIZONTAL);
    SetSizer(sizer_);

    wxSizerFlags flags = wxSizerFlags()
        .Proportion(0)
        .Expand()
        .Border(wxLEFT|wxRIGHT, 2)
        ;

    sizer_->Add(spin_             , flags);
    sizer_->Add(button_not_at_all_, flags);
    sizer_->Add(button_to_nearest_, flags);
    sizer_->Add(button_upward_    , flags);
    sizer_->Add(button_downward_  , flags);

    SetValue(previous_value_);
}

wxSize RoundingButtons::CalculateMinimumTextControlSize
    (wxWindow* window
    ,unsigned int n
    )
{
    wxClientDC dc(window);
    dc.SetFont(window->GetFont());

    wxCoord w, h;
    // Assume that 'W' is the widest letter.
    dc.GetTextExtent("W", &w, &h);

    wxSize size(w * n, h);
    size += window->GetSize() - window->GetClientSize();
    return size;
}

bool RoundingButtons::IsModified() const
{
    return !(GetValue() == previous_value_);
}

void RoundingButtons::Modify(bool modified)
{
    if(!modified)
        {
        // Modify(false) is called when the document is saved, so updating last
        // saved value here ensures that IsModified() returns whether the
        // value changed since the last save and not since the control was
        // created.
        previous_value_ = GetValue();
        }
}

void RoundingButtons::DiscardEdits()
{
    Modify(false);
}

void RoundingButtons::UponButtonClick( wxCommandEvent & event )
{
    rounding_style style;
    // This switch condition is not of enumerative type. The default
    // case is necessary for values that match none of the enumerators.
    switch(event.GetId())
        {
        case e_rbi_not_at_all:
            style = r_not_at_all;
            break;
        case e_rbi_to_nearest:
            style = r_to_nearest;
            break;
        case e_rbi_upward:
            style = r_upward;
            break;
        case e_rbi_downward:
            style = r_downward;
            break;
        default:
            event.Skip();
            return;
        }
    SetStyle(style);
}

void RoundingButtons::SetValue(rounding_parameters const& value)
{
    previous_value_ = value;
    SetNumber(value.decimals());
    SetStyle(value.raw_style());
}

rounding_parameters RoundingButtons::GetValue() const
{
    return rounding_parameters(GetNumber(), GetStyle(), "");
}

void RoundingButtons::SetNumber(int number)
{
    spin().SetValue(number);
}

int  RoundingButtons::GetNumber() const
{
    return spin().GetValue();
}

void RoundingButtons::SetStyle(rounding_style const style)
{
    style_ = style;
    style_button(button_not_at_all(), r_not_at_all, style == r_not_at_all);
    style_button(button_to_nearest(), r_to_nearest, style == r_to_nearest);
    style_button(button_upward    (), r_upward,     style == r_upward    );
    style_button(button_downward  (), r_downward,   style == r_downward  );
}

void RoundingButtons::FixStyle(rounding_style style)
{
    SetStyle(style);
    button_not_at_all().Enable(style == r_not_at_all);
    button_to_nearest().Enable(style == r_to_nearest);
    button_upward    ().Enable(style == r_upward);
    button_downward  ().Enable(style == r_downward);
}

rounding_style RoundingButtons::GetStyle() const
{
    return style_;
}

wxSpinCtrl& RoundingButtons::spin() const
{
    return *spin_;
}

wxBitmapButton& RoundingButtons::button_not_at_all() const
{
    return *button_not_at_all_;
}

wxBitmapButton& RoundingButtons::button_to_nearest() const
{
    return *button_to_nearest_;
}

wxBitmapButton& RoundingButtons::button_upward() const
{
    return *button_upward_;
}

wxBitmapButton& RoundingButtons::button_downward() const
{
    return *button_downward_;
}

IMPLEMENT_DYNAMIC_CLASS(RoundingButtonsXmlHandler, wxXmlResourceHandler)

RoundingButtonsXmlHandler::RoundingButtonsXmlHandler()
    :wxXmlResourceHandler {}
{
    AddWindowStyles();
}

wxObject* RoundingButtonsXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, RoundingButtons)

    control->Create
        (GetParentAsWindow()
        ,GetID()
        ,GetPosition()
        ,GetSize()
        ,GetStyle()
        ,GetName()
        );

    SetupWindow(control);

    return control;
}

bool RoundingButtonsXmlHandler::CanHandle(wxXmlNode* node)
{
    return IsOfClass(node, "RoundingButtons");
}
