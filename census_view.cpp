// Life insurance illustrations.
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

// $Id: census_view.cpp,v 1.4 2005-03-24 15:51:25 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "census_view.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "census_document.hpp"
#include "database.hpp"
#include "dbnames.hpp"
#include "global_settings.hpp"
#include "illustration_view.hpp"
#include "input.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "materially_equal.hpp"
#include "math_functors.hpp"
#include "miscellany.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/shared_ptr.hpp>

#include <wx/app.h> // wxTheApp
#include <wx/icon.h>
#include <wx/listctrl.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/xrc/xmlres.h>

#include <iomanip>
#include <sstream>
#include <stdexcept>

// TODO ?? Can't this macro be dispensed with?
#define ID_LISTWINDOW 12345

namespace
{
    bool already_reported_error;

    int progress_dialog_style =
            wxPD_APP_MODAL
        |   wxPD_AUTO_HIDE
        |   wxPD_CAN_ABORT
        |   wxPD_ELAPSED_TIME
        |   wxPD_ESTIMATED_TIME
        |   wxPD_REMAINING_TIME
        ;

    // TODO ?? Add description and unit tests; consider relocating.
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
    ,composite_ledger_               (e_ledger_type(e_nasd))
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

    // Skip GUI updates for non-interactive regression testing.
    if(global_settings::instance().regression_testing())
        {
        std::ofstream os
            (GetCRCFilename().c_str()
            ,std::ios_base::out | std::ios_base::trunc
            );
        os.close();
        DoAllCells(to_crc_file);
        if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasyU"))
            {
            DoAllCells(to_spreadsheet);
            }
        document().Modify(false);
        // Close view and doc when done.
        wxTheApp->ExitMainLoop();
        return 0;
        }

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
    int column = 1; // TODO ?? Not used: expunge
    for(i = all_headers.begin(); i != all_headers.end(); ++i)
        {
        if
            (  column_value_varies_across_cells(*i, class_parms())
            || column_value_varies_across_cells(*i, cell_parms ())
            )
            {
            headers_of_varying_parameters_.push_back(*i);
            ++column;
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
//        throw std::logic_error("No row selected.");
        }
    if(static_cast<int>(cell_parms().size()) <= row)
        {
// TODO ?? OK if about to delete?
//        throw std::logic_error("Invalid row selected.");
//status() << "Invalid row selected." << LMI_FLUSH;
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
            std::vector<Input>::iterator j =
                cell_parms().begin();
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
                hobsons_choice()
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
/* TODO ?? expunge?
std::ostringstream oss;
std::ostream_iterator<std::string> osi(oss, "\r\n");
std::copy(headers_of_changed_parameters.begin(), headers_of_changed_parameters.end(), osi);
warning() << oss.str().c_str() << "Changed parameters" << LMI_FLUSH;
*/
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

wxIcon const& CensusView::Icon() const
{
// TODO ?? Change that icon.
    static wxIcon const icon = wxICON(IDI_CENSUS_VIEW);
    return icon;
}

wxMenuBar* CensusView::MenuBar() const
{
    wxMenuBar* menu_bar = MenuBarFromXmlResource("census_view_menu");
    if(!menu_bar)
        {
// TODO ?? Clean up.        
//        wxLogError("Unable to load 'census_view_menu'.");
//        wxLog::FlushActive();
        throw std::runtime_error("Unable to load 'census_view_menu'.");
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
    ViewOneCell(cell_number)->Pdf("print");

    freeze(false);
}

void CensusView::OnPrintCase(wxCommandEvent&)
{
    DoAllCells(to_printer);
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

IllustrationView* CensusView::MakeNewDocAndView(char const* filename)
{
// WX !! Help for wxView says GetDocumentManager() returns a
// wxDocumentManager*, but it actually returns a wxDocManager* .
    wxDocManager* dm = document().GetDocumentManager();
    // We don't want this document to show up in the MRU list
    //   so we create it with the dtNewDoc flag
    //   and call SetTitle() rather than SetDocPath()
/*
// TODO ?? Is this still necessary?
    // Note that we're creating a child document here [wx: or so we wish]
    wxDocument* new_document = dm->CreateDocument
        (filename
        ,wxDOC_SILENT | 8 // | wxDOC_NEW
        );
//        (dm->MatchTemplate(filename)
//        ,filename
//        ,&document()
//        ,dtNewDoc
//        );
*/

// Keep it out of the file history.
// TODO ?? Using '8' to indicate 'child document' (a concept that wx
// does not support) is brittle at best.
wxDocTemplate* the_template = dm->FindTemplateForPath(filename);
wxDocument* new_document = the_template->CreateDocument(filename, wxDOC_SILENT | 8);

    IllustrationDocument* illdoc = dynamic_cast<IllustrationDocument*>(new_document);
    if(0 == illdoc)
        {
        fatal_error() << "dynamic_cast<IllustrationDocument*> failed." << LMI_FLUSH;
        return 0;
        }

    // TODO ?? Why do we need both of these?
    new_document->SetTitle(filename);
    new_document->SetFilename(filename);

    new_document->Modify(false);
    new_document->SetDocumentSaved(true);

// TODO ?? expunge?
////
//    IllusInputParms Parms = cell_parms()[idx];
//    *illdoc->inputctrl = Parms;
////
    IllustrationView* illview = 0;
    while(wxList::compatibility_iterator node = new_document->GetViews().GetFirst())
        {
        if(node->GetData()->IsKindOf(CLASSINFO(IllustrationView)))
            {
            illview = dynamic_cast<IllustrationView*>(node->GetData());
            break;
            }
        node = node->GetNext();
        }

    if(0 == illview)
        {
        fatal_error() << "dynamic_cast<IllustrationView*> failed." << LMI_FLUSH;
        return 0;
        }
    return illview;

// TODO ?? Buttons should be disabled here.
}

IllustrationView* CensusView::ViewOneCell(int index)
{
    // Name file based on index origin one, as expected by users.
    std::string file_name(serial_filename(1 + index, "ill"));

    // Create a document with that filename.
    // Get a ptr to the resulting view.
    IllustrationView* illview = MakeNewDocAndView(file_name.c_str());

    // Plug in parameters for this cell.
// TODO ?? expunge?
//    illview->SetIllusInput(cell_parms()[index]);
//    illview->document().input_ = cell_parms()[index];

    // Run the cell.
    illview->Run(&cell_parms()[index]);
    return illview;
}

IllustrationView* CensusView::ViewComposite()
{
// TODO ?? expunge?
//    if(is_invalid())
//        return;

    // Run all cells if necessary to (re)generate composite numbers.
    if(!composite_is_available_)
        {
        if(!DoAllCells(to_nowhere))
            {
            return 0;
            }
        }

    // Create a document with that filename.
    // Get a ptr to the resulting view.
    if(!was_canceled_)
        {
        std::string file_name = base_filename() + "composite.ill";
        IllustrationView* illview = MakeNewDocAndView(file_name.c_str());

        // Plug in composite totals.
        illview->SetLedger(composite_ledger_);
        illview->FormatSelectedValuesAsHtml();

        return illview;
        }
    else
        {
        return 0;
        }
}

//////////////////////////////////////////////////////////////////////////////

// The following functions probably should be factored out into a
// calculation-only module.

bool CensusView::DoAllCells(e_output_dest a_OutputDest)
{
    int number_of_cells = 0;
    std::vector<Input>::iterator ip;
    for(ip = cell_parms().begin(); ip != cell_parms().end(); ++ip)
        {
        if("Yes" != (*ip)["IncludeInComposite"].str())
            continue;
        number_of_cells += value_cast<int>((*ip)["NumberOfIdenticalLives"].str());
        }
    if(0 == number_of_cells)
        {
        warning() << "No cells to include in composite." << LMI_FLUSH;
        return false; // Return code apparently unused.
        }

// The run order depends on the case input and ignores any conflicting
// input for any individual cell. Perhaps we should detect any such
// conflicting input and signal an error? It would probably be cleaner
// to offer this input item (and a few similar ones) only at the case
// level. TODO ?? Fix this.
mce_run_order order = (case_parms()[0]["RunOrder"]).cast<mce_run_order>();
switch(order.ordinal())
// TODO ?? expunge
//    switch((case_parms()[0]["RunOrder"]).cast<mce_run_order>().ordinal())
        {
        // Perhaps this function should be run only in the month by month
        // case, but it does no harm to generalize it this way.
        case e_life_by_life:
            {
            RunAllLives(a_OutputDest);
            }
            break;
        case e_month_by_month:
            {
            RunAllMonths(a_OutputDest);
            }
            break;
        default:
            {
            fatal_error()
                << "Case '"
                << order
                << "' not found."
                << LMI_FLUSH
                ;
            }
        }
    return true;
}

// Since the ordering of calculations--by life or by month--has nothing
// to do with the GUI, this should be put in a non-GUI class.

void CensusView::RunAllLives(e_output_dest a_OutputDest)
{
    // Reports based on # cells, not # cells actually used, which may
    // differ because not all are included in composite.
    std::ostringstream progress_message;
    progress_message << "Completed " << 0 << " of " << cell_parms().size();
    wxProgressDialog Progress
        ("Calculating all cells"
        ,progress_message.str().c_str()
        ,cell_parms().size()
        ,wxTheApp->GetTopWindow()
        ,progress_dialog_style
        );

    // TODO ?? Want youngest cell instead of first cell.
IllusInputParms ihs_input0;
convert_to_ihs(ihs_input0, cell_parms()[0]);
// TODO ?? expunge?
//    TDatabase temp_db(cell_parms()[0]);
    TDatabase temp_db(ihs_input0);
    e_ledger_type ledger_type
        (static_cast<enum_ledger_type>
            (static_cast<int>(temp_db.Query(DB_LedgerType))
            )
        );
    if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasy9"))
        {
        ledger_type = e_ledger_type(e_prospectus);
        }
    Ledger Composite
        (ledger_type
        ,100
        ,true
        );

    Timer timer;
    was_canceled_ = false;
    for(unsigned int j = 0; j < cell_parms().size(); ++j)
        {
IllusInputParms Parms;
convert_to_ihs(Parms, cell_parms()[j]);
// TODO ?? expunge?
//        IllusInputParms Parms = cell_parms()[j];
        // Skip anyone not included in composite
        if(!Parms.IncludeInComposite)
            continue;

// TODO ?? expunge try-catch?
//        try
            {
            AccountValue AV(Parms);
            // Name files based on index origin one, reserving zero
            // for the composite.
            AV.SetDebugFilename    (serial_filename(1 + j, "debug"));

// TODO ?? expunge?
//            double value =
            AV.RunAV();
// TODO ?? expunge?
//            SetItemText(j, 3, value); // !! Would be nice to show result, but where?

            Composite.PlusEq(AV.LedgerValues());

            EmitEveryone(a_OutputDest, AV.LedgerValues(), j);
            }
//        catch([various exception types])
//            {
//            throw;
//            }

        progress_message.str("");
        progress_message << "Completed " << 1 + j << " of " << cell_parms().size();
        was_canceled_ = !Progress.Update
            (1 + j
            ,progress_message.str().c_str()
            );
        if(was_canceled_)
            {
            break;
            }
        }

    status() << timer.Stop().Report() << std::flush;

    if(!was_canceled_)
        {
        composite_ledger_ = Composite; // TODO ?? Then why do we need both?
        // Insert total specamt and payments.
//        composite_is_available_ = true;
        composite_is_available_ = false;    // TODO ?? Doesn't work.
        EmitEveryone(a_OutputDest, composite_ledger_, -1);
        }
}

void CensusView::RunAllMonths(e_output_dest a_OutputDest)
{
already_reported_error = false;

    std::ostringstream progress_message;
    progress_message << "Completed " << 0 << " of " << cell_parms().size();
    wxProgressDialog Progress
        ("Initializing all cells"
        ,progress_message.str().c_str()
        ,cell_parms().size()
        ,wxTheApp->GetTopWindow()
        ,progress_dialog_style
        );

    was_canceled_ = false;
    Timer timer;

    // Set up array of AccountValue objects for everyone in composite.
    // We might have kept this array for the lifetime of its owner
    // ("this"). Advantage: results have already been calculated and
    // would just need to be loaded. Disadvantage: any input change
    // could invalidate one cell, a set of cells, or the whole set,
    // and a bug in code to maintain validity could cause invalid
    // results to be printed--a silent error. Conclusion: rerunning a
    // cell when needed is fast enough, so the array is local to this
    // function, and we can drop cells out as they mature or lapse.
    // If we drop them, call FinalizeLife()... !!
    std::vector
        <boost::shared_ptr
            <AccountValue
            >
        > AVS;
    std::vector<Input>::iterator ip;
// TODO ?? expunge try-catch?
//    try
        {
        int j = 0;
        int first_cell_inforce_year  = value_cast<int>((*cell_parms().begin())["InforceYear"].str());
        int first_cell_inforce_month = value_cast<int>((*cell_parms().begin())["InforceMonth"].str());
        bool inforce_durations_differ = false;
        AVS.reserve(cell_parms().size());
        for(ip = cell_parms().begin(); ip != cell_parms().end(); ++ip, ++j)
            {
            // Skip any cell with zero lives.
            // TODO ?? Should that even be permitted?
            if(0 == value_cast<int>((*ip)["NumberOfIdenticalLives"].str()))
                continue;
            // Skip anyone not included in composite.
            if("Yes" != (*ip)["IncludeInComposite"].str())
                continue;

IllusInputParms Parms;
convert_to_ihs(Parms, *ip);
            boost::shared_ptr<AccountValue> AV(new AccountValue(Parms));
            // Name files based on index origin one, reserving zero
            // for the composite.
            AV->SetDebugFilename    (serial_filename(1 + j, "debug"));
            AVS.push_back(AV);

            if
                (   !global_settings::instance().ash_nazg
                &&  (   first_cell_inforce_year  != value_cast<int>((*ip)["InforceYear"].str())
                    ||  first_cell_inforce_month != value_cast<int>((*ip)["InforceMonth"].str())
                    )
                )
                {
                inforce_durations_differ = true;
                }

            progress_message.str("");
            progress_message << "Completed " << 1 + j << " of " << cell_parms().size();
            was_canceled_ = !Progress.Update
                (1 + j
                ,progress_message.str().c_str()
                );
            if(was_canceled_)
                {
                break;
                }
            }
        if(inforce_durations_differ)
            {
            hobsons_choice()
                << "Running census by month untested for inforce"
                << " with inforce duration varying across cells."
                << LMI_FLUSH
                ;
            }
        }
//    catch([various exception types])
//        {
//        throw;
//        }

    if(was_canceled_)
        {
        return;
        }

    // We're through with the above progress dialog, so get rid of it.
    // Without this step, it'd remain on the screen until this function
    // returns--showing a Cancel button that would do nothing.
// TODO ?? Is this a real problem?
//    Progress.Destroy();

// TODO ?? expunge?
//    std::vector<AccountValue>::iterator i;
    std::vector
        <boost::shared_ptr
            <AccountValue
            >
        >::iterator i;

    // TODO ?? Want youngest cell instead of first cell.
IllusInputParms ihs_input0;
convert_to_ihs(ihs_input0, cell_parms()[0]);
    TDatabase temp_db(ihs_input0);
    e_ledger_type ledger_type
        (static_cast<enum_ledger_type>
            (static_cast<int>(temp_db.Query(DB_LedgerType))
            )
        );
    if(std::string::npos != cell_parms()[0]["Comments"].str().find("idiosyncrasy9"))
        {
        ledger_type = e_ledger_type(e_prospectus);
        }

    Ledger Composite
        (ledger_type
        ,100
        ,true
        );

    int year_average_age_first_exceeds_80 = 0;

    {
    std::ofstream ofs
        ("experience_rating"
        ,std::ios_base::out | std::ios_base::trunc
        );
    ofs
        << '\t' << "year"
        << '\t' << "1+i(12)/12"
        << '\t' << "inforce"
        << '\t' << "coi"
        << '\t' << "cum_coi"
        << '\t' << "claims"
        << '\t' << "cum_claims"
        << '\t' << "ibnr_mos"
        << '\t' << "ibnr"
        << '\t' << "k"
        << '\n'
        ;
    }

    std::vector<e_run_basis> const& RunBases = Composite.GetRunBases();
    for
        (std::vector<e_run_basis>::const_iterator run_basis = RunBases.begin()
        ;run_basis != RunBases.end()
        ;++run_basis
        )
// TODO ?? expunge try-catch?
//    try
        {
        if(was_canceled_)
            {
            break;
            }

        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->GuessWhetherFirstYearPremiumExceedsRetaliationLimit();
            }
restart:
        // Initialize each cell.
        // Calculate duration when the youngest one ends.
        int MaxYr = 0;
        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->InitializeLife(*run_basis);
            MaxYr = std::max(MaxYr, (*i)->GetLength());
            }

    // TODO ?? WANT MONTHLY, NOT YEARLY? Why store it at all?
    // Perhaps use it for individual-cell solves?
        std::vector<double> Assets(MaxYr, 0.0);

        std::ostringstream progress_message;
        progress_message << "Completed " << 0 << " of " << cell_parms().size();
        wxProgressDialog Progress
            (run_basis->str().c_str()
            ,progress_message.str().c_str()
            ,MaxYr
            ,wxTheApp->GetTopWindow()
            ,progress_dialog_style
            );
// TODO ?? expunge        Progress->Create(); // Modeless.

        // Experience rating mortality reserve.
        double case_accum_net_mortchgs = 0.0;
        double case_accum_net_claims   = 0.0;
        double case_k_factor           = 0.0;

        double case_ibnr_months = temp_db.Query(DB_ExpRatIBNRMult);

        double CaseExpRatReserve = 0.0;
        double check_;

        {
        std::ofstream ofs
            ("experience_rating"
            ,std::ios_base::out | std::ios_base::ate | std::ios_base::app
            );
        ofs << *run_basis << '\n';
        }

        // Experience rating as implemented here requires a general-account
        // rate. Because that rate might vary across cells, the case-level
        // rate must be used. Extend its last element if it doesn't have
        // enough values.
        std::vector<double> general_account_rate;
        std::copy
            (ihs_input0.GenAcctRate.begin()
            ,ihs_input0.GenAcctRate.end()
            ,std::back_inserter(general_account_rate)
            );
        general_account_rate.resize(MaxYr, general_account_rate.back());

        std::vector<double> experience_reserve_rate = general_account_rate;

        // TODO ?? We don't start at InforceYear, because issue years may
        // differ between cells and we have not coded support for that yet.
        for(int year = 0; year < MaxYr; ++year)
            {
            double case_years_net_claims = 0.0;
            double case_years_net_mortchgs = 0.0;

            double CaseYearsCOICharges = 0.0;
            double CaseYearsIBNR = 0.0;

            double ExpRatMlyInt = 0.0;
            double experience_reserve_monthly_u =
                1.0 + i_upper_12_over_12_from_i<double>()
                    (experience_reserve_rate[year]
                    );

            // Process one month at a time for all cells.
            for(int month = 0; month < 12; ++month)
                {
                // Initialize year's assets to zero.
                // TODO ?? Uh--it already is, yearly...but this is monthly.
                // TODO ?? Perhaps we'll want a vector of monthly assets.
                Assets[year] = 0.0;

                // Get total case assets prior to interest crediting because
                // those assets determine the M&E charge.
{
////std::ofstream ofs("experience_rating", std::ios_base::out | std::ios_base::ate | std::ios_base::app);
                // Process transactions through monthly deduction.
                double current_mortchg = 0.0;
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    (*i)->Year = year;
                    (*i)->Month = month;
                    (*i)->CoordinateCounters();
                    if((*i)->PrecedesInforceDuration(year, month)) continue;
                    (*i)->IncrementBOM(year, month, case_k_factor);

                    // Add assets and COI charges to case totals.
                    Assets[year] += (*i)->GetSepAcctAssetsInforce();
                    CaseYearsCOICharges += (*i)->GetLastCOIChargeInforce();
                    CaseYearsIBNR += (*i)->GetIBNRContrib();

                    current_mortchg += (*i)->GetLastCOIChargeInforce();
////ofs << current_mortchg << " = mortchg" << std::endl;
                    }
////ofs << current_mortchg << " = total mortchg" << std::endl;

                case_accum_net_mortchgs *= experience_reserve_monthly_u;
                case_accum_net_mortchgs += current_mortchg;

                case_years_net_mortchgs *= experience_reserve_monthly_u;
                case_years_net_mortchgs += current_mortchg;

////ofs << "Month " << month << "ytd mortchg: " << case_years_net_mortchgs;
}
                check_ = 0.0;
// TODO ?? composite is off when first cell dies.
// Assertions are designed to ignore this.
// TODO ?? Assertions would also fire if enabled for other bases than current.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    check_ += (*i)->GetExpRatReserve();
                    }

                // Make experience rating reserve adjustments that
                // depend only on this cell.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    CaseExpRatReserve +=
                        (*i)->UpdateExpRatReserveBOM(ExpRatMlyInt);
                    }
                // Accumulate at interest after adding net COI.
                CaseExpRatReserve *= ExpRatMlyInt;

                check_ = 0.0;
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    check_ += (*i)->GetExpRatReserve();
                    }

                // Process transactions from int credit through end of month.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    if((*i)->PrecedesInforceDuration(year, month)) continue;
                    (*i)->IncrementEOM(year, month, Assets[year]);
                    }

                // Project claims using partial mortality rate.
                // Assume year's claims occur at end of last month and
                // are paid at that instant so no int adjustment required.
                //
                // We experimented with taking out 1/12 of the claims in
                // each month. But that's inconsistent with the inforce
                // factors, which change only at the end of the year.
                // So it would be quite a bit of work to offer any
                // consistent alternative to curtate partial mortality.
                double CaseMonthsClaims = 0.0;
                double current_claims = 0.0;
                if(month == 11)
                    {
                    for(i = AVS.begin(); i != AVS.end(); ++i)
                        {
                        (*i)->SetClaims();
                        CaseMonthsClaims += (*i)->GetCurtateNetClaimsInforce();
// TODO ?? AV released on death was added to the nearly-identical code in
// acctval.cpp, but not here.
                        current_claims += (*i)->GetCurtateNetClaimsInforce();
                        }
                    CaseExpRatReserve -= CaseMonthsClaims;
                    }

                case_accum_net_claims *= experience_reserve_monthly_u;
                case_accum_net_claims += current_claims;

                case_years_net_claims *= experience_reserve_monthly_u;
                case_years_net_claims += current_claims;

                // Allocate claims by total net COI deds for pol yr.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    (*i)->UpdateExpRatReserveEOM
                        (CaseYearsCOICharges
                        ,CaseMonthsClaims
                        );
                    }

