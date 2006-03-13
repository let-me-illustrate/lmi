// Product editor.
//
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: pedit.cpp,v 1.1.2.1 2006-03-13 19:56:52 etarassov Exp $

#include "pedit.hpp"
#include "dbnames.hpp"

#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/exception.hpp>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/snglinst.h>
#include <wx/log.h>

// Product editor menu items
enum
{
    MDI_QUIT = wxID_EXIT,
    MDI_OPEN_FILE = wxID_HIGHEST + 1000,
    MDI_SAVE_FILE,
    MDI_SAVE_FILE_AS,
    MDI_SAVE_ALL_FILES,
    MDI_CLOSE_FILE,
    MDI_CLOSE_ALL_FILES,
    MDI_ABOUT = wxID_ABOUT
};

// -----------------------------
// event table of PeditFrame
// -----------------------------
BEGIN_EVENT_TABLE( PeditFrame, wxMDIParentFrame )
    EVT_MENU( MDI_ABOUT,            PeditFrame::OnAbout )
    EVT_MENU( MDI_OPEN_FILE,        PeditFrame::OnOpenFile )
    EVT_MENU( MDI_SAVE_FILE,        PeditFrame::OnSaveFile )
    EVT_MENU( MDI_SAVE_FILE_AS,     PeditFrame::OnSaveFileAs )
    EVT_MENU( MDI_SAVE_ALL_FILES,   PeditFrame::OnSaveAllFiles )
    EVT_MENU( MDI_CLOSE_FILE,       PeditFrame::OnCloseActiveFile )
    EVT_MENU( MDI_CLOSE_ALL_FILES,  PeditFrame::OnCloseAllFiles )

    EVT_MENU( MDI_QUIT,             PeditFrame::OnQuit )

    EVT_CLOSE( PeditFrame::OnClose )

    EVT_UPDATE_UI( MDI_SAVE_FILE, PeditFrame::OnUpdateSaveFileMenuItem )
    EVT_UPDATE_UI( MDI_SAVE_FILE_AS, PeditFrame::OnUpdateMenuItem )
    EVT_UPDATE_UI( MDI_SAVE_ALL_FILES, PeditFrame::OnUpdateMenuItem )
    EVT_UPDATE_UI( MDI_CLOSE_FILE, PeditFrame::OnUpdateMenuItem )
    EVT_UPDATE_UI( MDI_CLOSE_ALL_FILES, PeditFrame::OnUpdateMenuItem )

END_EVENT_TABLE()

// -----------------------------
// PeditFrame implementation
// -----------------------------
void PeditFrame::OnUpdateMenuItem( wxUpdateUIEvent & event )
{
    event.Enable( GetActiveChild() != NULL );
}
void PeditFrame::OnUpdateSaveFileMenuItem( wxUpdateUIEvent & event )
{
    PeditFileFrame * win = dynamic_cast<PeditFileFrame *>( GetActiveChild() );
    event.Enable( win != NULL && win->IsModified() );
}

PeditFrame::PeditFrame( wxWindow *parent,
                        const wxWindowID id,
                        const wxString& title )
