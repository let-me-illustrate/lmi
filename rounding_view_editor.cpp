// Rounding rules manager helper widget.
//
// Copyright (C) 2007 Gregory W. Chicares.
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

// $Id: rounding_view_editor.cpp,v 1.1.2.1 2007-02-11 21:52:42 etarassov Exp $

#include "rounding_view_editor.hpp"


#include <wx/dcclient.h> // RoundingButtons::CalculateMinimumTextControlSize
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/tglbtn.h>
#include <wx/window.h>

namespace
{

enum enum_rounding_button_id
    {e_rbi_not_at_all = wxID_HIGHEST
    ,e_rbi_to_nearest
    ,e_rbi_upward
    ,e_rbi_downward
    };

} // unnamed namespace

BEGIN_EVENT_TABLE(RoundingButtons, wxPanel)
    EVT_TOGGLEBUTTON(e_rbi_not_at_all, RoundingButtons::UponButtonClick)
    EVT_TOGGLEBUTTON(e_rbi_to_nearest, RoundingButtons::UponButtonClick)
    EVT_TOGGLEBUTTON(e_rbi_upward    , RoundingButtons::UponButtonClick)
    EVT_TOGGLEBUTTON(e_rbi_downward  , RoundingButtons::UponButtonClick)
END_EVENT_TABLE()

RoundingButtons::RoundingButtons()
    :wxPanel()
    ,original_rule_(0, r_not_at_all)
{
}

RoundingButtons::RoundingButtons
    (wxWindow* parent
    ,wxWindowID id
    ,const wxPoint& pos
    ,const wxSize& size
    ,long style
    ,const wxString& name
    )
    :wxPanel()
    ,original_rule_(0, r_not_at_all)
{
    Create(parent, id, pos, size, style, name);
}

void RoundingButtons::Create
    (wxWindow* parent
    ,wxWindowID id
    ,const wxPoint& pos
    ,const wxSize& size
    ,long style
    ,const wxString& name
    )
{
    wxPanel::Create(parent, id, pos, size, style, name);

    button_not_at_all_ = new(wx) wxToggleButton(this, e_rbi_not_at_all, "@");
    button_to_nearest_ = new(wx) wxToggleButton(this, e_rbi_to_nearest, "x");
    button_upward_     = new(wx) wxToggleButton(this, e_rbi_upward,     "A");
    button_downward_   = new(wx) wxToggleButton(this, e_rbi_downward,   "V");
    button_not_at_all_->SetToolTip("does nothing");
    button_to_nearest_->SetToolTip("round to the nearest");
    button_upward_    ->SetToolTip("round upward");
    button_downward_  ->SetToolTip("round downward");
    spin_ = new(wx) wxSpinCtrl(this);

    // adjust widgets size to be as small as possible
    // 3 stands for the button label symbol itself plus two margins
    wxSize button_size
        = CalculateMinimumTextControlSize(button_not_at_all_, 3);
    button_size.SetHeight(button_not_at_all_->GetSize().GetHeight());
    button_not_at_all_->SetMinSize(button_size);
    button_to_nearest_->SetMinSize(button_size);
    button_upward_    ->SetMinSize(button_size);
    button_downward_  ->SetMinSize(button_size);

    // 6 stands for the digit and its margins, plus the same for spin arrows
    wxSize spin_size
        = CalculateMinimumTextControlSize(spin_, 6);
    spin_size.SetHeight(spin_->GetSize().GetHeight());
    spin_->SetMinSize(spin_size);


    wxBoxSizer* sizer_ = new(wx) wxBoxSizer(wxHORIZONTAL);
    SetSizer(sizer_);

    wxSizerFlags flags = wxSizerFlags()
        .Proportion(0)
        .Expand()
        .Center()
        .Border(wxLEFT|wxRIGHT, 2)
        ;

    sizer_->Add(spin_             , flags);
    sizer_->Add(button_not_at_all_, flags);
    sizer_->Add(button_to_nearest_, flags);
    sizer_->Add(button_upward_    , flags);
    sizer_->Add(button_downward_  , flags);

    SetValue(original_rule_);
}

wxSize RoundingButtons::CalculateMinimumTextControlSize
    (wxWindow* window
    ,unsigned int n
    )
{
    wxClientDC dc(window);
    dc.SetFont(window->GetFont());

    wxCoord w, h;
    // consider 'W' to be the widest letter
    dc.GetTextExtent("W", &w, &h);

    wxSize size(w * n, h);
    size += window->GetSize() - window->GetClientSize();
    return size;
}

RoundingButtons::~RoundingButtons()
{
}

bool RoundingButtons::IsModified() const
{
    return GetValue() == original_rule_;
}

void RoundingButtons::Modify(bool modified)
{
    if(!modified)
        SetValue(original_rule_);
}

void RoundingButtons::DiscardEdits()
{
    Modify(false);
}

void RoundingButtons::UponButtonClick( wxCommandEvent & event )
{
    int id = event.GetId();
    rounding_style style;
    switch(id)
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

void RoundingButtons::SetValue(round_to<double> const& value)
{
    original_rule_ = value;
    SetNumber(value.decimals());
    SetStyle(value.style());
}

round_to<double> RoundingButtons::GetValue() const
{
    return round_to<double>(GetNumber(), GetStyle());
}

void RoundingButtons::SetNumber(int number)
{
    spin_->SetValue(number);
}

int  RoundingButtons::GetNumber() const
{
    return spin_->GetValue();
}

void RoundingButtons::SetStyle(rounding_style const style)
{
    button_not_at_all_->SetValue(style == r_not_at_all);
    button_to_nearest_->SetValue(style == r_to_nearest);
    button_upward_    ->SetValue(style == r_upward);
    button_downward_  ->SetValue(style == r_downward);
}

void RoundingButtons::FixStyle(rounding_style style)
{
    SetStyle(style);
    button_not_at_all_->Enable(style == r_not_at_all);
    button_to_nearest_->Enable(style == r_to_nearest);
    button_upward_    ->Enable(style == r_upward);
    button_downward_  ->Enable(style == r_downward);
}

rounding_style RoundingButtons::GetStyle() const
{
    if(button_not_at_all_->GetValue())
        return r_not_at_all;

    if(button_to_nearest_->GetValue())
        return r_to_nearest;

    if(button_upward_->GetValue())
        return r_upward;

    if(button_downward_->GetValue())
        return r_downward;

    return r_indeterminate;
}


IMPLEMENT_DYNAMIC_CLASS(RoundingButtonsXmlHandler, wxXmlResourceHandler)

RoundingButtonsXmlHandler::RoundingButtonsXmlHandler()
    :wxXmlResourceHandler()
{
    AddWindowStyles();
}

wxObject* RoundingButtonsXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, RoundingButtons)

    control->Create
        (m_parentAsWindow
        ,GetID()
        ,GetPosition()
        ,GetSize()
        ,GetStyle()
        ,GetName()
        );

    SetupWindow(control);

    return control;
}

bool RoundingButtonsXmlHandler::CanHandle(wxXmlNode *node)
{
    return IsOfClass(node, "RoundingButtons");
}
