// Product data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005 Gregory W. Chicares.
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

// $Id: ihs_proddata.hpp,v 1.3 2005-12-21 01:22:51 chicares Exp $

#ifndef proddata_hpp
#define proddata_hpp

#include "config.hpp"

#include "expimp.hpp"

#include <string>

// Filenames and other free-form offline data that vary by product.

class LMI_EXPIMP TProductData
{
    friend class PolicyDoc;
    friend class PolicyView;

  public:
    explicit TProductData(std::string const& a_Filename);
    ~TProductData();

    static void WritePolFiles();
    static void WriteProprietaryPolFiles();

    std::string const& GetDatabaseFilename       () const {return DatabaseFilename;       }
    std::string const& GetFundFilename           () const {return FundFilename;           }
    std::string const& GetCorridorFilename       () const {return CorridorFilename;       }
    std::string const& GetCurrCOIFilename        () const {return CurrCOIFilename;        }
    std::string const& GetGuarCOIFilename        () const {return GuarCOIFilename;        }
    std::string const& GetWPFilename             () const {return WPFilename;             }
    std::string const& GetADDFilename            () const {return ADDFilename;            }
    std::string const& GetChildRiderFilename     () const {return ChildRiderFilename;     }
    std::string const& GetCurrSpouseRiderFilename() const {return CurrSpouseRiderFilename;}
    std::string const& GetGuarSpouseRiderFilename() const {return GuarSpouseRiderFilename;}
    std::string const& GetCurrTermFilename       () const {return CurrTermFilename;       }
    std::string const& GetGuarTermFilename       () const {return GuarTermFilename;       }
    std::string const& GetTableYFilename         () const {return TableYFilename;         }
    std::string const& GetPremTaxFilename        () const {return PremTaxFilename;        }
    std::string const& GetTAMRA7PayFilename      () const {return TAMRA7PayFilename;      }
    std::string const& GetTgtPremFilename        () const {return TgtPremFilename;        }
    std::string const& GetIRC7702Filename        () const {return IRC7702Filename;        }
    std::string const& GetGam83Filename          () const {return Gam83Filename;          }
    std::string const& GetRoundingFilename       () const {return RoundingFilename;       }
    std::string const& GetTierFilename           () const {return TierFilename;           }

    std::string const& GetPolicyForm             () const {return PolicyForm;             }
    std::string const& GetPolicyMktgName         () const {return PolicyMktgName;         }
    std::string const& GetPolicyLegalName        () const {return PolicyLegalName;        }
    std::string const& GetInsCoShortName         () const {return InsCoShortName;         }
    std::string const& GetInsCoName              () const {return InsCoName;              }
    std::string const& GetInsCoAddr              () const {return InsCoAddr;              }
    std::string const& GetInsCoStreet            () const {return InsCoStreet;            }
    std::string const& GetInsCoPhone             () const {return InsCoPhone;             }
    std::string const& GetInsCoDomicile          () const {return InsCoDomicile;          }
    std::string const& GetMainUnderwriter        () const {return MainUnderwriter;        }
    std::string const& GetMainUnderwriterAddress () const {return MainUnderwriterAddress; }
    std::string const& GetCoUnderwriter          () const {return CoUnderwriter;          }
    std::string const& GetCoUnderwriterAddress   () const {return CoUnderwriterAddress;   }
    std::string const& GetAvName                 () const {return AvName;                 }
    std::string const& GetCsvName                () const {return CsvName;                }
    std::string const& GetCsvHeaderName          () const {return CsvHeaderName;          }
    std::string const& GetNoLapseProvisionName   () const {return NoLapseProvisionName;   }
    std::string const& GetInterestDisclaimer     () const {return InterestDisclaimer;     }

  private:
    TProductData();

    void Init(std::string const& a_Filename);
    void Read(std::string const& a_Filename);
    void Write(std::string const& a_Filename) const;

    std::string DatabaseFilename;
    std::string FundFilename;
    std::string CorridorFilename;
    std::string CurrCOIFilename;
    std::string GuarCOIFilename;
    std::string WPFilename;
    std::string ADDFilename;
    std::string ChildRiderFilename;
    std::string CurrSpouseRiderFilename;
    std::string GuarSpouseRiderFilename;
    std::string CurrTermFilename;
    std::string GuarTermFilename;
    std::string TableYFilename;
    std::string PremTaxFilename;
    std::string TAMRA7PayFilename;
    std::string TgtPremFilename;
    std::string IRC7702Filename;
    std::string Gam83Filename;
    std::string RoundingFilename;
    std::string TierFilename;

    std::string PolicyForm;
    std::string PolicyMktgName;
    std::string PolicyLegalName;
    std::string InsCoShortName;
    std::string InsCoName;
    std::string InsCoAddr;
    std::string InsCoStreet;
    std::string InsCoPhone;
    std::string InsCoDomicile;
    std::string MainUnderwriter;
    std::string MainUnderwriterAddress;
    std::string CoUnderwriter;
    std::string CoUnderwriterAddress;

    // Illustration reg requires column headers to use names in
    // contract, e.g. for account and surrender values.
    std::string AvName;
    std::string CsvName;
    std::string CsvHeaderName;
    std::string NoLapseProvisionName;
    std::string InterestDisclaimer;
};

#endif  // proddata_hpp

