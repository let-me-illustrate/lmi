// Document view for illustrations.
//
// Copyright (C) 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.11 2005-05-01 00:50:28 chicares Exp $

// This is a derived work based on wxWindows file
//   samples/docvwmdi/view.cpp (C) 1998 Julian Smart and Markus Holzem
// which is covered by the wxWindows license.
//
// The originals were modified by GWC in 2003 to create a standalone
// view class customized for illustration documents, and in the later
// years given in the copyright notice above.

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif

#include "illustration_view.hpp"

#include "account_value.hpp"
#include "alert.hpp"
#include "argv0.hpp"
#include "illustration_document.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_text_formats.hpp"
#include "miscellany.hpp"
#include "system_command.hpp"
#include "timer.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <wx/app.h> // wxTheApp
#include <wx/html/htmlwin.h>
#include <wx/icon.h>
#include <wx/log.h> // TODO ?? Debugging only: consider removing.
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

#include <cstdio>
#include <fstream>
#include <ios>
#include <locale>
#include <sstream>
#include <stdexcept>

IMPLEMENT_DYNAMIC_CLASS(IllustrationView, ViewEx)

BEGIN_EVENT_TABLE(IllustrationView, ViewEx)
    EVT_MENU(XRCID("edit_cell"             ),IllustrationView::OnProperties)
    EVT_MENU(wxID_PREVIEW                   ,IllustrationView::OnPreviewPdf)
    EVT_MENU(wxID_PRINT                     ,IllustrationView::OnPrintPdf  )
    EVT_UPDATE_UI(wxID_SAVE                 ,IllustrationView::OnUpdateFileSave)
//    EVT_UPDATE_UI(wxID_SAVEAS               ,IllustrationView::OnUpdateFileSaveAs)
//    EVT_UPDATE_UI(XRCID("wxID_SAVEAS"      ),IllustrationView::OnUpdateFileSaveAs)
    EVT_MENU_OPEN(                           IllustrationView::OnMenuOpen  )
    EVT_UPDATE_UI(XRCID("edit_cell"        ),IllustrationView::OnUpdateProperties)

