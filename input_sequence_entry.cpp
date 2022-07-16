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

#include "pchfile_wx.hpp"

#include "input_sequence_entry.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "contains.hpp"
#include "input.hpp"
#include "input_sequence.hpp"
#include "input_sequence_aux.hpp"       // extract_keys_from_string_map()
#include "mvc_controller.hpp"
#include "numeric_io_cast.hpp"
#include "ssize_lmi.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/display.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valtext.h>
#include <wx/wupdlock.h>

#include <algorithm>                    // copy()
#include <exception>
#include <iterator>                     // back_inserter()
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

choice_value const duration_mode_choice_values[] =
  {
    {e_retirement,       "until retirement"},
    {e_attained_age,     "until age"},
    {e_duration,         "until duration"},
    {e_number_of_years,  "for a period of"},
    {e_maturity,         "until maturity"},    // e_maturity must be last
  };

int const duration_mode_choices = lmi::ssize(duration_mode_choice_values);

DurationModeChoice::DurationModeChoice(wxWindow* parent)
{
    Create(parent, wxID_ANY);

    {
    wxWindowUpdateLocker lock(this);
    for(auto const& i : duration_mode_choice_values)
        {
        Append(i.label);
        }
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
    for(auto const& i : duration_mode_choice_values)
        {
        if(x == i.mode)
            {
            SetStringSelection(i.label);
            return;
            }
        }

    alarum() << "Unexpected duration_mode value." << LMI_FLUSH;
}

duration_mode DurationModeChoice::value() const
{
    int const sel = GetSelection();

    LMI_ASSERT(0 <= sel);
    LMI_ASSERT(sel < duration_mode_choices);

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
    friend class LayoutOnceGuard;

  public:
    InputSequenceEditor(wxWindow* parent, wxString const& title, Input const& input);
    InputSequenceEditor(InputSequenceEditor const&) = delete;
    InputSequenceEditor& operator=(InputSequenceEditor const&) = delete;

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

    void associate_text_ctrl(wxTextCtrl* t)
    {
        associated_text_ctrl_ = t;
    }

    void sequence(InputSequence const& s);
    std::string sequence_string();

    bool TransferDataFromWindow() override;

  private:
    // Helper class ensuring that only one relayout is really done during its
    // lifetime even if redo_layout() is called multiple times: it is much
    // simpler to create an object of this class before calling a function
    // which may or not call redo_layout() than check whether it did or not and
    // calling it only if it hadn't been already done. With this helper, calls
    // to redo_layout() can be freely added everywhere where they might be
    // needed without slowing down the UI to a crawl because just a single
    // layout will be effectively performed.
    class LayoutOnceGuard
    {
      public:
        explicit LayoutOnceGuard(InputSequenceEditor* editor)
            :editor_ {editor}
        {
            ++editor_->layout_freeze_count_;
        }

        ~LayoutOnceGuard()
        {
            if(!--editor_->layout_freeze_count_)
                {
                editor_->really_do_layout();
                }
        }

        LayoutOnceGuard(LayoutOnceGuard const&) = delete;
        LayoutOnceGuard& operator=(LayoutOnceGuard const&) = delete;

      private:
        InputSequenceEditor* const editor_;
    };

    // If this is positive, layout is frozen and needs to wait until it's
    // thawed which will happen when all currently existing LayoutOnceGuards go
    // out of scope.
    int layout_freeze_count_;

    // This function may be called multiple times but if a LayoutOnceGuard
    // currently exists, it does nothing immediately and just requests a
    // layout at a later time.
    void redo_layout();

    // This function is only called by LayoutOnceGuard or redo_layout() itself
    // and really lays out the dialog.
    void really_do_layout();

    void add_row();
    void insert_row(int row);
    void remove_row(int row);
    void update_row(int row);
    void set_tab_order();
    wxString format_from_text(int row);

    enum Col
        {Col_Value
        ,Col_From
        ,Col_DurationMode
        ,Col_DurationNum
        ,Col_Then
        ,Col_Remove
        ,Col_Add
        ,Col_Max
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

    wxSpinCtrl& duration_num_field(int row)
    {
        return get_field<wxSpinCtrl>(Col_DurationNum, row);
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
    void adjust_duration_num_range(int row);

    void update_diagnostics();
    bool is_valid_value(wxString const&);
    wxString get_diagnostics_message();

    void UponValueChange(wxCommandEvent& event);
    void UponDurationModeChange(wxCommandEvent& event);
    void UponDurationNumChange(wxCommandEvent& event);
    void UponRemoveRow(wxCommandEvent& event);
    void UponAddRow(wxCommandEvent& event);

    Input const& input_;
    std::vector<std::string> keywords_;
    bool keywords_only_;
    std::string default_keyword_;

    int rows_count_;
    wxScrolledWindow* rows_area_;
    wxFlexGridSizer* sizer_;
    wxButton* ok_button_;
    wxButton* cancel_button_;
    wxStaticText* diagnostics_;
    typedef std::map<wxWindowID,int> id_to_row_map;
    id_to_row_map id_to_row_;

    // scalar absolute values for end durations; this is used to recompute
    // duration number for certain duration modes
    std::vector<int> duration_scalars_;

    wxTextCtrl* associated_text_ctrl_;
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
    ,layout_freeze_count_ {1}    // It will be thawed when sequence() is called.
    ,input_               {input}
    ,keywords_only_       {false}
    ,rows_count_          {0}
{
    wxSizer* top = new(wx) wxBoxSizer(wxVERTICAL);

    rows_area_ = new(wx) wxScrolledWindow(this);
    top->Add(rows_area_, wxSizerFlags(1).Expand().DoubleBorder());

    wxSize const SizerGap
        (wxSizerFlags::GetDefaultBorder()
        ,wxSizerFlags::GetDefaultBorder()
        );
    sizer_ = new(wx) wxFlexGridSizer(Col_Max, SizerGap);
    rows_area_->SetSizer(sizer_);

    diagnostics_ = new(wx) wxStaticText
        (this
        ,wxID_ANY
        ,""
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxST_NO_AUTORESIZE
        );
    top->Add(diagnostics_, wxSizerFlags().Expand().DoubleBorder(wxLEFT|wxRIGHT));

    wxStdDialogButtonSizer* buttons = new(wx) wxStdDialogButtonSizer();
    buttons->AddButton(ok_button_ = new(wx) wxButton(this, wxID_OK));
    buttons->AddButton(cancel_button_ = new(wx) wxButton(this, wxID_CANCEL));
    buttons->Realize();
    ok_button_->SetDefault();

    top->Add(buttons, wxSizerFlags().Expand().Border());

    SetSizerAndFit(top);

    add_row();

    // Now that we have a row, set up the row area to scroll by one its height
    // (as we assume they all have the same size) vertically.
    wxArrayInt const& row_heights = sizer_->GetRowHeights();
    LMI_ASSERT(!row_heights.empty());
    rows_area_->SetScrollRate(0, row_heights[0] + SizerGap.y);

    value_field_ctrl(0).SetFocus();
}

void InputSequenceEditor::sequence(InputSequence const& s)
{
    LayoutOnceGuard guard(this);

    // The layout was frozen initially, but it can be thawed as soon as this
    // function returns because we will finally be able to determine our real
    // size. Notice that thaw will only happen on function exit, in the dtor
    // of the layout guard defined just above.
    --layout_freeze_count_;

    while(0 < rows_count_)
        {
        remove_row(0);
        }

    std::vector<ValueInterval> const& intervals = s.intervals();

    for(int i = 0; i < lmi::ssize(intervals); ++i)
        {
        ValueInterval const& data = intervals[i];

        add_row();

        duration_mode_field(i).value(data.end_mode);

        int dur_num = 0;
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

        duration_num_field(i).SetValue(dur_num);

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

    update_diagnostics();
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

        auto const endpoint = duration_num_field(i).GetValue();
        std::string const z = value_cast<std::string>(endpoint);

        switch(duration_mode_field(i).value())
            {
            case e_retirement:
                {
                s.append(" retirement");
                }
                break;
            case e_attained_age:
                {
                s.append(" @").append(z);
                }
                break;
            case e_duration:
                {
                s.append(" ").append(z);
                }
                break;
            case e_number_of_years:
                {
                s.append(" #").append(z);
                }
                break;
            case e_maturity:
                {
                LMI_ASSERT(i == rows_count_ - 1);
                // " maturity" is implicit, don't add it
                }
                break;
            case e_invalid_mode:
            case e_inception:
            case e_inforce:
                {
                alarum() << "Unexpected duration_mode value." << LMI_FLUSH;
                }
                break;
            }
        }

    // This code largely duplicates InputSequence::canonical_form(),
    // but, unfortunately, the two cannot readily be combined. (It
    // would be possible to produce a std::vector<ValueInterval> here
    // and canonicalize it, but that would not be simpler.) Yet it
    // makes no sense to maintain the two in parallel, so just
    // recanonicalize the result to simplify it.
    return InputSequence
        (s
        ,input_.years_to_maturity()
        ,input_.issue_age        ()
        ,input_.retirement_age   ()
        ,input_.inforce_year     ()
        ,input_.effective_year   ()
        ,keywords_
        ,keywords_only_
        ,default_keyword_
        ).canonical_form()
        ;
}

void SizeWinForText(wxControl* win, wxString const& text)
{
    win->SetMinSize(win->GetSizeFromTextSize(win->GetTextExtent(text).x));
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

    wxSizerFlags const flags = wxSizerFlags().Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);

    wxControl* value_ctrl;
    if(!keywords_.empty())
        {
        wxComboBox* combo = new(wx) wxComboBox
            (rows_area_
            ,wxID_ANY
            ,"0"
            ,wxDefaultPosition
            ,wxDefaultSize
            ,0
            ,nullptr
            ,keywords_only_ ? wxCB_READONLY : 0
            );
        value_ctrl = combo;

        wxArrayString kw;
        std::copy(keywords_.begin(), keywords_.end(), std::back_inserter(kw));
        {
        wxWindowUpdateLocker lock_combo(combo);
        combo->Append(kw);
        }

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
        value_ctrl = new(wx) wxTextCtrl(rows_area_, wxID_ANY, "0");
        value_ctrl->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
        }

    sizer_->wxSizer::Insert(insert_pos++, value_ctrl, wxSizerFlags(flags).TripleBorder(wxRIGHT));
    wxStaticText* from_label = new(wx) wxStaticText
        (rows_area_
        ,wxID_ANY
        ,LARGEST_FROM_TEXT
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxST_NO_AUTORESIZE
        );
    SizeWinForText(from_label, LARGEST_FROM_TEXT);
    sizer_->wxSizer::Insert(insert_pos++, from_label, flags);
    wxChoice* duration_mode = new(wx) DurationModeChoice(rows_area_);
    sizer_->wxSizer::Insert(insert_pos++, duration_mode, flags);
    wxSpinCtrl* duration_num = new(wx) wxSpinCtrl(rows_area_, wxID_ANY, "");
    sizer_->wxSizer::Insert(insert_pos++, duration_num, flags);
    SizeWinForText(duration_num, "9999");
    wxStaticText* then_label = new(wx) wxStaticText
        (rows_area_
        ,wxID_ANY
        ,LARGEST_THEN_TEXT
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxST_NO_AUTORESIZE
        );
    sizer_->wxSizer::Insert(insert_pos++, then_label, flags);
    SizeWinForText(then_label, LARGEST_THEN_TEXT);

#undef LARGEST_FROM_TEXT
#undef LARGEST_THEN_TEXT

    // Note: We can't use wxID_REMOVE/wxID_ADD for these buttons, because
    // there's more than one of them and the ID is used to distinguish between
    // them. Consequently, we have to add stock graphics manually under wxGTK.
    wxButton* remove = new(wx) wxButton
        (rows_area_
        ,wxID_ANY
        ,"Remove"
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxBU_AUTODRAW | wxBU_EXACTFIT | wxBORDER_NONE
        );
#if defined __WXGTK__
    wxBitmap RemoveBmp = wxArtProvider::GetBitmap("gtk-remove", wxART_BUTTON);
    remove->SetBitmap(RemoveBmp);
#endif // defined __WXGTK__

    remove->SetToolTip("Remove this row");
    remove->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEditor::UponRemoveRow)
        ,nullptr
        ,this
        );
    sizer_->wxSizer::Insert(insert_pos++, remove, wxSizerFlags(flags).TripleBorder(wxLEFT));

    wxButton* add = new(wx) wxButton
        (rows_area_
        ,wxID_ANY
        ,"Add"
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxBU_AUTODRAW | wxBU_EXACTFIT | wxBORDER_NONE
        );
#if defined __WXGTK__
    wxBitmap AddBmp = wxArtProvider::GetBitmap("gtk-add", wxART_BUTTON);
    add->SetBitmap(AddBmp);
#endif // defined __WXGTK__

    add->SetToolTip("Insert a new row after this one");
    add->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEditor::UponAddRow)
        ,nullptr
        ,this
        );
    sizer_->wxSizer::Insert(insert_pos++, add, wxSizerFlags(flags).Border(wxLEFT, 0).Right());

    // update id_to_row_ mapping:
    for(auto& i : id_to_row_)
        {
        if(new_row <= i.second)
            {
            i.second = i.second + 1;
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

    ++rows_count_;
    duration_scalars_.insert(duration_scalars_.begin() + new_row, -1);

    set_tab_order();

    // connect event handlers
    ::Connect
        (value_ctrl
        ,wxEVT_COMMAND_TEXT_UPDATED
        ,&InputSequenceEditor::UponValueChange
        ,wxID_ANY
        ,this
        );
    ::Connect
        (duration_mode
        ,wxEVT_COMMAND_CHOICE_SELECTED
        ,&InputSequenceEditor::UponDurationModeChange
        ,wxID_ANY
        ,this
        );
    ::Connect
        (duration_num
        ,wxEVT_COMMAND_SPINCTRL_UPDATED
        ,&InputSequenceEditor::UponDurationNumChange
        ,wxID_ANY
        ,this
        );

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
    // The desired tab order for the items inside the row area is as follows:
    // 1. data entry fields from left to right, top to bottom:
    //      Col_Value
    //      Col_From
    //      Col_DurationMode
    //      Col_DurationNum
    //      Col_Then
    // 2. then Remove and Add buttons, top to bottom
    //      Col_Remove
    //      Col_Add
    //
    // The "OK" and "Cancel" buttons are outside of the rows area and are not
    // affected by this function.

    if(0 == rows_count_)
        return;

    std::vector<wxWindow*> order;
    for(int row = 0; row < rows_count_; ++row)
        {
        for(int col = Col_Value; col <= Col_Then; ++col)
            order.push_back(get_field_win(col, row));
        }
    for(int row = 0; row < rows_count_; ++row)
        {
        order.push_back(get_field_win(Col_Remove, row));
        order.push_back(get_field_win(Col_Add, row));
        }

    for(int i = 1; i < lmi::ssize(order); ++i)
        {
        order[i]->MoveAfterInTabOrder(order[i - 1]);
        }
}

void InputSequenceEditor::remove_row(int row)
{
    duration_scalars_.erase(duration_scalars_.begin() + row);
    --rows_count_;

    // remove all controls from the row
    for(int i = 0; i < Col_Max; ++i)
        {
        // Note that the index here is constant and always refers to
        // the first window in the given row: as the indices of the
        // subsequent elements adjust, by decreasing by one, when we
        // delete this index, repeatedly deleting Col_Max elements at
        // this position results in deleting the entire row contents.
        int index = row * Col_Max;
        wxWindow* win = sizer_->GetItem(index)->GetWindow();
        LMI_ASSERT(win);
        sizer_->Detach(index); // Superfluous--Destroy() does this anyway.
        win->Destroy();
        }

    redo_layout();

    // update id_to_row_ mapping:
    std::vector<wxWindowID> to_remove;
    for(auto& i : id_to_row_)
        {
        if(i.second == row)
            {
            to_remove.push_back(i.first);
            }
        else if(row < i.second)
            {
            i.second = i.second - 1;
            }
        }
    LMI_ASSERT(!to_remove.empty());
    for(auto const& rm : to_remove)
        {
        id_to_row_.erase(rm);
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

    // duration number visibility and range:
    duration_num_field(row).Show(duration_mode_field(row).needs_number());
    adjust_duration_num_range(row);

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
    if(layout_freeze_count_ == 0)
        {
        really_do_layout();
        }
}

void InputSequenceEditor::really_do_layout()
{
    wxSizer* sizer = GetSizer();

    // Try to avoid showing the vertical scrollbar by making the rows area as
    // big as it needs to be. An explicit call to SetMinSize() is required for
    // this because wxScrolledWindow ignores the best size of its contents in
    // its scrollable direction.
    //
    // Notice that if this size is too big and the window wouldn't fit on the
    // screen when using it, the size of the rows area will be adjusted down by
    // exactly as much as necessary because it is the only element of the
    // dialog with non-fixed size (i.e. proportion different from 0) and the
    // sizer code correctly considers that if there is not enough space for
    // everything, it's better to reduce the size of the variable size items
    // rather than of the fixed size ones.
    wxSize MinRowsSize = sizer_->GetMinSize();
    rows_area_->SetMinSize(MinRowsSize);

    // Now check if we're actually going to have a scrollbar or not by
    // comparing our ideal minimum size with the size we would actually have.
    if(sizer->ComputeFittingClientSize(this) != sizer->GetMinSize())
        {
        // The only possible reason for discrepancy is that the window would be
        // too big to fit on the screen and so the actual size of the rows area
        // will be smaller than its minimum size and hence the scrollbar will
        // be shown and we need to account for it in our horizontal size by
        // allocating enough space for the scrollbar itself and also an extra
        // border between the controls and this scrollbar as things would look
        // too cramped otherwise.
        MinRowsSize.x
            += wxSizerFlags::GetDefaultBorder()
            +  wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y)
            ;
        rows_area_->SetMinSize(MinRowsSize);
        }

    sizer->Layout();
    sizer->Fit(this);
}

wxString InputSequenceEditor::format_from_text(int row)
{
    if(0 == row)
        {
        return "from issue date";
        }

    duration_mode mode = duration_mode_field(row - 1).value();
    int num = 0;
    if(duration_mode_field(row - 1).needs_number())
        {
        num = duration_num_field(row - 1).GetValue();
        }

    switch(mode)
        {
        case e_retirement:
            {
            return "from retirement";
            }
        case e_attained_age:
            {
            return wxString::Format("from age %d", num);
            }
        case e_duration:
            {
            return wxString::Format("from duration %d", num);
            }
        case e_number_of_years:
            {
            int yrs = 0;
            int i = row - 1;
            while(0 <= i && duration_mode_field(i).value() == e_number_of_years)
                {
                int num_i = duration_num_field(i).GetValue();
                yrs += num_i;
                --i;
                }
            return wxString::Format
                ("%s + %d years"
                ,format_from_text(i + 1).c_str()
                ,yrs
                );
            }
        case e_maturity:
        case e_invalid_mode:
        case e_inception:
        case e_inforce:
            {
            alarum() << "Unexpected duration_mode value." << LMI_FLUSH;
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
    int duration_num = duration_num_field(row).GetValue();

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
            alarum() << "Unexpected duration_mode value." << LMI_FLUSH;
            return 0;
            }
        }
    throw "Unreachable--silences a compiler diagnostic.";
}

void InputSequenceEditor::adjust_duration_num_range(int row)
{
    if(!duration_mode_field(row).needs_number())
        return;

    int const prev_duration = (0 < row) ? duration_scalars_[row - 1] : 0;
    wxSpinCtrl& duration = duration_num_field(row);

    int range_min = 0;
    int range_max = 0;

    switch(duration_mode_field(row).value())
        {
        case e_attained_age:
            {
            range_min = input_.issue_age() + 1 + prev_duration;
            range_max = input_.maturity_age() - 1;
            break;
            }
        case e_duration:
            {
            range_min = 1 + prev_duration;
            range_max = input_.years_to_maturity() - 1;
            break;
            }
        case e_number_of_years:
            {
            range_min = 1;
            range_max = input_.years_to_maturity() - prev_duration - 1;
            break;
            }
        case e_maturity:
        case e_retirement:
        case e_invalid_mode:
        case e_inception:
        case e_inforce:
            {
            alarum() << "Unexpected duration_mode value." << LMI_FLUSH;
            break;
            }
        }

    // See:
    //   https://lists.nongnu.org/archive/html/lmi/2015-05/msg00006.html
    if(range_min <= range_max)
        {
        duration.Enable();
        duration.SetRange(range_min, range_max);
        }
    else
        {
        duration.Disable();
        }
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

    adjust_duration_num_range(row);

    duration_num_field(row).SetValue(num);
}

void InputSequenceEditor::update_diagnostics()
{
    // Validate the sequence and if it's not valid, show an error
    // and disable the OK button.

    wxString msg = get_diagnostics_message();

    if(diagnostics_->GetLabel() != msg)
        {
        diagnostics_->SetLabel(msg);
        redo_layout();
        }

    ok_button_->Enable(msg.empty());
}

bool InputSequenceEditor::is_valid_value(wxString const& w)
{
    std::string const s(w.c_str());

    if(contains(keywords_, s))
        {
        return true;
        }

    if(keywords_only_)
        {
        return false;
        }

    try
        {
        numeric_io_cast<double>(s);
        return true;
        }
    catch(...)
        {
        return false;
        }
}

wxString InputSequenceEditor::get_diagnostics_message()
{
    // Diagnose anticipated input errors; return an error message,
    // written in the context of the GUI, for the first error found.
    for(int row = 0; row < rows_count_; ++row)
        {
        wxString const value = value_field(row).GetValue();
        if(value.empty())
            return wxString::Format("Missing value on row %d.", row);

        if(!is_valid_value(value))
            return wxString::Format("Invalid keyword \"%s\" on row %d.", value.c_str(), row);
        }

    // Diagnose unanticipated input errors by invoking the parser;
    // return the first line of its diagnostics as an error message.
    try
        {
        InputSequence const sequence
            (sequence_string()
            ,input_.years_to_maturity()
            ,input_.issue_age        ()
            ,input_.retirement_age   ()
            ,input_.inforce_year     ()
            ,input_.effective_year   ()
            ,keywords_
            ,keywords_only_
            ,default_keyword_
            );
        }
    catch(std::exception const& e)
        {
        return abridge_diagnostics(e.what());
        }

    return wxString();
}

void InputSequenceEditor::UponValueChange(wxCommandEvent&)
{
    update_diagnostics();
}

void InputSequenceEditor::UponDurationModeChange(wxCommandEvent& event)
{
    LayoutOnceGuard guard(this);

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

    update_diagnostics();
}

void InputSequenceEditor::UponDurationNumChange(wxCommandEvent& event)
{
    LayoutOnceGuard guard(this);

    int row = id_to_row_[event.GetId()];

    for(int i = row; i < rows_count_; ++i)
        {
        update_row(i); // for "from ..." text and duration_scalars_
        }

    update_diagnostics();
}

void InputSequenceEditor::UponRemoveRow(wxCommandEvent& event)
{
    LayoutOnceGuard guard(this);

    int row = id_to_row_[event.GetId()];
    remove_row(row);

    update_diagnostics();
}

void InputSequenceEditor::UponAddRow(wxCommandEvent& event)
{
    LayoutOnceGuard guard(this);

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

    update_diagnostics();
}

bool InputSequenceEditor::TransferDataFromWindow()
{
    if(!wxDialog::TransferDataFromWindow())
        return false;

    // We need to set the value as soon as possible -- when used in wxDataViewCtrl, the value
    // is read from editor control as soon as focus changes, which is before ShowModal() returns.
    if(associated_text_ctrl_)
        associated_text_ctrl_->SetValue(sequence_string());

    return true;
}

class InputSequenceTextCtrl
    :public wxTextCtrl
{
  public:
    InputSequenceTextCtrl(wxWindow* parent, wxWindowID id);

  private:
    void UponChar(wxKeyEvent& event);
    void UponKeyDown(wxKeyEvent& event);
};

InputSequenceTextCtrl::InputSequenceTextCtrl(wxWindow* parent, wxWindowID id)
    :wxTextCtrl
        (parent
        ,id
        ,wxString()
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxTE_PROCESS_ENTER
        )
{
    ::Connect
            (this
            ,wxEVT_CHAR
            ,&InputSequenceTextCtrl::UponChar
            );
    ::Connect
            (this
            ,wxEVT_KEY_DOWN
            ,&InputSequenceTextCtrl::UponKeyDown
            );
}

void InputSequenceTextCtrl::UponChar(wxKeyEvent& event)
{
    if(!GetParent()->ProcessWindowEvent(event))
        event.Skip();
}

void InputSequenceTextCtrl::UponKeyDown(wxKeyEvent& event)
{
    if (!GetParent()->ProcessWindowEvent(event))
        event.Skip();
}

class InputSequenceButton
    :public wxButton
{
  public:
    InputSequenceButton(wxWindow* parent, wxWindowID id);
};

InputSequenceButton::InputSequenceButton(wxWindow* parent, wxWindowID id)
    :wxButton(parent, id, "...", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT)
{
}
} // Unnamed namespace.