: wxMDIParentFrame( parent, id, title,
    Pedit::config.ReadPoint( _T("topwindow") ),
    Pedit::config.ReadSize( _T("topwindow"), wxSize(500, 400) ),
    wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL )
{
    if( Pedit::config.ReadBool( _T("topwindow.maximized"), true ) )
        Maximize();

    {   // Accelerators
        wxAcceleratorEntry entries[5];
        entries[0].Set( wxACCEL_CTRL, static_cast<int>( 'O' ), MDI_OPEN_FILE );
        entries[1].Set( wxACCEL_CTRL, static_cast<int>( 'S' ), MDI_SAVE_FILE );
        entries[2].Set( wxACCEL_CTRL | wxACCEL_SHIFT, static_cast<int>( 'S' ), MDI_SAVE_ALL_FILES );
        entries[3].Set( wxACCEL_CTRL, static_cast<int>( 'W' ), MDI_CLOSE_FILE );
        entries[4].Set( wxACCEL_CTRL | wxACCEL_SHIFT, static_cast<int>( 'W' ), MDI_CLOSE_ALL_FILES );
        wxAcceleratorTable accel( WXSIZEOF(entries), entries );
        SetAcceleratorTable( accel );
    }
    {   // Menubar
        wxMenu *file_menu = new wxMenu;
    
        file_menu->Append( MDI_OPEN_FILE,        _T("&Open\tCtrl-O"), _T("Opens data file into a new child data window") );
        file_menu->AppendSeparator();
        file_menu->Append( MDI_CLOSE_FILE,       _T("&Close\tCtrl-W"), _T("Close child window") );
        file_menu->Append( MDI_CLOSE_ALL_FILES,  _T("C&lose All\tCtrl-Shift-W"), _T("Close all windows") );
        file_menu->AppendSeparator();
        file_menu->Append( MDI_SAVE_FILE,        _T("&Save\tCtrl-S"), _T("Save data file") );
        file_menu->Append( MDI_SAVE_FILE_AS,     _T("Save &As..."), _T("Save data to another file") );
        file_menu->Append( MDI_SAVE_ALL_FILES,   _T("Sav&e All\tCtrl-Shift-S"), _T("Save all data files") );
        file_menu->AppendSeparator();
        file_menu->Append( MDI_QUIT,             _T("E&xit\tCtrl-X"), _T("Quit the program") );

        wxMenu *help_menu = new wxMenu;
        help_menu->Append( MDI_ABOUT, _T("&About\tF1") );

        wxMenuBar *menu_bar = new wxMenuBar;

        menu_bar->Append( file_menu, _T("&File") );
        menu_bar->Append( help_menu, _T("&Help") );
    
        SetMenuBar( menu_bar );
    }
    {   // Statusbar
        CreateStatusBar();
    }
}

PeditFrame::~PeditFrame()
{}

void PeditFrame::OnAbout( wxCommandEvent& WXUNUSED(event) )
{
    (void)wxMessageBox(_T("LMI Product Edito v0.1\n"),
                       _T("About LMI Product Editor"));
}

void PeditFrame::OnOpenFile( wxCommandEvent& WXUNUSED(event) )
{
    wxString filename = 
        ::wxFileSelector( _T("Open File"),
                          Pedit::config.ReadString( _T("filedir"), _T("") ), _T(""), _T(""),
                          PeditFileFrame::GetFilesWildcard().c_str(),
                          wxOPEN | wxFILE_MUST_EXIST );
    if( filename.empty() )
    { // canceled
        return;
    }

    try
    {
        boost::filesystem::path path( filename.c_str() );
        if( !boost::filesystem::is_directory( path ) )
            path = path.branch_path();
        Pedit::config.WriteString( _T("filedir"), path.string() );
    }
    catch( boost::filesystem::filesystem_error const & fse )
    {
        // Invalid filename selected - do not save directory into config
    }

    PeditFileFrame * child = PeditFileFrame::CreateInstance( this, filename.c_str() );
    if( child )
    {
        // TODO: decorate the window apropriatly
//        child->SetIcon(wxIcon( mondrian_xpm ));
    }
}

void PeditFrame::OnSaveFile(wxCommandEvent& WXUNUSED(event) )
{
    PeditFileFrame * win = dynamic_cast<PeditFileFrame*>( GetActiveChild() );
    if( win )
        win->Save();
}

