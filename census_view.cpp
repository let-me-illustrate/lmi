// Census manager.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019 Gregory W. Chicares.
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
#include "bourn_cast.hpp"
#include "census_document.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "default_view.hpp"
#include "edit_mvc_docview_parameters.hpp"
#include "facets.hpp"                   // tab_is_not_whitespace_locale()
#include "global_settings.hpp"
#include "illustration_view.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "input_sequence_entry.hpp"
#include "istream_to_string.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp"               // is_ok_for_cctype(), ios_out_app_binary()
#include "path_utility.hpp"             // unique_filepath()
#include "rtti_lmi.hpp"                 // lmi::TypeInfo
#include "safely_dereference_as.hpp"
#include "ssize_lmi.hpp"
#include "timer.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"               // class ClipboardEx

#include <boost/filesystem/convenience.hpp> // basename()

#include <wx/datectrl.h>
#include <wx/headercol.h>               // wxCOL_WIDTH_DEFAULT
#include <wx/headerctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/utils.h>                   // wxBusyCursor
#include <wx/valnum.h>
#include <wx/wupdlock.h>                // wxWindowUpdateLocker
#include <wx/xrc/xmlres.h>

#include <algorithm>
#include <cctype>                       // isupper()
#include <fstream>
#include <istream>                      // ws
#include <iterator>                     // insert_iterator
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

// Declare the functions here, but implement after the CensusViewTable
// declaration to avoid the "use of undefined type" error.

// Get the cell value from the table.

any_member<Input> const& cell_at(wxGridTableBase const* table, int row, int col);

// Get the raw from the table.

Input const& row_at(wxGridTableBase const* table, int row);

// Get the column name from the table.

std::string const& col_name(wxGridTableBase const* table, int col);

// class RangeIntEditor

class RangeIntEditor
    :public wxGridCellNumberEditor
{
  public:
    // Construct wxGridCellNumberEditor with fake limits to use
    // the wxSpinCtrl. Real limits will be set in BeginEdit function.
    RangeIntEditor()
        :wxGridCellNumberEditor {0, 1}
    {
        LMI_ASSERT(HasRange());
    }

    void BeginEdit(int row, int col, wxGrid* grid) override
    {
        auto const& value{cell_at(grid->GetTable(), row, col)};
        auto const* as_range{member_cast<tn_range_base>(value)};
        LMI_ASSERT(typeid(int) == as_range->value_type());

        Spin()->SetRange
            (bourn_cast<int>(as_range->universal_minimum())
            ,bourn_cast<int>(as_range->universal_maximum())
            );

        wxGridCellNumberEditor::BeginEdit(row, col, grid);
    }
};

// class RangeDoubleEditor

class RangeDoubleEditor
    :public wxGridCellTextEditor
{
  public:
    RangeDoubleEditor() = default;

    void BeginEdit(int row, int col, wxGrid* grid) override
    {
        auto const& value{cell_at(grid->GetTable(), row, col)};
        auto const* as_range{member_cast<tn_range_base>(value)};
        LMI_ASSERT(typeid(double) == as_range->value_type());

        wxFloatingPointValidator<double> val;
        val.SetRange(as_range->universal_minimum(), as_range->universal_maximum());
        SetValidator(val);

        wxGridCellTextEditor::BeginEdit(row, col, grid);
    }
};

// class RangeDateEditor

class RangeDateEditor
    :public wxGridCellDateEditor
{
  public:
    RangeDateEditor() = default;

    void BeginEdit(int row, int col, wxGrid* grid) override
    {
        auto const& value{cell_at(grid->GetTable(), row, col)};
        auto const* as_range{member_cast<tn_range_base>(value)};
        LMI_ASSERT(typeid(calendar_date) == as_range->value_type());

        auto const min_value{as_range->universal_minimum()};
        auto const max_value{as_range->universal_maximum()};

        DatePicker()->SetRange
            (ConvertDateToWx(jdn_t(bourn_cast<int>(min_value)))
            ,ConvertDateToWx(jdn_t(bourn_cast<int>(max_value)))
            );

        wxGridCellDateEditor::BeginEdit(row, col, grid);
    }
};

class DatumSequenceEditor
    :public wxGridCellEditor
{
  public:
    DatumSequenceEditor() = default;
    DatumSequenceEditor(DatumSequenceEditor const&) = delete;
    DatumSequenceEditor& operator=(DatumSequenceEditor const&) = delete;

    void Create(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler) override;

    void BeginEdit(int row, int col, wxGrid* grid) override;
    bool EndEdit
        (int row
        ,int col
        ,wxGrid const* grid
        ,wxString const& oldval
        ,wxString* newval
        ) override;
    void ApplyEdit(int row, int col, wxGrid* grid) override;

    void Reset() override;

    wxGridCellEditor* Clone() const override;

    wxString GetValue() const override;

  private:
    InputSequenceEntry* Entry() const;

    wxString     value_;
    Input const* input_;
    std::string  field_;
};

