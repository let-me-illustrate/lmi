// Customize implementation details of library class wxDocManager.
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021 Gregory W. Chicares.
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

// This implementation is a derived work based on wxWindows code, viz.
//   samples/printing/printing.cpp (C) 1995 Julian Smart
//   src/common/docview.cpp (C) 1997 Julian Smart and Markus Holzem
// both of which are covered by the wxWindows license.
//
// The originals were modified by GWC in 2002 as follows:
//   'Page setup' uses member variables here, rather than globals.
//   'Preview' overlays its frame over the main application window.
//   'Print' moves fancier code from the sample into the doc manager.
//   Display no 'cancelled' messagebox when user cancels printing.
// and in the later years given in the copyright notice above as
// detailed in ChangeLog.

#include "pchfile_wx.hpp"

#include "docmanager_ex.hpp"

#include "bourn_cast.hpp"
#include "previewframe_ex.hpp"
#include "single_choice_popup_menu.hpp"
#include "wx_new.hpp"
#include "wx_utility.hpp"

#include <wx/cmndata.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/printdlg.h>

IMPLEMENT_DYNAMIC_CLASS(DocManagerEx, wxDocManager)

BEGIN_EVENT_TABLE(DocManagerEx, wxDocManager)
    EVT_MENU(wxID_PREVIEW, DocManagerEx::UponPreview)
    EVT_MENU(wxID_PRINT, DocManagerEx::UponPrint)
    EVT_MENU(wxID_PAGE_SETUP, DocManagerEx::UponPageSetup)
END_EVENT_TABLE()

// WX !! Printer settings should be configured globally, OAOO,
// for all classes that ought to use them. It was reported that
// 'A4' was used unless explicitly overridden, but it seems
// that the paper id was actually wxPAPER_NONE; if that causes
// 'A4' to be used, then should wx instead use wxPAPER_LETTER in
// a US locale, where 'A4' is a poor default? This workaround:
//   printer.GetPrintData()->SetPaperId(wxPAPER_LETTER);
// was used in 'illustration_view.cpp' through revision 1.98 .

DocManagerEx::DocManagerEx(long int flags, bool initialize)
    :wxDocManager     (flags, initialize)
    ,print_data_      {new(wx) wxPrintData}
    ,page_setup_data_ {new(wx) wxPageSetupDialogData}
{
}

// WX !! Perhaps xrc could support a menu-use-file-history tag,
// and this stuff could be made available by default in the library.

void DocManagerEx::AssociateFileHistoryWithFileMenu(wxMenuBar* menu_bar)
{
    if(menu_bar)
        {
        int file_menu_index = menu_bar->FindMenu("File");
        if(wxNOT_FOUND != file_menu_index)
            {
            wxMenu* file_menu = menu_bar->GetMenu(file_menu_index);
            FileHistoryUseMenu(file_menu);
            FileHistoryAddFilesToMenu(file_menu);
            }
        }
}

void DocManagerEx::DissociateFileHistoryFromFileMenu(wxMenuBar* menu_bar)
{
    if(menu_bar)
        {
        int file_menu_index = menu_bar->FindMenu("File");
        if(wxNOT_FOUND != file_menu_index)
            {
            wxMenu* file_menu = menu_bar->GetMenu(file_menu_index);
            FileHistoryRemoveMenu(file_menu);
            }
        }
}

void DocManagerEx::UponPageSetup(wxCommandEvent&)
{
    *page_setup_data_ = *print_data_;

    wxPageSetupDialog pageSetupDialog
        (&TopWindow()
        ,page_setup_data_.get()
        );
    pageSetupDialog.ShowModal();

    *print_data_ = pageSetupDialog.GetPageSetupData().GetPrintData();
    *page_setup_data_ = pageSetupDialog.GetPageSetupData();
}

/// This is a complete replacement for wxDocManager::OnPreview(),
/// and doesn't need to call the base-class version.

