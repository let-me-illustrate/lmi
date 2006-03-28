// Product editor part for pol file type.
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

// $Id: pedit_pol.hpp,v 1.1.2.2 2006-03-28 00:40:40 etarassov Exp $

#ifndef PEDIT_POL_HPP_
#define PEDIT_POL_HPP_

#include "config.hpp"

#include "multidimgrid.hpp"
#include "pedit.hpp"

#include "ihs_proddata.hpp"

class WXDLLEXPORT wxTextCtrl;
/**
   MDI Child frame for POL files editing.
   
 */
class PeditPOL : public PeditFileFrame,
                 protected TProductData
{
public:
    virtual ~PeditPOL();

    /// Extension of pol files
    static const std::string s_extension; // = "pol"

    /// Register this type of files in PeditFileFrame
    static bool RegisterPeditPOLSubtype()
    {
        PeditFileFrame::RegisterSubtype( PeditPOL::s_extension,
                                         PeditPOL::CreatePOLInstance,
                                         _("Policy") );
        return true; // a dummy value
    }

protected:
    /// This class shouldn't be instantiated out of the product editor
    PeditPOL( wxMDIParentFrame *parent, std::string const & filename );

    /// Implement the abstract methods of PeditFileFrame interface
    /// @{
    virtual void DoSave();
    virtual bool DoIsModified() const;
    virtual std::string const & DoGetExtension() const {  return s_extension;  }
    /// @}

    void SyncDataToUI();
    void SyncUIToData();

private:
    bool m_modified;
    void SetModified( bool modified = true );

    /// List of TDBValue entities
    wxTextCtrl * m_paramDatabase;
    wxTextCtrl * m_paramTiered;
    wxTextCtrl * m_paramRounding;
    wxTextCtrl * m_paramFunds;

    wxTextCtrl * m_oldTableY;
    wxTextCtrl * m_oldPremiumTax;
    wxTextCtrl * m_old83GAM;

    wxTextCtrl * m_tableCurrentCOI;
    wxTextCtrl * m_tableGuarranteedCOI;
    wxTextCtrl * m_tableWaiverPremium;
    wxTextCtrl * m_tableADD;
    wxTextCtrl * m_tableTermRider;
    wxTextCtrl * m_tableCVAT;
    wxTextCtrl * m_tableTAMRA;
    wxTextCtrl * m_table7702Q;

    /// Factory method for the class instantiation
    static PeditFileFrame * CreatePOLInstance( wxMDIParentFrame *parent,
                                               std::string const & filename );
};

#endif /*PEDIT_POL_HPP_*/
