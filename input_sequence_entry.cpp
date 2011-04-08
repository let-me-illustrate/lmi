// Pop-up input-sequence editor.
//
// Copyright (C) 2010, 2011 Gregory W. Chicares.
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

#include LMI_PCH_HEADER
#ifdef __BORLANDC__
#   pragma hdrstop
#endif // __BORLANDC__

#include "input_sequence_entry.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "input.hpp"
#include "input_seq_helpers.hpp"
#include "input_sequence.hpp"
#include "mvc_controller.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>

#include <algorithm>              // std::copy(), std::find()
#include <iterator>               // std::back_inserter
#include <map>
#include <vector>

namespace
{
class DurationModeChoice
    :public wxChoice
{
  public:
    DurationModeChoice(wxWindow* parent);

    void value(duration_mode x);
    duration_mode value() const;

    void allow_maturity(bool allow);
    bool needs_number() const;
};

struct choice_value
{
    duration_mode mode;
    char const*   label;
};

choice_value duration_mode_choice_values[] =
  {
    {e_retirement,       "until retirement"},
    {e_attained_age,     "until age"},
    {e_duration,         "until duration"},
    {e_number_of_years,  "for a period of"},
    {e_maturity,         "until maturity"}    // e_maturity must be last
  };

unsigned int const duration_mode_choices = sizeof(duration_mode_choice_values) / sizeof(choice_value);

DurationModeChoice::DurationModeChoice(wxWindow* parent)
{
    Create(parent, wxID_ANY);

    for(unsigned int i = 0; i < duration_mode_choices; ++i)
        {
        Append(duration_mode_choice_values[i].label);
        }

    // "maturity" is the default
    value(e_maturity);
}

void DurationModeChoice::allow_maturity(bool allow)
{
    LMI_ASSERT(e_maturity == duration_mode_choice_values[duration_mode_choices - 1].mode);

    if(allow == (duration_mode_choices == GetCount()))
        {
        return;
        }

    // "until maturity" is the last entry
    if(allow)
        {
        Append(duration_mode_choice_values[duration_mode_choices - 1].label);
        }
    else
        {
        if(e_maturity == value())
            {
            // "until maturity" is selected, but it's no longer allowed.
            // Replace it with arbitrary other value temporarily; we pick
            // e_retirement, because it (like e_maturity) doesn't have numeric
            // argument.
            //
            // This is done only to preserve the invariant that
            // DurationModeChoice always has a value selected; if this happens,
            // it will be replaced by a more appropriate choice by other UI
            // code (see UponAddRow).
            value(e_retirement);
            }
        Delete(duration_mode_choices - 1);
        }
}

void DurationModeChoice::value(duration_mode x)
{
    for(unsigned int i = 0; i < duration_mode_choices; ++i)
        {
        if(x == duration_mode_choice_values[i].mode)
            {
            SetSelection(i);
            return;
            }
        }

    fatal_error() << "unexpected duration_mode value" << LMI_FLUSH;
}

duration_mode DurationModeChoice::value() const
{
    int const sel = GetSelection();

    LMI_ASSERT(0 <= sel);
    LMI_ASSERT(sel < static_cast<int>(duration_mode_choices));

    return duration_mode_choice_values[sel].mode;
}

bool DurationModeChoice::needs_number() const
{
    switch(value())
        {
        case e_attained_age:
        case e_duration:
        case e_number_of_years:
            return true;

        case e_invalid_mode:
        case e_inception:
        case e_inforce:
        case e_retirement:
        case e_maturity:
            return false;
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

class InputSequenceEditor
    :public wxDialog
{
  public:
    InputSequenceEditor(wxWindow* parent, wxString const& title, Input const& input);

    void set_keywords
        (std::vector<std::string> const& keywords
        ,bool                            keywords_only
        ,std::string const&              default_keyword
        )
    {
        keywords_        = keywords;
        keywords_only_   = keywords_only;
        default_keyword_ = default_keyword;
    }

    void sequence(InputSequence const& s);
    std::string sequence_string();

  private:
    void add_row();
    void insert_row(int row);
    void remove_row(int row);
    void update_row(int row);
    void redo_layout();
    void set_tab_order();
    wxString format_from_text(int row);

    enum Col
    {
        Col_Value,
        Col_From,
        Col_DurationMode,
        Col_DurationNum,
        Col_Then,
        Col_Remove,
        Col_Add,
        Col_Max
    };

    wxTextEntry& value_field(int row)
    {
        return get_field<wxTextEntry>(Col_Value, row);
    }

    wxControl& value_field_ctrl(int row)
    {
        return get_field<wxControl>(Col_Value, row);
    }

    wxStaticText& from_field(int row)
    {
        return get_field<wxStaticText>(Col_From, row);
    }

    DurationModeChoice& duration_mode_field(int row)
    {
        return get_field<DurationModeChoice>(Col_DurationMode, row);
    }

    wxTextCtrl& duration_num_field(int row)
    {
        return get_field<wxTextCtrl>(Col_DurationNum, row);
    }

    wxStaticText& then_field(int row)
    {
        return get_field<wxStaticText>(Col_Then, row);
    }

    wxButton& remove_button(int row)
    {
        return get_field<wxButton>(Col_Remove, row);
    }

    wxButton& add_button(int row)
    {
        return get_field<wxButton>(Col_Add, row);
    }

    template<typename T>
    T& get_field(int col, int row);
    wxWindow* get_field_win(int col, int row);

    int compute_duration_scalar(int row);
    void adjust_duration_num(int row);

    void UponDurationModeChange(wxCommandEvent& event);
    void UponDurationNumChange(wxCommandEvent& event);
    void UponRemoveRow(wxCommandEvent& event);
    void UponAddRow(wxCommandEvent& event);

    Input const& input_;
    std::vector<std::string> keywords_;
    bool keywords_only_;
    std::string default_keyword_;

    int rows_count_;
    wxFlexGridSizer* sizer_;
    wxButton* ok_button_;
    wxButton* cancel_button_;
    typedef std::map<wxWindowID, int> id_to_row_map;
    id_to_row_map id_to_row_;

    // scalar absolute values for end durations; this is used to recompute
    // duration number for certain duration modes
    std::vector<int> duration_scalars_;
};

InputSequenceEditor::InputSequenceEditor(wxWindow* parent, wxString const& title, Input const& input)
    :wxDialog
        (parent
        ,wxID_ANY
        ,title
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxDEFAULT_DIALOG_STYLE
        )
    ,input_        (input)
    ,keywords_only_(false)
    ,rows_count_   (0)
{
    wxSizer* top = new(wx) wxBoxSizer(wxVERTICAL);

    sizer_ = new(wx) wxFlexGridSizer(Col_Max, 5, 5);
    top->Add(sizer_, wxSizerFlags(1).Expand().DoubleBorder());

    wxStdDialogButtonSizer* buttons = new(wx) wxStdDialogButtonSizer();
    buttons->AddButton(ok_button_ = new(wx) wxButton(this, wxID_OK));
    buttons->AddButton(cancel_button_ = new(wx) wxButton(this, wxID_CANCEL));
    buttons->Realize();

    top->Add(buttons, wxSizerFlags().Expand().Border());

    SetSizerAndFit(top);

    add_row();

    value_field_ctrl(0).SetFocus();

    ::Connect
        (this
        ,wxEVT_COMMAND_CHOICE_SELECTED
        ,&InputSequenceEditor::UponDurationModeChange
        );
    ::Connect
        (this
        ,wxEVT_COMMAND_TEXT_UPDATED
        ,&InputSequenceEditor::UponDurationNumChange
        );
}

void InputSequenceEditor::sequence(InputSequence const& s)
{
    while(0 < rows_count_)
        {
        remove_row(0);
        }

    std::vector<ValueInterval> const& intervals = s.interval_representation();
    int const num_intervals = intervals.size();

    if(intervals.empty())
        {
        // have single row (initial state)
        add_row();
        return;
        }

    LMI_ASSERT(0 == intervals.front().begin_duration);
    LMI_ASSERT(e_maturity == intervals.back().end_mode);
    for(int i = 1; i < num_intervals; ++i)
        {
        LMI_ASSERT(intervals[i].begin_duration == intervals[i - 1].end_duration);
        }

    for(int i = 0; i < num_intervals; ++i)
        {
        ValueInterval const& data = intervals[i];
        LMI_ASSERT(!data.insane);

        add_row();

        duration_mode_field(i).value(data.end_mode);

        int dur_num;
        switch(data.end_mode)
            {
            case e_number_of_years:
                {
                dur_num = data.end_duration - data.begin_duration;
                }
                break;
            case e_attained_age:
                {
                dur_num = input_.issue_age() + data.end_duration;
                }
                break;

            case e_invalid_mode:
            case e_duration:
            case e_inception:
            case e_inforce:
            case e_retirement:
            case e_maturity:
                {
                dur_num = data.end_duration;
                }
            }

        duration_num_field(i).SetValue(value_cast<std::string>(dur_num).c_str());

        if(data.value_is_keyword)
            {
            value_field(i).SetValue(data.value_keyword.c_str());
            }
        else
            {
            value_field(i).SetValue(value_cast<std::string>(data.value_number).c_str());
            }
        }

    // move focus to a reasonable place
    value_field_ctrl(0).SetFocus();
}

std::string InputSequenceEditor::sequence_string()
{
    std::string s;

    for(int i = 0; i < rows_count_; ++i)
        {
        if(!s.empty())
            {
            s.append("; ");
            }

        s.append(value_field(i).GetValue().c_str());

        switch(duration_mode_field(i).value())
            {
            case e_retirement:
                {
                s.append(" retirement");
                break;
                }
            case e_attained_age:
                {
                s.append(" @");
                s.append(duration_num_field(i).GetValue().c_str());
                break;
                }
            case e_duration:
                {
                s.append(" ");
                s.append(duration_num_field(i).GetValue().c_str());
                break;
                }
            case e_number_of_years:
                {
                s.append(" #");
                s.append(duration_num_field(i).GetValue().c_str());
                break;
                }
            case e_maturity:
                {
                LMI_ASSERT(i == rows_count_ - 1);
                // " maturity" is implicit, don't add it
                break;
                }

            case e_invalid_mode:
            case e_inception:
            case e_inforce:
                {
                fatal_error() << "unexpected duration_mode value" << LMI_FLUSH;
                return "";
                }
            }
        }

    return s;
}

void SizeWinForText(wxWindow* win, wxString const& text, int extra = 0)
{
    int x, y;
    win->GetTextExtent(text, &x, &y);
    win->SetMinSize(wxSize(x + extra, -1));
}

void InputSequenceEditor::add_row()
{
    insert_row(rows_count_);
}

void InputSequenceEditor::insert_row(int new_row)
{
    int const prev_row = new_row - 1;

    int insert_pos = Col_Max * new_row;

    //  Employee payment:
    //    [   0]  from issue date until [year] [ 5], then
    //    [1000]  from   year 5   until [year] [10], then
    //    [   0]  from   year 10  until [ age] [70], then
    //    [   0]  from   age 70   until [maturity].

#define LARGEST_FROM_TEXT "from duration 999 + 999 years"
#define LARGEST_THEN_TEXT "years, then"

    wxSizerFlags const flags = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL);

    wxControl* value_ctrl;
    if(!keywords_.empty())
        {
        wxComboBox* combo = new(wx) wxComboBox
            (this
            ,wxID_ANY
            ,"0"
            ,wxDefaultPosition
            ,wxDefaultSize
            ,0
            ,NULL
            ,keywords_only_ ? wxCB_READONLY : 0
            );
        value_ctrl = combo;

        wxArrayString kw;
        std::copy(keywords_.begin(), keywords_.end(), std::back_inserter(kw));
        combo->Append(kw);

        if(keywords_only_)
            {
            LMI_ASSERT(!default_keyword_.empty());
            combo->SetValue(default_keyword_.c_str());
            }

        if(!keywords_only_)
            {
            combo->AutoComplete(kw);
            }
        }
    else
        {
        // No keywords, only numeric values
        value_ctrl = new(wx) wxTextCtrl(this, wxID_ANY, "0");
        value_ctrl->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
        }

    sizer_->wxSizer::Insert(insert_pos++, value_ctrl, wxSizerFlags(flags).TripleBorder(wxRIGHT));
    wxStaticText* from_label = new(wx) wxStaticText(this, wxID_ANY, LARGEST_FROM_TEXT);
    SizeWinForText(from_label, LARGEST_FROM_TEXT);
    sizer_->wxSizer::Insert(insert_pos++, from_label, flags);
    sizer_->wxSizer::Insert(insert_pos++, new(wx) DurationModeChoice(this), flags);
    wxTextCtrl* duration_num = new(wx) wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_RIGHT);
    duration_num->SetValidator(wxTextValidator(wxFILTER_DIGITS));
    sizer_->wxSizer::Insert(insert_pos++, duration_num, flags);
    SizeWinForText(duration_num, "999", 20);
    wxStaticText* then_label = new(wx) wxStaticText(this, wxID_ANY, LARGEST_THEN_TEXT);
    sizer_->wxSizer::Insert(insert_pos++, then_label, flags);
    SizeWinForText(then_label, LARGEST_THEN_TEXT);

#undef LARGEST_FROM_TEXT
#undef LARGEST_THEN_TEXT

    // Note: We can't use wxID_REMOVE/wxID_ADD for these buttons, because
    // there's more than one of them and the ID is used to distinguish between
    // them. Consequently, we have to add stock graphics manually under wxGTK.
    wxButton* remove = new(wx) wxButton
        (this
        ,wxID_ANY
        ,"Remove"
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxBU_AUTODRAW | wxBU_EXACTFIT | wxBORDER_NONE
        );
#ifdef __WXGTK__
    wxBitmap removeBmp = wxArtProvider::GetBitmap("gtk-remove", wxART_BUTTON);
    remove->SetBitmap(removeBmp);
#endif

    remove->SetToolTip("Remove this row");
    remove->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEditor::UponRemoveRow)
        ,NULL
        ,this
        );
    sizer_->wxSizer::Insert(insert_pos++, remove, wxSizerFlags(flags).TripleBorder(wxLEFT));

    wxButton* add = new(wx) wxButton
        (this
        ,wxID_ANY
        ,"Add"
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxBU_AUTODRAW | wxBU_EXACTFIT | wxBORDER_NONE
        );
