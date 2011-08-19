// Census manager.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.
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

#include "census_view.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "census_document.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "default_view.hpp"
#include "edit_mvc_docview_parameters.hpp"
#include "illustration_view.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp" // is_ok_for_cctype()
#include "path_utility.hpp"
#include "safely_dereference_as.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp" // class ClipboardEx

#include <wx/dataview.h>
#include <wx/icon.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>
#include <wx/wupdlock.h>

#include <algorithm>
#include <cctype>
#include <cstdio>         // std::remove()
#include <istream>        // std::ws
#include <iterator>
#include <sstream>

// TODO ?? Can't this macro be dispensed with?
#define ID_LISTWINDOW 12345

namespace
{
    // TODO ?? Add description and unit tests; consider relocating,
    // and include "miscellany.hpp" only in ultimate location.
    std::string insert_spaces_between_words(std::string const& s)
    {
        std::string r;
        std::insert_iterator<std::string> j(r, r.begin());
        std::string::const_iterator i;
        for(i = s.begin(); i != s.end(); ++i)
            {
            if(is_ok_for_cctype(*i) && std::isupper(*i) && !r.empty())
                {
                *j++ = ' ';
                }
            *j++ = *i;
            }
        return r;
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

    virtual void GetValueByRow
        (wxVariant& variant
        ,unsigned int row
        ,unsigned int col
        ) const
    {
        if(col == Col_CellNum)
            {
            variant = wxString::Format("%d", 1 + row);
            }
        else
            {
            std::string s = view_.cell_parms()[row][all_headers()[col - 1]].str();
            variant = s;
            }
    }

    virtual bool SetValueByRow(wxVariant const&, unsigned int, unsigned int)
    {
        // in-place editing not yet implemented
        return false;
    }

    virtual unsigned int GetColumnCount() const
    {
        return all_headers().size() + 1;
    }

    virtual wxString GetColumnType(unsigned int) const
    {
        return "string";
    }

  private:
    std::vector<std::string> const& all_headers() const
    {
        return view_.case_parms()[0].member_names();
    }

    CensusView& view_;
};

IMPLEMENT_DYNAMIC_CLASS(CensusView, ViewEx)

BEGIN_EVENT_TABLE(CensusView, ViewEx)
    EVT_DATAVIEW_ITEM_CONTEXT_MENU(ID_LISTWINDOW, CensusView::UponRightClick)
    EVT_MENU(XRCID("edit_cell"             ),CensusView::UponEditCell )
    EVT_MENU(XRCID("edit_class"            ),CensusView::UponEditClass)
    EVT_MENU(XRCID("edit_case"             ),CensusView::UponEditCase )
    EVT_MENU(XRCID("run_cell"              ),CensusView::UponRunCell)
//    EVT_MENU(XRCID("run_class"             ),CensusView::UponRunClass)   // SOMEDAY !! This may be useful for large cases.
    EVT_MENU(XRCID("run_case"              ),CensusView::UponRunCase)
    EVT_MENU(XRCID("print_case"            ),CensusView::UponPrintCase)
    EVT_MENU(XRCID("print_case_to_disk"    ),CensusView::UponPrintCaseToDisk)
    EVT_MENU(XRCID("print_spreadsheet"     ),CensusView::UponRunCaseToSpreadsheet)
    EVT_MENU(XRCID("paste_census"          ),CensusView::UponPasteCensus)
    EVT_MENU(XRCID("add_cell"              ),CensusView::UponAddCell)
    EVT_MENU(XRCID("delete_cells"          ),CensusView::UponDeleteCells)
    EVT_MENU(XRCID("column_width_varying"  ),CensusView::UponColumnWidthVarying)
    EVT_MENU(XRCID("column_width_fixed"    ),CensusView::UponColumnWidthFixed)

    EVT_UPDATE_UI(XRCID("edit_cell"            ),CensusView::UponUpdateSingleSelection)
    EVT_UPDATE_UI(XRCID("edit_class"           ),CensusView::UponUpdateSingleSelection)
    EVT_UPDATE_UI(XRCID("edit_case"            ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("run_cell"             ),CensusView::UponUpdateSingleSelection)
    EVT_UPDATE_UI(XRCID("run_class"            ),CensusView::UponUpdateSingleSelection)
    EVT_UPDATE_UI(XRCID("run_case"             ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("print_case"           ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("paste_census"         ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("add_cell"             ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),CensusView::UponUpdateNonemptySelection)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),CensusView::UponUpdateAlwaysEnabled)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),CensusView::UponUpdateAlwaysEnabled)
END_EVENT_TABLE()

CensusView::CensusView()
    :ViewEx                          ()
    ,all_changes_have_been_validated_(true)
    ,autosize_columns_               (false)
    ,composite_is_available_         (false)
    ,was_cancelled_                  (false)
    ,list_window_                    (0)
    ,list_model_                     (new(wx) CensusViewDataViewModel(*this))
{
}

CensusView::~CensusView()
{
    list_model_->DecRef();
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
        // TODO ?? Add an any_member operator== instead.
        if(class_name == (*i)["EmployeeClass"].str())
            {
            return &*i;
            }
        ++i;
        }
    return 0;
}

    // Determine which columns need to be displayed because their rows
    // would not all be identical--i.e. because at least one cell or one
    // class default differs from the case default wrt that column.
