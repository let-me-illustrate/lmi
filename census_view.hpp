// Census manager.
//
// Copyright (C) 2004, 2005 Gregory W. Chicares.
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

// $Id: census_view.hpp,v 1.12 2005-11-06 15:28:25 chicares Exp $

#ifndef census_view_hpp
#define census_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "group_values.hpp"
#include "input.hpp"
#include "ledger.hpp"
#include "obstruct_slicing.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <wx/defs.h> // WXDLLEXPORT

#include <string>
#include <vector>

class CensusDocument;
class IllustrationView;
class WXDLLEXPORT wxListEvent;
class WXDLLEXPORT wxListView;

class CensusView
    :public ViewEx
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<CensusView>
{
    friend class CensusDocument;

  public:
    CensusView();
    virtual ~CensusView();

  private:
    void DisplayAllVaryingData();

    CensusDocument& document() const;

    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();
    virtual wxIcon Icon() const;
    virtual wxMenuBar* MenuBar() const;

// TODO ?? expunge the first:
    void OnBeginLabelEdit       (wxListEvent&);

    void OnAddCell              (wxCommandEvent&);
    void OnDeleteCells          (wxCommandEvent&);
    void OnEditCell             (wxCommandEvent&);
    void OnEditClass            (wxCommandEvent&);
    void OnEditCase             (wxCommandEvent&);
    void OnExpandColWidths      (wxCommandEvent&);
    void OnShrinkColWidths      (wxCommandEvent&);
    void OnPasteCensus          (wxCommandEvent&);
    void OnRightClick0          (wxCommandEvent&);
    void OnRightClick1          (wxMouseEvent&);
    void OnRightClick2          (wxContextMenuEvent&);
    void OnPrintCell            (wxCommandEvent&);
    void OnPrintCase            (wxCommandEvent&);
    void OnRunCell              (wxCommandEvent&);
    void OnRunCase              (wxCommandEvent&);
    void OnRunCaseToSpreadsheet (wxCommandEvent&);
    void OnUpdateApplicable     (wxUpdateUIEvent&);
    void OnUpdateFileSaveAs     (wxUpdateUIEvent&);

    bool DoAllCells  (e_emission_target emission_target = emit_to_nowhere);

    // Ascertain differences between old and new parameters and apply
    // each such difference to other cells...
    //   if 'for_this_class_only' is specified, to all cells in the
    //     employee class of the old parameters;
    //   otherwise, to all cells in the entire census.
    void ApplyChanges
        (Input const& new_parms
        ,Input const& old_parms
        ,bool         for_this_class_only
        );

    void Update();
    void UpdatePreservingSelection();
    IllustrationView* ViewOneCell(int);
    void ViewComposite();

    std::vector<Input>&       case_parms();
    std::vector<Input> const& case_parms() const;
    std::vector<Input>&       cell_parms();
    std::vector<Input> const& cell_parms() const;
    std::vector<Input>&       class_parms();
    std::vector<Input> const& class_parms() const;

    std::vector<Input>&       ihs_case_parms();
    std::vector<Input> const& ihs_case_parms() const;
    std::vector<Input>&       ihs_cell_parms();
    std::vector<Input> const& ihs_cell_parms() const;
    std::vector<Input>&       ihs_class_parms();
    std::vector<Input> const& ihs_class_parms() const;

    std::string cell_title(int);
    std::string class_title(int);
    std::string class_name_from_cell_number(int) const;
    Input* class_parms_from_class_name(std::string const&);

    bool column_value_varies_across_cells
        (std::string        const& header
        ,std::vector<Input> const& cells
        ) const;

    int edit_parameters
        (Input&             parameters
        ,std::string const& name
        );

    void freeze(bool);

    void identify_varying_columns();

    bool is_invalid();

    int selected_column();
    int selected_row();

    void update_class_names();

    bool all_changes_have_been_validated_;

    bool composite_is_available_;

    boost::shared_ptr<Ledger const> composite_ledger_;

    std::vector<std::string> headers_of_varying_parameters_;

    bool was_canceled_;

    wxListView* list_window_;

    DECLARE_DYNAMIC_CLASS(CensusView)
    DECLARE_EVENT_TABLE()
};

#endif // census_view_hpp

