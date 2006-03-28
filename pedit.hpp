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

// $Id: pedit.hpp,v 1.1.2.3 2006-03-28 00:46:24 etarassov Exp $

#ifndef PEDIT_HPP_
#define PEDIT_HPP_

#include "config.hpp"
#include "multidimgrid.hpp"
#include "pedit_config.hpp"

#include <string>
#include <map>
#include <wx/mdi.h>
#include <wx/frame.h>
#include <wx/app.h>
#include <wx/choice.h>
#include <wx/treectrl.h>
#include <wx/intl.h>

/**
   Parent MDI frame in the product editor.
   
   It is a helper class that handles any universal
   (data format independent) things such as open a dialog when opening a file,
   or saving to another file, saving all windows, showing file menu, etc.
 */
class PeditFrame : public wxMDIParentFrame
{
public:
    /// Constructor mimics the one of wxMDIParentFrame class
    PeditFrame( wxWindow *parent = NULL,
                const wxWindowID id = wxID_ANY,
                const wxString& title = _("LMI Product Editor") );

    virtual ~PeditFrame();
private:
    /// Menu event handlers
    /// @{
    void OnAbout(wxCommandEvent& event);
    void OnOpenFile(wxCommandEvent& event);
    void OnSaveFile(wxCommandEvent& event);
    void OnSaveFileAs(wxCommandEvent& event);
    void OnSaveAllFiles(wxCommandEvent& event);
    void OnCloseActiveFile(wxCommandEvent& event);
    /// @}

    /// Close events handlers
    /// @{
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnCloseAllFiles(wxCommandEvent& event);
    /// @}

    /// Update menu items depending on the topmost child window
    void OnUpdateMenuItem( wxUpdateUIEvent & event );
    /// Update "Save" menu items depending on the topmost child window state
    void OnUpdateSaveFileMenuItem( wxUpdateUIEvent & event );

    /**
       Helper class that handles 'Close All Files' events.

       If asks every child-window to close itself. And if child refuses it
       then tries to Veto the event.

       @param event event object that triggered the call or @c NULL if it's
                    a call from the user code
       @return true if every window was closed
     */
    bool CloseAllFiles( wxCloseEvent * event );

    DECLARE_EVENT_TABLE()
};

/**
   Child MDI frame window.
   
   Its an abstract class and has to be derived from to be instantiated.
   The class provides two features:
    - base class for all data file editors
       The deriving classes are passed a filename and are free to implement
       the interface needed
    - registry for those data file editors
       The deriving class have to statically register via RegisterSubtype
       to provide PeditFileFrame with:
        - file extension
        - Factory method
 */
class PeditFileFrame : public wxMDIChildFrame
{
public:
    PeditFileFrame( wxMDIParentFrame *parent,
                    std::string const & filename,
                    wxWindowID id,
                    wxString const & title );

    /// Save this child editor data into the file
    void Save() { DoSave(); }
    /// Return if the data was modified since the last save
    bool IsModified() const { return DoIsModified(); }

    /// Change corresponding file name
    void SetFilename( std::string const & filename );
    /// Getter for the corresponding file name
    std::string const & GetFilename() const { return m_filename; }

    /// just a convenience virtual getter for the extension
    std::string const & GetExtension() const { return DoGetExtension(); }

protected:
    /// PeditFileFrame shouldn't be instantiated as is, derive from it instead
    virtual ~PeditFileFrame();
    /// Save data into a file. Method to define in derived classes.
    virtual void DoSave() = 0;
    /// Whether the data is modified. Method to define in derived classes.
    virtual bool DoIsModified() const = 0;
    
    /// Getter for the extension. Method to define in derived classes.
    virtual std::string const & DoGetExtension() const = 0;

private:
    /// File name where the data resides on the disk.
    std::string m_filename;

