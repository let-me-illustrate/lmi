// Customize implementation details of library class wxDocManager.
//
// Copyright (C) 2002, 2003, 2004 Gregory W. Chicares.
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
// http://groups.yahoo.com/group/actuarialsoftware
// email: <chicares@mindspring.com>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id: docmanager_ex.cpp,v 1.1.1.1 2004-05-15 19:58:27 chicares Exp $

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
// and in the later years given in the copyright notice above.

#include "pchfile.hpp"

#ifdef __BORLANDC__
#   pragma hdrstop
#endif

#include "docmanager_ex.hpp"

#include "previewframe_ex.hpp"
#include "wx_new.hpp"

#include <wx/app.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/printdlg.h>

IMPLEMENT_DYNAMIC_CLASS(DocManagerEx, wxDocManager)

BEGIN_EVENT_TABLE(DocManagerEx, wxDocManager)
    EVT_MENU(wxID_PREVIEW, DocManagerEx::OnPreview)
    EVT_MENU(wxID_PRINT, DocManagerEx::OnPrint)
    // WX!! Propose adding wxID_PAGE_SETUP to the library.
    // In msw, 'Page setup' has superseded 'Print setup'. But wx
    // provides a builtin ID only for the former and not for the
    // latter. Yet wxxrc knows only builtin IDs. Therefore, to
    // make the modern 'Page setup' available with wxxrc,
    // wxID_PRINT_SETUP is hijacked and used for 'page setup'.
    EVT_MENU(wxID_PRINT_SETUP, DocManagerEx::OnPageSetup)
END_EVENT_TABLE()

DocManagerEx::DocManagerEx(long flags, bool initialize)
    :wxDocManager     (flags, initialize)
    ,print_data_      (new(wx) wxPrintData)
    ,page_setup_data_ (new(wx) wxPageSetupDialogData)
{
}

DocManagerEx::~DocManagerEx()
{
}

// WX!! Perhaps xrc could support a menu-use-file-history tag,
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

void DocManagerEx::OnPageSetup(wxCommandEvent&)
{
    *page_setup_data_ = *print_data_;

    wxPageSetupDialog pageSetupDialog
        (wxTheApp->GetTopWindow()
        ,page_setup_data_.get()
        );
    pageSetupDialog.ShowModal();

    *print_data_ = pageSetupDialog.GetPageSetupData().GetPrintData();
    *page_setup_data_ = pageSetupDialog.GetPageSetupData();
}

void DocManagerEx::OnPreview(wxCommandEvent&)
{
    wxView* view = GetCurrentView();
    if(!view)
        {
        return;
        }

    wxPrintout *printout = view->OnCreatePrintout();
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
    PreviewFrameEx *frame = new PreviewFrameEx(preview);
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
}

void DocManagerEx::OnPrint(wxCommandEvent&)
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
    // TODO ?? Change last argument to 'true' to display dialog before
    // printing. This should be a user-configurable option.
    if(!printer.Print(view->GetFrame(), printout, false))
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

