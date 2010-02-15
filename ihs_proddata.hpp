// Product data.
//
// Copyright (C) 1998, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010 Gregory W. Chicares.
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
// http://savannah.nongnu.org/projects/lmi
// email: <gchicares@sbcglobal.net>
// snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

// $Id$

#ifndef ihs_proddata_hpp
#define ihs_proddata_hpp

#include "config.hpp"

#include "so_attributes.hpp"

#include <string>

// Filenames and other free-form offline data that vary by product.

class LMI_SO TProductData
{
    friend class PolicyDocument;
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
    std::string const& GetSubstdTblMultFilename  () const {return SubstdTblMultFilename;  }
    std::string const& GetCurrSpecAmtLoadFilename() const {return CurrSpecAmtLoadFilename;}
    std::string const& GetGuarSpecAmtLoadFilename() const {return GuarSpecAmtLoadFilename;}
    std::string const& GetRoundingFilename       () const {return RoundingFilename;       }
    std::string const& GetTierFilename           () const {return TierFilename;           }

    std::string const& GetPolicyForm                    () const {return PolicyForm;                    }
    std::string const& GetPolicyMktgName                () const {return PolicyMktgName;                }
    std::string const& GetPolicyLegalName               () const {return PolicyLegalName;               }
    std::string const& GetInsCoShortName                () const {return InsCoShortName;                }
    std::string const& GetInsCoName                     () const {return InsCoName;                     }
    std::string const& GetInsCoAddr                     () const {return InsCoAddr;                     }
    std::string const& GetInsCoStreet                   () const {return InsCoStreet;                   }
    std::string const& GetInsCoPhone                    () const {return InsCoPhone;                    }
    std::string const& GetInsCoDomicile                 () const {return InsCoDomicile;                 }
    std::string const& GetMainUnderwriter               () const {return MainUnderwriter;               }
    std::string const& GetMainUnderwriterAddress        () const {return MainUnderwriterAddress;        }
    std::string const& GetCoUnderwriter                 () const {return CoUnderwriter;                 }
    std::string const& GetCoUnderwriterAddress          () const {return CoUnderwriterAddress;          }
    std::string const& GetAvName                        () const {return AvName;                        }
    std::string const& GetCsvName                       () const {return CsvName;                       }
    std::string const& GetCsvHeaderName                 () const {return CsvHeaderName;                 }
    std::string const& GetNoLapseProvisionName          () const {return NoLapseProvisionName;          }
    std::string const& GetInterestDisclaimer            () const {return InterestDisclaimer;            }
    std::string const& GetGuarMortalityFootnote         () const {return GuarMortalityFootnote;         }
    std::string const& GetAccountValueFootnote          () const {return AccountValueFootnote;          }
    std::string const& GetAttainedAgeFootnote           () const {return AttainedAgeFootnote;           }
    std::string const& GetCashSurrValueFootnote         () const {return CashSurrValueFootnote;         }
    std::string const& GetDeathBenefitFootnote          () const {return DeathBenefitFootnote;          }
    std::string const& GetInitialPremiumFootnote        () const {return InitialPremiumFootnote;        }
    std::string const& GetNetPremiumFootnote            () const {return NetPremiumFootnote;            }
    std::string const& GetOutlayFootnote                () const {return OutlayFootnote;                }
    std::string const& GetPolicyYearFootnote            () const {return PolicyYearFootnote;            }
    std::string const& GetADDFootnote                   () const {return ADDFootnote;                   }
    std::string const& GetChildFootnote                 () const {return ChildFootnote;                 }
    std::string const& GetSpouseFootnote                () const {return SpouseFootnote;                }
    std::string const& GetTermFootnote                  () const {return TermFootnote;                  }
    std::string const& GetWaiverFootnote                () const {return WaiverFootnote;                }
    std::string const& GetMinimumPremiumFootnote        () const {return MinimumPremiumFootnote;        }
    std::string const& GetPremAllocationFootnote        () const {return PremAllocationFootnote;        }
    std::string const& GetProductDescription            () const {return ProductDescription;            }
    std::string const& GetStableValueFootnote           () const {return StableValueFootnote;           }
    std::string const& GetNoVanishPremiumFootnote       () const {return NoVanishPremiumFootnote;       }
    std::string const& GetRejectPremiumFootnote         () const {return RejectPremiumFootnote;         }
    std::string const& GetExpRatingFootnote             () const {return ExpRatingFootnote;             }
    std::string const& GetMortalityBlendFootnote        () const {return MortalityBlendFootnote;        }
    std::string const& GetHypotheticalRatesFootnote     () const {return HypotheticalRatesFootnote;     }
    std::string const& GetSalesLoadRefundFootnote       () const {return SalesLoadRefundFootnote;       }
    std::string const& GetNoLapseFootnote               () const {return NoLapseFootnote;               }
    std::string const& GetMarketValueAdjFootnote        () const {return MarketValueAdjFootnote;        }
    std::string const& GetExchangeChargeFootnote0       () const {return ExchangeChargeFootnote0;       }
    std::string const& GetCurrentValuesFootnote         () const {return CurrentValuesFootnote;         }
    std::string const& GetDBOption1Footnote             () const {return DBOption1Footnote;             }
    std::string const& GetDBOption2Footnote             () const {return DBOption2Footnote;             }
    std::string const& GetExpRatRiskChargeFootnote      () const {return ExpRatRiskChargeFootnote;      }
    std::string const& GetExchangeChargeFootnote1       () const {return ExchangeChargeFootnote1;       }
    std::string const& GetFlexiblePremiumFootnote       () const {return FlexiblePremiumFootnote;       }
    std::string const& GetGuaranteedValuesFootnote      () const {return GuaranteedValuesFootnote;      }
    std::string const& GetCreditingRateFootnote         () const {return CreditingRateFootnote;         }
    std::string const& GetMecFootnote                   () const {return MecFootnote;                   }
    std::string const& GetMidpointValuesFootnote        () const {return MidpointValuesFootnote;        }
    std::string const& GetSinglePremiumFootnote         () const {return SinglePremiumFootnote;         }
    std::string const& GetMonthlyChargesFootnote        () const {return MonthlyChargesFootnote;        }
    std::string const& GetUltCreditingRateFootnote      () const {return UltCreditingRateFootnote;      }
    std::string const& GetMaxNaarFootnote               () const {return MaxNaarFootnote;               }
    std::string const& GetPremTaxSurrChgFootnote        () const {return PremTaxSurrChgFootnote;        }
    std::string const& GetPolicyFeeFootnote             () const {return PolicyFeeFootnote;             }
    std::string const& GetAssetChargeFootnote           () const {return AssetChargeFootnote;           }
    std::string const& GetInvestmentIncomeFootnote      () const {return InvestmentIncomeFootnote;      }
    std::string const& GetIrrDbFootnote                 () const {return IrrDbFootnote;                 }
    std::string const& GetIrrCsvFootnote                () const {return IrrCsvFootnote;                }
    std::string const& GetMortalityChargesFootnote      () const {return MortalityChargesFootnote;      }
    std::string const& GetLoanAndWithdrawalFootnote     () const {return LoanAndWithdrawalFootnote;     }
    std::string const& GetPresaleTrackingNumber         () const {return PresaleTrackingNumber;         }
    std::string const& GetCompositeTrackingNumber       () const {return CompositeTrackingNumber;       }
    std::string const& GetInforceTrackingNumber         () const {return InforceTrackingNumber;         }
    std::string const& GetInforceCompositeTrackingNumber() const {return InforceCompositeTrackingNumber;}
    std::string const& GetInforceNonGuaranteedFootnote0 () const {return InforceNonGuaranteedFootnote0 ;}
    std::string const& GetInforceNonGuaranteedFootnote1 () const {return InforceNonGuaranteedFootnote1 ;}
    std::string const& GetInforceNonGuaranteedFootnote2 () const {return InforceNonGuaranteedFootnote2 ;}
    std::string const& GetInforceNonGuaranteedFootnote3 () const {return InforceNonGuaranteedFootnote3 ;}
    std::string const& GetNonGuaranteedFootnote         () const {return NonGuaranteedFootnote         ;}
    std::string const& GetMonthlyChargesPaymentFootnote () const {return MonthlyChargesPaymentFootnote ;}

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
    std::string SubstdTblMultFilename;
    std::string CurrSpecAmtLoadFilename;
    std::string GuarSpecAmtLoadFilename;
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
    std::string GuarMortalityFootnote;

