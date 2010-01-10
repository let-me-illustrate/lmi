// Census manager.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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

#ifndef census_view_hpp
#define census_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "input.hpp"
#include "ledger.hpp"
#include "mc_enum_type_enums.hpp" // enum mcenum_emission
#include "obstruct_slicing.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <wx/defs.h> // wx shared-library 'attributes'

#include <string>
#include <vector>

class CensusDocument;
class WXDLLIMPEXP_FWD_CORE wxListEvent;
class WXDLLIMPEXP_FWD_CORE wxListView;

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

    void UponAddCell                (wxCommandEvent&);
    void UponBeginLabelEdit         (wxListEvent&); // TODO ?? Expunge.
    void UponDeleteCells            (wxCommandEvent&);
    void UponEditCell               (wxCommandEvent&);
    void UponEditClass              (wxCommandEvent&);
    void UponEditCase               (wxCommandEvent&);
    void UponColumnWidthVarying     (wxCommandEvent&);
    void UponColumnWidthFixed       (wxCommandEvent&);
    void UponPasteCensus            (wxCommandEvent&);
    void UponRightClick0            (wxCommandEvent&);
    void UponRightClick1            (wxMouseEvent&);
    void UponRightClick2            (wxContextMenuEvent&);
    void UponPrintCase              (wxCommandEvent&);
    void UponPrintCaseToDisk        (wxCommandEvent&);
    void UponRunCell                (wxCommandEvent&);
    void UponRunCase                (wxCommandEvent&);
    void UponRunCaseToSpreadsheet   (wxCommandEvent&);
    void UponUpdateApplicable       (wxUpdateUIEvent&);

    bool DoAllCells(mcenum_emission);

    void Update();
    void UpdatePreservingSelection();
    void ViewOneCell(int);
    void ViewComposite();

    void apply_changes
        (Input const& new_parms
        ,Input const& old_parms
        ,bool         for_this_class_only
        );

    std::vector<Input>&       case_parms();
    std::vector<Input> const& case_parms() const;
    std::vector<Input>&       cell_parms();
    std::vector<Input> const& cell_parms() const;
    std::vector<Input>&       class_parms();
    std::vector<Input> const& class_parms() const;

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

    void identify_varying_columns();

    bool is_invalid();

    int selected_column();
    int selected_row();

    void update_class_names();

    bool all_changes_have_been_validated_;

    bool composite_is_available_;

    boost::shared_ptr<Ledger const> composite_ledger_;

    std::vector<std::string> headers_of_varying_parameters_;

    bool was_cancelled_;

    wxListView* list_window_;

    DECLARE_DYNAMIC_CLASS(CensusView)
    DECLARE_EVENT_TABLE()
};

#endif // census_view_hpp