InputSequenceEntry::InputSequenceEntry
    (wxWindow*          parent
    ,wxWindowID         id
    ,wxString const&    name
    )
    :input_ {nullptr}
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

    text_ = new(wx) InputSequenceTextCtrl(this, wxID_ANY);
    button_ = new(wx) InputSequenceButton(this, wxID_ANY);

    sizer->Add(text_, wxSizerFlags(1).Expand());
    sizer->Add(button_, wxSizerFlags().Expand().Border(wxLEFT));

    SetSizer(sizer);

    text_->Connect
        (wxEVT_KILL_FOCUS
        ,wxFocusEventHandler(InputSequenceEntry::UponChildKillFocus)
        ,nullptr
        ,this
        );
    text_->Connect
        (wxEVT_TEXT_ENTER
        ,wxCommandEventHandler(InputSequenceEntry::UponEnter)
        ,nullptr
        ,this
        );

    button_->Connect
        (wxEVT_KILL_FOCUS
        ,wxFocusEventHandler(InputSequenceEntry::UponChildKillFocus)
        ,nullptr
        ,this
        );
    button_->Connect
        (wxEVT_COMMAND_BUTTON_CLICKED
        ,wxCommandEventHandler(InputSequenceEntry::UponOpenEditor)
        ,nullptr
        ,this
        );

    return true;
}

