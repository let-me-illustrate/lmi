// Input 'notebook' (tabbed dialog) driven by xml resources.
//
// Copyright (C) 2003, 2004 Gregory W. Chicares.
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

// $Id: xml_notebook.cpp,v 1.1.1.1 2004-05-15 19:59:44 chicares Exp $

#ifdef __BORLANDC__
#   include "pchfile.hpp"
#   pragma hdrstop
#endif // __BORLANDC__

#include "xml_notebook.hpp"

#include "input.hpp"
#include "transferor.hpp"

#include <wx/checkbox.h>
#include <wx/ctrlsub.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/xrc/xmlres.h>

#include <sstream>
#include <stdexcept>

// TODO ?? Eventually these will come from a document class.
Input input;
TransferData transfer_data;

BEGIN_EVENT_TABLE(XmlNotebook, wxDialog)
    EVT_UPDATE_UI(XRCID("dialog_containing_notebook"), XmlNotebook::OnUpdateGUI)
    EVT_NOTEBOOK_PAGE_CHANGING(XRCID("input_notebook"), XmlNotebook::OnPageChanging)
    // This ID is not an XRCID, but rather a standard wx ID.
    EVT_BUTTON(wxID_OK, XmlNotebook::OnOK)
END_EVENT_TABLE()

// WX!! wxDIALOG_EX_CONTEXTHELP is unknown to wxxrc.
// 'Extra' styles such as wxWS_EX_VALIDATE_RECURSIVELY  that wxxrc
// recognizes can be specified as an <exstyle> attribute, which is
// tidier than setting them dynamically as in this message:
//   http://lists.wxwindows.org/archive/wxPython-users/msg15676.html
// But that technique is needed for wxDIALOG_EX_CONTEXTHELP, which
// wxxrc does not recognize; it must be specified here, before
// LoadDialog() is called.
XmlNotebook::XmlNotebook(wxWindow* parent)
{
    SetExtraStyle(GetExtraStyle() | wxDIALOG_EX_CONTEXTHELP);
    if
        (!wxXmlResource::Get()->LoadDialog
            (this
            ,parent
            ,"dialog_containing_notebook"
            )
        )
        {
        wxLogError("Unable to load dialog.");
        throw std::runtime_error("");
        }

// TODO ?? Eventually this will come from a document class.
    transfer_data = input;

    // TODO ?? Instead, iterate over a list of names.
    // Probably names in the input class--some controls don't have validators.
    // Then again, a multimap would permit mapping one datum to multiple controls.
    // However, customized input might omit some data.
    Bind("comments" ,transfer_data.comments);
    Bind("check0"   ,transfer_data.check0  );
    Bind("check1"   ,transfer_data.check1  );
    Bind("check2"   ,transfer_data.check2  );
    Bind("option0"  ,transfer_data.option0 );
    Bind("option1"  ,transfer_data.option1 );
    Bind("option2"  ,transfer_data.option2 );
    Bind("option3"  ,transfer_data.option3 );
}

XmlNotebook::~XmlNotebook()
{
}

// The wxxrc sample program suggests something like this:
//    XRCCTRL(*this, "comments", wxTextCtrl)->SetValidator
//        (wxTextValidator(wxFILTER_ALPHA, &comments)
//        );
// which requires the application to know the type of the control
// (wxTextCtrl in this example); but normal controls are derived from
// class wxWindow, on which wxWindow::SetValidator() can be called
// without knowing the control type.

void XmlNotebook::Bind(std::string const& name, std::string& data)
{
    int id = XRCID(name.c_str());
    wxWindow* window = FindWindow(id);
    if(!window)
        {
        wxLogError(("No control named '" + name + "'.").c_str());
        return;
        }
    window->SetValidator(Transferor(data, name));
}

// TODO ?? Explain why it's OK if the window doesn't exist.
void XmlNotebook::ConditionallyEnable
    (std::string const& input_name
    ,wxWindow&          control
    )
{
    wxControlWithItems* itembox = dynamic_cast<wxControlWithItems*>(&control);
    // wxControlWithItems encompasses:
    //   wxCheckListBox, wxChoice, wxComboBox, wxListBox
    wxRadioBox* radiobox = dynamic_cast<wxRadioBox*>(&control);
    wxCheckBox* checkbox = dynamic_cast<wxCheckBox*>(&control);
    // TODO ?? Later add support for
    //   wxSpinCtrl, wxTextCtrl, and perhaps wxStaticText
    // by supporting range and string datatypes.
    if(radiobox || itembox)
        {
        e_option const& datum = input[input_name].cast<e_option>();
        control.Enable(datum.enabled_);
        }
    if(checkbox)
        {
        input_datum const& datum = input[input_name].cast<input_datum>();
        control.Enable(datum.enabled_);
        }
}

