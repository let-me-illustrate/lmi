// Product editor part for rnd file type.
//
// Copyright (C) 2005, 2006 Gregory W. Chicares.
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

// $Id: pedit_rnd.hpp,v 1.1.2.4 2006-04-10 20:26:03 etarassov Exp $

#ifndef PEDIT_RND_HPP_
#define PEDIT_RND_HPP_

#include "config.hpp"

#include "multidimgrid.hpp"
#include "pedit.hpp"
#include "ihs_rnddata.hpp"

// some forward declarations
class RNDStyleButtons;
class WXDLLEXPORT wxSpinCtrl;

/**
   MDI Child frame for RND files editing.
 */
class PeditRND : public PeditFileFrame,
                 protected StreamableRoundingRules
{
public:
    virtual ~PeditRND();

    /// Extension of rnd files
    static const std::string s_extension; // = "rnd"

    /// Register this type of files in PeditFileFrame
    static bool RegisterPeditRNDSubtype()
    {
        PeditFileFrame::RegisterSubtype( PeditRND::s_extension,
                                         PeditRND::CreateRNDInstance,
                                         _("Rounding") );
        return true; // a dummy value
    }

protected:
    /// This class shouldn't be instantiated out of the product editor
    PeditRND( wxMDIParentFrame *parent, std::string const & filename );

    /// Implement the abstract methods of PeditFileFrame interface
    /// @{
    virtual void DoSave();
    virtual bool DoIsModified() const;
    virtual std::string const & DoGetExtension() const {  return s_extension;  }
    /// @}

    void SetModified( bool modified = true )
    {   m_modified = modified;  }

private:
    /// Modified flag
    bool m_modified;

    /// Widgets representing data
    /// @{
    RNDStyleButtons * m_btnsSpecAmount;
    wxSpinCtrl      * m_spinSpecAmount;
    RNDStyleButtons * m_btnsDeathBenefit;
    wxSpinCtrl      * m_spinDeathBenefit;
    RNDStyleButtons * m_btnsNAAR;
    wxSpinCtrl      * m_spinNAAR;
    RNDStyleButtons * m_btnsCOIRate;
    wxSpinCtrl      * m_spinCOIRate;
    RNDStyleButtons * m_btnsMortality;
    wxSpinCtrl      * m_spinMortality;
    RNDStyleButtons * m_btnsGrossPremium;
    wxSpinCtrl      * m_spinGrossPremium;
    RNDStyleButtons * m_btnsNetPremium;
    wxSpinCtrl      * m_spinNetPremium;
    RNDStyleButtons * m_btnsIntRate;
    wxSpinCtrl      * m_spinIntRate;
    RNDStyleButtons * m_btnsIntCredit;
    wxSpinCtrl      * m_spinIntCredit;
    RNDStyleButtons * m_btnsWithdrawal;
    wxSpinCtrl      * m_spinWithdrawal;
    RNDStyleButtons * m_btnsLoan;
    wxSpinCtrl      * m_spinLoan;
    RNDStyleButtons * m_btns7702;
    wxSpinCtrl      * m_spin7702;
    RNDStyleButtons * m_btnsSurrCharge;
    wxSpinCtrl      * m_spinSurrCharge;
    RNDStyleButtons * m_btnsIRR;
    wxSpinCtrl      * m_spinIRR;
    /// @}

    /// Factory method for the class instantiation
    static PeditFileFrame * CreateRNDInstance( wxMDIParentFrame *parent,
                                               std::string const & filename );

    /// Serves to load Data into/from UI
    void DoOnAllFields( bool populate, wxWindow * parent = NULL, wxSizer * sizer = NULL );

    /// Read and transfer values from UI controls to underlying data structures
    void SyncUIToData();

    DECLARE_NO_COPY_CLASS( PeditRND )
};

#endif /*PEDIT_RND_HPP_*/