void InputSequenceEntry::open_editor()
{
    DoOpenEditor();

    // Put focus back on the control itself as normal focus restoring logic
    // doesn't work as we block some of the events in UponChildKillFocus().
    text_->SetFocus();
}

void InputSequenceEntry::input(Input const& input)
{
    input_ = &input;
}

Input const& InputSequenceEntry::input() const
{
    if(input_)
        {
        return *input_;
        }
    else
        {
        // MvcController's design uses editor controls that only have local
        // knowledge of the value they directly edit. This isn't an
        // unreasonable assumption and e.g. wxDataViewCtrl does the same.
        // Unfortunately, it doesn't fit InputSequenceEditor, which needs
        // additional information about the Input instance the sequence is used
        // in. Hence this hack -- it gets the Input from the parent
        // MvcController if used inside one.
        MvcController const* tlw = dynamic_cast<MvcController const*>(wxGetTopLevelParent(const_cast<InputSequenceEntry*>(this)));
        LMI_ASSERT(tlw);
        Input const* input = dynamic_cast<Input const*>(&tlw->Model());
        LMI_ASSERT(input);

        return *input;
        }
}

void InputSequenceEntry::field_name(std::string const& name)
{
    field_name_ = name;
}

std::string InputSequenceEntry::field_name() const
{
    if(!field_name_.empty())
        {
        return field_name_;
        }
    else
        {
        // see the explanation in input()
        return std::string(GetName().c_str());
        }
}