                check_ = 0.0;
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    check_ += (*i)->GetExpRatReserve();
                    }

                double persistency_adjustment;
                if(0.0 != check_ && !materially_equal(check_, CaseExpRatReserve))
                    {
                    persistency_adjustment = CaseExpRatReserve / check_;
                    check_ = 0.0;
                    for(i = AVS.begin(); i != AVS.end(); ++i)
                        {
                        check_ += (*i)->UpdateExpRatReserveForPersistency
                            (persistency_adjustment
                            );
                        }
                    }
                }

            bool need_to_restart = false;
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                if(!(*i)->TestWhetherFirstYearPremiumExceededRetaliationLimit())
                    {
                    need_to_restart = true;
                    }
                }
            if(need_to_restart)
                {
                // To satisfy the popular 'zero-tolerance' attitude toward
                // the goto statement, we could instead reinitialize
                // everything explicitly and decrement the loop counter,
                // but that would be more unnatural.
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    (*i)->DebugRestart
                        ("First-year premium did not meet retaliation limit"
                        " for at least one cell in the group."
                        );
                    }
                goto restart;
                }

            // Perform end of year calculations.

            // Experience rating.
            // NumLivesInforce determines exp rating std dev multiplier.
            double NumLivesInforce = 0.0;
            double StabResVariance = 0.0;
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                // Get total actual # lives for exp rating.
                NumLivesInforce += (*i)->GetInforceLives();
                // Add cell's contribution to variance of stabilization reserve.
                StabResVariance += (*i)->GetStabResContrib();
                }
            double StabRes = 0.0;
            if(0.0 != StabResVariance)
                {
                StabRes = std::sqrt(StabResVariance);
                }
