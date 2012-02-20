// Census manager--obsolescent listview version.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012 Gregory W. Chicares.
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

#include "census_view_old.hpp"

#include "alert.hpp"
#include "assert_lmi.hpp"
#include "census_document.hpp"
#include "configurable_settings.hpp"
#include "contains.hpp"
#include "default_view.hpp"
#include "illustration_view.hpp"
#include "illustrator.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp" // is_ok_for_cctype()
#include "mvc_controller.hpp"
#include "path_utility.hpp"
#include "safely_dereference_as.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp" // class ClipboardEx

#include <wx/icon.h>
#include <wx/listctrl.h>
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

// PORT !! Can't this macro be dispensed with?
#define ID_LISTWINDOW 23456

namespace
{
    // PORT !! Add description and unit tests; consider relocating,
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
}

IMPLEMENT_DYNAMIC_CLASS(CensusViewOld, ViewEx)

BEGIN_EVENT_TABLE(CensusViewOld, ViewEx)
    EVT_CONTEXT_MENU(                        CensusViewOld::UponRightClick)
    EVT_MENU(XRCID("edit_cell"             ),CensusViewOld::UponEditCell )
    EVT_MENU(XRCID("edit_class"            ),CensusViewOld::UponEditClass)
    EVT_MENU(XRCID("edit_case"             ),CensusViewOld::UponEditCase )
    EVT_MENU(XRCID("run_cell"              ),CensusViewOld::UponRunCell)
//    EVT_MENU(XRCID("run_class"             ),CensusViewOld::UponRunClass)   // SOMEDAY !! This may be useful for large cases.
    EVT_MENU(XRCID("run_case"              ),CensusViewOld::UponRunCase)
    EVT_MENU(XRCID("print_case"            ),CensusViewOld::UponPrintCase)
    EVT_MENU(XRCID("print_case_to_disk"    ),CensusViewOld::UponPrintCaseToDisk)
    EVT_MENU(XRCID("print_spreadsheet"     ),CensusViewOld::UponRunCaseToSpreadsheet)
    EVT_MENU(XRCID("paste_census"          ),CensusViewOld::UponPasteCensus)
    EVT_MENU(XRCID("add_cell"              ),CensusViewOld::UponAddCell)
    EVT_MENU(XRCID("delete_cells"          ),CensusViewOld::UponDeleteCells)
    EVT_MENU(XRCID("column_width_varying"  ),CensusViewOld::UponColumnWidthVarying)
    EVT_MENU(XRCID("column_width_fixed"    ),CensusViewOld::UponColumnWidthFixed)

