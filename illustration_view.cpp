// Document view for illustrations.
//
// Copyright (C) 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: illustration_view.cpp,v 1.1 2005-03-12 03:01:08 chicares Exp $

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
#include "calendar_date.hpp"
#include "illustration_document.hpp"
#include "inputillus.hpp"
#include "ledger.hpp"
#include "ledger_invariant.hpp"
#include "ledger_variant.hpp"
#include "miscellany.hpp"
#include "system_command.hpp"
#include "timer.hpp"
#include "value_cast.hpp"
#include "wx_new.hpp"
#include "xml_notebook.hpp"

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <wx/app.h> // wxTheApp
#include <wx/html/htmlwin.h>
// TODO ?? expunge #include <wx/html/htmprint.h>
#include <wx/icon.h>
#include <wx/log.h> // debugging only
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/toolbar.h>
#include <wx/xrc/xmlres.h>

// FSF !! Expunge this when we implement printing in a more general way.
#include "system_command.hpp"
#ifdef __WXMSW__
#   include <wx/msw/wrapwin.h>
#endif // __WXMSW__

#include <cstdio>
#include <fstream>
#include <ios>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace
{
// TODO ?? Move this to a place whence it can be reused, and add unit
// tests: it is suspected of formatting -100 as "-,100".
//
class comma_punct
    :public std::numpunct<char>
{
  protected:
    char do_thousands_sep() const {return ',';}
    std::string do_grouping() const {return "\3";}
  public:
    comma_punct()
        :std::numpunct<char>()
        {}
};
} // Unnamed namespace.

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

// WX !! The html string must be initialized here because passing an
// empty string to wxHtmlWindow::SetPage() crashes the application.
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

// The following functions probably should be factored out into a
// utility module.