#ifdef __WXGTK__
    wxBitmap addBmp = wxArtProvider::GetBitmap("gtk-add", wxART_BUTTON);
    add->SetBitmap(addBmp);
#endif

    add->SetToolTip("Insert a new row after this one");
    add->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEditor::UponAddRow)
        ,NULL
        ,this
        );
    sizer_->wxSizer::Insert(insert_pos++, add, wxSizerFlags(flags).Border(wxLEFT, 0).Right());

    // update id_to_row_ mapping:
    for(id_to_row_map::iterator i = id_to_row_.begin(); i != id_to_row_.end(); ++i)
        {
        if(i->second >= new_row)
            {
            i->second = i->second + 1;
            }
        }

    // and add newly created windows to it to keep track of which windows
    // belong to which row
    for(int i = 0; i < Col_Max; ++i)
        {
        id_to_row_[get_field_win(i, new_row)->GetId()] = new_row;
        }

    if(0 == rows_count_)
        {
        sizer_->SetMinSize(sizer_->CalcMin());
        }

    rows_count_++;
    duration_scalars_.insert(duration_scalars_.begin() + new_row, -1);

    set_tab_order();

    // update state of controls on the two rows affected by addition of
    // a new row
    if(prev_row != -1)
        {
        update_row(prev_row);
        }
    update_row(new_row);

    redo_layout();
}

