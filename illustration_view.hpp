// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005, 2006 Gregory W. Chicares.
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
// email: <chicares@cox.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: illustration_view.hpp,v 1.23.2.3 2006-11-20 15:32:29 etarassov Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The original was modified by GWC in 2003 to create a standalone
// view class customized for illustration documents, and in the later
// years given in the copyright notice above.

#ifndef illustration_view_hpp
#define illustration_view_hpp

#include "config.hpp"

#include "view_ex.hpp"

#include "obstruct_slicing.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <wx/event.h>

#include <string>

class IllustrationDocument;
class Input;
class Ledger;
class WXDLLEXPORT wxHtmlWindow;
class WXDLLEXPORT wxPrintout;

// TODO ?? Consider adding an input reference member. Here, it's used
// only for edit and run; in the census view class, it's used widely.

class IllustrationView
    :public ViewEx
    ,virtual private boost::noncopyable
    ,virtual private obstruct_slicing<IllustrationView>
{
    friend class IllustrationDocument;

    // TODO ?? CALCULATION_SUMMARY Couldn't enum_xml_version be used
    // instead?
    //
    // EVGENIY: IMHO it is better keep two separate enums, because even though
    // enum_tsv_copy_option and enum_xml_version are related, it is an indirect
    // relation, and this could possibly evaluate into two completly
    // different option sets.
    // As of 20/11/2006 enum_xml_version has already one more value than
    // enum_tsv_copy_option.
    // Two values e_xml_calculation_summary and e_xml_spreadsheet (which seem
    // to directly corespond to e_tsv_copy_full and e_tsv_copy_summary) could
    // merge into one single option, while enum_copy_option will keep distinct
    // values since its options correspond to distinct TSV outputs, which could
    // be generated using the same underlying xml data.
    enum enum_tsv_copy_option
        {e_tsv_copy_full
        ,e_tsv_copy_summary
        };

  public:
    IllustrationView();
    virtual ~IllustrationView();

    // Making these functions public so that they can be invoked by
    // class CensusView is arguably less bad than making that class a
    // friend.
    //
    void DisplaySelectedValuesAsHtml();
    void Pdf(std::string const& action) const;
    void Run(Input* = 0);
    void SetLedger(boost::shared_ptr<Ledger const>);

  private:
    IllustrationDocument& document() const;

    void CopyLedgerTSVToClipboard(enum_tsv_copy_option);
    int EditProperties();

    // ViewEx required implementation.
    wxWindow* CreateChildWindow();
    wxIcon Icon() const;
    wxMenuBar* MenuBar() const;

    // ViewEx overrides.
    virtual wxPrintout* OnCreatePrintout();
    bool OnCreate              (wxDocument*, long int);

    void UponCopyFull          (wxCommandEvent&);
    void UponCopySummary       (wxCommandEvent&);
    void UponMenuOpen          (wxMenuEvent&);
    void UponPreviewSummary    (wxCommandEvent&);
    void UponPreviewPdf        (wxCommandEvent&);
    void UponPrintSummary      (wxCommandEvent&);
    void UponPrintPdf          (wxCommandEvent&);
    void UponProperties        (wxCommandEvent&);
    void UponUpdateFileSave    (wxUpdateUIEvent&);
    void UponUpdateFileSaveAs  (wxUpdateUIEvent&);
    void UponUpdateInapplicable(wxUpdateUIEvent&);
    void UponUpdateProperties  (wxUpdateUIEvent&);

    wxHtmlWindow* html_window_;
    bool is_phony_;
    boost::shared_ptr<Ledger const> ledger_values_;
    std::string selected_values_as_html_;

    DECLARE_DYNAMIC_CLASS(IllustrationView)
    DECLARE_EVENT_TABLE()
};

IllustrationView& MakeNewIllustrationDocAndView
    (wxDocManager* dm
    ,char const*   filename
    );

bool RunSpecialInputFileIfPresent(wxDocManager*);

#endif // illustration_view_hpp