void DatumSequenceEditor::Create
    (wxWindow*     parent
    ,wxWindowID    id
    ,wxEvtHandler* evtHandler
    )
{
    m_control = new(wx) InputSequenceEntry(parent, id, "sequence_editor");

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void DatumSequenceEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    auto table{grid->GetTable()};
    value_ = table->GetValue(row, col);
    input_ = &row_at(table, row);
    field_ = col_name(table, col);

    auto entry{Entry()};

    entry->text_ctrl().SetValue(value_);
    entry->input(*input_);
    entry->field_name(field_);
}

bool DatumSequenceEditor::EndEdit
    (int
    ,int
    ,wxGrid const*
    ,wxString const&
    ,wxString*       newval
    )
{
    auto value{Entry()->text_ctrl().GetValue()};

    if(value == value_)
        {
        return false;
        }

    value_ = value;

    if(newval)
        {
        *newval = GetValue();
        }

    return true;
}

void DatumSequenceEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, GetValue());
}

void DatumSequenceEditor::Reset()
{
    Entry()->text_ctrl().SetValue(value_);
}

wxGridCellEditor* DatumSequenceEditor::Clone() const
{
    auto editor{new(wx) DatumSequenceEditor()};
    editor->value_ = value_;
    editor->input_ = input_;
    editor->field_ = field_;

    return editor;
}

wxString DatumSequenceEditor::GetValue() const
{
    return Entry()->text_ctrl().GetValue();
}

InputSequenceEntry* DatumSequenceEditor::Entry() const
{
    auto entry{dynamic_cast<InputSequenceEntry*>(m_control)};
    LMI_ASSERT(entry);
    return entry;
}

// This class is used to implement conversion to and from cells string for use
// by CensusViewTable in a single place.

class renderer_type_converter
{
  public:
    virtual ~renderer_type_converter() = default;

    virtual wxString to_renderer_value(std::string const& value) const
    {
        return value;
    }

    virtual std::string from_editor_value(wxString const& value) const
    {
        return value.ToStdString();
    }

    // Returns the name of derived type.
    virtual char const* type() const = 0;

    // Returns the grid value type, used by wxGridTypeRegistry.
    virtual wxString grid_value_type(any_member<Input> const&) const
    {
        return type();
    }

    virtual void register_data_type(wxGrid* grid) const
    {
        grid->RegisterDataType
            (type()
            ,create_renderer()
            ,create_editor()
            );
    }

    virtual wxGridCellRenderer* create_renderer() const
    {
        return nullptr;
    }

    virtual wxGridCellEditor* create_editor() const
    {
        return nullptr;
    }

    static std::map<std::string, renderer_type_converter const*>& get_all();
    static renderer_type_converter const& get_by_value(any_member<Input> const& value);

  private:
    template<typename T>
    static renderer_type_converter const& get_impl();
};

// class table_bool_converter

class table_bool_converter : public renderer_type_converter
{
  public:
    table_bool_converter()
    {
        wxGridCellBoolEditor::UseStringValues("Yes", "No");
    }

    char const* type() const override
    {
        return typeid(table_bool_converter).name();
    }

    wxString grid_value_type(any_member<Input> const&) const override
    {
        return wxGRID_VALUE_BOOL;
    }

    void register_data_type(wxGrid*) const
    {
        // Do not register the data type because we use standard value type.
    }
};

// class table_enum_converter

class table_enum_converter : public renderer_type_converter
{
  public:
    char const* type() const override
    {
        return typeid(table_enum_converter).name();
    }

    wxString grid_value_type(any_member<Input> const& value) const override
    {
        wxString type{wxGRID_VALUE_CHOICE};
        auto const* as_enum = member_cast<mc_enum_base>(value);
        auto const& all_strings = as_enum->all_strings();

        bool first = true;
        for(auto const& s : all_strings)
            {
            type.append(first ? ':' : ',');
            type.append(s);
            first = false;
            }

        return type;
    }

    void register_data_type(wxGrid*) const
    {
        // Do not register the data type because we use standard value type.
    }
};

// class table_sequence_converter

class table_sequence_converter : public renderer_type_converter
{
  public:
    char const* type() const override
    {
        return typeid(table_sequence_converter).name();
    }

    virtual wxGridCellRenderer* create_renderer() const
    {
        return new(wx) wxGridCellStringRenderer();
    }

    virtual wxGridCellEditor* create_editor() const
    {
        return new(wx) DatumSequenceEditor();
    }
};

// class table_int_range_converter

class table_int_range_converter : public renderer_type_converter
{
  public:
    char const* type() const override
    {
        return typeid(table_int_range_converter).name();
    }

    wxGridCellRenderer* create_renderer() const override
    {
        return new(wx) wxGridCellNumberRenderer();
    }

    wxGridCellEditor* create_editor() const override
    {
        return new(wx) RangeIntEditor();
    }
};

// class table_double_range_converter

class table_double_range_converter : public renderer_type_converter
{
  public:
    char const* type() const override
    {
        return typeid(table_double_range_converter).name();
    }