std::string const IllustrationView::base_filename() const
{
    std::string t = GetDocument()->GetFilename().c_str();
    if(0 == t.size())
        {
        t = GetDocument()->GetTitle().c_str();
        }
// TODO ?? This assertion fires for new documents.
//    LMI_ASSERT(0 != t.size());
    boost::filesystem::path path(t);
    return path.leaf();
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

// No longer 'Format': it's now 'Display'
void IllustrationView::FormatSelectedValuesAsHtml()
{
    LedgerInvariant const& Invar = ledger_values_->GetLedgerInvariant();
    LedgerVariant   const& Curr_ = ledger_values_->GetCurrFull();
    LedgerVariant   const& Guar_ = ledger_values_->GetGuarFull();
    int max_length = ledger_values_->GetMaxLength();

    std::ostringstream oss;

    std::locale loc;
    std::locale new_loc(loc, new comma_punct);
    oss.imbue(new_loc);
    oss.setf(std::ios_base::fixed, std::ios_base::floatfield);

    oss
        << "<html>"
        << "<head><title>Let me illustrate...</title></head>"
        << "<body text=\"#000000\" bgcolor=\"#B0B0B0\">"

// TODO ?? expunge        
//        << "<div align=left>"
//        << "  <img src=\"your-logo-here.png\" alt=\"logo\" width=88 height=31>"
//        << "</div>"
//
//        << "<p>"
//        << "</p>"
//
//        << "<p>"
//        << "</p>"

        << "<p>"
        << " <h5>"
        << "  <font color=\"#804040\">"
        << "Calculation summary for "
        ;

    if(ledger_values_->GetIsComposite())
        {
        oss
            << " composite<br>"
            ;
        }
    else
        {
        oss
            << Invar.Insured1
            << "<br>"
            << Invar.Gender << ", " << Invar.Smoker
            << std::setprecision(0)
            << ", age " << Invar.Age
            << "<br>"
            ;

        if(is_subject_to_ill_reg(ledger_values_->GetLedgerType()))
            {
            oss
                << std::setprecision(2)
                << Invar.GuarPrem << "   guaranteed premium<br>"
                ;
            }

        oss
            << std::setprecision(2)
            << "<font color=\"#FFBEAF\">I think Rakesh and George wanted these:</font><br>"
            << Invar.InitGLP          << "   initial guideline level premium<br>"
            << Invar.InitGSP          << "   initial guideline single premium<br>"
            << Invar.InitSevenPayPrem << "   initial seven-pay premium<br>"
            << Invar.InitTgtPrem      << "   initial target premium<br>"
            << "<font color=\"#FFBEAF\">and Tom wanted these:</font><br>"
            << Invar.IsMec            << "   MEC status<br>"
            << Invar.InitBaseSpecAmt  << "   initial base specified amount<br>"
            << Invar.InitTermSpecAmt  << "   initial term specified amount<br>"
            << Invar.InitBaseSpecAmt + Invar.InitTermSpecAmt << "   initial total specified amount<br>"
            << Invar.GetStatePostalAbbrev() << "   state of jurisdiction<br>"

            << "<font color=\"#FFBEAF\">as well as commas in numbers (done)</font><br>"
            << "<font color=\"#FFBEAF\">and 'composite' instead of a blank name for composites (done)</font><br>"
            ;
        }

    oss
        << "  </font>"
        << " </h5>"
        << "</p>"

        << "<hr>"
        << "<table align=right>"
        << "  <tr>"
        << "    <th>Age</th> <th>Outlay</th>"
        << "    <th>GuarAV</th> <th>GuarCSV</th> <th>GuarDB</th>"
        << "    <th>CurrAV</th> <th>CurrCSV</th> <th>CurrDB</th>"
        << "  </tr>"
// TODO ?? expunge        << std::fixed
        ;

    for(int j = 0; j < max_length; ++j)
        {
        oss
            << "<tr>"
            << "<td>" << std::setprecision(0) << j + Invar.Age        << "</td>"
            << "<td>" << std::setprecision(2) << Invar.Outlay[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.AcctVal[j]     << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.CSVNet[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Guar_.EOYDeathBft[j] << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.AcctVal[j]     << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.CSVNet[j]      << "</td>"
            << "<td>" << std::setprecision(2) << Curr_.EOYDeathBft[j] << "</td>"
            << "</tr>"
            ;
        }

    oss
        << "</table>"
        << "</body>"
        ;
    selected_values_as_html_ = oss.str();
    html_window_->SetPage(selected_values_as_html_.c_str());
}

wxIcon const& IllustrationView::Icon() const
{
    static wxIcon const icon = wxICON(IDI_ILLUSTRATION_VIEW);
    return icon;
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

void IllustrationView::Pdf(std::string action)
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

    std::string xsl_name;
    switch(ledger_values_->GetLedgerType())
        {
        case e_ill_reg:
            {
            xsl_name = "IllReg.xsl";
            }
            break;
        case e_individual_private_placement:
            {
            xsl_name = "IllIndivPP.xsl";
            }
            break;
        case e_nasd:
            {
            xsl_name = "IllNASD.xsl";
            }
            break;
        default:
            {
            xsl_name = "IllNASD.xsl";
            }
        }
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

    timer.Stop().Reset().Start();

    AccountValue av(ihs_input);
    av.SetDebugFilename    (base_filename() + ".debug");
    av.RunAV();

    status() << "Calculate: " << timer.Stop().Report() << std::flush;

    // Can the copy be avoided?
    ledger_values_ = std::auto_ptr<Ledger>(new Ledger(av.LedgerValues()));
    FormatSelectedValuesAsHtml();
}

void IllustrationView::SetLedger(Ledger const& values)
{
    ledger_values_ = std::auto_ptr<Ledger>(new Ledger(values));
}

// TODO ?? Obsolete section begins. This belongs in a separate file.

// GWC is the sole author of the following code adapted from the
// legacy branch.

#include "configurable_settings.hpp"
#include "financial.hpp"
#include "global_settings.hpp"
#include "input_sequence.hpp"
#include "ncnnnpnn.hpp"
#include "security.hpp"

// Experimental--see development notes for 2003-03-04 for a discussion
// of other approaches.
namespace
{
// Subtract loans and WDs from gross payments, yielding net payments.
// TODO ?? Obsolete--use LedgerInvariant::Outlay instead.
void set_net_payment
    (LedgerInvariant const& Invar
    ,std::vector<double>&   net_payment
    )
{
    net_payment = Invar.GrossPmt;
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,Invar.Loan.begin()
        ,net_payment.begin()
        ,std::minus<double>()
        );
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,Invar.NetWD.begin()
        ,net_payment.begin()
        ,std::minus<double>()
        );
}

// We can't use member templates with bc++5.02, so we need workarounds
// like this.
template<typename EnumType, unsigned int N>
std::vector<std::string> enum_vector_to_string_vector
    (std::vector<xenum<EnumType, N> > const& ve
    )
{
    std::vector<std::string> vs;
    typename std::vector<xenum<EnumType, N> >::const_iterator ve_i;
    for(ve_i = ve.begin(); ve_i != ve.end(); ++ve_i)
        {
        vs.push_back(ve_i->str());
        }
    return vs;
}
} // Unnamed namespace.

namespace
{
// Truncate string if necessary so that it's no longer than the given
// length, truncating at the last feasible terminal if any is given.
// Thus, an input sequence in its canonical form, with a ';' terminal
// at the end of each clause, is truncated at the last clause that can
// be completely represented within the given length. The function
// name uses 'width' because it's intended to be called in a context
// where string length is conformed to page width.
std::string truncate_to_width
    (std::string const& a_s
    ,unsigned int max_length
    ,std::string const& terminals
    )
{
    std::string s(a_s);
    if(max_length < s.size())
        {
        static std::string const ellipsis("...");
        s.resize(max_length - ellipsis.size());
        std::string::size_type xpos = s.find_last_of(terminals, max_length);
        if(std::string::npos != xpos)
            {
            s.resize(xpos);
            }
        s += ellipsis;
        }
    return s;
}

std::string show_as_sequence
    (std::vector<double> const& v
    ,unsigned int max_length
    )
{
    std::string s(InputSequence(v).mathematical_representation());
    return truncate_to_width(s, max_length, ";");
}

template<typename EnumType, unsigned int N>
std::string show_as_sequence
    (std::vector<xenum<EnumType, N> > const& ve
    ,unsigned int max_length
    )
{
    std::vector<std::string> vs(enum_vector_to_string_vector(ve));
    std::string s(InputSequence(vs).mathematical_representation());
    return truncate_to_width(s, max_length, ";");
}

std::string show_tiered_table
    (std::vector<double> const& bands
    ,std::vector<double> const& data
    )
{
    std::ostringstream oss;
    // An arbitrary limit that, for the nonce at least, is small
    // enough to keep the whole cover sheet on one page.
    std::vector<double>::size_type const max_rows = 10;
    for(std::vector<double>::size_type j = 0; j < std::min(max_rows, bands.size()); ++j)
        {
        oss << "    ";
        oss << value_cast_ihs<std::string>(data[j]);
        if(bands.size() == 1)
            {
            oss << " on all assets\n";
            }
        else if(0 == j)
            {
            oss << " on the first ";
            oss << value_cast_ihs<std::string>(bands[j]) << '\n';
            }
        else if(1 < bands.size() - j)
            {
            oss << " on the next ";
            oss << value_cast_ihs<std::string>(bands[j]) << '\n';
            }
        else
            {
            oss << " on all further assets\n";
            }
        }
    if(max_rows < bands.size())
        {
        oss << "[further bands omitted]\n";
        }
    return oss.str();
}
} // Unnamed namespace.

#if 0 // TODO ?? expunge
//==============================================================================
void IllustrationView::PrintCoverPage
    (Ledger const& a_LedgerValues
    ,std::string const& a_spreadsheet_range
    )
{
    int const width = 100; // TODO ?? This is completely arbitrary.

    LedgerInvariant const& Invar = a_LedgerValues.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = a_LedgerValues.GetCurrFull();

    std::string preparation_date(calendar_date().str());
    if(global_settings::instance().regression_testing())
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        preparation_date = Invar.EffDate;
        }

    std::ostringstream oss;
    oss
        << "        " << Invar.PolicyMktgName << '\n'
        << "        " << Invar.PolicyLegalName << " Cover Sheet\n"
        << '\n'
        << "The purpose of the attached illustration is to show how the "
           "performance of the underlying separate account divisions "
           "could affect the proposed\n"
        << "contract's cash values and death benefits. The illustration "
           "is hypothetical and may not be used to project or predict "
           "investment results. The illustration\n"
        << "must be accompanied or preceded by a Confidential Private "
           "Placement Memorandum offering the Contract.\n"
        << '\n'
        << "Placement agent: " << Invar.MainUnderwriter << ".\n"
        << '\n'
        ;
    if("" != Invar.ScaleUnit())
        {
        oss
            << "Values per " << Invar.ScaleUnit() << " US dollars.\n"
            << '\n'
            ;
        }
    oss
        << "        THIS ILLUSTRATION COVER SHEET IS FOR REGISTERED "
           "REPRESENTATIVE USE ONLY. NOT FOR USE WITH CLIENTS.\n"
        << '\n'
        << "Date prepared: " << preparation_date << '\n'
        << "Producer: " << truncate_to_width(Invar.ProducerName, width, "") << '\n'
        << "Client: " << truncate_to_width(Invar.Insured1, width, "") << '\n'
        << "Gender: " << Invar.Gender << '\n'
        << "Issue age: " << Invar.Age << '\n'
        << "State of jurisdiction: " << Invar.GetStatePostalAbbrev() << '\n'
        << "Country: " << Invar.CountryIso3166Abbrev << '\n'
        << "Non-US corridor factor: " << show_as_sequence(Invar.CorridorFactor, width) << '\n'
        << "Rate class: " << Invar.UWClass << '\n'
        << "7702 test: " << Invar.DefnLifeIns << '\n'
        << "MEC: " << e_yes_or_no(enum_yes_or_no(Invar.IsMec)) << '\n'
        << "Assumed gross rate: " << ncnnnpnn(Curr_.InitAnnSepAcctGrossRate()) << '\n'
        << "New cash loan: " << show_as_sequence(Invar.Loan, width) << '\n'
        << "Withdrawal: " << show_as_sequence(Invar.NetWD, width) << '\n'
        << "Comments: " << truncate_to_width(Invar.Comments, width, "") << '\n'
        << '\n'
        << "        Premiums and death benefits\n"
        << '\n'
        << "Premium: " << show_as_sequence(Invar.GrossPmt, width) << '\n'
        << "Dumpin: " << (Invar.Dumpin * Invar.ScaleFactor()) << '\n'
        << "Internal 1035 exchange: " << (Invar.Internal1035Amount * Invar.ScaleFactor()) << '\n'
        << "External 1035 exchange: " << (Invar.External1035Amount * Invar.ScaleFactor()) << '\n'
        << "Specified amount: " << show_as_sequence(Invar.SpecAmt, width) << '\n'
        << "Death benefit option: " << show_as_sequence(Invar.DBOpt, width) << '\n'
        << '\n'
        << "        Current fees and charges\n"
        << '\n'
        << "State premium tax load: " << value_cast_ihs<std::string>(Invar.GetStatePremTaxLoad()) << '\n'
        << "DAC tax premium load: " << value_cast_ihs<std::string>(Invar.GetDacTaxPremLoadRate()) << '\n'
        << "Asset-based compensation: " << show_as_sequence(Invar.AddonCompOnAssets, width) << '\n'
        << "Premium-based compensation: " << show_as_sequence(Invar.AddonCompOnPremium, width) << '\n'
        << "Add-on custodial fee: " << show_as_sequence(Invar.AddonMonthlyFee, width) << '\n'
        << "Country COI multiplier: " << value_cast_ihs<std::string>(Invar.CountryCOIMultiplier) << '\n'
        << "Separate account load: " << '\n'
        << show_tiered_table(Invar.TieredSepAcctLoadBands, Invar.TieredSepAcctLoadRates) << '\n'
        ;

    // COMPILER !! gcc-2.95.2-1 We'd rather just reset the stringstream's
    // get pointer, but that doesn't work with gcc-2.95.2-1 and Magnus
    // Fromreide's <sstream>.
    // TODO ?? So make this conditional.
    std::istringstream iss(oss.str(), std::ios_base::in | std::ios_base::binary);
    std::vector<std::string> vs;
    std::string line;
    while(std::getline(iss, line, '\n'))
        {
        vs.push_back(line);
        }
    unsigned int const cover_page_max_lines = 60;
    if(cover_page_max_lines < vs.size())
        {
        warning() << "Cover sheet truncated to fit on one page." << LMI_FLUSH;
        }
    vs.resize(cover_page_max_lines);
// TODO ?? WX PORT !!    SetRange(a_spreadsheet_range, vs, vs.size());
// If needed, return as string.
}
#endif // 0 [expunge]