    // Ledger column definitions.
    std::string AccountValueFootnote;
    std::string AttainedAgeFootnote;
    std::string CashSurrValueFootnote;
    std::string DeathBenefitFootnote;
    std::string InitialPremiumFootnote;
    std::string NetPremiumFootnote;
    std::string OutlayFootnote;
    std::string PolicyYearFootnote;

    // Rider footnotes.
    std::string ADDFootnote;
    std::string ChildFootnote;
    std::string SpouseFootnote;
    std::string TermFootnote;
    std::string WaiverFootnote;

    // Premium-specific footnotes.
    std::string MinimumPremiumFootnote;
    std::string PremAllocationFootnote;

    std::string ProductDescription;
    std::string StableValueFootnote;
    std::string NoVanishPremiumFootnote;
    std::string RejectPremiumFootnote;
    std::string ExpRatingFootnote;
    std::string MortalityBlendFootnote;
    std::string HypotheticalRatesFootnote;
    std::string SalesLoadRefundFootnote;
    std::string NoLapseFootnote;
    std::string MarketValueAdjFootnote;
    std::string ExchangeChargeFootnote0;
    std::string CurrentValuesFootnote;
    std::string DBOption1Footnote;
    std::string DBOption2Footnote;
    std::string ExpRatRiskChargeFootnote;
    std::string ExchangeChargeFootnote1;
    std::string FlexiblePremiumFootnote;
    std::string GuaranteedValuesFootnote;
    std::string CreditingRateFootnote;
    std::string MecFootnote;
    std::string MidpointValuesFootnote;
    std::string SinglePremiumFootnote;
    std::string MonthlyChargesFootnote;
    std::string UltCreditingRateFootnote;
    std::string MaxNaarFootnote;
    std::string PremTaxSurrChgFootnote;
    std::string PolicyFeeFootnote;
    std::string AssetChargeFootnote;
    std::string InvestmentIncomeFootnote;
    std::string IrrDbFootnote;
    std::string IrrCsvFootnote;
    std::string MortalityChargesFootnote;
    std::string LoanAndWithdrawalFootnote;
    std::string PresaleTrackingNumber;
    std::string CompositeTrackingNumber;
    std::string InforceTrackingNumber;
    std::string InforceCompositeTrackingNumber;
    std::string InforceNonGuaranteedFootnote0;
    std::string InforceNonGuaranteedFootnote1;
    std::string InforceNonGuaranteedFootnote2;
    std::string InforceNonGuaranteedFootnote3;
    std::string NonGuaranteedFootnote;
    std::string MonthlyChargesPaymentFootnote;
};

#endif // ihs_proddata_hpp