void InputSequenceEntry::UponChildKillFocus(wxFocusEvent& event)
{
    // Never prevent the default focus event handling from taking place,
    // whatever else we do with it.
    event.Skip();

    // Check whether the given possibly null window is a child of another one.
    auto const is_child_of = [](wxWindow const* c, wxWindow const* p)
        {
        return c && c->GetParent() == p;
        };

    // Suppress normal focus loss event processing if the focus simply goes to
    // another element of this composite window or changes inside an
    // InputSequenceEntry window opened from it and having our button as the
    // parent: this prevents the in-place editor in the census view from
    // closing whenever this happens.
    if
        (  is_child_of(event.GetWindow(), this)
        || is_child_of(wxGetTopLevelParent(event.GetWindow()), button_)
        )
        {
        return;
        }

    ProcessWindowEvent(event);
}

void InputSequenceEntry::UponEnter(wxCommandEvent& event)
{
    // Pressing Enter key without modifiers just accepts the changes, but we
    // allow using Alt-Enter to open the input sequence editor dialog from
    // keyboard.
    if(!wxGetKeyState(WXK_ALT))
        {
        event.Skip();
        return;
        }

    open_editor();
}

void InputSequenceEntry::UponOpenEditor(wxCommandEvent&)
{
    DoOpenEditor();

    // If this editor is used inside wxDataViewCtrl, don't keep focus after
    // showing the dialog but give it to the parent to ensure that the editor
    // is closed by it. Notice that there is no need to check if we actually
    // are inside wxDataViewCtrl before doing it as otherwise our parent (e.g.
    // a wxPanel) will just give focus back to us and nothing really happens.
    GetParent()->SetFocus();
}