void DocManagerEx::UponPreview(wxCommandEvent&)
{
    wxView* view = GetCurrentView();
    if(!view)
        {
        return;
        }

    wxPrintout* printout = view->OnCreatePrintout();
    if(!printout)
        {
        wxMessageBox
            ("Cannot create printout. Check printer settings and try again."
            ,"Print preview failure."
            );
        return;
        }

    // Pass two printout objects: for preview, and possible printing.
    wxPrintPreviewBase* preview = new(wx) wxPrintPreview
        (printout
        ,view->OnCreatePrintout()
        ,print_data_.get()
        );
    if(!preview->Ok())
        {
        delete preview;
        wxMessageBox
            ("Cannot create preview. Check printer settings and try again."
            ,"Print preview failure."
            );
        return;
        }
    PreviewFrameEx* frame = new(wx) PreviewFrameEx(preview);
    frame->Center(wxBOTH);
    frame->Initialize();
    frame->Show(true);
}

/// This is a complete replacement for wxDocManager::OnPrint(),
/// and doesn't need to call the base-class version.

void DocManagerEx::UponPrint(wxCommandEvent&)
{
    wxView* view = GetCurrentView();
    if(!view)
        {
        return;
        }

    wxPrintout* printout = view->OnCreatePrintout();
    if(!printout)
        {
        return;
        }

    wxPrintDialogData print_dialog_data(*print_data_);
    wxPrinter printer(&print_dialog_data);
    // WX !! Setting the last argument to 'false' to prevent displaying
    // a dialog before printing causes nothing to print on the msw
    // platform; probably the printer doesn't get initialized. Make
    // this a user-configurable option once that problem is fixed.
    if(!printer.Print(view->GetFrame(), printout, true))
        {
        if(wxPRINTER_CANCELLED != wxPrinter::GetLastError())
            {
            wxMessageBox
                ("Printing failed. Check printer settings and try again."
                ,"Print failure."
                );
            }
        }
    delete printout;
}

/// Use a popup menu, instead of wxGetSingleChoiceData with strings
/// that are not generally appropriate. Our users don't understand
/// "Select a document template", they'd rather not have to hit
/// Enter after typing the initial letter of the template, and they
/// find the dialog frame distracting.
///
/// GWC replaced 'wxGetSingleChoiceData', but elsewhere made only
/// trivial changes (using const accessors instead of data members)
/// to get this to compile in a translation unit outside the library.

wxDocTemplate* DocManagerEx::SelectDocumentType
    (wxDocTemplate** templates
    ,int             noTemplates
    ,bool            sort
    )
{
    wxArrayString strings;
    wxDocTemplate** data = new(wx) wxDocTemplate*[noTemplates];
    int i;
    int n = 0;

    for(i = 0; i < noTemplates; ++i)
    {
        if(templates[i]->IsVisible())
        {
            int j;
            bool want = TRUE;
            for(j = 0; j < n; ++j)
            {
                //filter out NOT unique documents + view combinations
                if( templates[i]->GetDocumentName() == data[j]->GetDocumentName() &&
                     templates[i]->GetViewName() == data[j]->GetViewName()
                   )
                    want = FALSE;
            }

            if(want)
            {
                strings.Add(templates[i]->GetDescription());

                data[n] = templates[i];
                n ++;
            }
        }
    }  // for

    if(sort)
    {
        strings.Sort(wxStringSortAscending);
        // Yes, this will be slow, but template lists
        // are typically short.
        int j;
        n = bourn_cast<int>(strings.Count());
        for(i = 0; i < n; ++i)
        {
            for(j = 0; j < noTemplates; ++j)
            {
                if(strings[i] == templates[j]->GetDescription())
                    data[i] = templates[j];
            }
        }
    }

    wxDocTemplate* theTemplate;

    switch(n)
    {
        case 0:
            // no visible templates, hence nothing to choose from
            theTemplate = nullptr;
            break;

        case 1:
            // don't propose the user to choose if he heas no choice
            theTemplate = data[0];
            break;

        default:
            // propose the user to choose one of several
// GWC changes begin
/*
            theTemplate = (wxDocTemplate*)wxGetSingleChoiceData
                          (
                            _("Select a document template"),
                            _("Templates"),
                            strings,
                            (void**)data,
                            wxFindSuitableParent()
                          );
*/
            int selection = SingleChoicePopupMenu(strings).Choose();
            theTemplate = nullptr;
            if(-1 != selection)
                {
                theTemplate = data[selection];
                }
// GWC changes end
    }

    delete[] data;

    return theTemplate;
}