// TODO ?? Parallel code in class AccountValue differs.
            double CaseExpRfd = CaseExpRatReserve - CaseYearsIBNR - StabRes;

            CaseExpRfd = std::max(0.0, CaseExpRfd);
            CaseExpRatReserve -= CaseExpRfd;

            // Allocate refund by total net COI deds for pol yr.
            // SS does this *after* end of year,
            // but refund is *at* end of year and it should
            // affect the corridor DB e.g.
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                (*i)->SetExpRatRfd
                    (CaseYearsCOICharges
                    ,CaseExpRfd
                    );
                }

            double check_ = 0.0;
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
// TODO ?? expunge? //                check_ += i->GetExpRatReserve();
                check_ += (*i)->GetExpRatReserveNonforborne();
                }

// TODO ?? Resolve this.
//
// My latest in a long series of hypotheses is that the problem is here.
// None of these asserts fires, but perhaps the one immediately above should.
// In AccountValue::SetExpRatRfd(), AccountValue::ExpRatReserve is forborne
// among those who will survive. But AccountValue::InforceLives is not
// updated for survivorhip until AccountValue::InitializeYear() is called
// at the beginning of the next year.
//
// This was done so that the EOY corridor DB would include the refund,
// which was important to corporate tax. The refund must be forborne,
// else we won't match the spreadsheet(?), and it must be calculated
// before the EOY DB; but the EOY DB must be per unit inforce during
// the year for curtate partial mortality, while the refund is per unit
// inforce for the next year. That's why we looked ahead to the inforce
// lives at the beginning of the next year.
//
// But why should the already-forborne reserve in class AccountValue
// total to the case-level reserve here? Shouldn't the summation need
// to be weighted by the next year's inforce factor?

            // Average age reflecting survivorship and persistency.
            // This must be done here, at the end of the year, in
            // order to reflect lapses.
            //
            // TODO ?? Instead of just stating the duration calculated
            // here, we need to do something with it, and document
            // what we do. We should also document everything here;
            // is there a reason for the development notes to give
            // more detail than inline comments?
            //
            double age_sum = 0.0;
            double average_age = 0.0;
            double lives_inforce = 0.0;
            if
                (   0 == year_average_age_first_exceeds_80
                &&  std::string::npos != cell_parms()[0]["Comments"].str().find
                        ("idiosyncrasy_average_age"
                        )
                )
                {
                for(i = AVS.begin(); i != AVS.end(); ++i)
                    {
                    lives_inforce += (*i)->GetInforceLives();
                    age_sum += lives_inforce * ((*i)->GetIssueAge() + year);
                    }
                if(0.0 != lives_inforce)
                    {
                    average_age = age_sum / lives_inforce;
                    }
                if(80 < average_age)
                    {
                    year_average_age_first_exceeds_80 = year;
                    warning()
                        << " Average age exceeds 80 in policy year "
                        << year_average_age_first_exceeds_80
                        << LMI_FLUSH
                        ;
                    }
                }

            // Increment year, update curtate inforce factor.
            for(i = AVS.begin(); i != AVS.end(); ++i)
                {
                if((*i)->PrecedesInforceDuration(year, 11)) continue;
                (*i)->IncrementEOY(year);
                }

            // Calculate next year's k factor.

            double case_ibnr =
                    case_years_net_mortchgs
                *   case_ibnr_months
                /   12.0
                ;

            // Current COI charges can actually be zero, e.g. when the
            // corridor factor is unity.
            if(0.0 == case_years_net_mortchgs)
                {
                case_k_factor = 0.0;
                }
            else
                {
                case_k_factor = -
                        (case_accum_net_mortchgs - case_accum_net_claims - case_ibnr)
                    *   (1.0 + case_k_factor)
// TODO ?? '4.0' is an arbitrary factor that belongs in the database.
                    /   (4.0 * case_years_net_mortchgs)
                    ;
                case_k_factor = std::max(-1.0, case_k_factor);
                }

            std::ofstream ofs
                ("experience_rating"
                ,std::ios_base::out | std::ios_base::ate | std::ios_base::app
                );
            ofs
                << '\t' << year
                << '\t' << experience_reserve_monthly_u
                << '\t' << NumLivesInforce
                << '\t' << case_years_net_mortchgs
                << '\t' << case_accum_net_mortchgs
                << '\t' << case_years_net_claims
                << '\t' << case_accum_net_claims
                << '\t' << case_ibnr_months
                << '\t' << case_ibnr
                << '\t' << case_k_factor
                << '\n' << std::flush
                ;

            progress_message.str("");
            progress_message << "Completed " << 1 + year << " of " << MaxYr;
            was_canceled_ = !Progress.Update
                (1 + year
                ,progress_message.str().c_str()
                );
            if(was_canceled_)
                {
                break;
                }
            } // End for year.

        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            (*i)->FinalizeLife(*run_basis);
            }

        } // End for...try.