void InputSequenceEntry::DoOpenEditor()
{
    Input const& in = input();

    // Center the window on the [...] button for best locality -- it will be
    // close to user's point of attention and the mouse cursor.
    // Note that if the parent used here changes, the code in
    // UponChildKillFocus() would need to be updated.
    InputSequenceEditor editor(button_, title_, in);

    std::string sequence_string = text_->GetValue().ToStdString(wxConvUTF8);
    sequence_base const& ds = *member_cast<sequence_base>(in[field_name()]);

    std::map<std::string,std::string> const kwmap = ds.allowed_keywords();
    std::vector<std::string> const keywords =
        detail::extract_keys_from_string_map(kwmap);

    bool keywords_only =
            ds.keyword_values_are_allowable()
        && !ds.numeric_values_are_allowable()
        ;
    LMI_ASSERT(!(keywords_only && keywords.empty()));
    editor.set_keywords(keywords, keywords_only, ds.default_keyword());

    try
        {
        InputSequence sequence
            (sequence_string
            ,in.years_to_maturity()
            ,in.issue_age        ()
            ,in.retirement_age   ()
            ,in.inforce_year     ()
            ,in.effective_year   ()
            ,keywords
            ,keywords_only
            ,ds.default_keyword()
            );
        editor.sequence(sequence);
        }
    catch(std::exception const& e)
        {
        warning()
            << "The sequence is invalid and cannot be edited visually.\n"
            << abridge_diagnostics(e.what())
            << std::flush
            ;
        return;
        }

    editor.associate_text_ctrl(text_);
    editor.CenterOnParent();

    editor.ShowModal();
}

IMPLEMENT_DYNAMIC_CLASS(InputSequenceEntryXmlHandler, wxXmlResourceHandler)

InputSequenceEntryXmlHandler::InputSequenceEntryXmlHandler()
    :wxXmlResourceHandler {}
{
    AddWindowStyles();
}

wxObject* InputSequenceEntryXmlHandler::DoCreateResource()
{
    XRC_MAKE_INSTANCE(control, InputSequenceEntry)

    control->Create
        (GetParentAsWindow()
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