void InputSequenceEditor::set_tab_order()
{
    // The desired tab order is as follows:
    // 1. data entry fields from left to right, top to bottom:
    //      Col_Value
    //      Col_From
    //      Col_DurationMode
    //      Col_DurationNum
    //      Col_Then
    // 2. dialog's OK button
    // 3. then Remove and Add buttons, top to bottom
    //      Col_Remove
    //      Col_Add
    // 4. dialog's Cancel button

    if(0 == rows_count_)
        return;

    std::vector<wxWindow*> order;
    for(int row = 0; row < rows_count_; ++row)
        {
        for (int col = Col_Value; col <= Col_Then; ++col)
            order.push_back(get_field_win(col, row));
        }
    order.push_back(ok_button_);
    for(int row = 0; row < rows_count_; ++row)
        {
        order.push_back(get_field_win(Col_Remove, row));
        order.push_back(get_field_win(Col_Add, row));
        }
    order.push_back(cancel_button_);

    for(size_t i = 1; i < order.size(); ++i)
        {
        order[i]->MoveAfterInTabOrder(order[i - 1]);
        }
}

void InputSequenceEditor::remove_row(int row)
{
    duration_scalars_.erase(duration_scalars_.begin() + row);
    rows_count_--;

    // remove all controls from the row
    for(int i = 0; i < Col_Max; ++i)
        {
        int index = row * Col_Max;
        wxWindow* win = sizer_->GetItem(index)->GetWindow();
        sizer_->Detach(index);
        win->Destroy();
        }

    redo_layout();

    // update id_to_row_ mapping:
    std::vector<wxWindowID> to_remove;
    for(id_to_row_map::iterator i = id_to_row_.begin(); i != id_to_row_.end(); ++i)
        {
        if(i->second == row)
            {
            to_remove.push_back(i->first);
            }
        else if(i->second > row)
            {
            i->second = i->second - 1;
            }
        }
    LMI_ASSERT(!to_remove.empty());
    for(std::vector<wxWindowID>::const_iterator rm = to_remove.begin()
        ;rm != to_remove.end()
        ;++rm)
        {
        id_to_row_.erase(*rm);
        }

    // update the row following the one we just removed and the one before it,
    // as well as all subsequent rows (because many "from ..." lines may be
    // affected):
    for(int i = wxMax(0, row - 1); i < rows_count_; ++i)
        {
        update_row(i); // for "from ..." text
        }
}

