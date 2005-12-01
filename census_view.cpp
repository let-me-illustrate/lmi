// Census manager.
//
// Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//
// http://savannah.nongnu.org/projects/lmi
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: census_view.cpp,v 1.38 2005-12-01 04:06:34 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "census_view.hpp"

#include "alert.hpp"
#include "census_document.hpp"
#include "configurable_settings.hpp"
#include "illustration_view.hpp"
#include "input.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp" // is_ok_for_cctype()
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <wx/app.h>     // wxTheApp
#include <wx/clipbrd.h> // Used only by GetClipboardText().
#include <wx/dataobj.h> // Used only by GetClipboardText().
#include <wx/icon.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/xrc/xmlres.h>

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
}

IMPLEMENT_DYNAMIC_CLASS(CensusView, ViewEx)

BEGIN_EVENT_TABLE(CensusView, ViewEx)
// TODO ?? Experimental: (Ask Vadim whether wxGTK supports this.)
//    EVT_COMMAND_RIGHT_CLICK(ID_LISTWINDOW   ,CensusView::OnRightClick0)
// TODO ?? No effect--dunno why; but prolly wanted for gtk:
//    EVT_RIGHT_UP(                            CensusView::OnRightClick1)
    EVT_CONTEXT_MENU(                        CensusView::OnRightClick2)
    EVT_MENU(XRCID("edit_case"             ),CensusView::OnEditCase )
    EVT_MENU(XRCID("edit_cell"             ),CensusView::OnEditCell )
    EVT_MENU(XRCID("edit_class"            ),CensusView::OnEditClass)
    EVT_MENU(XRCID("print_cell"            ),CensusView::OnPrintCell)
//    EVT_MENU(XRCID("print_class"           ),CensusView::OnPrintClass) // TODO ?? expunge?
    EVT_MENU(XRCID("print_case"            ),CensusView::OnPrintCase)
    EVT_MENU(XRCID("run_cell"              ),CensusView::OnRunCell)
//    EVT_MENU(XRCID("run_class"             ),CensusView::OnRunClass) // TODO ?? expunge?
    EVT_MENU(XRCID("run_case"              ),CensusView::OnRunCase)
    EVT_MENU(XRCID("paste_census"          ),CensusView::OnPasteCensus)
    EVT_MENU(XRCID("add_cell"              ),CensusView::OnAddCell)
    EVT_MENU(XRCID("delete_cells"          ),CensusView::OnDeleteCells)
    EVT_MENU(XRCID("expand_columns"        ),CensusView::OnExpandColWidths)
    EVT_MENU(XRCID("shrink_columns"        ),CensusView::OnShrinkColWidths)
    EVT_MENU(XRCID("print_spreadsheet"     ),CensusView::OnRunCaseToSpreadsheet)

//    EVT_UPDATE_UI(XRCID("wxID_SAVEAS"      ),CensusView::OnUpdateFileSaveAs) // TODO ?? expunge

// TODO ?? There has to be a better way than this.
    EVT_UPDATE_UI(XRCID("edit_cell"        ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("edit_class"       ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),CensusView::OnUpdateApplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),CensusView::OnUpdateApplicable)
// TODO ?? Not label-edit.
//    EVT_LIST_BEGIN_LABEL_EDIT(ID_LISTWINDOW,CensusView::OnBeginLabelEdit)
// Don't do this either--it's triggered by spacebar.
//    EVT_LIST_ITEM_ACTIVATED(ID_LISTWINDOW  ,CensusView::OnBeginLabelEdit)
END_EVENT_TABLE()

CensusView::CensusView()
    :ViewEx                          ()
    ,all_changes_have_been_validated_(true)
    ,composite_is_available_         (false)
    ,was_canceled_                   (false)
    ,list_window_                    (0)
{
}

CensusView::~CensusView()
{
}

inline std::vector<Input>& CensusView::case_parms()
{
    return document().case_parms_;
}

