// Census manager.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.
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

#include "pchfile_wx.hpp"

#include "census_view.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "census_document.hpp"
#include "contains.hpp"
#include "default_view.hpp"
#include "edit_mvc_docview_parameters.hpp"
#include "facets.hpp"                   // tab_is_not_whitespace_locale()
#include "illustration_view.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "input_sequence_entry.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp"               // is_ok_for_cctype()
#include "path_utility.hpp"
#include "rtti_lmi.hpp"                 // lmi::TypeInfo
#include "safely_dereference_as.hpp"
#include "single_choice_popup_menu.hpp"
#include "timer.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"               // class ClipboardEx

#include <wx/dataview.h>
#include <wx/datectrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/utils.h>                   // wxBusyCursor
#include <wx/valnum.h>
#include <wx/wupdlock.h>
#include <wx/xrc/xmlres.h>

#include <algorithm>
#include <cctype>
#include <cstddef>                      // std::size_t
#include <istream>                      // std::ws
#include <iterator>                     // std::insert_iterator
#include <sstream>

namespace
{
std::string insert_spaces_between_words(std::string const& s)
{
    std::string r;
    std::insert_iterator<std::string> j(r, r.begin());
    for(auto const& i : s)
        {
        if(is_ok_for_cctype(i) && std::isupper(i) && !r.empty())
            {
            *j++ = ' ';
            }
        *j++ = i;
        }
    return r;
}

/// Data needed to create UI for tn_range<> types.

struct tn_range_variant_data
    :public wxVariantData
{
    tn_range_variant_data(std::string const& value_, double min_, double max_)
        :value(value_), min(min_), max(max_)
    {
    }

    tn_range_variant_data(tn_range_base const& r)
        :value(r.str()), min(r.universal_minimum()), max(r.universal_maximum())
    {
    }

    bool Eq(wxVariantData& data) const override
    {
        tn_range_variant_data* d = dynamic_cast<tn_range_variant_data*>(&data);
        if(!d)
            return false;
        return value == d->value && min == d->min && max == d->max;
    }

    wxString GetType() const override { return typeid(tn_range_variant_data).name(); }

    wxVariantData* Clone() const override
    {
        return new(wx) tn_range_variant_data(value, min, max);
    }

    std::string value;
    double min, max;
};

// class RangeTypeRenderer

class RangeTypeRenderer
    :public wxDataViewCustomRenderer
{
  protected:
    RangeTypeRenderer();

  public:
    bool HasEditorCtrl() const override { return true; }
    wxWindow* CreateEditorCtrl(wxWindow* parent, wxRect labelRect, wxVariant const& value) override;
    bool GetValueFromEditorCtrl(wxWindow* editor, wxVariant& value) override;
    bool Render(wxRect rect, wxDC* dc, int state) override;
    wxSize GetSize() const override;
    bool SetValue(wxVariant const& value) override;
    bool GetValue(wxVariant& value) const override;

  protected:
    virtual wxWindow* DoCreateEditor(wxWindow* parent, wxRect const& rect, tn_range_variant_data const& data) = 0;
    virtual std::string DoGetValueFromEditor(wxWindow* editor) = 0;

    std::string m_value;
    double m_min, m_max;
};

RangeTypeRenderer::RangeTypeRenderer()
    :wxDataViewCustomRenderer
    (typeid(tn_range_variant_data).name()
    ,wxDATAVIEW_CELL_EDITABLE
    ,wxDVR_DEFAULT_ALIGNMENT)
{
}

wxWindow* RangeTypeRenderer::CreateEditorCtrl(wxWindow* parent, wxRect labelRect, wxVariant const& value)
{
    tn_range_variant_data const* data = dynamic_cast<tn_range_variant_data*>(value.GetData());
    LMI_ASSERT(data);

    // Always use default height for editor controls
    wxRect rect(labelRect);
    rect.height = -1;

    return DoCreateEditor(parent, rect, *data);
}

bool RangeTypeRenderer::GetValueFromEditorCtrl(wxWindow* editor, wxVariant& value)
{
    std::string const val = DoGetValueFromEditor(editor);
    value = new(wx) tn_range_variant_data(val, m_min, m_max);
    return true;
}

bool RangeTypeRenderer::Render(wxRect rect, wxDC* dc, int state)
{
    RenderText(m_value, 0, rect, dc, state);
    return true;
}

wxSize RangeTypeRenderer::GetSize() const
{
    wxSize sz = GetTextExtent(m_value);

    // Allow some space for the spin button, which is approximately the size of
    // a scrollbar (and getting pixel-exact value would be complicated). Also
    // add some whitespace between the text and the button:
    sz.x += wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
    sz.x += GetTextExtent("M").x;

    return sz;
}

bool RangeTypeRenderer::SetValue(wxVariant const& value)
{
    tn_range_variant_data const* data = dynamic_cast<tn_range_variant_data*>(value.GetData());
    LMI_ASSERT(data);

    m_value = data->value;
    m_min = data->min;
    m_max = data->max;
    return true;
}

bool RangeTypeRenderer::GetValue(wxVariant& value) const
{
    value = new(wx) tn_range_variant_data(m_value, m_min, m_max);
    return true;
}

// class IntSpinRenderer

class IntSpinRenderer
    :public RangeTypeRenderer
{
  public:
    IntSpinRenderer() : RangeTypeRenderer() {}

  protected:
    wxWindow* DoCreateEditor(wxWindow* parent, wxRect const& rect, tn_range_variant_data const& data) override;
    std::string DoGetValueFromEditor(wxWindow* editor) override;
};

wxWindow* IntSpinRenderer::DoCreateEditor
    (wxWindow* parent
     ,wxRect const& rect
     ,tn_range_variant_data const& data)
{
    return new(wx) wxSpinCtrl
        (parent
        ,wxID_ANY
        ,data.value
        ,rect.GetTopLeft()
        ,rect.GetSize()
        ,wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER
        ,static_cast<long>(data.min)
        ,static_cast<long>(data.max)
        ,value_cast<long>(data.value));
}

std::string IntSpinRenderer::DoGetValueFromEditor(wxWindow* editor)
{
    wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(editor);
    LMI_ASSERT(spin);

    return value_cast<std::string>(spin->GetValue());
}

// class DoubleRangeRenderer

class DoubleRangeRenderer
    :public RangeTypeRenderer
{
  public:
    DoubleRangeRenderer() : RangeTypeRenderer() {}

  protected:
    wxWindow* DoCreateEditor(wxWindow* parent, wxRect const& rect, tn_range_variant_data const& data) override;
    std::string DoGetValueFromEditor(wxWindow* editor) override;
};

wxWindow* DoubleRangeRenderer::DoCreateEditor
    (wxWindow* parent
     ,wxRect const& rect
     ,tn_range_variant_data const& data)
{
    wxFloatingPointValidator<double> val;
    val.SetRange(data.min, data.max);

    wxTextCtrl* ctrl = new(wx) wxTextCtrl
        (parent
        ,wxID_ANY
        ,data.value
        ,rect.GetTopLeft()
        ,rect.GetSize()
        ,wxTE_PROCESS_ENTER
        ,val);

    // select the text in the control an place the cursor at the end
    // (same as wxDataViewTextRenderer)
    ctrl->SetInsertionPointEnd();
    ctrl->SelectAll();

    return ctrl;
}

std::string DoubleRangeRenderer::DoGetValueFromEditor(wxWindow* editor)
{
    wxTextCtrl* ctrl = dynamic_cast<wxTextCtrl*>(editor);
    LMI_ASSERT(ctrl);

    return ctrl->GetValue().ToStdString();
}

// class DateRenderer

class DateRenderer
    :public RangeTypeRenderer
{
  public:
    DateRenderer() : RangeTypeRenderer() {}
    bool Render(wxRect rect, wxDC* dc, int state) override;

  protected:
    wxWindow* DoCreateEditor(wxWindow* parent, wxRect const& rect, tn_range_variant_data const& data) override;
    std::string DoGetValueFromEditor(wxWindow* editor) override;
};

wxWindow* DateRenderer::DoCreateEditor
    (wxWindow* parent
     ,wxRect const& rect
     ,tn_range_variant_data const& data)
{
    // Always use default height for editor controls
    wxRect r(rect);
    r.height = -1;

    wxDatePickerCtrl* ctrl = new(wx) wxDatePickerCtrl
        (parent
        ,wxID_ANY
        ,ConvertDateToWx(value_cast<calendar_date>(data.value))
        ,r.GetTopLeft()
        ,r.GetSize());

    ctrl->SetRange
        (ConvertDateToWx(jdn_t(static_cast<int>(data.min)))
        ,ConvertDateToWx(jdn_t(static_cast<int>(data.max)))
        );

    return ctrl;
}

bool DateRenderer::Render(wxRect rect, wxDC* dc, int state)
{
    // Use wx for date formatting so that it is identical to the way wxDatePickerCtrl does it.
    wxDateTime const date = ConvertDateToWx(value_cast<calendar_date>(m_value));
    RenderText(date.FormatDate(), 0, rect, dc, state);
    return true;
}

std::string DateRenderer::DoGetValueFromEditor(wxWindow* editor)
{
    wxDatePickerCtrl* ctrl = dynamic_cast<wxDatePickerCtrl*>(editor);
    LMI_ASSERT(ctrl);

    return value_cast<std::string>(ConvertDateFromWx(ctrl->GetValue()));
}

/// Data needed to create UI for input sequences.

struct input_sequence_variant_data
    :public wxVariantData
{
    input_sequence_variant_data(std::string const& value_, Input const* input_, std::string const& field_)
        :value(value_), input(input_), field(field_)
    {
    }

    input_sequence_variant_data(input_sequence_variant_data const&) = delete;
    input_sequence_variant_data& operator=(input_sequence_variant_data const&) = delete;

    bool Eq(wxVariantData& data) const override
    {
        input_sequence_variant_data* d = dynamic_cast<input_sequence_variant_data*>(&data);
        if(!d)
            return false;
        return value == d->value;
    }

    wxString GetType() const override { return typeid(input_sequence_variant_data).name(); }

    wxVariantData* Clone() const override
    {
        return new(wx) input_sequence_variant_data(value, input, field);
    }

    std::string value;
    Input const* input;
    std::string field;
};

class DatumSequenceRenderer
    :public wxDataViewCustomRenderer
{
  public:
    DatumSequenceRenderer();
    DatumSequenceRenderer(DatumSequenceRenderer const&) = delete;
    DatumSequenceRenderer& operator=(DatumSequenceRenderer const&) = delete;
    bool HasEditorCtrl() const override { return true; }
    wxWindow* CreateEditorCtrl(wxWindow* parent, wxRect labelRect, wxVariant const& value) override;
    bool GetValueFromEditorCtrl(wxWindow* editor, wxVariant& value) override;
    bool Render(wxRect rect, wxDC* dc, int state) override;
    wxSize GetSize() const override;
    bool SetValue(wxVariant const& value) override;
    bool GetValue(wxVariant& value) const override;

    std::string  m_value;
    Input const* m_input;
    std::string  m_field;
};

DatumSequenceRenderer::DatumSequenceRenderer()
    :wxDataViewCustomRenderer(typeid(input_sequence_variant_data).name(), wxDATAVIEW_CELL_EDITABLE, wxDVR_DEFAULT_ALIGNMENT)
    ,m_input(nullptr)
{
}

wxWindow* DatumSequenceRenderer::CreateEditorCtrl(wxWindow* parent, wxRect labelRect, wxVariant const& value)
{
    input_sequence_variant_data const* data = dynamic_cast<input_sequence_variant_data*>(value.GetData());
    LMI_ASSERT(data);
    LMI_ASSERT(data->input);

    InputSequenceEntry* ctrl = new(wx) InputSequenceEntry(parent, wxID_ANY, "sequence_editor");

    ctrl->text_ctrl().SetValue(data->value.c_str());
    ctrl->input(*data->input);
    ctrl->field_name(data->field);

    ctrl->SetSize(labelRect);

    return ctrl;
}

bool DatumSequenceRenderer::GetValueFromEditorCtrl(wxWindow* editor, wxVariant& value)
{
    InputSequenceEntry* ctrl = dynamic_cast<InputSequenceEntry*>(editor);
    LMI_ASSERT(ctrl);

    value = new(wx) input_sequence_variant_data
        (ctrl->text_ctrl().GetValue().ToStdString()
        ,&ctrl->input()
        ,ctrl->field_name());
    return true;
}

bool DatumSequenceRenderer::Render(wxRect rect, wxDC* dc, int state)
{
    RenderText(m_value, 0, rect, dc, state);
    return true;
}

wxSize DatumSequenceRenderer::GetSize() const
{
    wxSize sz = GetTextExtent(m_value);

    // Add size of the "..." button. We assume it will use the same font that this renderer
    // uses and add some extra whitespace in addition to InputSequenceButton's 8px padding.
    sz.x += 16 + GetTextExtent("...").x;

    return sz;
}

bool DatumSequenceRenderer::SetValue(wxVariant const& value)
{
    input_sequence_variant_data const* data = dynamic_cast<input_sequence_variant_data*>(value.GetData());
    LMI_ASSERT(data);

    m_value = data->value;
    m_input = data->input;
    m_field = data->field;
    return true;
}

bool DatumSequenceRenderer::GetValue(wxVariant& value) const
{
    value = new(wx) input_sequence_variant_data(m_value, m_input, m_field);
    return true;
}

// This class is used to implement conversion to and from wxVariant for use by
// wxDVC renderers in a single place.

class renderer_type_converter
{
  public:
    virtual wxVariant to_variant(any_member<Input> const& x, Input const& row, std::string const& col) const = 0;
    virtual std::string from_variant(wxVariant const& x) const = 0;
    virtual char const* variant_type() const = 0;
    virtual wxDataViewRenderer* create_renderer(any_member<Input> const& representative_value) const = 0;