void XmlNotebook::ConditionallyEnableItems
    (std::string const& input_name
    ,wxWindow&          control
    )
{
/* TODO ?? Assert that the input name matches the XRCID?
    int id = XRCID(input_name.c_str());
    wxWindow* window = FindWindow(id);
    if(control != window)
        {
        throw "Oops";
        }
*/

    wxRadioBox* radiobox = dynamic_cast<wxRadioBox*>(&control);
    wxControlWithItems* itembox = dynamic_cast<wxControlWithItems*>(&control);
    if(radiobox)
        {
        e_option const& datum = input[input_name].cast<e_option>();
        for(std::size_t j = 0; j < datum.cardinality(); ++j)
            {
            radiobox->Enable(j, datum.allowed_[j]);
            }
        radiobox->SetSelection(datum.ordinal());
        }
    else if(itembox)
        {
// WX!! Broken in wx-2.5.1:
// http://cvs.wxwidgets.org/viewcvs.cgi/wxWidgets/include/wx/ctrlsub.h?annotate=1.20
//                itembox->SetStringSelection("A");
// WX!! Broken in wx-2.5.1:
//                itembox->SetSelection(itembox->FindString("A"));
        e_option const& datum = input[input_name].cast<e_option>();
        // WX!! Freeze() doesn't seem to help much.
        itembox->Freeze();
        itembox->Clear();
        for(std::size_t j = 0; j < datum.cardinality(); ++j)
            {
            if(datum.allowed_[j])
                {
                itembox->Append(datum.strings()[j]);
                }
            }
        itembox->Select(datum.ordinal());
        itembox->Thaw();
        }
    else
        {
// TODO ?? Handle other control types eventually.
//        wxLogMessage("Oops!");
//        throw std::runtime_error("");
        }
}

// Just a demo--eventually do something meaningful with input.
void XmlNotebook::OnOK(wxCommandEvent& event)
{
    wxDialog::OnOK(event);
    if(0 == GetReturnCode())
        {
        return;
        }
    std::ostringstream oss;
    // TODO ?? Instead, iterate over a list of names.
    oss
        << "Testing data transfer...\n"
        << "comments: " << input.comments << '\n'
        << "check0: "   << input.check0   << '\n'
        << "check1: "   << input.check1   << '\n'
        << "check2: "   << input.check2   << '\n'
        << "option0: "  << input.option0  << '\n'
        << "option1: "  << input.option1  << '\n'
        << "...data transfer completed."
        ;
    wxLogMessage(oss.str().c_str());
    wxLog::FlushActive();
}

void XmlNotebook::OnPageChanging(wxNotebookEvent& event)
{
    if(!Validate())
        {
        event.Veto();
        }
    TransferDataFromWindow();
}

void XmlNotebook::OnUpdateGUI(wxUpdateUIEvent& event)
{
    // Exit immediately if nothing changed. The library calls this
    // function continually in idle time, and it's pointless to fret
    // over inputs that didn't change on this update because they've
    // already been handled. Complex processing of many inputs has
    // been observed to consume excessive CPU time when a malloc
    // debugger is running, so this optimization is significant.
    static TransferData cached_transfer_data;
    TransferDataFromWindow();
    if(cached_transfer_data == transfer_data)
        {
        return;
        }

    cached_transfer_data = transfer_data;
    // TODO ?? Abstract this. Since control enablement is a simple
    // function that works the same way on any control, this could
    // be driven by a matrix mapping all control states onto all
    // control states. Other needs may make it better to push
    // enablement into an input class.
    input = transfer_data;
    input.Harmonize();

    // This window could be held as a reference elsewhere.
    int id = XRCID("input_notebook");
    wxNotebook* notebook = dynamic_cast<wxNotebook*>(FindWindow(id));
    if(!notebook)
        {
        wxLogError("No notebook window.");
        return;
        }
    wxNotebookPage* page = notebook->GetPage(notebook->GetSelection());
    if(!page)
        {
        wxLogError("No page selected in notebook.");
        return;
        }
    wxWindowList::compatibility_iterator node = page->GetChildren().GetFirst();
//wxString s;
//s << "Number of child windows: " << page->GetChildren().GetCount() << '\n';
    while(node)
        {
        wxWindow* w = node->GetData();
//        s << w->GetClassInfo()->GetClassName() << '\n';
        // Some windows don't have validators--most static controls, e.g.
        Transferor* t = dynamic_cast<Transferor*>(w->GetValidator());
        if(t)
            {
//            s << "    with name '" << t->name() << "'\n";
            ConditionallyEnableItems(t->name(), *w);
            ConditionallyEnable     (t->name(), *w);
            }
        node = node->GetNext();
        }
//wxLogError(s);
}

// TODO ?? This shows that wxDialog::Validate() isn't generally useful
// for our purposes. The library calls it when the user signals that
// input is complete, e.g. by pressing the 'OK' button. Even though we
// call it here on notebook page changes too, this isn't ideal: users
// shouldn't be able to leave an invalid field. Refocusing the invalid
// field upon later detection of error isn't good enough: the user's
// train of thought has already moved on.
bool XmlNotebook::Validate()
{
    // It's not obvious whether wxDialog::Validate() should be called
    // here, or where in relation to other processing. Probably this
    // is the right place; it doesn't matter much because we won't use
    // this approach in production.
    wxDialog::Validate();

    // Don't signal an error when this function is called before the
    // dialog is fully created.
    if(!IsShown())
        {
        return true;
        }

    // A static 'diagnostics' control is required.
    // TODO ?? Needs to be done only when page changes?
    int id = XRCID("diagnostics");
    wxStaticText* diagnostics_window = dynamic_cast<wxStaticText*>(FindWindow(id));
    if(!diagnostics_window)
        {
        wxLogError("No diagnostics window.");
        return true;
        }

    if(std::string::npos != transfer_data.comments.find_first_of("0123456789"))
        {
        diagnostics_window->SetLabel("Error:\nDigits not allowed in comments field.");
        FindWindow(XRCID("comments"))->SetFocus();
        return false;
        }

    diagnostics_window->SetLabel("");
    return true;
}