//==============================================================================
//void IllustrationView::PrintFormTabDelimited
void PrintFormTabDelimited
    (Ledger const& a_LedgerValues
    ,std::string const& file_name
//    ) const
    )
{
    LedgerInvariant const& Invar = a_LedgerValues.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = a_LedgerValues.GetCurrFull();
    LedgerVariant   const& Guar_ = a_LedgerValues.GetGuarFull();

    int max_length = a_LedgerValues.GetMaxLength();

    std::vector<double> net_payment;
    set_net_payment(Invar, net_payment);

    std::vector<double> real_claims;
    if(a_LedgerValues.GetIsComposite())
        {
        real_claims = Curr_.ClaimsPaid;
        }
    else
        {
        real_claims.assign(Curr_.ClaimsPaid.size(), 0.0);
        }

    std::vector<double> cash_flow;
    cash_flow.push_back(0.0); // No claims paid on issue date.
    std::copy
        (real_claims.begin()
        ,real_claims.end()
        ,std::inserter(cash_flow, cash_flow.end())
        );
    std::transform
        (cash_flow.begin()
        ,cash_flow.end()
        ,cash_flow.begin()
        ,std::negate<double>()
        );
    std::transform
        (net_payment.begin()
        ,net_payment.end()
        ,cash_flow.begin()
        ,cash_flow.begin()
        ,std::plus<double>()
        );

    cash_flow.pop_back(); // Here we no longer need cash_flow[omega].

    std::vector<double> csv_plus_claims(Curr_.CSVNet);
    std::transform
        (csv_plus_claims.begin()
        ,csv_plus_claims.end()
        ,real_claims.begin()
        ,csv_plus_claims.begin()
        ,std::plus<double>()
        );
// TODO ?? Is this irr valid?        
    std::vector<double> irr_on_surrender(Curr_.CSVNet.size());
    if(!Invar.IsInforce)
        {
        irr
            (cash_flow
            ,csv_plus_claims
            ,irr_on_surrender
            ,static_cast<std::vector<double>::size_type>(Curr_.LapseYear)
            ,max_length
            ,Invar.irr_precision
            );

#ifdef DEBUGGING_IRR
std::ofstream os
        ("irr.txt"
        ,   std::ios_base::out
          | std::ios_base::ate
          | std::ios_base::app
        );
os
<< "  IllustrationView::PrintFormTabDelimited():\n"
        << "\n\tcash_flow.size() = " << cash_flow.size()
        << "\n\tcsv_plus_claims.size() = " << csv_plus_claims.size()
        << "\n\tirr_on_surrender.size() = " << irr_on_surrender.size()
        << "\n\tCurr_.LapseYear = " << Curr_.LapseYear
        << "\n\tmax_length = " << max_length
        ;
os << "\n\tcash_flow = ";
std::copy(cash_flow.begin(), cash_flow.end(), std::ostream_iterator<double>(os, " "));
os << "\n\tcsv_plus_claims = ";
std::copy(csv_plus_claims.begin(), csv_plus_claims.end(), std::ostream_iterator<double>(os, " "));
os << "\n\tirr_on_surrender = ";
std::copy(irr_on_surrender.begin(), irr_on_surrender.end(), std::ostream_iterator<double>(os, " "));
os << "\n\n" ;
#endif // DEBUGGING_IRR
        }

    std::vector<double> db_plus_claims(Curr_.EOYDeathBft);
    std::transform
        (db_plus_claims.begin()
        ,db_plus_claims.end()
        ,real_claims.begin()
        ,db_plus_claims.begin()
        ,std::plus<double>()
        );
    std::vector<double> irr_on_death(Curr_.EOYDeathBft.size(), -1.0);
    if(!Invar.IsInforce)
        {
        irr
            (cash_flow
            ,db_plus_claims
            ,irr_on_death
            ,static_cast<std::vector<double>::size_type>(Curr_.LapseYear)
            ,max_length
            ,Invar.irr_precision
            );
        }

    std::ofstream os
        (file_name.c_str()
        ,  std::ios_base::out
         | std::ios_base::ate
         | std::ios_base::app
        );

    os << "\n\nFOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.\n\n";

    os << "ProducerName\t\t"      << Invar.value_str("ProducerName"   ) << '\n';
    os << "ProducerStreet\t\t"    << Invar.value_str("ProducerStreet" ) << '\n';
    os << "ProducerCity\t\t"      << Invar.value_str("ProducerCity"   ) << '\n';
    os << "CorpName\t\t"          << Invar.value_str("CorpName"       ) << '\n';
    os << "Insured1\t\t"          << Invar.value_str("Insured1"       ) << '\n';
    os << "Gender\t\t"            << Invar.value_str("Gender"         ) << '\n';
    os << "Smoker\t\t"            << Invar.value_str("Smoker"         ) << '\n';
    os << "IssueAge\t\t"          << Invar.value_str("Age"            ) << '\n';
    os << "InitBaseSpecAmt\t\t"   << Invar.value_str("InitBaseSpecAmt") << '\n';
    os << "InitTermSpecAmt\t\t"   << Invar.value_str("InitTermSpecAmt") << '\n';
    double total_spec_amt = Invar.InitBaseSpecAmt + Invar.InitTermSpecAmt;
    os << "  Total:\t\t"     << value_cast_ihs<std::string>(total_spec_amt) << '\n';
    os << "PolicyMktgName\t\t"    << Invar.value_str("PolicyMktgName" ) << '\n';
    os << "PolicyLegalName\t\t"   << Invar.value_str("PolicyLegalName") << '\n';
    os << "PolicyForm\t\t"        << Invar.value_str("PolicyForm"     ) << '\n';
    os << "UWClass\t\t"           << Invar.value_str("UWClass"        ) << '\n';
    os << "UWType\t\t"            << Invar.value_str("UWType"         ) << '\n';

    // We surround the date in single quotes because one popular
    // spreadsheet would otherwise interpret it as a date, which
    // is likely not to fit in a default-width cell.
    if(!global_settings::instance().regression_testing())
        {
        // Skip security validation for the most privileged password.
        validate_security(!global_settings::instance().ash_nazg);
        os << "DatePrepared\t\t'" << calendar_date().str() << "'\n";
        }
    else
        {
        // For regression tests, use EffDate as date prepared,
        // in order to avoid gratuitous failures.
        os << "DatePrepared\t\t'" << Invar.EffDate << "'\n";
        }

        os << '\n';

    // This column ordering reflects the natural processing order, but
    // with the most commonly wanted columns placed first.
    char const* cheaders[] =
        {"PolicyYear"
        ,"AttainedAge"
        ,"DeathBenefitOption"
        ,"TotalNetOutlay"
        ,"GuaranteedAccountValue"
        ,"GuaranteedNetCashSurrenderValue"
        ,"GuaranteedYearEndDeathBenefit"
        ,"CurrentAccountValue"
        ,"CurrentNetCashSurrenderValue"
        ,"CurrentYearEndDeathBenefit"
        ,"IrrOnSurrender"
        ,"IrrOnDeath"
        ,"EmployeeGrossPremium"
        ,"CorporationGrossPremium"
        ,"GrossWithdrawal"
        ,"NewCashLoan"
        ,"NetPayment"
        ,"PremiumTaxLoad"
        ,"DacTaxLoad"
        ,"MonthlyPolicyFee"
        ,"AnnualPolicyFee"
        ,"SpecifiedAmountLoad"
        ,"AccountValueLoadBeforeMonthlyDeduction"
        ,"AccountValueLoadAfterMonthlyDeduction"
        ,"MonthlyFlatExtra"
        ,"MortalityCharge"
        ,"SeparateAccountInterestRate"
        ,"GeneralAccountInterestRate"
        ,"GrossInterestCredited"
        ,"NetInterestCredited"
        ,"YearEndInforceLives"
        ,"ClaimsPaid"
        ,"ProducerCompensation"
        };

    std::vector<std::vector<std::string> > sheaders;

    unsigned int max_header_rows = 0;
    for(unsigned int j = 0; j < lmi_array_size(cheaders); ++j)
        {
//        std::istringstream iss(insert_spaces_between_words(cheaders[j]));
        std::istringstream iss(cheaders[j]);
        std::vector<std::string> v;
        std::copy
            (std::istream_iterator<std::string>(iss)
            ,std::istream_iterator<std::string>()
            ,std::back_inserter(v)
            );
        sheaders.push_back(v);
        max_header_rows = std::max(max_header_rows, v.size());
        }
    std::vector<std::vector<std::string> >::iterator shi;
    for(shi = sheaders.begin(); shi != sheaders.end(); ++shi)
        {
        std::reverse(shi->begin(), shi->end());
        shi->resize(max_header_rows);
        std::reverse(shi->begin(), shi->end());
        }
    for(unsigned int j = 0; j < max_header_rows; ++j)
        {
        for(shi = sheaders.begin(); shi != sheaders.end(); ++shi)
            {
            os << (*shi)[j] << '\t';
            }
        os << '\n';
        }
    os << '\n';

    for(int j = 0; j < max_length; ++j)
        {
        os << (j + 1                                    ) << '\t';
        os << (j + Invar.Age                            ) << '\t';

        os << Invar.DBOpt[j]                              << '\t';

        double net_outlay =
              Invar.GrossPmt[j]
            - Invar.NetWD     [j]
            - Invar.Loan      [j]
            ;
        os << value_cast_ihs<std::string>(net_outlay) << '\t';

        os << Guar_.value_str("AcctVal"               ,j) << '\t';
        os << Guar_.value_str("CSVNet"                ,j) << '\t';
        os << Guar_.value_str("EOYDeathBft"           ,j) << '\t';
        os << Curr_.value_str("AcctVal"               ,j) << '\t';
        os << Curr_.value_str("CSVNet"                ,j) << '\t';
        os << Curr_.value_str("EOYDeathBft"           ,j) << '\t';

        if(Invar.IsInforce)
            {
            os << "(inforce)"                             << '\t';
            os << "(inforce)"                             << '\t';
            }
        else
            {
            os << irr_on_surrender[j]                     << '\t';
            os << irr_on_death[j]                         << '\t';
            }

        os << Invar.value_str("EeGrossPmt"            ,j) << '\t';
        os << Invar.value_str("ErGrossPmt"            ,j) << '\t';
        os << Invar.value_str("NetWD"                 ,j) << '\t'; // TODO ?? It's *gross* WD.
        os << Invar.value_str("Loan"                  ,j) << '\t';
        os << Curr_.value_str("NetPmt"                ,j) << '\t';

        os << Curr_.value_str("PremTaxLoad"           ,j) << '\t';
        os << Curr_.value_str("DacTaxLoad"            ,j) << '\t';
        os << Curr_.value_str("MlyPolFee"             ,j) << '\t';
        os << Curr_.value_str("AnnPolFee"             ,j) << '\t';
        os << Curr_.value_str("SpecAmtLoad"           ,j) << '\t';
        os << Curr_.value_str("AcctValLoadBOM"        ,j) << '\t';
        os << Curr_.value_str("AcctValLoadAMD"        ,j) << '\t';
        os << Invar.value_str("MonthlyFlatExtra"      ,j) << '\t';
        os << Curr_.value_str("COICharge"             ,j) << '\t';

        os << Curr_.value_str("AnnSAIntRate"          ,j) << '\t';
        os << Curr_.value_str("AnnGAIntRate"          ,j) << '\t';
        os << Curr_.value_str("GrossIntCredited"      ,j) << '\t';
        os << Curr_.value_str("NetIntCredited"        ,j) << '\t';

        // First element of InforceLives is BOY--show only EOY.
        os << value_cast_ihs<std::string>(Invar.InforceLives[1 + j]) << '\t';
        os << Curr_.value_str("ClaimsPaid"            ,j) << '\t';
        os << Invar.value_str("ProducerCompensation"  ,j) << '\t';

        os << '\n';
        }
// TODO ?? Should we add death benefit option?
}