// PORT !! There has to be a better way than this.
    EVT_UPDATE_UI(XRCID("edit_cell"            ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("edit_class"           ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("edit_case"            ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_cell"             ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_class"            ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_case"             ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_case"           ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_case_to_disk"   ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"    ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("paste_census"         ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("add_cell"             ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"         ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("column_width_varying" ),CensusViewOld::UponUpdateApplicable)
    EVT_UPDATE_UI(XRCID("column_width_fixed"   ),CensusViewOld::UponUpdateApplicable)
// PORT !! Not label-edit.
//    EVT_LIST_BEGIN_LABEL_EDIT(ID_LISTWINDOW,CensusViewOld::UponBeginLabelEdit)
// Don't do this either--it's triggered by spacebar.
//    EVT_LIST_ITEM_ACTIVATED(ID_LISTWINDOW  ,CensusViewOld::UponBeginLabelEdit)
END_EVENT_TABLE()

CensusViewOld::CensusViewOld()
    :ViewEx                          ()
    ,all_changes_have_been_validated_(true)
    ,composite_is_available_         (false)
    ,was_cancelled_                  (false)
    ,list_window_                    (0)
{
}

CensusViewOld::~CensusViewOld()
{
}

inline std::vector<Input>& CensusViewOld::case_parms()
{
    return document().doc_.case_parms_;
}

inline std::vector<Input> const& CensusViewOld::case_parms() const
{
    return document().doc_.case_parms_;
}

inline std::vector<Input>& CensusViewOld::cell_parms()
{
    return document().doc_.cell_parms_;
}

inline std::vector<Input> const& CensusViewOld::cell_parms() const
{
    return document().doc_.cell_parms_;
}

inline std::vector<Input>& CensusViewOld::class_parms()
{
    return document().doc_.class_parms_;
}

inline std::vector<Input> const& CensusViewOld::class_parms() const
{
    return document().doc_.class_parms_;
}

// PORT !! Is this abstraction actually useful?
std::string CensusViewOld::cell_title(int index)
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

// PORT !! Is this abstraction actually useful?
std::string CensusViewOld::class_title(int index)
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

// PORT !! Is this abstraction actually useful?
std::string CensusViewOld::class_name_from_cell_number(int cell_number) const
{
    return cell_parms()[cell_number]["EmployeeClass"].str();
}

Input* CensusViewOld::class_parms_from_class_name(std::string const& class_name)
{
    std::vector<Input>::iterator i = class_parms().begin();
    while(i != class_parms().end())
        {
        // PORT !! Add an any_member operator== instead.
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
bool CensusViewOld::column_value_varies_across_cells
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

wxWindow* CensusViewOld::CreateChildWindow()
{
    list_window_ = new(wx) wxListView
        (GetFrame()
        ,ID_LISTWINDOW
        );

    // Show headers.
    Update();
    document().Modify(false);

    status() << std::flush;

    return list_window_;
}

CensusDocument& CensusViewOld::document() const
{
    return safely_dereference_as<CensusDocument>(GetDocument());
}

    // Display exactly those columns whose rows aren't all identical. For
    // this purpose, consider as "rows" the individual cells--and also the
    // case and class defaults, even though they aren't displayed in rows.
    // Reason: although the case and class defaults are hidden, they're
    // still information--so if the user made them different from any cell
    // wrt some column, we respect that conscious decision.
//
// Only DisplayAllVaryingData() uses the data member this assigns,
// so move the logic into that function (if that remains true).
//
void CensusViewOld::identify_varying_columns()
{
    headers_of_varying_parameters_.clear();
    std::vector<std::string> const& all_headers(case_parms()[0].member_names());
    std::vector<std::string>::const_iterator i;
    for(i = all_headers.begin(); i != all_headers.end(); ++i)
        {
        if
            (  column_value_varies_across_cells(*i, class_parms())
            || column_value_varies_across_cells(*i, cell_parms ())
            )
            {
            headers_of_varying_parameters_.push_back(*i);
            }
        }
}

int CensusViewOld::edit_parameters
    (Input&             lmi_input
    ,std::string const& name
    )
{
    if(is_invalid())
        {
        return false;
        }

    bool dirty = document().IsModified();

    Input edited_lmi_input = lmi_input;
    DefaultView const default_view;
    MvcController controller(GetFrame(), edited_lmi_input, default_view);
    controller.SetTitle(name);
    int rc = controller.ShowModal();
    if(wxID_OK == rc)
        {
        if(lmi_input != edited_lmi_input)
            {
            lmi_input = edited_lmi_input;
            dirty = true;
            }
        document().Modify(dirty);
        }
    return rc;
}

bool CensusViewOld::is_invalid()
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
            // PORT !! Reserved for grid implementation.
            }
        }
    return false;
}

// PORT !! Reserved for a grid implementation.
int CensusViewOld::selected_column()
{
    return 0;
}

int CensusViewOld::selected_row()
{
// PORT !! Lossy type conversion: GetFirstSelected() returns a long
// int, here and elsewhere in this file.
    int row = list_window_->GetFirstSelected();
    if(row < 0)
        {
        row = 0;
// PORT !! Reserve for grid implementation.
//        fatal_error() << "No row selected." << LMI_FLUSH;
        }
    if(static_cast<int>(cell_parms().size()) <= row)
        {
// PORT !! OK if about to delete?
//        fatal_error() << "Invalid row selected." << LMI_FLUSH;
        }
    return row;
}

// Make a vector of all class names used by any individual, from
// scratch; and update the vector of class default parameters,
// adding any new classes, and purging any that are no longer in use
// by any cell.
void CensusViewOld::update_class_names()
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

// PORT !! need parms for each?
//    if find name in class array
//        OK
//    else
//        create: copy from first matching individual
// PORT !! and if unmatching element in class array: delete it?

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
            // PORT !! There has to be a nicer way to do this with STL.
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

void CensusViewOld::apply_changes
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

    // PORT !! temp string for new value, eeclass?
    // PORT !! combine class and indv vectors for case changes?

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

void CensusViewOld::DisplayAllVaryingData()
{
    // Column zero (cell serial number) is always shown.
    list_window_->InsertColumn(0, "Cell");
    for(unsigned int column = 0; column < headers_of_varying_parameters_.size(); ++column)
        {
        list_window_->InsertColumn
            (1 + column
            ,insert_spaces_between_words(headers_of_varying_parameters_[column])
            );
        }
    for(unsigned int row = 0; row < cell_parms().size(); ++row)
        {
        list_window_->InsertItem
            (row
            ,value_cast<std::string>(row)
            ,0
            );
        // PORT !! Necessary? Move to subfunction?
//        long index = ?
//        list_window_->SetItemData(index, row);

        list_window_->SetItem(row, 0, value_cast<std::string>(1 + row));

        for(unsigned int column = 0; column < headers_of_varying_parameters_.size(); ++column)
            {
            std::string s = cell_parms()[row][headers_of_varying_parameters_[column]].str();
            list_window_->SetItem(row, 1 + column, s);
            }
        }
}

wxIcon CensusViewOld::Icon() const
{
    return IconFromXmlResource("census_view_icon");
}

wxMenuBar* CensusViewOld::MenuBar() const
{
    return MenuBarFromXmlResource("census_view_menu");
}

///* TODO expunge?
// Double-click handler.
// Factor out code: exact duplicate of CensusViewOld::UponEditCell().
void CensusViewOld::UponBeginLabelEdit(wxListEvent& event)
{
    int cell_number = selected_row();
    Input& original_parms = cell_parms()[cell_number];
    Input temp_parms(original_parms);

    if(wxID_OK != edit_parameters(temp_parms, cell_title(cell_number)))
        {
        return;
        }

    // PORT !! Wouldn't it be better just to have edit_parameters()
    // say whether it changed anything?
    if(temp_parms != original_parms)
        {
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}
//*/

void CensusViewOld::UponEditCell(wxCommandEvent&)
{
    int cell_number = selected_row();
    Input& original_parms = cell_parms()[cell_number];
    Input temp_parms(original_parms);

    if(wxID_OK != edit_parameters(temp_parms, cell_title(cell_number)))
        {
        return;
        }

    // PORT !! Wouldn't it be better just to have edit_parameters()
    // say whether it changed anything?
    if(temp_parms != original_parms)
        {
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}

void CensusViewOld::UponEditClass(wxCommandEvent&)
{
    int cell_number = selected_row();
    std::string class_name = class_name_from_cell_number(cell_number);
    Input& original_parms = *class_parms_from_class_name(class_name);
    Input temp_parms(original_parms);

    if(wxID_OK != edit_parameters(temp_parms, class_title(cell_number)))
        {
        return;
        }

    if(!(temp_parms == original_parms))
        {
        int z = wxMessageBox
            ("Apply all changes to every cell in this class?"
            ,"Confirm changes"
            ,wxYES_NO | wxICON_QUESTION
            );
        if(wxYES == z)
            {
            apply_changes(temp_parms, original_parms, true);
            }
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}

void CensusViewOld::UponEditCase(wxCommandEvent&)
{
    Input& original_parms = case_parms()[0];
    Input temp_parms(original_parms);
    if(wxID_OK != edit_parameters(temp_parms, "Default parameters for case"))
        {
        return;
        }

    if(!(temp_parms == original_parms))
        {
        int z = wxMessageBox
            ("Apply all changes to every cell?"
            ,"Confirm changes"
            ,wxYES_NO | wxICON_QUESTION
            );
        if(wxYES == z)
            {
            apply_changes(temp_parms, original_parms, false);
            }
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}

// Make each nonfrozen column wide enough to display its widest entry,
// ignoring column headers.
//
// VZ note from sample program (is this true?):
// "note that under MSW for SetColumnWidth() to work we need to create the
// items with images initially even if we specify dummy image id"
//
// PORT !! Offer both ways of autosizing.
//
void CensusViewOld::UponColumnWidthVarying(wxCommandEvent&)
{
    wxWindowUpdateLocker u(list_window_);
    for(int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
// PORT !! Pick one, and remove the other?
//        list_window_->SetColumnWidth(j, wxLIST_AUTOSIZE);
        list_window_->SetColumnWidth(j, wxLIST_AUTOSIZE_USEHEADER);
        }
}

// Shrink all nonfrozen columns to default width.
void CensusViewOld::UponColumnWidthFixed(wxCommandEvent&)
{
    wxWindowUpdateLocker u(list_window_);
    for(int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        // WX !! Sad to hardcode '80', but that's the undocumented wx default.
        // PORT !! If it's a default, then why must it be specified?
        list_window_->SetColumnWidth(j, 80);
        }
}

void CensusViewOld::UponRightClick(wxContextMenuEvent&)
{
    wxMenu* census_menu = wxXmlResource::Get()->LoadMenu("census_menu_ref");
    LMI_ASSERT(census_menu);
    list_window_->PopupMenu(census_menu);
    delete census_menu;
}

void CensusViewOld::UponUpdateApplicable(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

// Update the spreadsheet display.
// If a parameter was formerly the same for all cells but now differs due
//  to editing, then display its column for all cells.
// If a column was previously displayed but is now the same for all cells
//  due to editing, then display it no longer.
// Similarly, if an old employee class is no longer used, remove it; and
//  if a new one comes into use, display it.
void CensusViewOld::Update()
{
    wxWindowUpdateLocker u(list_window_);

    list_window_->ClearAll();

    update_class_names();
    identify_varying_columns();
    DisplayAllVaryingData();

    // All displayed data is valid when this function ends.
    all_changes_have_been_validated_ = true;
}

void CensusViewOld::UpdatePreservingSelection()
{
    wxWindowUpdateLocker u(list_window_);

    // Save active cell.
    int selection = selected_row();
    int top_row = list_window_->GetTopItem();
// PORT !! Reserve for grid implementation.
//    int c = selected_column();

    Update();

    // Restore active cell.
    // PORT !! Better would be to restore to previously active col and row
    // as determined by col hdr and cell #.
    //
    // This is kind of nasty. There's no SetTopItem(). Maybe it can be
    // faked by 'ensuring' that the last row is visible first.
    selection = std::min(selection, list_window_->GetItemCount());
    list_window_->Select(selection);
    list_window_->EnsureVisible(list_window_->GetItemCount());
    list_window_->EnsureVisible(top_row);
    list_window_->EnsureVisible(selection);
}

void CensusViewOld::UponPrintCase(wxCommandEvent&)
{
    DoAllCells(mce_emit_pdf_to_printer);
}

void CensusViewOld::UponPrintCaseToDisk(wxCommandEvent&)
{
    DoAllCells(mce_emit_pdf_file);
}

void CensusViewOld::UponRunCase(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    ViewComposite();
}

void CensusViewOld::UponRunCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    int cell_number = selected_row();
    ViewOneCell(cell_number);
}

void CensusViewOld::ViewOneCell(int index)
{
    std::string const name(cell_parms()[index]["InsuredName"].str());
    IllustrationView& illview = MakeNewIllustrationDocAndView
        (document().GetDocumentManager()
        ,serial_file_path(base_filename(), name, index, "ill").string().c_str()
        );
    illview.Run(&cell_parms()[index]);
}

void CensusViewOld::ViewComposite()
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

bool CensusViewOld::DoAllCells(mcenum_emission emission)
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

void CensusViewOld::UponAddCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    cell_parms().push_back(case_parms()[0]);
    UpdatePreservingSelection();
    document().Modify(true);
}

void CensusViewOld::UponDeleteCells(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    unsigned int n_items = list_window_->GetItemCount();
    unsigned int n_sel_items = list_window_->GetSelectedItemCount();

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

    std::vector<unsigned int> erasures;
    int index = list_window_->GetFirstSelected();
    while(-1 != index)
        {
        erasures.push_back(index);
        index = list_window_->GetNextSelected(index);
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

//    cell_parms().swap(expurgated_cell_parms); // PORT !! Would this be better?
    cell_parms() = expurgated_cell_parms;

    Update();
    document().Modify(true);
}

// Print tab-delimited output to file loadable in spreadsheet programs.
void CensusViewOld::UponRunCaseToSpreadsheet(wxCommandEvent&)
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

void CensusViewOld::UponPasteCensus(wxCommandEvent&)
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
// PORT !! It would be better to use fatal_error() instead of
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
    Update();
    status() << std::flush;

    LMI_ASSERT(!case_parms ().empty());
    LMI_ASSERT(!cell_parms ().empty());
    LMI_ASSERT(!class_parms().empty());
}