inline std::vector<Input> const& CensusView::case_parms() const
{
    return document().case_parms_;
}

inline std::vector<Input>& CensusView::cell_parms()
{
    return document().cell_parms_;
}

inline std::vector<Input> const& CensusView::cell_parms() const
{
    return document().cell_parms_;
}

inline std::vector<Input>& CensusView::class_parms()
{
    return document().class_parms_;
}

inline std::vector<Input> const& CensusView::class_parms() const
{
    return document().class_parms_;
}

// TODO ?? Is this abstraction actually useful?
std::string CensusView::cell_title(int index)
{
// TODO ?? Another reason to use just one name.
//    std::string full_name(cell_parms()[index].InsdFullName());
    std::string full_name(cell_parms()[index]["InsuredName"].str());
    std::ostringstream title;
    title << "Parameters for cell " << (1 + index);
    if(!full_name.empty())
        {
        title << " (" << full_name << ")";
        }
    return title.str();
}

// TODO ?? Is this abstraction actually useful?
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

// TODO ?? Is this abstraction actually useful?
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
    (std::string const&                  header
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
    list_window_ = new(wx) wxListView
        (GetFrame()
        ,ID_LISTWINDOW
// TODO ?? expunge unless census name is to be specified as title.
// These are builtin defaults:
//        ,wxDefaultPosition
//        ,wxDefaultSize
//        ,wxLC_REPORT
//        ,wxDefaultValidator
//        ,"Census name here..."
        );

    // Show headers.
    Update();
    document().Modify(false);

    // TODO ?? Is this relevant?
    // Looks like the default is wider than we'd like (hardcoded number
    // in the following function). Do this or else "shrink" on a new
    // census will widen the columns.
////    CensusView::OnShrinkColWidths();

    status() << std::flush;

    return list_window_;
}