    /// Event handlers
    void OnQuit( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

    // Static part of the class
public:
    /// Dispatch child creation to the corresponding factory method
    static PeditFileFrame * CreateInstance( wxMDIParentFrame *parent,
                                            std::string const & filename );

protected:
    /// Factory Method type (for subclasses)
    typedef PeditFileFrame * (*FactoryMethod)( wxMDIParentFrame *parent,
                                               std::string const & filename );
    /// Register subtype class to provide extension and its factory method
    static bool RegisterSubtype( std::string const & extension, 
                                 FactoryMethod method,
                                 std::string const & description = "" );

private:
    /// Subtypes information storage
    typedef std::map<std::string, std::pair< FactoryMethod, std::string > >
            Extension2Subtype;
    /// Container for the subtypes storage
    static Extension2Subtype & GetSubtypes();

    /// Return the list of wildcards registered in the class
    static std::string const & GetFilesWildcard();
    /// Reset internal cache concerning file wildcards
    static void ResetFilesWildrcard();
    friend class PeditFrame;

    /// Contains file wildcards cache
    static std::string & FilesWildcardCache();

    DECLARE_EVENT_TABLE()
};

// forward declaration
class WXDLLEXPORT wxSingleInstanceChecker;

/**
   Product Editor application class
 */
class Pedit : public wxApp
{
public:
    virtual bool OnInit();
    virtual int OnExit();

    static PeditConfig config;

private:
    /// The main MDI parent window
    PeditFrame * m_peditFrame;
    /// Check that there is only one application (per user) running
    wxSingleInstanceChecker * m_checker;
};



/**
   Non-template base for MaxValueAdjuster template-class

   wxWidgets won't accept a template class with an event_table - corresponding
   macros are taking only the plain old classes, not templates.

   This helper registers handler for wxChoice selection change events and
   defines virtual function DoOnChange() that serves as real handler.

   @sa MaxValueAdjuster
 */
class MaxValueAdjusterChoice : public wxChoice
{
public:
    MaxValueAdjusterChoice( MultiDimGrid & grid )
    : wxChoice( &grid, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxArrayString() )
    {}

protected:
    /// Function called by OnChange. applies adjustment value
    virtual void DoOnChange() = 0;

    /// Gets the parent and cast it to the MultiDimGrid type
    MultiDimGrid & GetGrid()
    {   return *static_cast< MultiDimGrid * >( GetParent() );    }
    MultiDimGrid const & GetGrid() const
    {   return *static_cast< MultiDimGrid const * >( GetParent() );    }

private:
    /// Calls DoOnChange
    void OnChange( wxCommandEvent & event );

    DECLARE_NO_COPY_CLASS( MaxValueAdjusterChoice )
    DECLARE_EVENT_TABLE()
};

/**
   Drop down value list for an axis adjustment.

   This control lets adjust the upper bound for a range based integral axis.
   Taking lower and upper bounds for the right bound of the axis values range
   it allows user to change it.
 */
template< typename Integral >
class MaxValueAdjuster : public MaxValueAdjusterChoice
{
public:
    /**
       @param lowerBound minimal possible value for the highest axis value (inclusive)
       @param upperBound maximal possible value for the highest axis value (inclusive)
     */
    MaxValueAdjuster( MultiDimAxisAny & axis,
                      MultiDimGrid & grid,
                      Integral lowerBound,
                      Integral upperBound );

    /// Currently chosen maximum axis value
    Integral GetMaxValue() const;
    /// Set the maximum axis value
    void SetMaxValue( Integral maxValue );