void PeditFrame::OnSaveFileAs(wxCommandEvent& WXUNUSED(event) )
{
    PeditFileFrame * win = dynamic_cast<PeditFileFrame*>( GetActiveChild() );
    if( !win )
        return;
    boost::filesystem::path filename( win->GetFilename() );
    filename.normalize();
    std::string path_dir = filename.branch_path().string();
    std::string path_file = filename.leaf();
    wxString newFilename =
        ::wxFileSelector( _T("Save As"),
                          Pedit::config.ReadString( _T("filedir"), _T("") ),
                          path_file.c_str(),
                          _T("db4"),
                          _T("*.db4"),
                          wxSAVE | wxOVERWRITE_PROMPT,
                          this );
    if( newFilename.empty() )
    {   // cancelled
        return;
    }

    try
    {
        boost::filesystem::path path( newFilename.c_str() );
        if( !boost::filesystem::is_directory( path ) )
            path = path.branch_path();
        Pedit::config.WriteString( _T("filedir"), path.string() );
    }
    catch( boost::filesystem::filesystem_error const & fse )
    {
        // Invalid filename selected - do not save directory into config
    }

    win->SetFilename( newFilename.c_str() );
    win->Save();
}

void PeditFrame::OnSaveAllFiles( wxCommandEvent& WXUNUSED(event) )
{
    wxList & children = GetChildren();
    wxNode * node = children.GetFirst();
    while (node)
    {
        PeditFileFrame * win = dynamic_cast<PeditFileFrame*>( node->GetData() );
        if( win )
        {
            win->Save();
        }
        node = node->GetNext();
    }
}

void PeditFrame::OnCloseActiveFile(wxCommandEvent& WXUNUSED(event) )
{
    wxMDIChildFrame * win = GetActiveChild();
    if( win )
    {
        win->Close();
    }
}

void PeditFrame::OnQuit( wxCommandEvent& WXUNUSED(event) )
{
    Close();
}
void PeditFrame::OnClose( wxCloseEvent& event )
{
    if( !CloseAllFiles( &event ) )
    {   // workaround for vetoed close event still being processed
        // if CloseAllFiles returns false we absorb the veent and return
        return;
    }

    event.Skip();

    bool maximized = IsMaximized();
    if( maximized )
    {
        Freeze();
        Show( false );
        Maximize( false );
    }
    if( !maximized )
    {
        Pedit::config.WritePoint( _T("topwindow"),
                                  GetPosition() );
        Pedit::config.WriteSize( _T("topwindow"), 
                                 GetSize() );
    }
    if( maximized )
    {
        Maximize( true );
        Show( true );
        Thaw();
    }
    Pedit::config.WriteBool( _T("topwindow.maximized"),
                             IsMaximized() );
}
void PeditFrame::OnCloseAllFiles( wxCommandEvent& WXUNUSED(event) )
{
    CloseAllFiles( NULL );
}
bool PeditFrame::CloseAllFiles( wxCloseEvent * event )
{
    wxMDIChildFrame * first = GetActiveChild();
    wxMDIChildFrame * child = first;
    if( first )
    {
        do
        {
            if( !child->Close( event && !event->CanVeto() ) )
            {
                wxASSERT_MSG( !event || event->CanVeto(), _T("event is unVetoable but was Vetoed in some way... Data could be lost...") );
                if( event )
                {
                    event->Veto();
                    return false;
                }
            }
    
            ActivateNext();
            child = GetActiveChild();
        }
        while( child != first );
    }
    return true;
}

// -----------------------------
// event table of PeditFileFrame
// -----------------------------
BEGIN_EVENT_TABLE(PeditFileFrame, wxMDIChildFrame)
    EVT_MENU(MDI_CLOSE_FILE, PeditFileFrame::OnQuit)

    EVT_CLOSE(PeditFileFrame::OnClose)
END_EVENT_TABLE()

// -----------------------------
// PeditFileFrame implementation
// -----------------------------
PeditFileFrame::Extension2Subtype & PeditFileFrame::GetSubtypes()
{
    static Extension2Subtype s_subtypes;
    return s_subtypes;
}

bool PeditFileFrame::RegisterSubtype( std::string const & extension,
                                      FactoryMethod method,
                                      std::string const & description  )
{
    GetSubtypes()[extension] = std::make_pair( method, description );
    ResetFilesWildrcard();
    return true;
}