    static renderer_type_converter const& get(any_member<Input> const& value);

  private:
    template<typename T>
    static renderer_type_converter const& get_impl();
};

// class renderer_bool_converter

class renderer_bool_converter : public renderer_type_converter
{
    wxVariant to_variant(any_member<Input> const& x, Input const&, std::string const&) const override
    {
        std::string const s(x.str());
        return
              "Yes" == s ? true
            : "No"  == s ? false
            : throw "Invalid boolean value."
            ;
    }

    std::string from_variant(wxVariant const& x) const override
    {
        return x.GetBool() ? "Yes" : "No";
    }

    char const* variant_type() const override
    {
        return "bool";
    }

    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) wxDataViewToggleRenderer("bool", wxDATAVIEW_CELL_ACTIVATABLE, wxALIGN_CENTER);
    }
};

// class renderer_enum_converter

class renderer_enum_converter : public renderer_type_converter
{
    wxVariant to_variant(any_member<Input> const& x, Input const&, std::string const&) const override
    {
        return wxString(x.str());
    }

    std::string from_variant(wxVariant const& x) const override
    {
        return x.GetString().ToStdString();
    }

    char const* variant_type() const override
    {
        return "string";
    }

    wxDataViewRenderer* create_renderer(any_member<Input> const& representative_value) const override
    {
        mc_enum_base const* as_enum = member_cast<mc_enum_base>(representative_value);

        std::vector<std::string> const& all_strings = as_enum->all_strings();
        wxArrayString choices;
        choices.assign(all_strings.begin(), all_strings.end());
        return new(wx) wxDataViewChoiceRenderer(choices, wxDATAVIEW_CELL_EDITABLE);
    }
};