bool CensusView::column_value_varies_across_cells
    (std::string        const& header
    ,std::vector<Input> const& cells
    ) const
{
    std::vector<Input>::const_iterator j;
    for(j = cells.begin(); j != cells.end(); ++j)
        {
        if(!((*j)[header] == case_parms()[0][header]))
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
        ,ID_LISTWINDOW
        ,wxDefaultPosition
        ,wxDefaultSize
        ,wxDV_ROW_LINES | wxDV_MULTIPLE
        );

    // Use same row height as used by wxListCtrl without icons. By default,
    // wxDataViewCtrl uses slightly larger spacing, but we prefer to fit more
    // on the screen over slightly improved readability.
    list_window_->SetRowHeight(list_window_->GetCharHeight() + 1);

    list_window_->AssociateModel(list_model_);

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
    if(is_invalid())
        {
        return oe_mvc_dv_cancelled;
        }

    return edit_mvc_docview_parameters<DefaultView>
        (parameters
        ,document()
        ,GetFrame()
        ,title
        );
}

bool CensusView::is_invalid()
{
    if(!all_changes_have_been_validated_)
        {
        int z = wxMessageBox
            ("Cannot proceed without first validating changes."
            ,"Validate changes now?"
            ,wxYES_NO | wxICON_QUESTION
            );
        if(wxYES == z)
            {
            // TODO ?? Reserved for grid implementation.
            }
        }
    return false;
}

// TODO ?? Reserved for a grid implementation.
int CensusView::selected_column()
{
    return 0;
}

int CensusView::selected_row()
{
    int row = list_model_->GetRow(list_window_->GetSelection());
    LMI_ASSERT(0 <= row && static_cast<unsigned int>(row) < list_model_->GetCount());
    return row;
}