    wxGridCellRenderer* create_renderer() const override
    {
        // Use wxGridCellStringRenderer instead of wxGridCellFloatRenderer to
        // keep the number format as is.
        return new(wx) wxGridCellStringRenderer();
    }

    wxGridCellEditor* create_editor() const override
    {
        return new(wx) RangeDoubleEditor();
    }
};

// class table_date_converter

class table_date_converter : public renderer_type_converter
{
  public:
    wxString to_renderer_value(std::string const& value) const override
    {
        auto const date{ConvertDateToWx(value_cast<calendar_date>(value))};
        return date.FormatDate();
    }

    std::string from_editor_value(wxString const& value) const override
    {
        wxDateTime date;
        auto date_parse_ok = date.ParseISODate(value);
        LMI_ASSERT(date_parse_ok);

        return value_cast<std::string>(ConvertDateFromWx(date));
    }

    char const* type() const override
    {
        return typeid(table_date_converter).name();
    }

    wxGridCellRenderer* create_renderer() const override
    {
        return new(wx) wxGridCellDateRenderer();
    }

    wxGridCellEditor* create_editor() const override
    {
        return new(wx) RangeDateEditor();
    }
};

// class table_fallback_converter

class table_fallback_converter : public renderer_type_converter
{
  public:
    wxString to_renderer_value(std::string const& value) const override
    {
        // Strings containing new line characters are currently not displayed
        // correctly by wxDataViewCtrl, so display the value on a single line
        // after converting any optional new lines to the Unicode character
        // representing them.
        wxString s{value};
        s.Replace("\n", RETURN_SYMBOL, true);
        return s;
    }

    std::string from_editor_value(wxString const& value) const override
    {
        // Undo the replacement done above. Notice that this will (wrongly)
        // translate any RETURN_SYMBOL characters entered by the user into the
        // string to new lines, but this character is not supposed to be used
        // in any of the cells values, so just ignore this problem for now.
        wxString s{value};
        s.Replace(RETURN_SYMBOL, "\n", true);
        return s.ToStdString();
    }

    char const* type() const override
    {
        return typeid(table_fallback_converter).name();
    }

    wxString grid_value_type(any_member<Input> const&) const override
    {
        return wxGRID_VALUE_STRING;
    }

    void register_data_type(wxGrid*) const
    {
        // Do not register the data type because we use standard value type.
    }

  private:
    static const wchar_t RETURN_SYMBOL = 0x23ce;
};

std::map<std::string, renderer_type_converter const*>&
renderer_type_converter::get_all()
{
    static std::map<std::string, renderer_type_converter const*> all{
        { get_impl<table_bool_converter>().type()
        ,&get_impl<table_bool_converter>()
        },
        { get_impl<table_fallback_converter>().type()
        ,&get_impl<table_fallback_converter>()
        },
        { get_impl<table_sequence_converter>().type()
        ,&get_impl<table_sequence_converter>()
        },
        { get_impl<table_enum_converter>().type()
        ,&get_impl<table_enum_converter>()
        },
        { get_impl<table_int_range_converter>().type()
        ,&get_impl<table_int_range_converter>()
        },
        { get_impl<table_double_range_converter>().type()
        ,&get_impl<table_double_range_converter>()
        },
        { get_impl<table_date_converter>().type()
        ,&get_impl<table_date_converter>()
        },
    };
    return all;
}