std::string & PeditFileFrame::FilesWildcardCache()
{
    static std::string s_filesWildcard;
    return s_filesWildcard;
}

std::string const & PeditFileFrame::GetFilesWildcard()
{
    if( FilesWildcardCache().empty() )
    {
        std::stringstream buf;
        for( Extension2Subtype::const_iterator cit = GetSubtypes().begin();
                                               cit != GetSubtypes().end(); ++cit )
        {
            buf << (cit == GetSubtypes().begin() ? "" : "|")
                // <description> (*.<extension>)
                << cit->second.second << " (*." << cit->first << ")|"
                // *.<extension>
                << "*." << cit->first;
        }
        FilesWildcardCache() = buf.str();
    }
    return FilesWildcardCache();
}
void PeditFileFrame::ResetFilesWildrcard()
{
    FilesWildcardCache().clear();
}

PeditFileFrame * PeditFileFrame::CreateInstance( wxMDIParentFrame *parent,
                                                 std::string const & filename )
{
    try
    {
        boost::filesystem::path path( filename );
        std::string extension = ::boost::filesystem::extension( path );
        if( extension.empty() )
        {
            return NULL;
        }
        extension.erase( 0, 1 );
        if( GetSubtypes().count( extension ) ) try
        {
            FactoryMethod method = GetSubtypes()[ extension ].first;
            PeditFileFrame * child = method( parent, filename );
            return child;
        }
        catch( std::exception const & ex )
        {
            wxMessageBox( wxString::Format( _T("Error %s\nwhile opening file\n %s"),
                                              ex.what(), filename.c_str() ),
                                              _T("Error opening file"), wxOK | wxICON_ERROR );
        }
    }
    catch( boost::filesystem::filesystem_error const & ex )
    {
        wxMessageBox( wxString::Format( _T("Error %s opening file %s"),
                                        ex.what(), filename.c_str() ),
                      _T("Invalid file"), wxOK | wxICON_ERROR );
    }
    catch( std::exception const & ex )
    {
        wxMessageBox( wxString::Format( _T("Error %s opening file %s"),
                                        ex.what(), filename.c_str() ),
                      _T("Invalid file"), wxOK | wxICON_ERROR );
    }
    catch(...)
    {
        wxMessageBox( wxString::Format(_T("Unknown exception opening %s"), filename.c_str() ),
                      _T("Open"), wxOK | wxICON_ERROR );
    }
    return NULL;
}

PeditFileFrame::PeditFileFrame( wxMDIParentFrame *parent,
                                std::string const & filename,
                                wxWindowID id,
                                wxString const & title )
: wxMDIChildFrame( parent, id, title )
{
    SetFilename( filename );
    CreateStatusBar();
}

PeditFileFrame::~PeditFileFrame()
{}

void PeditFileFrame::SetFilename( std::string const & filename )
{
    m_filename = filename;
    boost::filesystem::path path( filename );
    ::std::stringstream buf;
    buf << path.leaf();
    path = path.branch_path();
    if( !path.empty() )
        buf << " (" << path.string() << ")";
    SetTitle( buf.str() );
}

void PeditFileFrame::OnQuit( wxCommandEvent& WXUNUSED(event) )
{
    Close( true );
}

