// Census manager.
//
// Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020 Gregory W. Chicares.
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

#ifndef census_view_hpp
#define census_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "input.hpp"
#include "ledger.hpp"
#include "mc_enum_type_enums.hpp"       // enum mcenum_emission
#include "oecumenic_enumerations.hpp"

#include <wx/object.h>                  // wxObjectDataPtr

#include <memory>                       // shared_ptr
#include <string>
#include <vector>

class CensusDocument;
class CensusViewDataViewModel;
class CensusViewGridTable;

class WXDLLIMPEXP_FWD_ADV wxDataViewEvent;
class WXDLLIMPEXP_FWD_ADV wxDataViewCtrl;
class WXDLLIMPEXP_FWD_ADV wxGrid;
class WXDLLIMPEXP_FWD_ADV wxGridEvent;
class WXDLLIMPEXP_FWD_ADV wxGridSizeEvent;


class CensusView
    :public ViewEx
{
  public:
    CensusView();

  private:
    CensusView(CensusView const&) = delete;
    CensusView& operator=(CensusView const&) = delete;

    virtual void update_visible_columns() = 0;

  protected:
    CensusDocument& document() const;

    // ViewEx required implementation.
    char const* icon_xrc_resource   () const override;
    char const* menubar_xrc_resource() const override;

    // Event handlers, in event-table order (reflecting GUI order)
    void         UponEditCell               (wxCommandEvent&);
    void         UponEditClass              (wxCommandEvent&);
    void         UponEditCase               (wxCommandEvent&);
    void         UponRunCell                (wxCommandEvent&);
    void         UponRunCase                (wxCommandEvent&);
    void         UponPrintCase              (wxCommandEvent&);
    void         UponPrintCaseToDisk        (wxCommandEvent&);
    void         UponRunCaseToSpreadsheet   (wxCommandEvent&);
    void         UponRunCaseToGroupRoster   (wxCommandEvent&);
    void         UponRunCaseToGroupQuote    (wxCommandEvent&);
    void         UponCopyCensus             (wxCommandEvent&);
    virtual void UponPasteCensus            (wxCommandEvent&) = 0;
    virtual void UponAddCell                (wxCommandEvent&) = 0;
    virtual void UponDeleteCells            (wxCommandEvent&) = 0;
    virtual void UponColumnWidthVarying     (wxCommandEvent&) = 0;
    virtual void UponColumnWidthFixed       (wxCommandEvent&) = 0;
    void         UponUpdateAlwaysDisabled   (wxUpdateUIEvent&);
    void         UponUpdateAlwaysEnabled    (wxUpdateUIEvent&);
    virtual void UponUpdateNonemptySelection(wxUpdateUIEvent&) = 0;
    virtual void UponUpdateColumnValuesVary (wxUpdateUIEvent&) = 0;

    bool DoAllCells(mcenum_emission);

    virtual void Update() = 0;
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

    virtual int current_row() = 0;

    void update_class_names();

    void DoCopyCensus() const;

    bool autosize_columns_;

    std::shared_ptr<Ledger const> composite_ledger_;

    DECLARE_EVENT_TABLE()
};

class CensusDVCView final
    :public CensusView
{
    friend class CensusDVCDocument;
    friend class CensusViewDataViewModel;

  public:
    CensusDVCView();

  private:
    CensusDVCView(CensusDVCView const&) = delete;
    CensusDVCView& operator=(CensusDVCView const&) = delete;

    void update_visible_columns() override;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;

    // Event handlers, in event-table order (reflecting GUI order)
    void UponRightClick             (wxDataViewEvent&);
    void UponValueChanged           (wxDataViewEvent&);
    void UponPasteCensus            (wxCommandEvent&) override;
    void UponAddCell                (wxCommandEvent&) override;
    void UponDeleteCells            (wxCommandEvent&) override;
    void UponColumnWidthVarying     (wxCommandEvent&) override;
    void UponColumnWidthFixed       (wxCommandEvent&) override;
    void UponUpdateSingleSelection  (wxUpdateUIEvent&);
    void UponUpdateNonemptySelection(wxUpdateUIEvent&) override;
    void UponUpdateColumnValuesVary (wxUpdateUIEvent&) override;

    void Update() override;

    int current_row() override;

    wxDataViewCtrl* list_window_;
    wxObjectDataPtr<CensusViewDataViewModel> list_model_;

    DECLARE_DYNAMIC_CLASS(CensusDVCView)
    DECLARE_EVENT_TABLE()
};

class CensusGridView final
    :public CensusView
{
    friend class CensusGridDocument;
    friend class CensusViewGridTable;

  public:
    CensusGridView() = default;

  private:
    CensusGridView(CensusGridView const&) = delete;
    CensusGridView& operator=(CensusGridView const&) = delete;

    void update_visible_columns() override;

    // ViewEx required implementation.
    wxWindow* CreateChildWindow() override;

    // Event handlers, in event-table order (reflecting GUI order)
    void UponRightClick             (wxGridEvent&);
    void UponValueChanged           (wxGridEvent&);
    void UponColumnAutoSize         (wxGridSizeEvent&);
    void UponPasteCensus            (wxCommandEvent&) override;
    void UponAddCell                (wxCommandEvent&) override;
    void UponDeleteCells            (wxCommandEvent&) override;
    void UponColumnWidthVarying     (wxCommandEvent&) override;
    void UponColumnWidthFixed       (wxCommandEvent&) override;
    void UponUpdateNonemptySelection(wxUpdateUIEvent&) override;
    void UponUpdateColumnValuesVary (wxUpdateUIEvent&) override;

    void Update() override;

    int current_row() override;

    wxGrid*              grid_window_ {nullptr};
    CensusViewGridTable* grid_table_  {nullptr};

    DECLARE_DYNAMIC_CLASS(CensusGridView)
    DECLARE_EVENT_TABLE()
};

#endif // census_view_hpp