//    catch(xmsg& x) // TODO ?? or let it leak through to application?
//        {
//        ::MessageBox(0, x.why().c_str(), "Exception", MB_OK);
//        }
//    catch([various exception types])
//        {
//        throw;
//        }

    for(i = AVS.begin(); i != AVS.end(); ++i)
        {
        (*i)->FinalizeLifeAllBases();
        Composite.PlusEq((*i)->LedgerValues());
        }

    status() << timer.Stop().Report() << std::flush;

    if(!was_canceled_)
        {
        int j = 0;
        for(i = AVS.begin(); i != AVS.end(); ++i)
            {
            EmitEveryone(a_OutputDest, (*i)->LedgerValues(), ++j);
            }

        composite_ledger_ = Composite;
        // Insert total specamt and payments.
//        composite_is_available_ = true;
        composite_is_available_ = false; // TODO ?? Doesn't work.

        EmitEveryone(a_OutputDest, composite_ledger_, -1);
        }
}

void CensusView::EmitEveryone
    (e_output_dest a_OutputDest
    ,Ledger const& a_Values
    ,int           a_idx
    )
{
    if(a_OutputDest & to_printer)
        {
        PrintAnonymousIllustration(a_Values, a_idx);
        }
    if(a_OutputDest & to_tab_delim)
        {
        // This is no longer reachable by any path through the code.
        // We leave it here so that someday we can offer finer
        // control over census output, perhaps through a GUI.
        composite_is_available_ = true; // TODO ?? Doesn't work.
        SaveRegressionTestFile(a_Values, a_idx);
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
    if(a_OutputDest & to_crc_file)
        {
        composite_is_available_ = true; // TODO ?? Doesn't work.
        // Optionally generate tab delimited regression-test output too.
        if(global_settings::instance().regression_test_full)
            {
            SaveRegressionTestFile(a_Values, a_idx);
            }
        SaveCRCToFile(a_Values, a_idx);
        if(a_Values.GetIsComposite())
            {
            // For composite, emit all the vectors in Ledger.
            std::ofstream os
                (GetCRCFilename().c_str()
                ,  std::ios_base::out
                 | std::ios_base::ate
                 | std::ios_base::app
                );
            a_Values.Spew(os);
            }
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
    if(a_OutputDest & to_spreadsheet)
        {
        composite_is_available_ = true; // TODO ?? Doesn't work.
        SaveSpreadsheetFile(a_Values, a_idx);
        composite_is_available_ = false; // TODO ?? Doesn't work.
        }
}

void CensusView::PrintAnonymousIllustration(Ledger const& a_Values, int index)
{
// TODO ?? Use an auto_ptr instead?
    std::string file_name(serial_filename(1 + index, "ill"));
    IllustrationView* illview = MakeNewDocAndView(file_name.c_str());

    illview->SetLedger(a_Values);
    illview->FormatSelectedValuesAsHtml();
    illview->Pdf("print");

//    illview->Destroy();
//    delete illview; // TODO ?? Want to get rid of this thing.
}

void CensusView::SaveRegressionTestFile(Ledger const& a_Values, long idx)
{
    // Name file based on index origin one, as expected by users.
//    std::string file_name(serial_filename(1 + idx, "ill"));

    // Create a document with that filename.
    // Get a pointer to the resulting view.
//    IllustrationView* illview = MakeNewDocAndView(file_name.c_str());
std::runtime_error("Not supported yet");
//    illview->SetLedger(a_Values);
//    illview->FormatSelectedValuesAsHtml();
//    illview->FileSaveAsTDT();

//    illview->Destroy();
//    delete illview; // TODO ?? Want to get rid of this thing.
}

// Make the extension a configurable input.
void CensusView::SaveSpreadsheetFile(Ledger const& a_Values, long idx)
{
/* TODO ?? Clean up.
    // Name file based on index origin one, as expected by users.
    std::string file_name(serial_filename("XL4", 1 + idx, "ill"));

    // Create a document with that filename
    // Get a ptr to the resulting view
    IllustrationView* illview = MakeNewDocAndView(file_name.c_str());
*/
    std::string spreadsheet_filename = base_filename() + ".xls";

    PrintFormTabDelimited(a_Values, spreadsheet_filename);

// TODO ?? expunge
//    illview->SetLedger(a_Values);
//    illview->FormatSelectedValuesAsHtml();
//    illview->NormalizePrintArea();
//    illview->FileSaveAsXLS();

//    illview->Destroy();
//    delete illview;
}

std::string const CensusView::GetCRCFilename() const
{
    return base_filename() + ".crc";
}

void CensusView::SaveCRCToFile(Ledger const& a_Values, long idx)
{
    std::ofstream os
        (GetCRCFilename().c_str()
        ,   std::ios_base::out
          | std::ios_base::ate
          | std::ios_base::app
        );
    os << idx << "\t" << a_Values.CalculateCRC() << std::endl;
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
            warning()
                << "Input validation problems:\n"
                << oss.str()
                << LMI_FLUSH
                ;
            }
        }
}
} // Unnamed namespace.