// class renderer_sequence_converter

class renderer_sequence_converter : public renderer_type_converter
{
  public:
    wxVariant to_variant(any_member<Input> const& x, Input const& row, std::string const& col) const override
    {
        return new(wx) input_sequence_variant_data(x.str(), &row, col);
    }

    std::string from_variant(wxVariant const& x) const override
    {
        input_sequence_variant_data const* data = dynamic_cast<input_sequence_variant_data*>(x.GetData());
        LMI_ASSERT(data);
        return data->value;
    }

    char const* variant_type() const override
    {
        return typeid(input_sequence_variant_data).name();
    }

    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) DatumSequenceRenderer();
    }
};

// class renderer_range_converter

class renderer_range_converter : public renderer_type_converter
{
  public:
    wxVariant to_variant(any_member<Input> const& x, Input const&, std::string const&) const override
    {
        tn_range_base const* as_range = member_cast<tn_range_base>(x);
        LMI_ASSERT(as_range);
        return new(wx) tn_range_variant_data(*as_range);
    }

    std::string from_variant(wxVariant const& x) const override
    {
        tn_range_variant_data const* data = dynamic_cast<tn_range_variant_data*>(x.GetData());
        LMI_ASSERT(data);
        return data->value;
    }

    char const* variant_type() const override
    {
        return typeid(tn_range_variant_data).name();
    }
};

class renderer_int_range_converter : public renderer_range_converter
{
  public:
    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) IntSpinRenderer();
    }
};

class renderer_double_range_converter : public renderer_range_converter
{
  public:
    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) DoubleRangeRenderer();
    }
};

class renderer_date_converter : public renderer_range_converter
{
  public:
    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) DateRenderer();
    }
};

// class renderer_fallback_converter

class renderer_fallback_converter : public renderer_type_converter
{
  public:
    wxVariant to_variant(any_member<Input> const& x, Input const&, std::string const&) const override
    {
        // Strings containing new line characters are currently not displayed
        // correctly by wxDataViewCtrl, so display the value on a single line
        // after converting any optional new lines to the Unicode character
        // representing them.
        wxString s(x.str());
        s.Replace("\n", RETURN_SYMBOL, true);
        return s;
    }

    std::string from_variant(wxVariant const& x) const override
    {
        // Undo the replacement done above. Notice that this will (wrongly)
        // translate any RETURN_SYMBOL characters entered by the user into the
        // string to new lines, but this character is not supposed to be used
        // in any of the cells values, so just ignore this problem for now.
        wxString s = x.GetString();
        s.Replace(RETURN_SYMBOL, "\n", true);
        return s.ToStdString();
    }

    char const* variant_type() const override
    {
        return "string";
    }

    wxDataViewRenderer* create_renderer(any_member<Input> const&) const override
    {
        return new(wx) wxDataViewTextRenderer("string", wxDATAVIEW_CELL_EDITABLE);
    }

  private:
    static const wchar_t RETURN_SYMBOL = 0x23ce;
};

renderer_type_converter const& renderer_type_converter::get(any_member<Input> const& value)
{
    if(exact_cast<mce_yes_or_no>(value))
        {
        return get_impl<renderer_bool_converter>();
        }
    else if(exact_cast<datum_string>(value))
        {
        return get_impl<renderer_fallback_converter>();
        }
    else if(is_reconstitutable_as<datum_sequence>(value))
        {
        return get_impl<renderer_sequence_converter>();
        }
    else if(is_reconstitutable_as<mc_enum_base  >(value))
        {
        return get_impl<renderer_enum_converter>();
        }
    else if(is_reconstitutable_as<tn_range_base >(value))
        {
        tn_range_base const* as_range = member_cast<tn_range_base>(value);
        if(typeid(int) == as_range->value_type())
            {
            return get_impl<renderer_int_range_converter>();
            }
        else if(typeid(double) == as_range->value_type())
            {
            return get_impl<renderer_double_range_converter>();
            }
        else if(typeid(calendar_date) == as_range->value_type())
            {
            return get_impl<renderer_date_converter>();
            }
        else
            {
            // Fall through to warn and treat datum as string.
            }
        }
    else
        {
        warning()
            << "Type '"
            << lmi::TypeInfo(value.type())
            << "' not recognized. Please report this anomaly."
            << LMI_FLUSH
            ;
        // Fall through to treat datum as string.
        }

    return get_impl<renderer_fallback_converter>();
}

