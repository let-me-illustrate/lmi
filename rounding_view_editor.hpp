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

// $Id: rounding_view_editor.hpp,v 1.1.2.2 2007-02-15 14:42:19 etarassov Exp $

#ifndef rounding_view_editor_hpp
#define rounding_view_editor_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "obstruct_slicing.hpp"
#include "rounding_rules.hpp"
#include "wx_new.hpp"

#include <boost/utility.hpp>

#include <wx/icon.h>
#include <wx/xrc/xmlres.h>

class WXDLLEXPORT wxMenuBar;
class WXDLLEXPORT wxSpinCtrl;
class WXDLLEXPORT wxWindow;
class WXDLLEXPORT wxToggleButton;

class RoundingButtons
  :public wxPanel
{
  public:
    RoundingButtons();
    RoundingButtons
        (wxWindow* parent
        ,wxWindowID id
        ,const wxPoint& pos = wxDefaultPosition
        ,const wxSize& size = wxDefaultSize
        ,long style = 0
        ,const wxString& name = wxPanelNameStr
        );
    virtual ~RoundingButtons();

    void Create
        (wxWindow* parent
        ,wxWindowID id
        ,const wxPoint& pos = wxDefaultPosition
        ,const wxSize& size = wxDefaultSize
        ,long style = 0
        ,const wxString& name = wxPanelNameStr
        );

    bool IsModified() const;
    void Modify(bool modified = true);
    void DiscardEdits();

    void             SetValue(round_to<double> const& value);
    round_to<double> GetValue() const;


  private:
    void SetNumber(int number);
    int  GetNumber() const;
    void           SetStyle(rounding_style style);
    rounding_style GetStyle() const;
    void           FixStyle(rounding_style style);

    void UponButtonClick(wxCommandEvent&);

    static wxSize CalculateMinimumTextControlSize
        (wxWindow* window
        ,unsigned int n = 1
        );

    round_to<double> original_rule_;
    wxToggleButton* button_not_at_all_;
    wxToggleButton* button_to_nearest_;
    wxToggleButton* button_upward_;
    wxToggleButton* button_downward_;

    wxSpinCtrl* spin_;

    DECLARE_EVENT_TABLE()
};

class RoundingButtonsXmlHandler
  :public wxXmlResourceHandler
{
  public:
    RoundingButtonsXmlHandler();

  private:
    virtual wxObject *DoCreateResource();
    virtual bool CanHandle(wxXmlNode *node);

    DECLARE_DYNAMIC_CLASS(RoundingButtonsXmlHandler)
};


#endif // rounding_view_editor_hpp

