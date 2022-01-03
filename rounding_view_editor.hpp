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

#ifndef rounding_view_editor_hpp
#define rounding_view_editor_hpp

#include "config.hpp"

#include "rounding_rules.hpp"

#include <wx/panel.h>
#include <wx/xrc/xmlres.h>

#include <string>

class WXDLLIMPEXP_FWD_CORE wxBitmapButton;
class WXDLLIMPEXP_FWD_CORE wxSpinCtrl;
class WXDLLIMPEXP_FWD_CORE wxWindow;

class RoundingButtons
  :public wxPanel
{
  public:
    RoundingButtons();
    RoundingButtons
        (wxWindow*          parent
        ,wxWindowID         id
        ,wxPoint const&     pos   = wxDefaultPosition
        ,wxSize const&      size  = wxDefaultSize
        ,long int           style = 0
        ,std::string const& name  = wxPanelNameStr
        );
    RoundingButtons(RoundingButtons const&) = delete;
    RoundingButtons& operator=(RoundingButtons const&) = delete;
    ~RoundingButtons() override = default;

    void Create
        (wxWindow*          parent
        ,wxWindowID         id
        ,wxPoint const&     pos   = wxDefaultPosition
        ,wxSize const&      size  = wxDefaultSize
        ,long int           style = 0
        ,wxString const&    name  = wxPanelNameStr
        );

    bool IsModified() const;
    void Modify(bool modified = true);
    void DiscardEdits();

    void                SetValue(rounding_parameters const& value);
    rounding_parameters GetValue() const;

  private:
    void SetNumber(int number);
    int  GetNumber() const;
    void           SetStyle(rounding_style style);
    rounding_style GetStyle() const;
    void           FixStyle(rounding_style style);

    wxBitmapButton& button_not_at_all() const;
    wxBitmapButton& button_to_nearest() const;
    wxBitmapButton& button_upward() const;
    wxBitmapButton& button_downward() const;
    wxSpinCtrl& spin() const;

    void UponButtonClick(wxCommandEvent&);

    static wxSize CalculateMinimumTextControlSize
        (wxWindow* window
        ,unsigned int n = 1
        );

    // initial or last saved value (IsModified() returns true iff current
    // value is different from this)
    rounding_parameters previous_value_;

    rounding_style style_;
    wxBitmapButton* button_not_at_all_;
    wxBitmapButton* button_to_nearest_;
    wxBitmapButton* button_upward_;
    wxBitmapButton* button_downward_;

    wxSpinCtrl* spin_;

    DECLARE_EVENT_TABLE()
};

class RoundingButtonsXmlHandler
  :public wxXmlResourceHandler
{
  public:
    RoundingButtonsXmlHandler();

  private:
    wxObject* DoCreateResource() override;
    bool CanHandle(wxXmlNode* node) override;

    DECLARE_DYNAMIC_CLASS(RoundingButtonsXmlHandler)
};

#endif // rounding_view_editor_hpp