//==============================================================================
void IllustrationView::PrintFormSpecial(Ledger const& a_LedgerValues)
{
    LedgerInvariant const& Invar = a_LedgerValues.GetLedgerInvariant();
    LedgerVariant   const& Curr_ = a_LedgerValues.GetCurrFull();

    std::ofstream os
        (configurable_settings::instance().custom_output_filename().c_str()
        ,std::ios_base::out | std::ios_base::trunc
        );

    if(!os.good())
        {
        hobsons_choice() << "Error initializing output file." << LMI_FLUSH;
        }

    os
        << "CashValu,SurrValu,DeathBen,IntEarned,"
        << "MortCost,Load,MinPrem,SurrCost,PremAmt,IntRate\n"
        ;

// TODO ?? "??? Is this exactly what the customer wanted?"
    // Surr charge = acct val - cash surr val
    std::vector<double> surr_chg(Curr_.AcctVal);
    std::transform
        (surr_chg.begin()
        ,surr_chg.end()
        ,Curr_.CSVNet.begin()
        ,surr_chg.begin()
        ,std::minus<double>()
        );

    // Load = gross pmt - net pmt.
    std::vector<double> prem_load(Invar.GrossPmt);
    std::transform
        (prem_load.begin()
        ,prem_load.end()
        ,Curr_.NetPmt.begin()
        ,prem_load.begin()
        ,std::minus<double>()
        );

    os.setf(std::ios_base::fixed, std::ios_base::floatfield);

    int max_duration = static_cast<int>(std::min(95.0, Invar.EndtAge) - Invar.Age);
    for(int j = 0; j < max_duration; j++)
        {
        // Customer requirement: show interest rate in bp.
        double gen_acct_int_rate_bp = 10000.0 * Curr_.AnnGAIntRate[j];
        os
            << std::setprecision(0) << Curr_.AcctVal         [j] << ','
            << std::setprecision(0) << Curr_.CSVNet          [j] << ','
            << std::setprecision(0) << Curr_.EOYDeathBft     [j] << ','
            << std::setprecision(0) << Curr_.NetIntCredited  [j] << ','
            << std::setprecision(0) << Curr_.COICharge       [j] << ','
            // Assume 'min prem' is zero--see comments below.
            << std::setprecision(0) << 0 << ','
            << std::setprecision(0) << prem_load             [j] << ','
            << std::setprecision(0) << surr_chg              [j] << ','
            << std::setprecision(0) << Invar.GrossPmt        [j] << ','
            << std::setprecision(0) << gen_acct_int_rate_bp
            << std::setprecision(0) << '\n'
            ;
        }
/*
TODO ?? Resolve these comments.

dir where installed: instead, dir from which run
age 95 even though pol goes to 100
behavior if file locked?

values: all as of EOY
assume "interest earned" is net interest credited, net of any spread
assume "mortality cost" is sum of actual COIs deducted throughout the year
assume "load" is premium load including any sales load and premium-based
  loads for premium tax and dac tax, but excluding policy fee
assume "minimum premium" is a required premium as is typical of interest
  sensitive whole life, and should be zero for flexible premium universal life
assume "surrender cost" is account value minus cash surrender value; if
  there is any refund in the early years, this value can be negative
assume file is terminated with a CRLF at the end of the last line,
  with no EOF character following

*/
    if(!os.good())
        {
        hobsons_choice() << "Error writing output file." << LMI_FLUSH;
        }
}