void InputSequenceEditor::update_row(int row)
{
    bool const is_last_row = (row == rows_count_ - 1);

    // update duration_scalars_ to reflect current UI state
    duration_scalars_[row] = compute_duration_scalar(row);

    // "from" column:
    from_field(row).SetLabel(format_from_text(row));

    // "maturity" should be an option only on the last row:
    duration_mode_field(row).allow_maturity(is_last_row);

    // duration number visibility:
    duration_num_field(row).Show(duration_mode_field(row).needs_number());

    if(duration_mode_field(row).value() == e_number_of_years)
        {
        // ", then" is not shown on the last row:
        if(is_last_row)
            {
            then_field(row).SetLabel("years");
            }
        else
            {
            then_field(row).SetLabel("years, then");
            }
        }
    else
        {
        // ", then" is not shown on the last row:
        if(is_last_row)
            {
            then_field(row).SetLabel("");
            }
        else
            {
            then_field(row).SetLabel(", then");
            }
        }

    // remove/add buttons aren't shown on the last row:
    remove_button(row).Show(!is_last_row);
    add_button(row).Show(!is_last_row);

    redo_layout();
}

void InputSequenceEditor::redo_layout()
{
    wxSizer* sizer = GetSizer();
    sizer->Layout();
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

wxString InputSequenceEditor::format_from_text(int row)
{
    if(0 == row)
        {
        return "from issue date";
        }

    duration_mode mode = duration_mode_field(row - 1).value();
    long num = 0;
    if(duration_mode_field(row - 1).needs_number())
        {
        if(!duration_num_field(row - 1).GetValue().ToLong(&num))
            {
            return "";
            }
        }

    switch(mode)
        {
        case e_retirement:
            {
            return "from retirement";
            }
        case e_attained_age:
            {
            return wxString::Format("from age %ld", num);
            }
        case e_duration:
            {
            return wxString::Format("from duration %ld", num);
            }
        case e_number_of_years:
            {
            long yrs = 0;
            int i = row - 1;
            while(0 <= i && duration_mode_field(i).value() == e_number_of_years)
                {
                long num_i = 0;
                duration_num_field(i).GetValue().ToLong(&num_i);
                yrs += num_i;
                i--;
                }
            return wxString::Format
                ("%s + %ld years",
                format_from_text(i + 1).c_str(),
                yrs
                );
            }
        case e_maturity:
        case e_invalid_mode:
        case e_inception:
        case e_inforce:
            {
            fatal_error() << "unexpected duration_mode value" << LMI_FLUSH;
            return "";
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

wxWindow* InputSequenceEditor::get_field_win(int col, int row)
{
    wxSizerItem* i = sizer_->GetItem(col + Col_Max * row);
    LMI_ASSERT(i);

    wxWindow* w = i->GetWindow();
    LMI_ASSERT(w);

    return w;
}

template<typename T>
T& InputSequenceEditor::get_field(int col, int row)
{
    T* t = dynamic_cast<T*>(get_field_win(col, row));
    LMI_ASSERT(t);

    return *t;
}

int InputSequenceEditor::compute_duration_scalar(int row)
{
    long duration_num = -1;
    wxString const duration_num_str = duration_num_field(row).GetValue();
    if(duration_num_str.empty())
        {
        duration_num = 0;
        }
    else
        {
        duration_num_str.ToLong(&duration_num);
        }
    LMI_ASSERT(-1 != duration_num);

    switch(duration_mode_field(row).value())
        {
        case e_retirement:
            {
            return input_.retirement_age() - input_.issue_age();
            }
        case e_attained_age:
            {
            return duration_num - input_.issue_age();
            }
        case e_duration:
            {
            return duration_num;
            }
        case e_number_of_years:
            {
            if(0 == row)
                {
                return duration_num;
                }
            else
                {
                return compute_duration_scalar(row - 1) + duration_num;
                }
            }
        case e_maturity:
            {
            return input_.years_to_maturity();
            }

        case e_invalid_mode:
        case e_inception:
        case e_inforce:
            {
            fatal_error() << "unexpected duration_mode value" << LMI_FLUSH;
            return 0;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

void InputSequenceEditor::adjust_duration_num(int row)
{
    int const scalar = duration_scalars_[row];
    int num = -1;

    switch(duration_mode_field(row).value())
        {
        case e_attained_age:
            {
            num = scalar + input_.issue_age();
            break;
            }
        case e_duration:
            {
            num = scalar;
            break;
            }
        case e_number_of_years:
            {
            if(0 == row)
                {
                num = scalar;
                }
            else
                {
                num = scalar - duration_scalars_[row - 1];
                }
            break;
            }

        case e_invalid_mode:
        case e_inception:
        case e_inforce:
        case e_retirement:
        case e_maturity:
            {
            return; // no visible number field to update
            }
        }

    duration_num_field(row).SetValue(wxString::Format("%d", num));
}

void InputSequenceEditor::UponDurationModeChange(wxCommandEvent& event)
{
    int row = id_to_row_[event.GetId()];

    adjust_duration_num(row);

    update_row(row);

    if(row == rows_count_ - 1)
        {
        if(duration_mode_field(row).value() != e_maturity)
            {
            add_row();
            }
        }
    else
        {
        for(int i = row + 1; i < rows_count_; ++i)
            {
            update_row(i); // for "from ..." text
            }
        }
}

void InputSequenceEditor::UponDurationNumChange(wxCommandEvent& event)
{
    int row = id_to_row_[event.GetId()];

    for(int i = row; i < rows_count_; ++i)
        {
        update_row(i); // for "from ..." text and duration_scalars_
        }
}

void InputSequenceEditor::UponRemoveRow(wxCommandEvent& event)
{
    int row = id_to_row_[event.GetId()];
    remove_row(row);
}

void InputSequenceEditor::UponAddRow(wxCommandEvent& event)
{
    int prev_row = id_to_row_[event.GetId()];
    int new_row = prev_row + 1;

    insert_row(new_row);

    // as a reasonable default for the value, use previous row's
    value_field(new_row).SetValue(value_field(prev_row).GetValue());

    // the best choice for the new row is e_number_of_years, so choose it;
    // set focus to the number to adjust it immediately
    duration_mode_field(new_row).value(e_number_of_years);
    for(int i = new_row; i < rows_count_; ++i)
        {
        update_row(i);
        }

    duration_num_field(new_row).SetFocus();
}
} // Unnamed namespace.

InputSequenceEntry::InputSequenceEntry()
{
}

InputSequenceEntry::InputSequenceEntry
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxString const&    name
    )
{
    Create(parent, id, name);
}

bool InputSequenceEntry::Create
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxString const&    name
    )
{
    title_ = "Edit Sequence";

    if(!wxPanel::Create(parent, id))
        {
        return false;
        }

    SetName(name);

    wxSizer* sizer = new(wx) wxBoxSizer(wxHORIZONTAL);

    text_ = new(wx) wxTextCtrl(this, wxID_ANY);
    button_ = new(wx) wxButton
        (this
        ,wxID_ANY
        ,"..."
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxBU_EXACTFIT
        );
    button_->SetToolTip("Open sequence editor");

    sizer->Add(text_, wxSizerFlags(1).Expand());
    sizer->Add(button_, wxSizerFlags().Expand().Border(wxLEFT, 1));

    SetSizer(sizer);

    button_->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEntry::UponOpenEditor)
        ,NULL
        ,this
        );

    return true;
}

void InputSequenceEntry::UponOpenEditor(wxCommandEvent&)
{
    MvcController const* tlw = dynamic_cast<MvcController const*>(wxGetTopLevelParent(this));
    LMI_ASSERT(tlw);
    Input const* input = dynamic_cast<Input const*>(&tlw->Model());
    LMI_ASSERT(input);

    // Center the window on the [...] button for best locality -- it will be
    // close to user's point of attention and the mouse cursor.
    InputSequenceEditor editor(button_, title_, *input);
    editor.CentreOnParent();

    std::string sequence_string = std::string(text_->GetValue());
    std::string const name(GetName().c_str());
    datum_sequence const& ds = *member_cast<datum_sequence>(input->operator[](name));

    std::map<std::string,std::string> const kwmap = ds.allowed_keywords();
    std::vector<std::string> const keywords =
        detail::extract_keys_from_string_map(kwmap);

    bool keywords_only =
            ds.keyword_values_are_allowable()
        && !ds.numeric_values_are_allowable()
        ;
    LMI_ASSERT(!(keywords_only && keywords.empty()));
    editor.set_keywords(keywords, keywords_only, ds.default_keyword());

    InputSequence sequence
        (sequence_string
        ,input->years_to_maturity()
        ,input->issue_age        ()
        ,input->retirement_age   ()
        ,input->inforce_year     ()
        ,input->effective_year   ()
        ,0
        ,keywords
        );

    std::string const diagnostics = sequence.formatted_diagnostics();
    if(!diagnostics.empty())
        {
        warning()
            << "The sequence is invalid and cannot be edited visually.\n"
            << diagnostics
            << LMI_FLUSH
            ;
        return;
        }

    editor.sequence(sequence);

    if(wxID_OK == editor.ShowModal())
        {
        text_->SetValue(editor.sequence_string());
        }
}

IMPLEMENT_DYNAMIC_CLASS(InputSequenceEntryXmlHandler, wxXmlResourceHandler)

InputSequenceEntryXmlHandler::InputSequenceEntryXmlHandler()
    :wxXmlResourceHandler()
{
    AddWindowStyles();
}

wxObject* InputSequenceEntryXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, InputSequenceEntry)

    control->Create
        (m_parentAsWindow
        ,GetID()
        ,GetName()
        );

    SetupWindow(control);

    if(HasParam("title"))
        {
        control->set_popup_title(GetText("title"));
        }

    return control;
}

bool InputSequenceEntryXmlHandler::CanHandle(wxXmlNode* node)
{
    return IsOfClass(node, "InputSequenceEntry");
}