template<typename T>
renderer_type_converter const& renderer_type_converter::get_impl()
{
    static T singleton;
    return singleton;
}

} // Unnamed namespace.

/// Interface to the data for wxDataViewCtrl.

class CensusViewDataViewModel : public wxDataViewIndexListModel
{
  public:
    static unsigned int const Col_CellNum = 0;

    CensusViewDataViewModel(CensusView& view)
        :view_(view)
    {
    }

    void GetValueByRow(wxVariant& variant, unsigned int row, unsigned int col) const override;
    bool SetValueByRow(wxVariant const&, unsigned int, unsigned int) override;

    unsigned int GetColumnCount() const override;

    wxString GetColumnType(unsigned int col) const override;

    std::string const& col_name(unsigned col) const;
    any_member<Input>& cell_at(unsigned row, unsigned col);
    any_member<Input> const& cell_at(unsigned row, unsigned col) const;

  private:
    std::vector<std::string> const& all_headers() const;

    CensusView& view_;
};

void CensusViewDataViewModel::GetValueByRow(wxVariant& variant, unsigned row, unsigned col) const
{
    if(col == Col_CellNum)
        {
        variant = static_cast<long>(1 + row);
        }
    else
        {
        any_member<Input> const& cell = cell_at(row, col);
        renderer_type_converter const& conv = renderer_type_converter::get(cell);
        Input const& row_data = view_.cell_parms()[row];

        variant = conv.to_variant(cell, row_data, col_name(col));
        }
}

bool CensusViewDataViewModel::SetValueByRow(wxVariant const& variant, unsigned row, unsigned col)
{
    LMI_ASSERT(col != Col_CellNum);

    any_member<Input>& cell = cell_at(row, col);
    renderer_type_converter const& conv = renderer_type_converter::get(cell);

    std::string const prev_val = cell.str();
    std::string new_val = conv.from_variant(variant);

    if(prev_val == new_val)
        return false;

    cell = new_val;

    Input& model = view_.cell_parms()[row];
    model.Reconcile();

    view_.document().Modify(true);

    return true;
}

unsigned int CensusViewDataViewModel::GetColumnCount() const
{
    return all_headers().size() + 1;
}

wxString CensusViewDataViewModel::GetColumnType(unsigned int col) const
{
    if(col == Col_CellNum)
        {
        return "long";
        }
    else
        {
        any_member<Input> const& representative_value = cell_at(0, col);
        renderer_type_converter const& conv = renderer_type_converter::get(representative_value);

        return conv.variant_type();
        }
}

std::string const& CensusViewDataViewModel::col_name(unsigned col) const
{
    LMI_ASSERT(0 < col);
    return all_headers()[col - 1];
}

inline any_member<Input>& CensusViewDataViewModel::cell_at(unsigned row, unsigned col)
{
    return view_.cell_parms()[row][col_name(col)];
}

inline any_member<Input> const& CensusViewDataViewModel::cell_at(unsigned row, unsigned col) const
{
    return view_.cell_parms()[row][col_name(col)];
}

inline std::vector<std::string> const& CensusViewDataViewModel::all_headers() const
{
    return view_.case_parms()[0].member_names();
}

// class CensusView

IMPLEMENT_DYNAMIC_CLASS(CensusView, ViewEx)

BEGIN_EVENT_TABLE(CensusView, ViewEx)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY     ,CensusView::UponRightClick             )
    EVT_MENU(XRCID("edit_cell"                 ),CensusView::UponEditCell               )
    EVT_MENU(XRCID("edit_class"                ),CensusView::UponEditClass              )
    EVT_MENU(XRCID("edit_case"                 ),CensusView::UponEditCase               )
    EVT_MENU(XRCID("run_cell"                  ),CensusView::UponRunCell                )