renderer_type_converter const&
renderer_type_converter::get_by_value(any_member<Input> const& value)
{
    if(exact_cast<mce_yes_or_no>(value))
        {
        return get_impl<table_bool_converter>();
        }
    else if(exact_cast<datum_string>(value))
        {
        return get_impl<table_fallback_converter>();
        }
    else if(is_reconstitutable_as<datum_sequence>(value))
        {
        return get_impl<table_sequence_converter>();
        }
    else if(is_reconstitutable_as<mc_enum_base  >(value))
        {
        return get_impl<table_enum_converter>();
        }
    else if(is_reconstitutable_as<tn_range_base >(value))
        {
        tn_range_base const* as_range = member_cast<tn_range_base>(value);
        if(typeid(int) == as_range->value_type())
            {
            return get_impl<table_int_range_converter>();
            }
        else if(typeid(double) == as_range->value_type())
            {
            return get_impl<table_double_range_converter>();
            }
        else if(typeid(calendar_date) == as_range->value_type())
            {
            return get_impl<table_date_converter>();
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

    return get_impl<table_fallback_converter>();
}

template<typename T>
renderer_type_converter const& renderer_type_converter::get_impl()
{
    static T singleton;
    return singleton;
}

} // Unnamed namespace.

/// Interface to the data for wxGrid.

class CensusViewTable : public wxGridTableBase
{
  public:
    // Cell serial number: always shown in first column.
    static int const Col_CellNum = 0;

    explicit CensusViewTable(CensusView& view)
        :view_ {view}
    {
    }

    // return the number of rows and columns in this table.
    int GetNumberRows() override;
    int GetNumberCols() override;

    wxString GetValue(int row, int col) override;
    void SetValue(int row, int col, wxString const& value) override;

    // Data type determination.
    wxString GetTypeName(int row, int col) override;

    // Override only used rows/cols handling functions.
    bool AppendRows(size_t numRows) override;
    bool DeleteRows(size_t pos, size_t numRows) override;
    bool AppendCols(size_t numCols) override;
    bool DeleteCols(size_t pos, size_t numCols) override;

    wxString GetColLabelValue(int col) override;

    void SetFirstColumnReadOnly();

    std::string const& col_name(int col) const;

    Input& row_at(int row);
    Input const& row_at(int row) const;

    any_member<Input>& cell_at(int row, int col);
    any_member<Input> const& cell_at(int row, int col) const;

  private:
      any_member<Input>& cell_at(int row, std::string const& col);
      any_member<Input> const& cell_at(int row, std::string const& col) const;
      std::vector<std::string> const& all_headers() const;

    CensusView& view_;
};

int CensusViewTable::GetNumberRows()
{
    return lmi::ssize(view_.cell_parms());
}

int CensusViewTable::GetNumberCols()
{
    // "+ 1" for cell serial number in first column.
    return lmi::ssize(view_.visible_columns_) + 1;
}

wxString CensusViewTable::GetValue(int row, int col)
{
    if(col == Col_CellNum)
        {
        return value_cast<std::string>(1 + row);
        }

    auto const& cell{cell_at(row, col)};
    auto const& conv{renderer_type_converter::get_by_value(cell)};
    return conv.to_renderer_value(cell.str());
}

void CensusViewTable::SetValue(int row, int col, wxString const& value)
{
    LMI_ASSERT(col != Col_CellNum);

    auto& cell{cell_at(row, col)};
    auto const& conv{renderer_type_converter::get_by_value(cell)};
    auto const& prev_val{cell.str()};
    auto const& new_val{conv.from_editor_value(value)};

    if(prev_val == new_val)
        {
        return;
        }

    cell = new_val;

    Input& model{view_.cell_parms()[row]};
    model.Reconcile();

    view_.document().Modify(true);
}

wxString CensusViewTable::GetTypeName(int row, int col)
{
    if(col == Col_CellNum)
        {
        return wxGRID_VALUE_NUMBER;
        }

    auto const& value{cell_at(row, col)};
    auto const& conv{renderer_type_converter::get_by_value(value)};

    auto type{conv.grid_value_type(value)};

    return type;
}

wxString CensusViewTable::GetColLabelValue(int col)
{
    if(col == Col_CellNum)
        {
        return "Cell";
        }

    auto const& header{all_headers()[view_.visible_columns_[col - 1]]};
    return insert_spaces_between_words(header);
}

bool CensusViewTable::AppendRows(size_t numRows)
{
    auto grid{GetView()};
    LMI_ASSERT(grid != nullptr);

    wxGridTableMessage msg
        {this
        ,wxGRIDTABLE_NOTIFY_ROWS_APPENDED
        ,bourn_cast<int>(numRows)
        };
    grid->ProcessTableMessage(msg);

    return true;
}

bool CensusViewTable::DeleteRows(size_t pos, size_t num_rows)
{
    auto grid{GetView()};
    LMI_ASSERT(grid != nullptr);

    auto const ipos{bourn_cast<int>(pos)};
    auto inum_rows{bourn_cast<int>(num_rows)};
    auto const cur_num_rows{GetView()->GetNumberRows()};

    if(ipos >= cur_num_rows)
        {
        wxFAIL_MSG(wxString::Format
            (wxT("Called CensusViewTable::DeleteRows(pos=%d, N=%d)\nPos value is invalid for present table with %d rows")
            ,ipos
            ,inum_rows
            ,cur_num_rows
            )
        );

        return false;
        }

    if(inum_rows > cur_num_rows - ipos)
        {
        inum_rows = cur_num_rows - ipos;
        }

    if(grid)
        {
        wxGridTableMessage msg
            {this
            ,wxGRIDTABLE_NOTIFY_ROWS_DELETED
            ,ipos
            ,inum_rows
            };
        grid->ProcessTableMessage(msg);
        }

    return true;
}

bool CensusViewTable::AppendCols(size_t numCols)
{
    auto grid{GetView()};
    LMI_ASSERT(grid != nullptr);

    wxGridTableMessage msg
        {this
        ,wxGRIDTABLE_NOTIFY_COLS_APPENDED
        ,bourn_cast<int>(numCols)
        };
    grid->ProcessTableMessage(msg);

    return true;
}

bool CensusViewTable::DeleteCols(size_t pos, size_t num_cols)
{
    auto grid{GetView()};
    LMI_ASSERT(grid != nullptr);

    auto const ipos{bourn_cast<int>(pos)};
    auto inum_cols{bourn_cast<int>(num_cols)};
    auto const cur_num_cols{GetView()->GetNumberCols()};

    if(ipos >= cur_num_cols)
        {
        wxFAIL_MSG(wxString::Format
            (wxT("Called CensusViewTable::DeleteCols(pos=%d, N=%d)\nPos value is invalid for present table with %d cols")
            ,ipos
            ,inum_cols
            ,cur_num_cols
            )
        );

        return false;
        }

    if(inum_cols > cur_num_cols - ipos)
        {
        inum_cols = cur_num_cols - ipos;
        }

    if(grid)
        {
        wxGridTableMessage msg
            {this
            ,wxGRIDTABLE_NOTIFY_COLS_DELETED
            ,ipos
            ,inum_cols
            };
        grid->ProcessTableMessage(msg);
        }

    return true;
}

void CensusViewTable::SetFirstColumnReadOnly()
{
    if(CanHaveAttributes())
        {
        auto attr{new wxGridCellAttr()};
        attr->SetReadOnly();
        SetColAttr(attr, 0);
        }
}

inline std::string const& CensusViewTable::col_name(int col) const
{
    LMI_ASSERT(0 < col);
    // "- 1" because first column is cell serial number.
    return all_headers()[view_.visible_columns_[col - 1]];
}

inline Input& CensusViewTable::row_at(int row)
{
    return view_.cell_parms()[row];
}

inline Input const& CensusViewTable::row_at(int row) const
{
    return view_.cell_parms()[row];
}

inline any_member<Input>& CensusViewTable::cell_at(int row, int col)
{
    return cell_at(row, col_name(col));
}

inline any_member<Input> const& CensusViewTable::cell_at(int row, int col) const
{
    return cell_at(row, col_name(col));
}

inline any_member<Input>& CensusViewTable::cell_at(int row, std::string const& col)
{
    return row_at(row)[col];
}

inline any_member<Input> const& CensusViewTable::cell_at(int row, std::string const& col) const
{
    return row_at(row)[col];
}

inline std::vector<std::string> const& CensusViewTable::all_headers() const
{
    return view_.case_parms()[0].member_names();
}

namespace
{
any_member<Input> const& cell_at(wxGridTableBase const* table, int row, int col)
{
    LMI_ASSERT(table != nullptr);
    return dynamic_cast<CensusViewTable const&>(*table).cell_at(row, col);
}

Input const& row_at(wxGridTableBase const* table, int row)
{
    LMI_ASSERT(table != nullptr);
    return dynamic_cast<CensusViewTable const&>(*table).row_at(row);
}

std::string const& col_name(wxGridTableBase const* table, int col)
{
    LMI_ASSERT(table != nullptr);
    return dynamic_cast<CensusViewTable const&>(*table).col_name(col);
}
}

// class CensusView

IMPLEMENT_DYNAMIC_CLASS(CensusView, ViewEx)

BEGIN_EVENT_TABLE(CensusView, ViewEx)
    EVT_GRID_CELL_RIGHT_CLICK(                   CensusView::UponRightClick             )
    EVT_GRID_CELL_CHANGED(                       CensusView::UponValueChanged           )
    EVT_MENU(XRCID("edit_cell"                 ),CensusView::UponEditCell               )
    EVT_MENU(XRCID("edit_class"                ),CensusView::UponEditClass              )
    EVT_MENU(XRCID("edit_case"                 ),CensusView::UponEditCase               )
    EVT_MENU(XRCID("run_cell"                  ),CensusView::UponRunCell                )
// SOMEDAY !! This may be useful for large cases.
//  EVT_MENU(XRCID("run_class"                 ),CensusView::UponRunClass               )
    EVT_MENU(XRCID("run_case"                  ),CensusView::UponRunCase                )
    EVT_MENU(XRCID("print_case"                ),CensusView::UponPrintCase              )
    EVT_MENU(XRCID("print_case_to_disk"        ),CensusView::UponPrintCaseToDisk        )
    EVT_MENU(XRCID("print_spreadsheet"         ),CensusView::UponRunCaseToSpreadsheet   )
    EVT_MENU(XRCID("print_group_roster"        ),CensusView::UponRunCaseToGroupRoster   )
    EVT_MENU(XRCID("print_group_quote"         ),CensusView::UponRunCaseToGroupQuote    )
    EVT_MENU(XRCID("copy_census"               ),CensusView::UponCopyCensus             )
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
    EVT_UPDATE_UI(XRCID("print_group_quote"    ),CensusView::UponUpdateAlwaysEnabled    )
    EVT_UPDATE_UI(XRCID("copy_census"          ),CensusView::UponUpdateColumnValuesVary )
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
    :ViewEx            {}
    ,autosize_columns_ {false}
    ,grid_window_      {nullptr}
    ,grid_table_       {new(wx) CensusViewTable(*this)}
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

bool CensusView::column_value_varies_across_cells(std::string const& header) const
{
    auto const z = case_parms()[0][header];
    for(auto const& j : class_parms()) {if(z != j[header]) return true;}
    for(auto const& j : cell_parms() ) {if(z != j[header]) return true;}
    return false;
}

wxWindow* CensusView::CreateChildWindow()
{
    grid_window_ = new(wx) wxGrid
        (GetFrame()
        ,wxID_ANY
        ,wxDefaultPosition
        ,wxDefaultSize
        );
    grid_window_->HideRowLabels();
    grid_window_->SetUseNativeColLabels();

    grid_window_->SetTable(grid_table_, true, wxGrid::wxGridSelectRows);

    for(auto const& it : renderer_type_converter::get_all())
        {
        it.second->register_data_type(grid_window_);
        }

    grid_table_->SetFirstColumnReadOnly();

    // Show headers.
    document().Modify(false);
    Update();

    return grid_window_;
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
    auto const selected_rows = grid_window_->GetSelectedRows();
    LMI_ASSERT(lmi::ssize(selected_rows) == 1);
    auto const row = selected_rows.front();
    LMI_ASSERT(0 <= row && row < bourn_cast<int>(grid_table_->GetRowsCount()));
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
    wxBusyCursor reverie;

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
    // Display exactly those columns whose rows aren't all identical. For
    // this purpose, consider as "rows" the individual cells--and also the
    // case and class defaults, even though they aren't displayed in rows.
    // Reason: although the case and class defaults are hidden, they're
    // still information--so if the user made them different from any cell
    // wrt some column, we respect that conscious decision.
    auto const& all_headers{case_parms()[0].member_names()};
    std::vector<int> new_visible_columns;
    int column = 0;
    for(auto const& header : all_headers)
        {
        if(column_value_varies_across_cells(header))
            {
            new_visible_columns.push_back(column);
            }
        ++column;
        }

    if(new_visible_columns != visible_columns_)
        {
        auto const selected_row = grid_window_->GetGridCursorRow();
        auto const cursor_col = grid_window_->GetGridCursorCol();

        auto const old_cols = lmi::ssize(visible_columns_)    + 1;
        auto const new_cols = lmi::ssize(new_visible_columns) + 1;

        grid_window_->BeginBatch();

        if(grid_window_->IsCellEditControlEnabled())
            {
            grid_window_->DisableCellEditControl();
            }

        std::swap(new_visible_columns, visible_columns_);

        if(old_cols != new_cols)
            {
            grid_window_->DeleteCols(0, old_cols);
            grid_window_->AppendCols(new_cols);
            grid_table_->SetFirstColumnReadOnly();
            }

        if(autosize_columns_)
            {
            grid_window_->AutoSize();
            }

        grid_window_->SetGridCursor
            (selected_row
            ,std::min(cursor_col, new_cols - 1)
            );
        grid_window_->EndBatch();
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
    auto cell_number            { selected_row()};
    auto class_name             { class_name_from_cell_number(cell_number)};
    auto& modifiable_parms      {*class_parms_from_class_name(class_name)};
    auto const unmodified_parms { modifiable_parms};
    auto const title            { class_title(cell_number)};

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
    auto& modifiable_parms{case_parms()[0]};
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

    wxWindowUpdateLocker u(grid_window_);
    grid_window_->AutoSizeColumns();
    Update();
}

/// Shrink all nonfrozen columns to default width.

void CensusView::UponColumnWidthFixed(wxCommandEvent&)
{
    autosize_columns_ = false;

    wxWindowUpdateLocker u(grid_window_);
    for(int j = 0; j < bourn_cast<int>(grid_window_->GetNumberCols()); ++j)
        {
        grid_window_->SetColSize(j, wxCOL_WIDTH_DEFAULT);
        }
    Update();
}

void CensusView::UponRightClick(wxGridEvent& e)
{
    if(e.GetEventObject() != grid_window_)
        {
        // This event should come only from the window pointed to by
        // grid_window_. Ignore it if it happens to come elsewhence.
        e.Skip();
        return;
        }

    wxMenu* census_menu = wxXmlResource::Get()->LoadMenu("census_menu_ref");
    LMI_ASSERT(census_menu);
    grid_window_->PopupMenu(census_menu);
    delete census_menu;
}

void CensusView::UponValueChanged(wxGridEvent&)
{
    Timer timer;
    Update();
    status() << "Update: " << timer.stop().elapsed_msec_str() << std::flush;
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
    auto const selected_rows = grid_window_->GetSelectedRows();
    e.Enable(lmi::ssize(selected_rows) == 1);
}

void CensusView::UponUpdateNonemptySelection(wxUpdateUIEvent& e)
{
    auto const selected_rows = grid_window_->GetSelectedRows();
    e.Enable(lmi::ssize(selected_rows) > 0);
}

/// Conditionally enable copying.
///
/// Copying is forbidden if it would produce only whitespace; i.e.,
/// if no "interesting" column varies across cells. All columns are
/// interesting except:
///  - the cell serial number that is always shown in the first column
///    (that's just a GUI artifact, not an actual cell datum); and
///  - the "UseDOB" and "IssueAge" columns that are filtered out by
///    DoCopyCensus().
/// This implementation ignores the first column, then enables the
/// command unless "UseDOB" is the only other column; the rationale is
/// clear only in the problem domain: "IssueAge" needn't be tested,
/// because if "IssueAge" varies, then so must either "DateOfBirth"
/// or "EffectiveDate".

void CensusView::UponUpdateColumnValuesVary(wxUpdateUIEvent& e)
{
    static const std::string dob_header = insert_spaces_between_words("UseDOB");
    int const n_cols = grid_table_->GetColsCount();
    bool const disable =
            1 == n_cols
        || (2 == n_cols && dob_header == grid_table_->GetColLabelValue(1))
        ;
    e.Enable(!disable);
}

/// Update the grid display.
///
/// If a parameter was formerly the same for all cells but now differs due
///  to editing, then display its column for all cells.
/// If a column was previously displayed but is now the same for all cells
///  due to editing, then display it no longer.
/// Similarly, if an old employee class is no longer used, remove it; and
///  if a new one comes into use, display it.

void CensusView::Update()
{
    LMI_ASSERT(grid_table_->GetRowsCount() == lmi::ssize(cell_parms()));

    wxWindowUpdateLocker u(grid_window_);

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
    wxBusyCursor reverie;
    Timer timer;

    cell_parms().push_back(case_parms()[0]);

    grid_window_->ClearSelection();
    grid_window_->AppendRows();

    Update();
    document().Modify(true);

    grid_window_->GoToCell
        (grid_table_->GetRowsCount() - 1
        ,grid_window_->GetGridCursorCol()
        );

    status() << "Add: " << timer.stop().elapsed_msec_str() << std::flush;
}

void CensusView::UponDeleteCells(wxCommandEvent&)
{
    auto const n_items{grid_table_->GetRowsCount()};
    auto selection{grid_window_->GetSelectedRows()};
    auto const n_sel_items{lmi::ssize(selection)};
    LMI_ASSERT(n_sel_items == lmi::ssize(selection));
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

    wxBusyCursor reverie;
    Timer timer;

    const auto cursor_col{grid_window_->GetGridCursorCol()};

    std::sort(selection.begin(), selection.end());

    LMI_ASSERT(lmi::ssize(cell_parms()) == n_items);

    for(int j = n_sel_items - 1; 0 <= j; --j)
        {
        cell_parms().erase(selection[j] + cell_parms().begin());
        }
    LMI_ASSERT(lmi::ssize(cell_parms()) == n_items - n_sel_items);

    // Send notifications about changes to the wxDataViewCtrl model. Two things
    // changed: some rows were deleted and cell number of some rows shifted
    // accordingly.
    grid_window_->ClearSelection();
    for(int j = n_sel_items - 1; 0 <= j; --j)
        {
        grid_window_->DeleteRows(selection[j]);
        }

    int const new_selected_row{std::min
        (selection.front()
        ,lmi::ssize(cell_parms()) - 1
        )};
    grid_window_->GoToCell(new_selected_row, cursor_col);

    Update();
    document().Modify(true);

    status() << "Delete: " << timer.stop().elapsed_msec_str() << std::flush;
}

/// Print tab-delimited details to file loadable in spreadsheet programs.

void CensusView::UponRunCaseToSpreadsheet(wxCommandEvent&)
{
    DoAllCells(mce_emit_spreadsheet);
}

/// Print group roster to a "spreadsheet" (TSV) file.

void CensusView::UponRunCaseToGroupRoster(wxCommandEvent&)
{
    DoAllCells(mce_emit_group_roster);
}

/// Print group premium quote to a PDF file.

void CensusView::UponRunCaseToGroupQuote(wxCommandEvent&)
{
    DoAllCells(mce_emit_group_quote);
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
    auto const census_data{ClipboardEx::GetText()};

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

    // Use a modifiable copy of case defaults as an archetype for new
    // cells to be created by pasting. Modifications are conditionally
    // written back to case defaults later.
    Input archetype(case_parms()[0]);

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
        archetype["UseDOB"] = "Yes";
        }
    else if(age_pasted)
        {
        archetype["UseDOB"] = "No";
        }
    else
        {
        ; // Do nothing: neither age nor DOB pasted.
        }

    cells.reserve(std::count(census_data.begin(), census_data.end(), '\n'));

    // Read each subsequent line into an input object representing one cell.
    int current_line = 0;
    while(std::getline(iss_census, line, '\n'))
        {
        ++current_line;

        iss_census >> std::ws;

        Input current_cell(archetype);

        std::istringstream iss_line(line);
        std::string token;
        std::vector<std::string> values;

        while(std::getline(iss_line, token, '\t'))
            {
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

        for(int j = 0; j < lmi::ssize(headers); ++j)
            {
            if(exact_cast<tnr_date>(current_cell[headers[j]]))
                {
                static int const jdn_min = calendar_date::gregorian_epoch_jdn;
                static int const jdn_max = calendar_date::last_yyyy_date_jdn;
                static int const ymd_min = JdnToYmd(jdn_t(jdn_min)).value();
                static int const ymd_max = JdnToYmd(jdn_t(jdn_max)).value();
                int z = value_cast<int>(values[j]);
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
        }

    if(0 == current_line)
        {
        warning() << "No cells to paste." << LMI_FLUSH;
        return;
        }

    if(!document().IsModified() && !document().GetDocumentSaved())
        {
        case_parms ().clear();
        case_parms ().push_back(archetype);
        class_parms().clear();
        class_parms().push_back(archetype);
        cell_parms ().swap(cells);
        }
    else if(configurable_settings::instance().census_paste_palimpsestically())
        {
        cell_parms().swap(cells);
        // Cells that were copied from lmi have DOB and not IssueAge,
        // so pasting them back in sets UseDOB. Force UseDOB for case
        // and class defaults to prevent showing a UseDOB column with
        // each cell set to "Yes".
        for(auto& j : case_parms ()) {j["UseDOB"] = "Yes";}
        for(auto& j : class_parms()) {j["UseDOB"] = "Yes";}
        }
    else
        {
        cell_parms().reserve(cell_parms().size() + cells.size());
        std::back_insert_iterator<std::vector<Input>> iip(cell_parms());
        std::copy(cells.begin(), cells.end(), iip);
        }

    auto const old_rows{grid_window_->GetNumberRows()};
    auto const old_cols{grid_window_->GetNumberCols()};
    auto const new_rows{grid_table_->GetRowsCount()};
    auto const new_cols{grid_table_->GetColsCount()};

    grid_window_->BeginBatch();
    grid_window_->ClearSelection();

    if(grid_window_->IsCellEditControlEnabled())
        {
        grid_window_->DisableCellEditControl();
        }

    if(old_rows != new_rows || old_cols != new_cols)
        {
        grid_window_->DeleteRows(0, old_rows);
        grid_window_->DeleteCols(0, old_cols);
        grid_window_->AppendRows(new_rows);
        grid_window_->AppendCols(new_cols);
        grid_table_->SetFirstColumnReadOnly();
        if(autosize_columns_)
            {
            grid_window_->AutoSize();
            }
        }

    grid_window_->EndBatch();

    document().Modify(true);
    Update();

    LMI_ASSERT(1 == case_parms().size());
    LMI_ASSERT(!cell_parms ().empty());
    LMI_ASSERT(!class_parms().empty());
}

/// Copy from census manager to clipboard and TSV file.
///
/// Simply calls DoCopyCensus(), q.v.

void CensusView::UponCopyCensus(wxCommandEvent&)
{
    DoCopyCensus();
}

/// Copy from census manager to clipboard and TSV file.
///
/// Include exactly those columns whose rows aren't all identical,
/// considering as "rows" the individual cells--and also the case
/// defaults, even though they aren't displayed in any row.
///
/// Motivation: Some census changes are more easily made by exporting
/// data from lmi, manipulating it in a spreadsheet, and then pasting
/// it back into lmi.
///
/// Never extract "UseDOB": it's always set by UponPasteCensus().
/// Never extract "IssueAge". If it's present, then "UseDOB" must also
/// be, and "UseDOB" preserves information that "IssueAge" loses.
///
/// Implementation notes
///
/// Output lines use '\t' as a terminator following each field, rather
/// than as a separator between each pair of fields as might have been
/// expected; thus, they end in "'t\n". This makes the code slightly
/// simpler by avoiding a "loop and a half". In practice, it doesn't
/// make any difference: gnumeric, libreoffice calc, a popular msw
/// spreadsheet program, and lmi's own UponPasteCensus() all ignore
/// the extra '\t'.

void CensusView::DoCopyCensus() const
{
    Timer timer;
    std::vector<std::string> distinct_headers;
    std::vector<std::string> const& all_headers(case_parms()[0].member_names());
    for(auto const& header : all_headers)
        {
        bool const varies = column_value_varies_across_cells(header);
        if(header != "UseDOB" && header != "IssueAge" && varies)
            {
            distinct_headers.push_back(header);
            }
        }

    if(distinct_headers.empty())
        {
        alarum() << "All cells identical: nothing to copy." << LMI_FLUSH;
        }

    configurable_settings const& c = configurable_settings::instance();
    std::string const& e = c.spreadsheet_file_extension();
    std::string const  f = fs::basename(base_filename()) + ".census.cns";
    std::string file_name = unique_filepath(f, e).string();
    std::ofstream ofs(file_name.c_str(), ios_out_app_binary());

    for(auto const& header : distinct_headers)
        {
        ofs << header << '\t';
        }
    ofs << '\n';

    int counter = 0;
    for(auto const& cell : cell_parms())
        {
        for(auto const& header : distinct_headers)
            {
            // Show calendar dates as YYYYMMDD rather than JDN.
            std::string s = cell[header].str();
            if(exact_cast<tnr_date>(cell[header]))
                {
                int z = JdnToYmd(jdn_t(value_cast<int>(s))).value();
                s = value_cast<std::string>(z);
                }
            ofs << s << '\t';
            }
        ofs << '\n';
        status() << "Copied cell number " << ++counter << '.' << std::flush;
        }

    if(!ofs)
        {
        alarum() << "Unable to write '" << file_name << "'." << LMI_FLUSH;
        }

    ofs.close();
    std::ifstream ifs(file_name.c_str());
    std::string s;
    istream_to_string(ifs, s);
    ClipboardEx::SetText(s);
    status() << "Copy: " << timer.stop().elapsed_msec_str() << std::flush;
}
