// Pop-up input-sequence editor.
//
// Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef input_sequence_entry_hpp
#define input_sequence_entry_hpp

#include "config.hpp"

#include "input_sequence.hpp"

#include <wx/panel.h>
#include <wx/xrc/xmlres.h>

#include <string>

class Input;
class WXDLLIMPEXP_FWD_CORE wxButton;
class WXDLLIMPEXP_FWD_CORE wxTextCtrl;

class InputSequenceEntry final
    :public wxPanel
{
  public:
    InputSequenceEntry() = default;
    InputSequenceEntry(wxWindow* parent, wxWindowID id, wxString const& name);
    InputSequenceEntry(InputSequenceEntry const&) = delete;
    InputSequenceEntry& operator=(InputSequenceEntry const&) = delete;
    bool Create(wxWindow* parent, wxWindowID id, wxString const& name);

    void input(Input const& input);
    Input const& input() const;

    void field_name(std::string const& name);
    std::string field_name() const;

    wxTextCtrl& text_ctrl() {return *text_;}

    void set_popup_title(wxString const& title) {title_ = title;}

    void open_editor();

  private:
    void UponChildKillFocus(wxFocusEvent&);
    void UponEnter(wxCommandEvent&);
    void UponOpenEditor(wxCommandEvent&);

    void DoOpenEditor();

    Input const* input_     {nullptr};
    std::string field_name_ {};

    wxTextCtrl* text_       {nullptr};
    wxButton*   button_     {nullptr};
    wxString    title_      {};
};

class InputSequenceEntryXmlHandler
    :public wxXmlResourceHandler
{
  public:
    InputSequenceEntryXmlHandler();

  private:
    wxObject* DoCreateResource() override;
    bool CanHandle(wxXmlNode* node) override;

    DECLARE_DYNAMIC_CLASS(InputSequenceEntryXmlHandler)
};

#endif // input_sequence_entry_hpp