//  EVT_MENU(XRCID("run_class"                 ),CensusView::UponRunClass               )   // SOMEDAY !! This may be useful for large cases.
    EVT_MENU(XRCID("run_case"                  ),CensusView::UponRunCase                )
    EVT_MENU(XRCID("print_case"                ),CensusView::UponPrintCase              )
    EVT_MENU(XRCID("print_case_to_disk"        ),CensusView::UponPrintCaseToDisk        )
    EVT_MENU(XRCID("print_spreadsheet"         ),CensusView::UponRunCaseToSpreadsheet   )
    EVT_MENU(XRCID("print_group_roster"        ),CensusView::UponRunCaseToGroupRoster   )
    EVT_MENU(XRCID("paste_census"              ),CensusView::UponPasteCensus            )
    EVT_MENU(XRCID("add_cell"                  ),CensusView::UponAddCell                )
    EVT_MENU(XRCID("delete_cells"              ),CensusView::UponDeleteCells            )
    EVT_MENU(XRCID("column_width_varying"      ),CensusView::UponColumnWidthVarying     )
    EVT_MENU(XRCID("column_width_fixed"        ),CensusView::UponColumnWidthFixed       )
    EVT_UPDATE_UI(XRCID("edit_cell"            ),CensusView::UponUpdateSingleSelection  )
    EVT_UPDATE_UI(XRCID("edit_class"           ),CensusView::UponUpdateSingleSelection  )
    EVT_UPDATE_UI(XRCID("edit_case"            ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("run_cell"             ),CensusView::UponUpdateSingleSelection  )
    EVT_UPDATE_UI(XRCID("run_class"            ),CensusView::UponUpdateSingleSelection  )
    EVT_UPDATE_UI(XRCID("run_case"             ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("print_case"           ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("print_group_roster"   ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("paste_census"         ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("add_cell"             ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("delete_cells"         ),CensusView::UponUpdateNonemptySelection)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),CensusView::UponUpdateAlwaysEnabled    )
    // Disable these printing commands on the "File" menu: specialized
    // printing commands are offered on the "Census" menu.
    EVT_UPDATE_UI(wxID_PRINT                    ,CensusView::UponUpdateAlwaysDisabled   )
    EVT_UPDATE_UI(wxID_PREVIEW                  ,CensusView::UponUpdateAlwaysDisabled   )
    EVT_UPDATE_UI(wxID_PAGE_SETUP               ,CensusView::UponUpdateAlwaysDisabled   )
    EVT_UPDATE_UI(XRCID("print_pdf"            ),CensusView::UponUpdateAlwaysDisabled   )
END_EVENT_TABLE()

CensusView::CensusView()
    :ViewEx                          ()
    ,autosize_columns_               (false)
    ,list_window_                    (nullptr)
    ,list_model_                     (new(wx) CensusViewDataViewModel(*this))
{
}

inline std::vector<Input>& CensusView::case_parms()
{
    return document().doc_.case_parms_;
}

inline std::vector<Input> const& CensusView::case_parms() const
{
    return document().doc_.case_parms_;
}

inline std::vector<Input>& CensusView::cell_parms()
{
    return document().doc_.cell_parms_;
}

inline std::vector<Input> const& CensusView::cell_parms() const
{
    return document().doc_.cell_parms_;
}

inline std::vector<Input>& CensusView::class_parms()
{
    return document().doc_.class_parms_;
}

inline std::vector<Input> const& CensusView::class_parms() const
{
    return document().doc_.class_parms_;
}

std::string CensusView::cell_title(int index)
{
    std::string full_name(cell_parms()[index]["InsuredName"].str());
    std::ostringstream title;
    title << "Parameters for cell " << (1 + index);
    if(!full_name.empty())
        {
        title << " (" << full_name << ")";
        }
    return title.str();
}

std::string CensusView::class_title(int index)
{
    std::string class_name = class_name_from_cell_number(index);

    std::ostringstream title;
    title << "Default parameters for employee class ";
    if(class_name.empty())
        {
        title << "[unnamed]";
        }
    else
        {
        title << "'" << class_name << "'";
        }
    return title.str();
}

std::string CensusView::class_name_from_cell_number(int cell_number) const
{
    return cell_parms()[cell_number]["EmployeeClass"].str();
}

Input* CensusView::class_parms_from_class_name(std::string const& class_name)
{
    std::vector<Input>::iterator i = class_parms().begin();
    while(i != class_parms().end())
        {
        if(class_name == (*i)["EmployeeClass"].str())
            {
            return &*i;
            }
        ++i;
        }
    return nullptr;
}

/// Determine which columns need to be displayed because their rows
/// would not all be identical--i.e. because at least one cell or one
/// class default differs from the case default wrt that column.

bool CensusView::column_value_varies_across_cells
    (std::string        const& header
    ,std::vector<Input> const& cells
    ) const
{
    for(auto const& j : cells)
        {
        if(!(j[header] == case_parms()[0][header]))
            {
            return true;
            }
        }
    return false;
}

wxWindow* CensusView::CreateChildWindow()
{
    list_window_ = new(wx) wxDataViewCtrl
        (GetFrame()
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxDV_ROW_LINES | wxDV_MULTIPLE
        );

    list_window_->AssociateModel(list_model_.get());

    // Show headers.
    document().Modify(false);
    list_model_->Reset(cell_parms().size());
    Update();

    list_window_->Select(list_model_->GetItem(0));

    status() << std::flush;

    return list_window_;
}

CensusDocument& CensusView::document() const
{
    return safely_dereference_as<CensusDocument>(GetDocument());
}

oenum_mvc_dv_rc CensusView::edit_parameters
    (Input&             parameters
    ,std::string const& title
    )
{
    return edit_mvc_docview_parameters<DefaultView>
        (parameters
        ,document()
        ,GetFrame()
        ,title
        );
}

int CensusView::selected_row()
{
    int row = list_model_->GetRow(list_window_->GetSelection());
    LMI_ASSERT(0 <= row && static_cast<unsigned int>(row) < list_model_->GetCount());
    return row;
}

/// Make a vector of all class names used by any individual, from
/// scratch; and update the vector of class default parameters,
/// adding any new classes, and purging any that are no longer in use
/// by any cell.

void CensusView::update_class_names()
{
    // Extract names and add them even if they might be duplicates.
    std::vector<std::string> all_class_names;

    for(auto const& i : cell_parms())
        {
        all_class_names.push_back(i["EmployeeClass"].str());
        }

    std::vector<std::string> unique_class_names;

    std::insert_iterator<std::vector<std::string>> iin
        (unique_class_names
        ,unique_class_names.begin()
        );
    std::sort(all_class_names.begin(), all_class_names.end());
    std::unique_copy(all_class_names.begin(), all_class_names.end(), iin);

    // Rebuild vector of class parameters so that it contains
    // an element for each class in use.
    std::vector<Input> rebuilt_class_parms;
    std::vector<std::string>::iterator n = unique_class_names.begin();
    while(n != unique_class_names.end())
        {
        Input* parms = class_parms_from_class_name(*n);
        if(nullptr != parms)
            {
            // If we already have default parameters for the class,
            // insert them into the rebuilt vector.
            rebuilt_class_parms.push_back(*parms);
            }
        else
            {
            // If we do not already have default parameters for the class,
            // find the first individual that belongs to the class and
            // insert its parameters into the rebuilt vector.
            std::vector<Input>::const_iterator j = cell_parms().begin();
            bool found = false;
            while(j != cell_parms().end())
                {
                if(*n == (*j)["EmployeeClass"].str())
                    {
                    rebuilt_class_parms.push_back(*j);
                    found = true;
                    break;
                    }
                ++j;
                }
            // It should not be possible for no cell to be found in the class.
            if(!found)
                {
                alarum()
                    << "Cannot find any cell in class "
                    << "'" << *n << "'."
                    << LMI_FLUSH
                    ;
                }
            }
        ++n;
        }

    // Replace the vector of class parameters with the one we rebuilt.
    class_parms().clear();
    std::insert_iterator<std::vector<Input>> iip(class_parms(), class_parms().begin());
    std::copy(rebuilt_class_parms.begin(), rebuilt_class_parms.end(), iip);
}

/// Ascertain differences between old and new parameters and apply
/// each such difference to other cells:
///   if 'for_this_class_only' is specified, to all cells in the
///     employee class of the old parameters;
///   otherwise, to all cells in the entire census.

void CensusView::apply_changes
    (Input const& new_parms
    ,Input const& old_parms
    ,bool         for_this_class_only
    )
{
    wxBusyCursor wait;

    // Case or class default parameters were edited and changed.
    // Compare the default parameters before and after editing;
    // for every parameter that was changed, assign the new value
    // to all applicable cells, i.e.
    //   if case  defaults changed: all cells and all class defaults;
    //   if class defaults changed: all cells in the class.

    std::vector<std::string> headers_of_changed_parameters;
    std::vector<std::string> const& all_headers(case_parms()[0].member_names());
    for(auto const& i : all_headers)
        {
        if(!(old_parms[i] == new_parms[i]))
            {
            headers_of_changed_parameters.push_back(i);
            }
        }
    for(auto const& i : headers_of_changed_parameters)
        {
        if(!for_this_class_only)
            {
            for(auto& j : class_parms())
                {
                j[i] = new_parms[i].str();
                }
            for(auto& j : cell_parms())
                {
                j[i] = new_parms[i].str();
                }
            }
        else
            {
            for(auto& j : cell_parms())
                {
                if(j["EmployeeClass"] == old_parms["EmployeeClass"])
                    {
                    j[i] = new_parms[i].str();
                    }
                }
            }
        }

    // Probably this should be factored out into a member function
    // that's called elsewhere too--e.g., when a cell is read from
    // file, or when a census is pasted.
    for(auto& j : class_parms())
        {
        j.Reconcile();
        }
    for(auto& j : cell_parms())
        {
        j.Reconcile();
        }
}

void CensusView::update_visible_columns()
{
    int width = autosize_columns_ ? wxCOL_WIDTH_AUTOSIZE : wxCOL_WIDTH_DEFAULT;

    list_window_->ClearColumns();

    // Column zero (cell serial number) is always shown.
    list_window_->AppendColumn
        (new(wx) wxDataViewColumn
            ("Cell"
            ,new(wx) wxDataViewTextRenderer("long", wxDATAVIEW_CELL_INERT)
            ,CensusViewDataViewModel::Col_CellNum
            ,width
            ,wxALIGN_LEFT
            ,wxDATAVIEW_COL_RESIZABLE
            )
        );

    // Display exactly those columns whose rows aren't all identical. For
    // this purpose, consider as "rows" the individual cells--and also the
    // case and class defaults, even though they aren't displayed in rows.
    // Reason: although the case and class defaults are hidden, they're
    // still information--so if the user made them different from any cell
    // wrt some column, we respect that conscious decision.
    std::vector<std::string> const& all_headers(case_parms()[0].member_names());
    unsigned int column = 0;
    for(auto const& i : all_headers)
        {
        ++column;
        if
            (  column_value_varies_across_cells(i, class_parms())
            || column_value_varies_across_cells(i, cell_parms ())
            )
            {
            any_member<Input> const& representative_value = list_model_->cell_at(0, column);

            wxDataViewRenderer* renderer = renderer_type_converter::get(representative_value).create_renderer(representative_value);
            LMI_ASSERT(renderer);

            list_window_->AppendColumn
                (new(wx) wxDataViewColumn
                    (insert_spaces_between_words(i)
                    ,renderer
                    ,column
                    ,width
                    ,wxALIGN_LEFT
                    ,wxDATAVIEW_COL_RESIZABLE
                    )
                );
            }
        }
}

char const* CensusView::icon_xrc_resource() const
{
    return "census_view_icon";
}

char const* CensusView::menubar_xrc_resource() const
{
    return "census_view_menu";
}

void CensusView::UponEditCell(wxCommandEvent&)
{
    int cell_number = selected_row();
    Input& modifiable_parms = cell_parms()[cell_number];
    std::string const title = cell_title(cell_number);

    if(oe_mvc_dv_changed == edit_parameters(modifiable_parms, title))
        {
        Update();
        document().Modify(true);
        }
}

void CensusView::UponEditClass(wxCommandEvent&)
{
    int cell_number = selected_row();
    std::string class_name = class_name_from_cell_number(cell_number);
    Input& modifiable_parms = *class_parms_from_class_name(class_name);
    Input const unmodified_parms(modifiable_parms);
    std::string const title = class_title(cell_number);

    if(oe_mvc_dv_changed == edit_parameters(modifiable_parms, title))
        {
        int z = wxMessageBox
            ("Apply all changes to every cell in this class?"
            ,"Confirm changes"
            ,wxYES_NO | wxICON_QUESTION
            );
        if(wxYES == z)
            {
            apply_changes(modifiable_parms, unmodified_parms, true);
            }
        Update();
        document().Modify(true);
        }
}

void CensusView::UponEditCase(wxCommandEvent&)
{
    Input& modifiable_parms = case_parms()[0];
    Input const unmodified_parms(modifiable_parms);
    std::string const title = "Default parameters for case";

    if(oe_mvc_dv_changed == edit_parameters(modifiable_parms, title))
        {
        int z = wxMessageBox
            ("Apply all changes to every cell?"
            ,"Confirm changes"
            ,wxYES_NO | wxICON_QUESTION
            );
        if(wxYES == z)
            {
            apply_changes(modifiable_parms, unmodified_parms, false);
            }
        Update();
        document().Modify(true);
        }
}

/// Make each nonfrozen column wide enough to display its widest entry,
/// ignoring column headers.

void CensusView::UponColumnWidthVarying(wxCommandEvent&)
{
    autosize_columns_ = true;

    wxWindowUpdateLocker u(list_window_);
    for(unsigned int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        list_window_->GetColumn(j)->SetWidth(wxCOL_WIDTH_AUTOSIZE);
        }
    Update();
}

/// Shrink all nonfrozen columns to default width.

void CensusView::UponColumnWidthFixed(wxCommandEvent&)
{
    autosize_columns_ = false;

    wxWindowUpdateLocker u(list_window_);
    for(unsigned int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        list_window_->GetColumn(j)->SetWidth(wxCOL_WIDTH_DEFAULT);
        }
    Update();
}

void CensusView::UponRightClick(wxDataViewEvent& e)
{
    if(e.GetEventObject() != list_window_)
        {
        // This event should come only from the window pointed to by
        // list_window_. Ignore it if it happens to come elsewhence.
        e.Skip();
        return;
        }

    wxMenu* census_menu = wxXmlResource::Get()->LoadMenu("census_menu_ref");
    LMI_ASSERT(census_menu);
    list_window_->PopupMenu(census_menu);
    delete census_menu;
}

void CensusView::UponUpdateAlwaysDisabled(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

void CensusView::UponUpdateAlwaysEnabled(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void CensusView::UponUpdateSingleSelection(wxUpdateUIEvent& e)
{
    bool const is_single_sel = list_window_->GetSelection().IsOk();
    e.Enable(is_single_sel);
}

void CensusView::UponUpdateNonemptySelection(wxUpdateUIEvent& e)
{
    wxDataViewItemArray selection;
    unsigned int n_sel_items = list_window_->GetSelections(selection);
    e.Enable(0 < n_sel_items);
}

/// Update the dataview display.
///
/// If a parameter was formerly the same for all cells but now differs due
///  to editing, then display its column for all cells.
/// If a column was previously displayed but is now the same for all cells
///  due to editing, then display it no longer.
/// Similarly, if an old employee class is no longer used, remove it; and
///  if a new one comes into use, display it.

void CensusView::Update()
{
    LMI_ASSERT(list_model_->GetCount() == cell_parms().size());

    wxWindowUpdateLocker u(list_window_);

    update_class_names();
    update_visible_columns();
}

void CensusView::UponPrintCase(wxCommandEvent&)
{
    DoAllCells(mce_emit_pdf_to_printer);
}

void CensusView::UponPrintCaseToDisk(wxCommandEvent&)
{
    DoAllCells(mce_emit_pdf_file);
}

void CensusView::UponRunCase(wxCommandEvent&)
{
    ViewComposite();
}

void CensusView::UponRunCell(wxCommandEvent&)
{
    int cell_number = selected_row();
    ViewOneCell(cell_number);
}

void CensusView::ViewOneCell(int index)
{
    std::string const name(cell_parms()[index]["InsuredName"].str());
    IllustrationView& illview = MakeNewIllustrationDocAndView
        (document().GetDocumentManager()
        ,serial_file_path(base_filename(), name, index, "ill").string().c_str()
        );
    illview.Run(&cell_parms()[index]);
}

void CensusView::ViewComposite()
{
    if(!DoAllCells(mce_emit_nothing))
        {
        return;
        }

    std::string const name("composite");
    IllustrationView& illview = MakeNewIllustrationDocAndView
        (document().GetDocumentManager()
        ,serial_file_path(base_filename(), name, -1, "ill").string().c_str()
        );

    // This is necessary for the view to be able to print.
    illview.SetLedger(composite_ledger_);

    illview.DisplaySelectedValuesAsHtml();
}

bool CensusView::DoAllCells(mcenum_emission emission)
{
    test_census_consensus(emission, case_parms()[0], cell_parms());

    illustrator z(emission);
    if(!z(base_filename(), cell_parms()))
        {
        // Cancelled during run_census::operator().
        return false;
        }

    composite_ledger_ = z.principal_ledger();
    return true;
}

void CensusView::UponAddCell(wxCommandEvent&)
{
    wxBusyCursor wait;
    Timer timer;

    cell_parms().push_back(case_parms()[0]);
    list_model_->RowAppended();

    Update();
    document().Modify(true);

    wxDataViewItem const& z = list_model_->GetItem(list_model_->GetCount() - 1);
    list_window_->UnselectAll();
    list_window_->Select(z);
    list_window_->EnsureVisible(z);

    double total_seconds = timer.stop().elapsed_seconds();
    status() << Timer::elapsed_msec_str(total_seconds) << std::flush;
}

void CensusView::UponDeleteCells(wxCommandEvent&)
{
    unsigned int n_items = list_model_->GetCount();
    wxDataViewItemArray selection;
    unsigned int n_sel_items = list_window_->GetSelections(selection);
    LMI_ASSERT(n_sel_items == selection.size());
    // This handler should have been disabled if no cell is selected.
    LMI_ASSERT(0 < n_sel_items);

    if(n_items == n_sel_items)
        {
        warning()
            << "Cannot delete all cells. A census must always"
            << " contain at least one cell."
            << LMI_FLUSH
            ;
        return;
        }

    LMI_ASSERT(n_sel_items < n_items);

    std::ostringstream oss;
    oss
        << "Irrevocably delete "
        << n_sel_items
        << " of "
        << n_items
        << " cells?"
        ;
    int yes_or_no = wxMessageBox
        (oss.str()
        ,"Confirm deletion"
        ,wxYES_NO | wxICON_QUESTION
        );
    if(wxYES != yes_or_no)
        {
        return;
        }

    wxBusyCursor wait;
    Timer timer;

    wxArrayInt erasures;
    for(auto const& i : selection)
        {
        erasures.push_back(list_model_->GetRow(i));
        }
    std::sort(erasures.begin(), erasures.end());

    LMI_ASSERT(cell_parms().size() == n_items);

    for(int j = erasures.size() - 1; 0 <= j; --j)
        {
        cell_parms().erase(erasures[j] + cell_parms().begin());
        }
    LMI_ASSERT(cell_parms().size() == n_items - n_sel_items);

#if !wxCHECK_VERSION(2,9,3)
    // Remove selection to work around wx-2.9.2 bug in GetSelections()
    // (we'll set it again below).
    list_window_->UnselectAll();
#endif

    // Send notifications about changes to the wxDataViewCtrl model. Two things
    // changed: some rows were deleted and cell number of some rows shifted
    // accordingly.
    list_model_->RowsDeleted(erasures);
    for(unsigned int j = erasures.front(); j < cell_parms().size(); ++j)
        {
        list_model_->RowValueChanged(j, CensusViewDataViewModel::Col_CellNum);
        }

    unsigned int const newsel = std::min
        (static_cast<std::size_t>(erasures.front())
        ,cell_parms().size() - 1
        );
    wxDataViewItem const& z = list_model_->GetItem(newsel);
    list_window_->Select(z);
    list_window_->EnsureVisible(z);

    Update();
    document().Modify(true);

    double total_seconds = timer.stop().elapsed_seconds();
    status() << Timer::elapsed_msec_str(total_seconds) << std::flush;
}

/// Print tab-delimited details to file loadable in spreadsheet programs.

void CensusView::UponRunCaseToSpreadsheet(wxCommandEvent&)
{
    DoAllCells(mce_emit_spreadsheet);
}

/// Print group roster in an interactively-chosen format.

void CensusView::UponRunCaseToGroupRoster(wxCommandEvent&)
{
    wxArrayString strings;
    // Make sure ampersands don't look like references, e.g., in a
    // string such as "premium quote" with an ampersand preceding 'q',
    // which 'make check_concinnity' rejects.
    strings.Add("Print r&oster to spreadsheet");
    strings.Add("Print group premium ""&quote to PDF");
    int const selection = SingleChoicePopupMenu(strings).Choose();
    switch(selection)
        {
        case -1:
            {
            // No selection: do nothing; let the popup just vanish.
            }
            break;
        case 0:
            {
            // Print tab-delimited roster to file loadable in spreadsheet programs.
            DoAllCells(mce_emit_group_roster);
            }
            break;
        case 1:
            {
            // Print group quote to a PDF file.
            DoAllCells(mce_emit_group_quote);
            }
            break;
        default:
            {
            alarum() << "Case " << selection << " not found." << LMI_FLUSH;
            }
        }
}

/// Paste a census from the clipboard.
///
/// See unit tests in Skeleton::UponTestPasting().
///
/// A newly-created census contains one default cell, which doesn't
/// represent user input, so it is erased before pasting new cells
/// from the clipboard. The clipboard contents are validated before
/// this erasure, so the document is not changed if pasting failed.
///
/// But if the census contains any user input, it shouldn't be erased.
/// User input is present if either the document has been modified
/// (e.g., if the default cell in a new census has been changed) or
/// the document was loaded from a file (even if it was saved with
/// only an unmodified default cell, because the contents of any saved
/// file are assumed to represent user intention). In this case,
/// pasted data is appended to the cells that were already present.

void CensusView::UponPasteCensus(wxCommandEvent&)
{
    std::string const census_data = ClipboardEx::GetText();

    std::vector<std::string> headers;
    std::vector<Input> cells;

    std::istringstream iss_census(census_data);
    iss_census.imbue(tab_is_not_whitespace_locale());
    std::string line;

    // Get header line; parse into field names.
    if(std::getline(iss_census, line, '\n'))
        {
        iss_census >> std::ws;

        std::istringstream iss_line(line);
        std::string token;

        while(std::getline(iss_line, token, '\t'))
            {
            headers.push_back(token);
            }
        }
    else
        {
        warning() << "Error pasting census data: no header line." << LMI_FLUSH;
        return;
        }

    // Use a modifiable copy of case defaults as an exemplar for new
    // cells to be created by pasting. Modifications are conditionally
    // written back to case defaults later.
    Input exemplar(case_parms()[0]);

    // Force 'UseDOB' prn. Pasting it as a column never makes sense.
    if(contains(headers, "UseDOB"))
        {
        warning() << "'UseDOB' is unnecessary and will be ignored." << std::flush;
        }
    bool const dob_pasted = contains(headers, "DateOfBirth");
    bool const age_pasted = contains(headers, "IssueAge");
    if(dob_pasted && age_pasted)
        {
        alarum()
            << "Cannot paste both 'DateOfBirth' and 'IssueAge'."
            << LMI_FLUSH
            ;
        }
    else if(dob_pasted)
        {
        exemplar["UseDOB"] = "Yes";
        }
    else if(age_pasted)
        {
        exemplar["UseDOB"] = "No";
        }
    else
        {
        ; // Do nothing: neither age nor DOB pasted.
        }

    // Read each subsequent line into an input object representing one cell.
    int current_line = 0;
    while(std::getline(iss_census, line, '\n'))
        {
        ++current_line;

        iss_census >> std::ws;

        Input current_cell(exemplar);

        std::istringstream iss_line(line);
        std::string token;
        std::vector<std::string> values;

        while(std::getline(iss_line, token, '\t'))
            {
#if 0 // expunge soon?
            static std::string const space(" ");
            if(std::string::npos == token.find_first_not_of(space))
                {
                warning()
                    << "Line #" << current_line << ": "
                    << " (" << line << ") "
                    << "has a value that contains no non-blank characters. "
                    << "Last valid value, if any: " << values.back()
                    << LMI_FLUSH
                    ;
// TODO ?? It would be better to use alarum() instead of
// warning() followed by alarum() with a short string, but
// apparently that can segfault with very long strings. Is there
// a limit on exception size that should be tested here? See:
//   http://savannah.nongnu.org/bugs/?20240
                alarum() << "Invalid input." << LMI_FLUSH;
                }
#endif // 0
            values.push_back(token);
            }

        if(values.size() != headers.size())
            {
            alarum()
                << "Line #" << current_line << ": "
                << "  (" << line << ") "
                << "should have one value per column. "
                << "Number of values: " << values.size() << "; "
                << "number expected: " << headers.size() << "."
                << LMI_FLUSH
                ;
            }

        for(unsigned int j = 0; j < headers.size(); ++j)
            {
            if(exact_cast<tnr_date>(current_cell[headers[j]]))
                {
                static long int const jdn_min = calendar_date::gregorian_epoch_jdn;
                static long int const jdn_max = calendar_date::last_yyyy_date_jdn;
                static long int const ymd_min = JdnToYmd(jdn_t(jdn_min)).value();
                static long int const ymd_max = JdnToYmd(jdn_t(jdn_max)).value();
                long int z = value_cast<long int>(values[j]);
                if(jdn_min <= z && z <= jdn_max)
                    {
                    ; // Do nothing: JDN is the default expectation.
                    }
                else if(ymd_min <= z && z <= ymd_max)
                    {
                    z = YmdToJdn(ymd_t(z)).value();
                    values[j] = value_cast<std::string>(z);
                    }
                else
                    {
                    alarum()
                        << "Invalid date " << values[j]
                        << " for '" << headers[j] << "'"
                        << " on line " << current_line << "."
                        << LMI_FLUSH
                        ;
                    }
                }
            current_cell[headers[j]] = values[j];
            }
        current_cell.Reconcile();
        current_cell.RealizeAllSequenceInput();
        cells.push_back(current_cell);

        status() << "Added cell number " << cells.size() << '.' << std::flush;
        wxSafeYield();
        }

    if(0 == current_line)
        {
        warning() << "No cells to paste." << LMI_FLUSH;
        return;
        }

    if(!document().IsModified() && !document().GetDocumentSaved())
        {
        case_parms ().clear();
        case_parms ().push_back(exemplar);
        class_parms().clear();
        class_parms().push_back(exemplar);
        cell_parms ().clear();
        }

    auto selection = cell_parms().size();
    std::back_insert_iterator<std::vector<Input>> iip(cell_parms());
    std::copy(cells.begin(), cells.end(), iip);
    document().Modify(true);
    list_model_->Reset(cell_parms().size());
    Update();
    // Reset() leaves the listview unreachable from the keyboard
    // because no row is selected--so select the first added row
    // if possible, else the row after which no row was inserted.
    selection = std::min(selection, cell_parms().size());
    wxDataViewItem const& z = list_model_->GetItem(selection);
    list_window_->Select(z);
    list_window_->EnsureVisible(z);

    status() << std::flush;

    LMI_ASSERT(1 == case_parms().size());
    LMI_ASSERT(!cell_parms ().empty());
    LMI_ASSERT(!class_parms().empty());
}