// TODO ?? Belongs elsewhere.
// TODO ?? FSF !! Use wx facilities instead.
namespace
{
std::string GetClipboardText()
{
    std::string s;
    if
        (   !::IsClipboardFormatAvailable(CF_TEXT)
        ||  !::OpenClipboard(0)
        )
        {
        return s;
        }

    HGLOBAL hglb = ::GetClipboardData(CF_TEXT);
    if(hglb)
        {
        LPTSTR lptstr = (char*)::GlobalLock(hglb);
        if(lptstr != NULL)
            {
            s = lptstr;
            ::GlobalUnlock(hglb);
            }
        }
    ::CloseClipboard();

    return s;
}
} // Unnamed namespace.

// Print tab-delimited output to file loadable in spreadsheet programs.
void CensusView::OnRunCaseToSpreadsheet(wxCommandEvent&)
{
    std::string spreadsheet_filename = base_filename() + ".xls";
    std::remove(spreadsheet_filename.c_str());
    DoAllCells(to_spreadsheet);
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
    // In the solution domain:
    //   document modified <--> dirty flag set
    //   loaded from file  <--> document path not null
// Except that the wx port is different.    
    //
    // Furthermore, a brand-new census has default parameters for
    // the case, the class, and the default cell. As this is written,
    // the default parameters involve a premium solve. Generally, this
    // is not wanted in a pasted census unless a solve is specified in
    // the pasted data. Therefore, in the case where we erase the
    // default cell as described above, we turn off solves too, for
    // case and class defaults.
    //
    // We do all of this as soon as we know that the user wants to
    // paste census data. If pasting fails, the user is left with a
    // census that contains no cells, but that's OK; in this event,
    // they're likely to correct the data and repaste it.
    //

// TODO ?? WX PORT !! How to do this with wx?
//    if(!document().IsModified() && 0 == document().GetDocPath())
//    if(!document().IsModified())
        {
        cell_parms().clear();
/* TODO Expunge this old comment...
        // If this is an old census, leave solves alone. Otherwise,
        // turn off solves at the case level. The default is to solve for
        // individual (employee) premium; that would override pasted payment
        // values. And set solve target durations as a function of age, so
        // that they won't vary when cells with different ages are pasted in.
...and answer this new question: */
// TODO ?? Is there any actual benefit to changing solve target age or
// duration? If we make it "target at age", then "target-at duration"
// differs across cells. If we make it "target at duration", then
// "target-at age" differs across cells. Is that right?

// TODO ?? The input["member_name"] idiom isn't desirable here.
// Compile-time errors are wanted for invalid names.
        case_parms()[0]["SolveType"      ] = "SolveNone";
// TODO ?? expunge if no longer necessary.
//        case_parms()[0]["SolveTgtAtWhich"] = "TgtAtAge";
//        case_parms()[0]["SolveToWhich"   ] = "ToAge";
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
                std::ostringstream error;
                error
                    << "Line #" << current_line << ": "
                    << " (" << line << ") "
                    << "has a value that contains no non-blank characters. "
                    << "Last valid value, if any: " << values.back()
                    ;
// TODO ?? We'd like to do this:
//                throw std::range_error(error.str());
// but it segfaults with very long strings. So we do this instead:
                warning() << error.str() << LMI_FLUSH;
                throw std::range_error("Invalid input.");
                }
// warning() << "'" << token << "'" << "push_back value" << LMI_FLUSH;
            values.push_back(token);
            }

        if(values.size() != headers.size())
            {
            std::ostringstream error;
            error
                << "Line #" << current_line << ": "
                << "  (" << line << ") "
                << "should have one value per column. "
                << "Number of values: " << values.size() << "; "
                << "number expected: " << headers.size() << "."
                ;
            throw std::range_error(error.str());
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

//    OnValidate(); // TODO ?? expunge

    status() << std::flush;
}


//////////////////////////////////////////////////////////////////////////////

// The following functions probably should be factored out into a
// utility module.

std::string const CensusView::base_filename() const
{
    std::string t = GetDocument()->GetFilename().c_str();
    if(0 == t.size())
        {
        t = GetDocument()->GetTitle().c_str();
        }
    LMI_ASSERT(0 != t.size());
    fs::path path(t);
    return path.leaf();
}

std::string const CensusView::serial_filename
    (std::string const prefix
    ,int serial_number
    ,std::string const suffix
    ) const
{
    std::ostringstream oss;
    oss
        << prefix
        << '.'
        << std::setfill('0') << std::setw(9) << serial_number
        << '.'
        << suffix
        ;
    return oss.str();
}

std::string const CensusView::serial_filename
    (int serial_number
    ,std::string const suffix
    ) const
{
    return serial_filename(base_filename(), serial_number, suffix);
}