CensusDocument& CensusView::document() const
{
    return dynamic_cast<CensusDocument&>(*GetDocument());
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
void CensusView::identify_varying_columns()
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

int CensusView::edit_parameters
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

    XmlNotebook xml_notebook(GetFrame(), edited_lmi_input);
    xml_notebook.SetTitle(name);
    int rc = xml_notebook.ShowModal();
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

// TODO ?? Implement this optimization. Instead of simply calling
// Freeze() and Thaw() on the list view, consider a class that
// freezes when created on the stack and thaws when it goes out of
// scope, to guard against exceptions.
void CensusView::freeze(bool)
{
    return;
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
    int row = list_window_->GetFocusedItem();
    if(row < 0)
        {
        row = 0;
// TODO ?? Reserve for grid implementation.
//        fatal_error() << "No row selected." << LMI_FLUSH;
        }
    if(static_cast<int>(cell_parms().size()) <= row)
        {
// TODO ?? OK if about to delete?
//        fatal_error() << "Invalid row selected." << LMI_FLUSH;
        }
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
            std::vector<Input>::iterator j = cell_parms().begin();
            bool found = false;
            // TODO ?? There has to be a nicer way to do this with STL.
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

// TODO ?? Lowercase this. Why does it freeze() a GUI that it doesn't touch?
void CensusView::ApplyChanges
    (Input const& new_parms
    ,Input const& old_parms
    ,bool         for_this_class_only
    )
{
    freeze(true);

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

    composite_is_available_ = false;
    freeze(false);
}

void CensusView::DisplayAllVaryingData()
{
    // Column zero (cell serial number) is always shown.
    list_window_->InsertColumn(0, "Cell");
    for(unsigned int column = 0; column < headers_of_varying_parameters_.size(); ++column)
        {
        list_window_->InsertColumn
            (1 + column
            ,insert_spaces_between_words(headers_of_varying_parameters_[column]).c_str()
            );
        }
    for(unsigned int row = 0; row < cell_parms().size(); ++row)
        {
        list_window_->InsertItem
            (row
            ,value_cast<std::string>(row)
            ,0
            );
        // TODO ?? Necessary? Move to subfunction?
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

wxIcon CensusView::Icon() const
{
    return IconFromXmlResource("census_view_icon");
}

wxMenuBar* CensusView::MenuBar() const
{
    wxMenuBar* menu_bar = MenuBarFromXmlResource("census_view_menu");
    if(!menu_bar)
        {
        fatal_error() << "Unable to load menubar." << LMI_FLUSH;
        };
    return menu_bar;
}

///* TODO expunge?
// Double-click handler.
// Factor out code: exact duplicate of CensusView::OnEditCell().
void CensusView::OnBeginLabelEdit(wxListEvent& event)
{
    int cell_number = selected_row();
    Input& original_parms = cell_parms()[cell_number];
    Input temp_parms(original_parms);

    if(wxID_OK != edit_parameters(temp_parms, cell_title(cell_number)))
        {
        return;
        }

    // TODO ?? Wouldn't it be better just to have edit_parameters()
    // say whether it changed anything?
    if(temp_parms != original_parms)
        {
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}
//*/

void CensusView::OnEditCell(wxCommandEvent&)
{
    int cell_number = selected_row();
    Input& original_parms = cell_parms()[cell_number];
    Input temp_parms(original_parms);

    if(wxID_OK != edit_parameters(temp_parms, cell_title(cell_number)))
        {
        return;
        }

    // TODO ?? Wouldn't it be better just to have edit_parameters()
    // say whether it changed anything?
    if(temp_parms != original_parms)
        {
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}

void CensusView::OnEditClass(wxCommandEvent&)
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
            ApplyChanges(temp_parms, original_parms, true);
            }
        original_parms = temp_parms;
        UpdatePreservingSelection();
        document().Modify(true);
        }
}

void CensusView::OnEditCase(wxCommandEvent&)
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
            ApplyChanges(temp_parms, original_parms, false);
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
// TODO ?? Offer both ways of autosizing.
// TODO ?? 'shrink' and 'expand' don't do what they sound like.
//
void CensusView::OnExpandColWidths(wxCommandEvent&)
{
    freeze(true);
    for(int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
// TODO ?? Pick one, and remove the other?
//        list_window_->SetColumnWidth(j, wxLIST_AUTOSIZE);
        list_window_->SetColumnWidth(j, wxLIST_AUTOSIZE_USEHEADER);
        }

    freeze(false);
}

// Shrink all nonfrozen columns to default width.
void CensusView::OnShrinkColWidths(wxCommandEvent&)
{
    freeze(true);
    for(int j = 0; j < list_window_->GetColumnCount(); ++j)
        {
        // WX !! Sad to hardcode '80', but that's the undocumented wx default.
        // TODO ?? If it's a default, then why must it be specified?
        list_window_->SetColumnWidth(j, 80);
        }
    freeze(false);
}

// TODO ?? Right-click handlers: pick one approach, and remove failed experiments.

void CensusView::OnRightClick0(wxCommandEvent&)
{
    status() << "OnRightClick0()" << LMI_FLUSH;
    wxMenuBar* menu_bar = MenuBar();
    if(menu_bar)
        {
        int census_menu_index = menu_bar->FindMenu("Census");
        if(wxNOT_FOUND != census_menu_index)
            {
            status() << "Menu found." << LMI_FLUSH;
            wxMenu* census_menu = menu_bar->GetMenu(census_menu_index);
            list_window_->PopupMenu(census_menu, list_window_->GetPosition());
            }
        else
            {
            status() << "Menu not found." << LMI_FLUSH;
            }
        }
}

void CensusView::OnRightClick1(wxMouseEvent& e)
{
    status() << "OnRightClick1()" << LMI_FLUSH;
    wxMenuBar* menu_bar = MenuBar();
    if(menu_bar)
        {
        int census_menu_index = menu_bar->FindMenu("Census");
        if(wxNOT_FOUND != census_menu_index)
            {
            status() << "Menu found." << LMI_FLUSH;
            wxMenu* census_menu = menu_bar->GetMenu(census_menu_index);
            wxPoint* point = new wxPoint(e.m_x, e.m_y);
            list_window_->PopupMenu(census_menu, *point);
            }
        else
            {
            status() << "Menu not found." << LMI_FLUSH;
            }
        }
}

void CensusView::OnRightClick2(wxContextMenuEvent& e)
{
    wxMenuBar* menu_bar = MenuBar();
    if(menu_bar)
        {
        int census_menu_index = menu_bar->FindMenu("Census");
        if(wxNOT_FOUND != census_menu_index)
            {
            wxMenu* census_menu = menu_bar->GetMenu(census_menu_index);
            list_window_->PopupMenu(census_menu, GetFrame()->ScreenToClient(e.GetPosition()));
            }
        else
            {
            status() << "Menu not found." << LMI_FLUSH;
            }
        }
}

void CensusView::OnUpdateApplicable(wxUpdateUIEvent& e)
{
    e.Enable(true);
}

void CensusView::OnUpdateFileSaveAs(wxUpdateUIEvent& e)
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
void CensusView::Update()
{
    freeze(true);

    list_window_->ClearAll();

    update_class_names();
    identify_varying_columns();
    DisplayAllVaryingData();

    // All displayed data is valid when this function ends.
    all_changes_have_been_validated_ = true;

    freeze(false);
}

void CensusView::UpdatePreservingSelection()
{
    freeze(true);

    // Save active cell.
    int selection = selected_row();
    int top_row = list_window_->GetTopItem();
// TODO ?? Reserve for grid implementation.
//    int c = selected_column();

    Update();

    // Restore active cell.
    // TODO ?? Better would be to restore to previously active col and row
    // as determined by col hdr and cell #.
    //
    // This is kind of nasty. There's no SetTopItem(). Maybe it can be
    // faked by 'ensuring' that the last row is visible first.
    selection = std::min(selection, list_window_->GetItemCount());
    list_window_->Select(selection);
    list_window_->EnsureVisible(list_window_->GetItemCount());
    list_window_->EnsureVisible(top_row);
    list_window_->EnsureVisible(selection);

    freeze(false);
}

void CensusView::OnPrintCell(wxCommandEvent&)
{
// TODO ?? Refactor.
    if(is_invalid())
        {
        return;
        }

    int cell_number = selected_row();
    freeze(true);
    // TODO ?? Is it desirable to create a view here, or would it be
    // better to print invisibly? If the latter, then probably
    // ViewOneCell() could be simplified to return void.
    ViewOneCell(cell_number).Pdf("print");

    freeze(false);
}

void CensusView::OnPrintCase(wxCommandEvent&)
{
    DoAllCells(emit_to_printer);
}

void CensusView::OnRunCase(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    ViewComposite();
}

void CensusView::OnRunCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    int cell_number = selected_row();
    ViewOneCell(cell_number);
}

IllustrationView& CensusView::ViewOneCell(int index)
{
    std::string file_name(serial_filename(index, "ill"));
    IllustrationView& illview = MakeNewIllustrationDocAndView
        (document().GetDocumentManager()
        ,file_name.c_str()
        );
    illview.Run(&cell_parms()[index]);
    return illview;
}

void CensusView::ViewComposite()
{
    // Run all cells if necessary to (re)generate composite numbers.
    if(!composite_is_available_)
        {
        if(!DoAllCells(emit_to_nowhere))
            {
            return;
            }
        }

    if(!was_canceled_)
        {
        std::string file_name(serial_filename(-1, "ill"));
        IllustrationView& illview = MakeNewIllustrationDocAndView
            (document().GetDocumentManager()
            ,file_name.c_str()
            );

        // This is necessary for the view to be able to print.
        illview.SetLedger(composite_ledger_);

        illview.DisplaySelectedValuesAsHtml();
        }
}

bool CensusView::DoAllCells(e_emission_target emission_target)
{
    mce_run_order order0 = (case_parms()[0]["RunOrder"]).cast<mce_run_order>();
    mce_run_order order1 = (cell_parms()[0]["RunOrder"]).cast<mce_run_order>();
    if(order0 != order1)
        {
        fatal_error()
            << "Case-default run order '"
            << order0
            << "' differs from first cell's run order '"
            << order1
            << "'. Make them consistent then run again."
            << LMI_FLUSH
            ;
        }

    // TODO ?? The census document already stores the cells as objects
    // of the class they're turned into here. They might be accessed
    // directly as such, but instead the old input class should be
    // completely replaced with the new. In the meantime, a progress
    // dialog might be wanted here.
    std::vector<IllusInputParms> cells;
    cells.reserve(cell_parms().size());
    for(unsigned int j = 0; j < cell_parms().size(); ++j)
        {
        IllusInputParms cell;
        convert_to_ihs(cell, cell_parms()[j]);
        cells.push_back(cell);
        }
    run_census runner;
    if(!runner(base_filename(), emission_target, cells))
        {
        return false;
        }
    composite_ledger_ = runner.composite();
    return true;
}

void CensusView::OnAddCell(wxCommandEvent&)
{
    if(is_invalid())
        {
        return;
        }

    cell_parms().push_back(case_parms()[0]);
    UpdatePreservingSelection();
    document().Modify(true);
}

void CensusView::OnDeleteCells(wxCommandEvent&)
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
        (oss.str().c_str()
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
        if(erasures.end() == std::find(erasures.begin(), erasures.end(), j))
            {
            expurgated_cell_parms.push_back(cell_parms()[j]);
            }
        }
    LMI_ASSERT(expurgated_cell_parms.size() == n_items - n_sel_items);

//    cell_parms().swap(expurgated_cell_parms); // TODO ?? Would this be better?
    cell_parms() = expurgated_cell_parms;

    Update();
    document().Modify(true);
}

namespace
{
// TODO ?? Does this deserve to be a member now?
void diagnose_sequence_string_problems(IllusInputParms& input)
{
    // TODO ?? We could pass the input row number, to print it here.
    // Messages don't say which life is the problem.
    std::vector<std::string> errors = input.realize_all_sequence_strings(false);
    for
        (std::vector<std::string>::iterator i = errors.begin()
        ;i != errors.end()
        ;++i
        )
        {
        std::ostringstream oss;
        bool diagnostics_present = false;
        if(!i->empty())
            {
            diagnostics_present = true;
            oss << (*i) << "\r\n";
            }
        if(diagnostics_present)
            {
            fatal_error()
                << "Input validation problems:\n"
                << oss.str()
                << LMI_FLUSH
                ;
            }
        }
}
} // Unnamed namespace.

// INELEGANT !! Shouldn't this be in a different translation unit?

namespace
{
// Based on 2005-03-14T13:12:15 +0100 email from <vadim@tt-solutions.com>.
std::string GetClipboardText()
{
    std::string s;
    // Lock opens clipboard in ctor, and closes it in dtor.
    wxClipboardLocker lock;
    if(!lock)
        {
        warning() << "Unable to acquire lock for clipboard." << LMI_FLUSH;
        }
    else if(!wxTheClipboard->IsSupported(wxDF_TEXT))
        {
        warning() << "Clipboard does not support text format." << LMI_FLUSH;
        }
    else
        {
        wxTextDataObject z;
        wxTheClipboard->GetData(z);
        s = z.GetText().c_str();
        }

    return s;
}
} // Unnamed namespace.

// Print tab-delimited output to file loadable in spreadsheet programs.
void CensusView::OnRunCaseToSpreadsheet(wxCommandEvent&)
{
    std::string spreadsheet_filename =
            base_filename()
        +   configurable_settings::instance().spreadsheet_file_extension()
        ;
    std::remove(spreadsheet_filename.c_str());
    DoAllCells(emit_to_spreadsheet);
}

void CensusView::OnPasteCensus(wxCommandEvent&)
{
// TODO ?? expunge debugging code in this function.

    // A brand-new census contains one default cell, which doesn't
    // represent user input, so we should erase it before pasting
    // new cells from the clipboard.
    //
    // But if the census contains any user input, we shouldn't
    // erase it. User input is present if either the document has
    // been modified (this covers the case where the default cell
    // in a new census has been changed) or the document was loaded
    // from a file (even if it was saved with only an unmodified
    // default cell, we assume that the contents of any explicitly-
    // saved file represent user intention). In this case, users
    // say they want to append pasted data to the cells that are
    // already present.
    //
    // We do all of this as soon as we know that the user wants to
    // paste census data. If pasting fails, the user is left with a
    // census that contains no cells, but that's OK; in this event,
    // they're likely to correct the data and repaste it.
    //
    // In the solution domain:
    //   document modified <--> dirty flag set
    //   loaded from file  <--> document path not null
// TODO ?? WX PORT !! How to do this with wx?
//    if(!document().IsModified() && 0 == document().GetDocPath())
//    if(!document().IsModified())
        {
        cell_parms().clear();
        class_parms().clear();
        class_parms().push_back(case_parms()[0]);
        }

    std::string census_data = GetClipboardText();

    std::vector<std::string> headers;
    std::vector<Input> cells;

    std::istringstream iss_census(census_data, std::ios_base::in);
// warning() << census_data << "entire clipboard text" << LMI_FLUSH;
    std::string line;

    // Get header line; parse into field names.
    if(std::getline(iss_census, line, '\r'))
        {
// warning() << "'" << line << "'" << "header line" << LMI_FLUSH;
        iss_census >> std::ws;

        std::istringstream iss_line(line, std::ios_base::in);
        std::string token;

        while(std::getline(iss_line, token, '\t'))
            {
// warning() << "'" << token << "'" << "push_back header" << LMI_FLUSH;
            headers.push_back(token);
            }
        }
    else
        {
        warning()
            << "Error pasting census data: no header line."
            << LMI_FLUSH
            ;
        return;
        }

    // Read each subsequent line into an input object representing one cell.
    int current_line = 0;
    while(std::getline(iss_census, line, '\r'))
        {
// warning() << "'" << line << "'" << "processing data line" << LMI_FLUSH;
        ++current_line;

        iss_census >> std::ws;

IllusInputParms ihs_input;
convert_to_ihs(ihs_input, case_parms()[0]);
// TODO ?? expunge?
//        IllusInputParms input(case_parms()[0]);

        std::istringstream iss_line(line, std::ios_base::in);
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
// warning() << "'" << token << "'" << "push_back value" << LMI_FLUSH;
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
// warning() << "'" << headers[j] << "'" << " setting variable..." << LMI_FLUSH;
// warning() << "'" << values[j] << "'" << " to value..." << LMI_FLUSH;
//            input[headers[j]] = values[j];
            ihs_input[std::string(headers[j].c_str())] = values[j];
// warning() << "OK...next value..." << LMI_FLUSH;
            }
// warning() << "diagnosing sequence string problems..." << LMI_FLUSH;
        diagnose_sequence_string_problems(ihs_input);
        ihs_input.propagate_changes_to_base_and_finalize();
Input lmi_input;
convert_from_ihs(ihs_input, lmi_input);
        cells.push_back(lmi_input);
// warning() << "Life added." << LMI_FLUSH;

        status() << "Added cell number " << cells.size() << '.' << std::flush;
        wxSafeYield();
        }

    if(0 == current_line)
        {
        warning() << "No cells to paste." << LMI_FLUSH;
        return;
        }

    std::back_insert_iterator<std::vector<Input> > iip
        (cell_parms()
        );
    std::copy(cells.begin(), cells.end(), iip);

    document().Modify(true);

    Update();

    status() << std::flush;
}

