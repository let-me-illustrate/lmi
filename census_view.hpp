// Life insurance illustrations.
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

// $Id: census_view.hpp,v 1.1 2005-03-12 03:01:08 chicares Exp $

#ifndef census_view_hpp
#define census_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "input.hpp"
#include "ledger.hpp"
#include "obstruct_slicing.hpp"

#include <boost/utility.hpp>

#include <string>
#include <vector>

class CensusDocument;
class IllustrationView;
class wxListEvent;
class wxListView;

class CensusView
    :public ViewEx
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<CensusView>
{
    friend class CensusDocument;

    // Output destination for composite and *each* individual life when
    // running a census. Usually only the composite is emitted, and only
    // to the screen. Other options let you additionally emit everything
    // to the printer or to various file formats. Enumerators are binary
    // powers so that more than one can be specified in an int.
    enum e_output_dest
        {to_nowhere     = 0
        ,to_printer     = 1
        ,to_tab_delim   = 2
        ,to_spreadsheet = 4 // TODO ?? Does this just mean tab delimited?
        ,to_crc_file    = 8
        };

  public:
    CensusView();
    virtual ~CensusView();

  private:
    void DisplayAllVaryingData();

    CensusDocument& document() const;

    // ViewEx required implementation.
    virtual wxWindow* CreateChildWindow();
    virtual wxIcon const& Icon() const;
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

    // TODO ?? Realphabetize.

    void Run();

    void EmitEveryone(e_output_dest a_OutputDest, Ledger const& a_Values, int a_idx);
    bool DoAllCells(e_output_dest a_OutputDest = to_nowhere);
    void RunAllMonths(e_output_dest a_OutputDest = to_nowhere);
    void RunAllLives(e_output_dest a_OutputDest = to_nowhere);

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
    IllustrationView* MakeNewDocAndView(char const*);
    IllustrationView* ViewOneCell(int);
    IllustrationView* ViewComposite();
    void PrintAnonymousIllustration(Ledger const& a_Values, int index);
    void SaveRegressionTestFile(Ledger const& a_Values, long idx);
    void SaveSpreadsheetFile(Ledger const& a_Values, long idx);
    std::string const GetCRCFilename() const;
    void SaveCRCToFile(Ledger const& a_Values, long idx);

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

    Ledger composite_ledger_;

    std::vector<std::string> headers_of_varying_parameters_;

    bool was_canceled_;

//////////////////////////////////////////////////////////////////////////////

// These filename functions probably should be factored out into
// a utility module. The originals are
// Copyright (C) 1998, 1999, 2000, 2002, 2003, 2004 Gregory W. Chicares.

    // Extract the base portion of the filename, excluding path and extension.
    std::string const base_filename() const;

    // Make up an output filename based on a supplied serial number,
    // and a prefix and suffix, either or both of which may be empty.
    // The serial number is zero filled with at least nine characters;
    // the range [0, 100000000) should be adequate for any foreseeable
    // insured group (it can hold all US social security numbers, e.g.),
    // and formatting it this way makes the filenames sort nicely.
    //
    // If more than nine characters are required, then all digits will
    // be printed, but then the filenames won't sort as nicely.
    //
    // String arguments are of value rather than reference type in order
    // to allow character array arguments.
    //
    // A terminating null is added so that result.c_str() can used with
    // functions that require C strings.
    //
    std::string const serial_filename
        (std::string const prefix
        ,int serial_number
        ,std::string const suffix
        ) const;

    // This form, with no prefix argument, uses the corresponding document's
    // name to be used as the prefix. A default argument wouldn't work, for
    // it would need to refer to the 'this' pointer [8.3.6/8].
    //
    std::string const serial_filename
        (int serial_number
        ,std::string const suffix
        ) const;

    wxListView* list_window_;

    DECLARE_DYNAMIC_CLASS(CensusView)
    DECLARE_EVENT_TABLE()
};

#endif // census_view_hpp