    /// Apply adjustment value
    virtual void DoOnChange();

private:
    MultiDimAxisAny & m_axis;
    Integral m_lowerBound;
    Integral m_upperBound;
};

// -------------------------------
// MaxValueAdjuster implementation
// -------------------------------
template< typename Integral >
MaxValueAdjuster<Integral>::MaxValueAdjuster( MultiDimAxisAny & axis,
                                              MultiDimGrid & grid,
                                              Integral lowerBound,
                                              Integral upperBound )
: MaxValueAdjusterChoice( grid ),
  m_axis( axis ), m_lowerBound( lowerBound ), m_upperBound( upperBound )
{
    wxASSERT( lowerBound <= upperBound );
    SetToolTip( wxString::Format(_("Upper bound for \"%s\" axis"), axis.GetName().c_str() ) );
    for( Integral i = m_lowerBound; i <= m_upperBound; ++i )
        wxChoice::Append( wxString::Format( _T("%d"), i + 1 ) );
}

template< typename Integral >
void MaxValueAdjuster<Integral>::DoOnChange()
{
    GetGrid().ApplyAxisAdjustment( m_axis.GetName() );
}

template< typename Integral >
void MaxValueAdjuster<Integral>::SetMaxValue( Integral maxValue )
{
    wxASSERT_MSG( m_lowerBound <= maxValue && maxValue <= m_upperBound,
                  _T("maxValue is out of allowed value range") );
    wxChoice::SetSelection( maxValue - m_lowerBound );
}

template< typename Integral >
Integral MaxValueAdjuster<Integral>::GetMaxValue() const
{
    int value = wxChoice::GetSelection();
    if( value == wxNOT_FOUND )
        value = 0;
    Integral maxValue = m_lowerBound + static_cast<unsigned int>( value );
    wxASSERT_MSG( m_lowerBound <= maxValue && maxValue <= m_upperBound,
                  _T("maxValue is out of allowed value range") );
    return maxValue;
}

/**
   Adjustable axis - the only difference with AdjustableMaxIntegralAxis is that
   it provides adjustment control and allows narrowing of value range,
   precisely it allows the user to control the maximum value the axis values
   could change.
 */
template<typename Integral>
class AdjustableMaxIntegralAxis : public MultiDimAdjustableAxis
                                         <
                                                MaxValueAdjuster<Integral>,
                                                MultiDimIntegralAxis<Integral>
                                         >
{
    typedef MaxValueAdjuster<Integral> Adjuster;
    typedef MultiDimIntegralAxis<Integral> ProBaseClass;
    typedef MultiDimAdjustableAxis< Adjuster, ProBaseClass > BaseClass;
public:

    /**
       Constructor

       @sa MultiDimIntAxis::MultiDimIntAxis
       @param minValue current minimal axis value
       @param maxValue current maximal axis value
       @param lowerBound the lowest possible value for the maximal axis value
       @param upperBound the highest possible value for the maximal axis value
     */
    AdjustableMaxIntegralAxis( const wxString& name,
                               Integral minValue,
                               Integral maxValue,
                               Integral lowerBound,
                               Integral upperBound );

    /// Use SetValue to set the corresponding values
    AdjustableMaxIntegralAxis( const wxString& name );

    /// Change current maximal value
    void SetMaxValue( Integral maxValue );

    /// Change lower/upper bounds
    void SetBounds( Integral lowerBound, Integral upperBound );

    Integral GetLowerBound() const
    {   return m_lowerBound;    }

    Integral GetUpperBound() const
    {   return m_upperBound;    }

    /// If the axis has been adjusted, then refresh value choice control of the axis
    void UpdateChoiceControl( wxWindow & choiceControl ) const;

protected:
    /// Create the adjustment control
    virtual
    Adjuster * DoGetAdjustControl( MultiDimGrid & grid,
                                   MultiDimTableAny & table );

    /// Applies user changes to this axis, reads adjustment window
    virtual
    bool DoApplyAdjustment( Adjuster * adjustWin, unsigned int n );

    /// Sync the corresponding adjustment control with itself
    virtual
    bool DoRefreshAdjustment( Adjuster * adjustWin, unsigned int n );

private:
    Integral m_lowerBound;
    Integral m_upperBound;
};

// ----------------------------------------
// AdjustableMaxIntegralAxis implementation
// ----------------------------------------
template<typename Integral>
AdjustableMaxIntegralAxis<Integral>::AdjustableMaxIntegralAxis( const wxString& name,
                                                                Integral minValue,
                                                                Integral maxValue,
                                                                Integral lowerBound,
                                                                Integral upperBound )
: BaseClass(name)
{
    ProBaseClass::SetValues( minValue, maxValue, 1 );
    SetBounds( lowerBound, upperBound );
}

template<typename Integral>
AdjustableMaxIntegralAxis<Integral>::AdjustableMaxIntegralAxis( const wxString& name )
: BaseClass(name)
{
    SetBounds( 0, 0 );
    ProBaseClass::SetValues( 0, 0, 1 );
}

template<typename Integral>
void AdjustableMaxIntegralAxis<Integral>::SetMaxValue( Integral maxValue )
{
    ProBaseClass::SetValues(
        ProBaseClass::GetMinValue(),
        maxValue,
        ProBaseClass::GetStep() );
}

template<typename Integral>
void AdjustableMaxIntegralAxis<Integral>::SetBounds( Integral lowerBound, Integral upperBound )
{
    wxASSERT_MSG( 0 <= lowerBound && lowerBound <= upperBound,
              _T("lowerBound/upperBound values are invalid") );
    m_lowerBound = lowerBound;
    m_upperBound = upperBound;
}

template<typename Integral>
void AdjustableMaxIntegralAxis<Integral>::UpdateChoiceControl( wxWindow & choiceControl ) const
{
    wxCHECK_RET( dynamic_cast<MultiDimAxisAnyChoice*>(&choiceControl),
                 _T("Wrong choice Control type") );
    MultiDimAxisAnyChoice & choice =
            static_cast<MultiDimAxisAnyChoice&>(choiceControl);
    Integral minValue = ProBaseClass::GetMinValue();
    Integral maxValue = ProBaseClass::GetMaxValue();
    Integral newCount = maxValue - minValue + 1;
    int commonCount = wxMin( choice.GetCount(), static_cast<int>( newCount ) );
    int selection = choice.GetSelection();
    if( selection != wxNOT_FOUND && selection >= commonCount )
        selection = wxNOT_FOUND;
    while( choice.GetCount() > commonCount )
        choice.Delete( choice.GetCount() - 1 );

    while( choice.GetCount() < static_cast<int>( newCount ) )
    {
        choice.Append( GetLabel( choice.GetCount() + minValue ) );
    }

    if( selection == wxNOT_FOUND )
    {   // selection was changed
        if( choice.GetCount() > 0 )
        {
            choice.SetSelection( choice.GetCount() - 1 );
        }
        choice.SelectionChanged();
    }
}

template<typename Integral>
typename AdjustableMaxIntegralAxis<Integral>::Adjuster *
AdjustableMaxIntegralAxis<Integral>::DoGetAdjustControl( MultiDimGrid & grid,
                                                         MultiDimTableAny & table )
{
    // called only once
    if( m_lowerBound == m_upperBound )
        return NULL;
    return new Adjuster(*this, grid, m_lowerBound, m_upperBound );
}

template<typename Integral>
bool AdjustableMaxIntegralAxis<Integral>::DoApplyAdjustment( Adjuster * adjustWin,
                                                      unsigned int n )
{
    if( !adjustWin )
        return false;
    Integral newMaxValue = adjustWin->GetMaxValue();
    wxASSERT_MSG( m_lowerBound <= newMaxValue && newMaxValue <= m_upperBound,
                  _T("newMaxValue is out of valid value range") );
    bool updated = ( ProBaseClass::GetMaxValue() != newMaxValue );
    SetMaxValue( newMaxValue );
    return updated;
}

template<typename Integral>
bool AdjustableMaxIntegralAxis<Integral>::DoRefreshAdjustment( Adjuster * adjustWin,
                                                        unsigned int n )
{
    if( !adjustWin )
        return false;
    Integral maxValue = adjustWin->GetMaxValue();
    bool updated = ( ProBaseClass::GetMaxValue() != maxValue );
    Integral tmp = ProBaseClass::GetMaxValue();
    adjustWin->SetMaxValue( tmp );
    return updated;
}

class AutoSizeTreeCtrl : public wxTreeCtrl
{
public:
    AutoSizeTreeCtrl() : wxTreeCtrl() {}
    AutoSizeTreeCtrl( wxWindow* parent,
                      wxWindowID id,
                      const wxPoint& pos = wxDefaultPosition,
                      const wxSize& size = wxDefaultSize,
                      long style = wxTR_HAS_BUTTONS,
                      const wxValidator& validator = wxDefaultValidator )
    : wxTreeCtrl( parent, id, pos, size, style, validator )
    {}

    bool Create( wxWindow* parent,
                 wxWindowID id,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTR_HAS_BUTTONS,
                 const wxValidator& validator = wxDefaultValidator )
    {   return wxTreeCtrl::Create( parent, id, pos, size, style, validator );  }

    virtual ~AutoSizeTreeCtrl() {}

protected:
    virtual wxSize DoGetBestSize() const;

private:
    void DoGetBestSizePrivate( wxSize & size,
                               const wxTreeItemId & node,
                               bool isRoot = false );
};

#endif /*PEDIT_HPP_*/