// Make a vector of all class names used by any individual, from
// scratch; and update the vector of class default parameters,
// adding any new classes, and purging any that are no longer in use
// by any cell.
void CensusView::update_class_names()
{
    // Extract names and add them even if they might be duplicates.
    std::vector<std::string> all_class_names;

    for
        (std::vector<Input>::iterator i = cell_parms().begin()
        ;i != cell_parms().end()
        ;++i
        )
        {
        all_class_names.push_back((*i)["EmployeeClass"].str());
        }

    std::vector<std::string> unique_class_names;

    std::insert_iterator<std::vector<std::string> > iin
        (unique_class_names
        ,unique_class_names.begin()
        );
    std::sort(all_class_names.begin(), all_class_names.end());
    std::unique_copy(all_class_names.begin(), all_class_names.end(), iin);

// TODO ?? need parms for each?
//    if find name in class array
//        OK
//    else
//        create: copy from first matching individual
// TODO ?? and if unmatching element in class array: delete it?

    // Rebuild vector of class parameters so that it contains
    // an element for each class in use.
    std::vector<Input> rebuilt_class_parms;
    std::vector<std::string>::iterator n = unique_class_names.begin();
    while(n != unique_class_names.end())
        {
        Input* parms = class_parms_from_class_name(*n);
        if(0 != parms)
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
                fatal_error()
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
    std::insert_iterator<std::vector<Input> > iip(class_parms(), class_parms().begin());
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
    // Case or class default parameters were edited and changed.
    // Compare the default parameters before and after editing;
    // for every parameter that was changed, assign the new value
    // to all applicable cells, i.e.
    //   if case  defaults changed: all cells and all class defaults;
    //   if class defaults changed: all cells in the class.

    // TODO ?? temp string for new value, eeclass?
    // TODO ?? combine class and indv vectors for case changes?

    std::vector<std::string> headers_of_changed_parameters;
    std::vector<std::string> const& all_headers(case_parms()[0].member_names());
    std::vector<std::string>::const_iterator i;
    for
        (i  = all_headers.begin()
        ;i != all_headers.end  ()
        ;++i
        )
        {
        if(!(old_parms[*i] == new_parms[*i]))
            {
            headers_of_changed_parameters.push_back(*i);
            }
        }
    for
        (i  = headers_of_changed_parameters.begin()
        ;i != headers_of_changed_parameters.end  ()
        ;++i
        )
        {
        if(!for_this_class_only)
            {
            std::vector<Input>::iterator j;
            for
                (j  = class_parms().begin()
                ;j != class_parms().end  ()
                ;++j
                )
                {
                (*j)[*i] = new_parms[*i].str();
                }
            for
                (j  = cell_parms().begin()
                ;j != cell_parms().end  ()
                ;++j
                )
                {
                (*j)[*i] = new_parms[*i].str();
                }
            }
        else
            {
            std::vector<Input>::iterator j;
            for
                (j  = cell_parms().begin()
                ;j != cell_parms().end  ()
                ;++j
                )
                {
                if((*j)["EmployeeClass"] == new_parms["EmployeeClass"])
                    {
                    (*j)[*i] = new_parms[*i].str();
                    }
                }
            }
        }

    // Probably this should be factored out into a member function
    // that's called elsewhere too--e.g., when a cell is read from
    // file, or when a census is pasted. For this to work fully as
    // desired, however, the DATABASE !! must be changed. Today,
    // it caches exactly one product, and its cache-invalidation
    // discipline isn't sufficiently strict. For now, applying the
    // present technique elsewhere might well exacerbate crosstalk
    // in a census that comprises more than one product.
    std::vector<Input>::iterator j;
    for(j = class_parms().begin(); j != class_parms().end(); ++j)
        {
        j->Reconcile();
        }
    for(j = cell_parms() .begin(); j != cell_parms() .end(); ++j)
        {
        j->Reconcile();
        }
    composite_is_available_ = false;
}

void CensusView::update_visible_columns()
{
    int width = autosize_columns_ ? wxCOL_WIDTH_AUTOSIZE : wxCOL_WIDTH_DEFAULT;

    list_window_->ClearColumns();

    // Column zero (cell serial number) is always shown.
    list_window_->AppendColumn
        (new(wx) wxDataViewColumn
            ("Cell"
            ,new(wx) wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT)
            ,CensusViewDataViewModel::Col_CellNum
            ,width
            ,wxALIGN_NOT
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
    std::vector<std::string>::const_iterator i;
    unsigned int column;
    for(i = all_headers.begin(), column = 0; i != all_headers.end(); ++i, ++column)
        {
        if
            (  column_value_varies_across_cells(*i, class_parms())
            || column_value_varies_across_cells(*i, cell_parms ())
            )
            {
            list_window_->AppendColumn
                (new(wx) wxDataViewColumn
                    (insert_spaces_between_words(*i)
                    ,new(wx) wxDataViewTextRenderer("string", wxDATAVIEW_CELL_INERT)
                    ,1 + column
                    ,width
                    ,wxALIGN_NOT
                    ,wxDATAVIEW_COL_RESIZABLE
                    )
                );
            }
        }
}

wxIcon CensusView::Icon() const
{
    return IconFromXmlResource("census_view_icon");
}

wxMenuBar* CensusView::MenuBar() const
{
    return MenuBarFromXmlResource("census_view_menu");
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

// Make each nonfrozen column wide enough to display its widest entry,
// ignoring column headers.
//
void CensusView::UponColumnWidthVarying(wxCommandEvent&)
{
    autosize_columns_ = true;

    wxWindowUpdateLocker u(list_window_);
    for(unsigned int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        list_window_->GetColumn(j)->SetWidth(wxCOL_WIDTH_AUTOSIZE);
        }
}

// Shrink all nonfrozen columns to default width.
void CensusView::UponColumnWidthFixed(wxCommandEvent&)
{
    autosize_columns_ = false;

    wxWindowUpdateLocker u(list_window_);
    for(unsigned int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        list_window_->GetColumn(j)->SetWidth(wxCOL_WIDTH_DEFAULT);
        }
}

void CensusView::UponRightClick(wxDataViewEvent&)
{
    wxMenu* census_menu = wxXmlResource::Get()->LoadMenu("census_menu_ref");
    LMI_ASSERT(census_menu);
    list_window_->PopupMenu(census_menu);
    delete census_menu;
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

    // All displayed data is valid when this function ends.
    all_changes_have_been_validated_ = true;
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
    if(is_invalid())
        {
        return;
        }

    ViewComposite();
}

void CensusView::UponRunCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

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
    // Run all cells if necessary to (re)generate composite numbers.
    if(!composite_is_available_)
        {
        if(!DoAllCells(mce_emit_nothing))
            {
            return;
            }
        }

    if(!was_cancelled_)
        {
        std::string const name("composite");
        IllustrationView& illview = MakeNewIllustrationDocAndView
            (document().GetDocumentManager()
            ,serial_file_path(base_filename(), name, -1, "ill").string().c_str()
            );

        // This is necessary for the view to be able to print.
        illview.SetLedger(composite_ledger_);

        illview.DisplaySelectedValuesAsHtml();
        }
}

bool CensusView::DoAllCells(mcenum_emission emission)
{
    assert_consistency(case_parms()[0], cell_parms()[0]);

    illustrator z(emission);
    if(!z(base_filename(), cell_parms()))
        {
        return false;
        }

    composite_ledger_ = z.principal_ledger();
    return true;
}

void CensusView::UponAddCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    cell_parms().push_back(case_parms()[0]);
    list_model_->RowAppended();

    Update();
    document().Modify(true);

    wxDataViewItem const& z = list_model_->GetItem(list_model_->GetCount() - 1);
    list_window_->UnselectAll();
    list_window_->Select(z);
    list_window_->EnsureVisible(z);
}

void CensusView::UponDeleteCells(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

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
    int z = wxMessageBox
        (oss.str()
        ,"Confirm deletion"
        ,wxYES_NO | wxICON_QUESTION
        );
    if(wxYES != z)
        {
        return;
        }

    wxArrayInt erasures;
    typedef wxDataViewItemArray::const_iterator dvci;
    for(dvci i = selection.begin(); i != selection.end(); ++i)
        {
        erasures.push_back(list_model_->GetRow(*i));
        }
    std::sort(erasures.begin(), erasures.end());

    LMI_ASSERT(cell_parms().size() == n_items);

    std::vector<Input> expurgated_cell_parms;
    expurgated_cell_parms.reserve
        (n_items - n_sel_items
        );

    for(unsigned int j = 0; j < cell_parms().size(); ++j)
        {
        if(!contains(erasures, j))
            {
            expurgated_cell_parms.push_back(cell_parms()[j]);
            }
        }
    LMI_ASSERT(expurgated_cell_parms.size() == n_items - n_sel_items);

//    cell_parms().swap(expurgated_cell_parms); // TODO ?? Would this be better?
    cell_parms() = expurgated_cell_parms;

    // Send notifications about changes to the wxDataViewCtrl model. Two things
    // changed: some rows were deleted and cell number of some rows shifted
    // accordingly.
    list_model_->RowsDeleted(erasures);
    for(unsigned int j = erasures.front(); j < cell_parms().size(); ++j)
        list_model_->RowValueChanged(j, CensusViewDataViewModel::Col_CellNum);

    unsigned int const newsel = std::min
        (static_cast<unsigned int>(erasures.front())
        ,cell_parms().size() - 1
        );
    wxDataViewItem const& y = list_model_->GetItem(newsel);
    list_window_->Select(y);
    list_window_->EnsureVisible(y);

    Update();
    document().Modify(true);
}

// Print tab-delimited output to file loadable in spreadsheet programs.
void CensusView::UponRunCaseToSpreadsheet(wxCommandEvent&)
{
    std::string spreadsheet_filename =
            base_filename()
        +   configurable_settings::instance().spreadsheet_file_extension()
        ;
    std::remove(spreadsheet_filename.c_str());
    DoAllCells(mce_emit_spreadsheet);
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

    // Read each subsequent line into an input object representing one cell.
    int current_line = 0;
    while(std::getline(iss_census, line, '\n'))
        {
        ++current_line;

        iss_census >> std::ws;

        Input current_cell(case_parms()[0]);

        std::istringstream iss_line(line);
        std::string token;
        std::vector<std::string> values;

        while(std::getline(iss_line, token, '\t'))
            {
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
// TODO ?? It would be better to use fatal_error() instead of
// warning() followed by fatal_error() with a short string, but
// apparently that can segfault with very long strings. Is there
// a limit on exception size that should be tested here?
                fatal_error() << "Invalid input." << LMI_FLUSH;
                }
            values.push_back(token);
            }

        if(values.size() != headers.size())
            {
            fatal_error()
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
        cell_parms().clear();
        class_parms().clear();
        class_parms().push_back(case_parms()[0]);
        }

    std::back_insert_iterator<std::vector<Input> > iip(cell_parms());
    std::copy(cells.begin(), cells.end(), iip);
    document().Modify(true);
    list_model_->Reset(cell_parms().size());
    Update();
    status() << std::flush;

    LMI_ASSERT(!case_parms ().empty());
    LMI_ASSERT(!cell_parms ().empty());
    LMI_ASSERT(!class_parms().empty());
}