// There has to be a better way.
    EVT_UPDATE_UI(XRCID("edit_class"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("edit_case"        ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_cell"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_class"        ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("run_case"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_cell"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_class"      ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_case"       ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("print_spreadsheet"),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("paste_census"     ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("add_cell"         ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("delete_cells"     ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("expand_columns"   ),IllustrationView::OnUpdateInapplicable)
    EVT_UPDATE_UI(XRCID("shrink_columns"   ),IllustrationView::OnUpdateInapplicable)
END_EVENT_TABLE()

// WX !! The html string must be initialized here, because passing an
// empty string to wxHtmlWindow::SetPage() would crash the application.
//
IllustrationView::IllustrationView()
    :ViewEx                  ()
    ,html_window_            (0)
    ,is_phony_               (false)
    ,ledger_values_          (0)
    ,selected_values_as_html_("<html><body>Hello, world!</body></html>")
{
}

IllustrationView::~IllustrationView()
{
}

IllustrationDocument& IllustrationView::document() const
{
    return dynamic_cast<IllustrationDocument&>(*GetDocument());
}

wxWindow* IllustrationView::CreateChildWindow()
{
    return html_window_ = new(wx) wxHtmlWindow(GetFrame());
}

int IllustrationView::EditProperties()
{
// TODO ?? expunge the commented-out testing code
// (well, move it into skeleton with an alert-test menu)
//        status() << "Test..." << LMI_FLUSH;
//        warning() << "Test..." << LMI_FLUSH;
//        hobsons_choice() << "Test..." << LMI_FLUSH;
//        fatal_error() << "Test..." << LMI_FLUSH;

    if(is_phony_)
        {
warning() << "That command should have been disabled." << LMI_FLUSH;
        return wxID_CANCEL;
        }

    bool dirty = document().IsModified();

    Input edited_lmi_input = document().input_;

    XmlNotebook xml_notebook(GetFrame(), edited_lmi_input);
    int rc = xml_notebook.ShowModal();
    if(wxID_OK == rc)
        {
        if(document().input_ != edited_lmi_input)
            {
// wxLogMessage(document().input_.differing_fields(edited_lmi_input).c_str());
// wxLog::FlushActive();
            document().input_ = edited_lmi_input;
            dirty = true;
            }
        document().Modify(dirty);
        }
    return rc;
}

void IllustrationView::DisplaySelectedValuesAsHtml()
{
    selected_values_as_html_ = FormatSelectedValuesAsHtml(*ledger_values_);
    html_window_->SetPage(selected_values_as_html_.c_str());
}

wxIcon IllustrationView::Icon() const
{
    return IconFromXmlResource("illustration_view_icon");
}

wxMenuBar* IllustrationView::MenuBar() const
{
    wxMenuBar* menu_bar = MenuBarFromXmlResource("illustration_view_menu");
    if(!menu_bar)
        {
//        wxLogError("Unable to load 'illustration_view_menu'.");
//        wxLog::FlushActive();
        throw std::runtime_error("Unable to load 'illustration_view_menu'.");
        };
    return menu_bar;
}

bool IllustrationView::OnCreate(wxDocument* doc, long flags)
{
    if(flags & 8)
        {
        is_phony_ = true;
        return ViewEx::OnCreate(doc, flags);
        }

    if(wxID_OK != EditProperties())
        {
        return false;
        }

    if(!ViewEx::OnCreate(doc, flags))
        {
        return false;
        }

    Run();
    return true;
}

/* TODO ?? expunge
wxPrintout* IllustrationView::OnCreatePrintout()
{
    // WX !! The string argument seems pretty pointless: it simply
    // follows "Please wait while printing" in messagebox text. The
    // argument is documented as a "title", but the messagebox title
    // is immutably "Printing failed".
    wxHtmlPrintout* printout = new(wx) wxHtmlPrintout("");
    printout->SetHtmlText(selected_values_as_html_.c_str());
    return printout;
}
*/

void IllustrationView::OnMenuOpen(wxMenuEvent&)
{
// TODO ?? WX !! Never gets called. Does this need to be in the document class?
wxLogMessage("IllustrationView::OnMenuOpen() called.");
wxLog::FlushActive();

    wxMenuItem* file_save = MenuBar()->FindItem
        (XRCID("wxID_SAVE")
        );
    if(file_save)
        {
        file_save->Enable(!is_phony_ && document().IsModified());
        }

    wxMenuItem* file_save_as = MenuBar()->FindItem
        (XRCID("wxID_SAVEAS")
        );
    if(file_save_as)
        {
        file_save_as->Enable(!is_phony_);
        }

    wxMenuItem* edit_cell = MenuBar()->FindItem
        (XRCID("edit_cell")
        );
    if(edit_cell)
        {
        edit_cell->Enable(!is_phony_);
        }
}

void IllustrationView::OnPreviewPdf(wxCommandEvent&)
{
    Pdf("open");
}

void IllustrationView::OnPrintPdf(wxCommandEvent&)
{
    Pdf("print");
}

void IllustrationView::OnProperties(wxCommandEvent&)
{
// may have to check is_phony_ here--but that's bogus
    if(is_phony_)
        {
//        return;
        }
    if(wxID_OK == EditProperties())
        {
        Run();
        }
}

void IllustrationView::OnUpdateFileSave(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_ && document().IsModified());
}

void IllustrationView::OnUpdateFileSaveAs(wxUpdateUIEvent& e)
{
// TODO ?? Doesn't seem to get called.
wxLogMessage("OnUpdateFileSaveAs");
wxLog::FlushActive();
    e.Enable(!is_phony_);
}

void IllustrationView::OnUpdateInapplicable(wxUpdateUIEvent& e)
{
    e.Enable(false);
}

// TODO ?? Doesn't seem to handle the menuitem--just the toolbar.
void IllustrationView::OnUpdateProperties(wxUpdateUIEvent& e)
{
    e.Enable(!is_phony_);
}

void IllustrationView::Pdf(std::string const& action)
{
    // TODO ?? Experimental. Want an platform-independent solution.
    // Consider this one:
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/583826b8d2f14f2c?dmode=source
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/b05623f68906edbd?dmode=source
// and also
// http://groups-beta.google.com/group/comp.soft-sys.wxwindows/msg/900fd00738b9b71a?dmode=source

    if("open" != action && "print" != action)
        {
        warning()
            << "Action '"
            << action
            << "' unrecognized."
            << LMI_FLUSH
            ;
        return;
        }

    // TODO ?? This should be a configurable string.
    fs::path fop("C:\\fop-0.20.5");

    wxString z;
    document().GetPrintableName(z);
    std::string document_file(z.c_str());

    fs::path xml_out_file(fop / document_file);
    xml_out_file = fs::change_extension(xml_out_file, ".xml");

    fs::remove(xml_out_file);
    if(fs::exists(xml_out_file))
        {
        warning()
            << "Attempt to remove xml output file failed."
            << LMI_FLUSH
            ;
        return;
        }

    LMI_ASSERT(ledger_values_.get());
    fs::ofstream ofs(xml_out_file, std::ios_base::out | std::ios_base::trunc);
    ledger_values_->write(ofs);
    ofs.close();

    std::string xsl_name = ledger_values_->GetLedgerType().str() + ".xsl";
    fs::path xsl_file(fop / xsl_name);
    if(!fs::exists(xml_out_file))
        {
        warning()
            << "Unable to read file '"
            << xsl_file.string()
            << "'."
            << LMI_FLUSH
            ;
        return;
        }

    fs::ifstream ifs(xml_out_file);
    ifs >> std::noskipws;
    std::string s
        ((std::istreambuf_iterator<char>(ifs))
        ,std::istreambuf_iterator<char>()
        );

    fs::path pdf_out_file(fop / document_file);
    pdf_out_file = fs::change_extension(pdf_out_file, ".pdf");

    try
        {
        fs::remove(pdf_out_file);
        }
    catch(fs::filesystem_error const& e)
        {
//        status() << e.what() << LMI_FLUSH;
        std::string basename = fs::basename(pdf_out_file);
        basename += '-' + iso_8601_datestamp_terse() + ".pdf";
        pdf_out_file = fop / basename;
        status() << pdf_out_file.string() << std::flush;
        }
    if(fs::exists(pdf_out_file))
        {
        throw std::runtime_error("Oops");
        }

    std::ostringstream oss;
    oss
        << "CMD /c " << (fop / "fop").string()
        << " -xsl "  << '"' << xsl_file.string()     << '"'
        << " -xml "  << '"' << xml_out_file.string() << '"'
        << " "       << '"' << pdf_out_file.string() << '"'
        ;

    int rc0 = system_command(oss.str());

    // TODO ?? This seems not to catch any problem--perhaps the
    // batch file eats the error code.
    if(rc0)
        {
        warning()
            << "Report formatting failed.\n"
            << "The specific command that failed was '"
            << oss.str()
            << "'."
            << LMI_FLUSH
            ;
        return;
        }

    HINSTANCE rc1 = ShellExecute
        ((HWND)GetFrame()->GetHandle()
        ,action.c_str()
        ,pdf_out_file.string().c_str()
        ,0
        ,0 // directory
        ,SW_SHOWNORMAL
        );
    if(reinterpret_cast<long int>(rc1) <= 32)
        {
        warning()
            << "Failed to "
            << action
            << " file '"
            << pdf_out_file.string()
            << "'. If it was already open, then it could not"
            << " be updated--close it first, then try again."
            ;
        return;
        }
}

void IllustrationView::Run(Input* overriding_input)
{
    Timer timer;

    if(overriding_input)
        {
        document().input_ = *overriding_input;
        }

    // TODO ?? For now, convert input on the fly.
    IllusInputParms ihs_input;
    convert_to_ihs(ihs_input, document().input_);

    AccountValue av(ihs_input);
    av.SetDebugFilename(base_filename() + ".debug");
    av.RunAV();

    status() << "Calculate: " << timer.Stop().Report() << std::flush;

    // Can the copy be avoided?
    ledger_values_ = std::auto_ptr<Ledger>(new Ledger(av.LedgerValues()));
    DisplaySelectedValuesAsHtml();
}

void IllustrationView::SetLedger(Ledger const& values)
{
    ledger_values_ = std::auto_ptr<Ledger>(new Ledger(values));
}