void PeditFileFrame::OnClose( wxCloseEvent & event )
{
    if( IsModified() )
    {
        int style = wxYES_NO | wxICON_QUESTION;
        if( event.CanVeto() )
            style |= wxCANCEL;
        int answer = wxMessageBox( wxString(_T("You have unsaved chages in\n"))
                                   + _T("  ") + GetTitle() + _T("\n")
                                   + _T("Do you want to save the changes?"),
                                   _T("Unsaved changes"), style, this );
        if( answer == wxCANCEL )
        {
            event.Veto();
            return;
        }
        else if( answer == wxYES )
        {
            for(;;)
            {
                try
                {
                    Save();
                    // exit infinite loop after a successful save and close
                    break;
                }
                catch( std::exception const & ex )
                {
                    int style = wxYES_NO | wxICON_ERROR;
                    if( event.CanVeto() )
                        style |= wxCANCEL;
                    int retry = wxMessageBox( wxString::Format(
                            _T("While saving into\n  %s\nerror\n  %s\nDo you want to retry (Yes) or abandon saving (No), losing changes?"),
                            GetFilename().c_str(), wxString( ex.what() ).c_str() ),
                            _T("Error saving data"),
                            style );
                    if( retry == wxCANCEL )
                    {
                        event.Veto();
                        // Cancel - veto the event triggered the window closing,
                        // don't close
                        return;
                    }
                    if( retry == wxNO )
                    {
                        // No - discard changes, close and continue
                        break;
                    }
                }
            }
        }
    }

    event.Skip();
}

// -----------------------------
// Pedit implementation
// -----------------------------
// initialize static variable
PeditConfig Pedit::config;

bool Pedit::OnInit()
{
    {
        const wxString name
            = wxString::Format( _T("LmiPedit-%s"), wxGetUserId().c_str() );
        m_checker = new wxSingleInstanceChecker( name );
        if ( m_checker->IsAnotherRunning() )
        {
            wxLogError( _T("Another program instance is already running.") );

            return false;
        }
    }

    m_peditFrame = new PeditFrame( NULL, wxID_ANY, _T( "LMI Product Editor" ) );

    m_peditFrame->Show( true );

    SetTopWindow( m_peditFrame );

    return true;
}

int Pedit::OnExit()
{
    delete m_checker;

    return 0;
}

// ----------------------------------
// MaxValueAdjusterChoice implementation
// ----------------------------------
BEGIN_EVENT_TABLE( MaxValueAdjusterChoice, wxChoice )
    EVT_CHOICE( wxID_ANY, MaxValueAdjusterChoice::OnChange )
END_EVENT_TABLE()

void MaxValueAdjusterChoice::OnChange( wxCommandEvent & event )
{
    DoOnChange();
}

// -------------------------------
// AutoSizeTreeCtrl implementation
// -------------------------------

wxSize AutoSizeTreeCtrl::DoGetBestSize() const
{
    wxTreeItemId root = GetRootItem();
    wxSize best;

    AutoSizeTreeCtrl * me = wxStaticCast( this, AutoSizeTreeCtrl );

    me->Freeze();

    try
    {
        me->DoGetBestSizePrivate( best, root, true );
    }
    catch(...)
    {
        me->Thaw();
        throw;
    }
    me->Thaw();

    // need some minimal size even for an empty tree
    if ( best.x == 0 || best.y == 0 )
    {
        wxSize bestSize = wxTreeCtrl::DoGetBestSize();
        if ( best.x == 0 )
            best.x = bestSize.x;
        if( best.y == 0 )
            best.y = bestSize.y;
    }
    best += GetSize() - GetClientSize();

    CacheBestSize( best );

    return best;
}

void AutoSizeTreeCtrl::DoGetBestSizePrivate( wxSize & size,
                                             const wxTreeItemId & node,
                                             bool isRoot )
{
    if ( !isRoot )
    {
        wxRect rect;
        if ( GetBoundingRect(node, rect, true) )
        {
            if ( size.x < rect.x + rect.width )
                size.x = rect.x + rect.width;
            // vertical size is not important here - just don't calculate it
//            if ( size.y < rect.y + rect.height )
//                size.y = rect.y + rect.height;
        }
    }
    
    if ( node.IsOk() && GetChildrenCount(node) )
    {
        bool expanded = isRoot || IsExpanded(node);
        if ( !expanded )
            Expand(node);
        wxTreeItemIdValue cookie;
        for(wxTreeItemId child = GetFirstChild(node, cookie);
            child.IsOk();
            child = GetNextChild(node, cookie))
        {
            DoGetBestSizePrivate(size, child);
        }

        if ( !expanded )
            Collapse(node);
    }
}
