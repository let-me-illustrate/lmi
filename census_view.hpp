// Census manager.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022 Gregory W. Chicares.
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

#ifndef census_view_hpp
#define census_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "input.hpp"
#include "ledger.hpp"
#include "mc_enum_type_enums.hpp"       // mcenum_emission
#include "oecumenic_enumerations.hpp"

#include <wx/object.h>                  // wxObjectDataPtr

#include <memory>                       // shared_ptr
#include <string>
#include <vector>

class CensusDocument;
class CensusViewGridTable;

class WXDLLIMPEXP_FWD_ADV wxGrid;
class WXDLLIMPEXP_FWD_ADV wxGridEvent;
class WXDLLIMPEXP_FWD_ADV wxGridSizeEvent;

class CensusView final
    :public ViewEx
{
    friend class CensusDocument;
    friend class CensusViewGridTable;

  public:
    CensusView();

  private:
    CensusView(CensusView const&) = delete;
    CensusView& operator=(CensusView const&) = delete;

    void update_visible_columns();

    CensusDocument& document() const;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // Event handlers, in event-table order (reflecting GUI order)
    void UponRightClick             (wxGridEvent&);
    void UponValueChanged           (wxGridEvent&);
    void UponColumnAutoSize         (wxGridSizeEvent&);
    void UponEditCell               (wxCommandEvent&);
    void UponEditClass              (wxCommandEvent&);
    void UponEditCase               (wxCommandEvent&);
    void UponRunCell                (wxCommandEvent&);
    void UponRunCase                (wxCommandEvent&);
    void UponPrintCase              (wxCommandEvent&);
    void UponPrintCaseToDisk        (wxCommandEvent&);
    void UponRunCaseToSpreadsheet   (wxCommandEvent&);
    void UponRunCaseToGroupRoster   (wxCommandEvent&);
    void UponRunCaseToGroupQuote    (wxCommandEvent&);
    void UponCopyCensus             (wxCommandEvent&);
    void UponPasteCensus            (wxCommandEvent&);
    void UponAddCell                (wxCommandEvent&);
    void UponDeleteCells            (wxCommandEvent&);
    void UponColumnWidthVarying     (wxCommandEvent&);
    void UponColumnWidthFixed       (wxCommandEvent&);
    void UponUpdateAlwaysDisabled   (wxUpdateUIEvent&);
    void UponUpdateAlwaysEnabled    (wxUpdateUIEvent&);
    void UponUpdateSingleSelection  (wxUpdateUIEvent&);
    void UponUpdateColumnValuesVary (wxUpdateUIEvent&);

    bool DoAllCells(mcenum_emission);

    void Update();
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

    bool column_value_varies_across_cells(std::string const& header) const;

    oenum_mvc_dv_rc edit_parameters
        (Input&             parameters
        ,std::string const& title
        );

    int current_row();

    void update_class_names();

    void DoCopyCensus() const;

    bool autosize_columns_;

    std::shared_ptr<Ledger const> composite_ledger_;

    wxGrid*              grid_window_ {nullptr};
    CensusViewGridTable* grid_table_  {nullptr};

    DECLARE_DYNAMIC_CLASS(CensusView)
    DECLARE_EVENT_TABLE()
};

#endif // census_view_hpp
