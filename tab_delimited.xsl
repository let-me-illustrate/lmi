<?xml version="1.0"?>
<!--
    Xsl template used to render illustration data taken from data.xml into csv.

    Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    http://savannah.nongnu.org/projects/lmi
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: tab_delimited.xsl,v 1.1.2.1 2006-10-20 17:46:02 etarassov Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<!DOCTYPE xsl:stylesheet
[
    <!ENTITY tab "&#x9;">
    <!ENTITY nl  "&#xA;">
]>
<xsl:stylesheet version="1.0"
	      xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="text" />

<xsl:include href="common.xsl" />

<xsl:variable name="headers_xml">
    <header name="DBOpt" title="DeathBenefitOption" />
    <header name="EeGrossPmt" title="EmployeeGrossPremium" />
    <header name="ErGrossPmt" title="CorporationGrossPremium" />
    <header name="NetWD" title="GrossWithdrawal" />
    <header name="NewCashLoan" title="NewCashLoan" />
    <header name="TotalLoanBalance" basis="run_curr_basis"
                    title="LoanBalance" />
    <header name="Outlay"
                    title="Outlay" />

    <header name="NetPmt" basis="run_curr_basis"
                    title="NetPremium" />

    <header name="PremTaxLoad" basis="run_curr_basis"
                    title="PremiumTaxLoad" />
    <header name="DacTaxLoad" basis="run_curr_basis"
                    title="DacTaxLoad" />
    <header name="PolicyFee" basis="run_curr_basis"
                    title="PolicyFee" />
    <header name="SpecAmtLoad" basis="run_curr_basis"
                    title="SpecifiedAmountLoad" />
    <header name="MonthlyFlatExtra"
                    title="MonthlyFlatExtra" />
    <header name="COICharge" basis="run_curr_basis"
                    title="MortalityCharge" />
    <header name="NetCOICharge" basis="run_curr_basis"
                    title="NetMortalityCharge" />
    <header name="SepAcctLoad" basis="run_curr_basis"
                    title="AccountValueLoadAfterMonthlyDeduction" />

    <header name="AnnSAIntRate" basis="run_curr_basis"
                    title="CurrentSeparateAccountInterestRate" />
    <header name="AnnGAIntRate" basis="run_curr_basis"
                    title="CurrentGeneralAccountInterestRate" />
    <header name="GrossIntCredited" basis="run_curr_basis"
                    title="CurrentGrossInterestCredited" />
    <header name="NetIntCredited" basis="run_curr_basis"
                    title="CurrentNetInterestCredited" />

    <header name="AcctVal" basis="run_guar_basis"
                    title="GuaranteedAccountValue" />
    <header name="CSVNet" basis="run_guar_basis"
                    title="GuaranteedNetCashSurrenderValue" />
    <header name="EOYDeathBft" basis="run_guar_basis"
                    title="GuaranteedYearEndDeathBenefit" />
    <header name="AcctVal" basis="run_curr_basis"
                    title="CurrentAccountValue" />
    <header name="CSVNet" basis="run_curr_basis"
                    title="CurrentNetCashSurrenderValue" />
    <header name="EOYDeathBft" basis="run_curr_basis"
                    title="CurrentYearEndDeathBenefit" />

    <header name="IrrOnSurrender"
                    title="IrrOnSurrender" />
    <header name="IrrOnDeath"
                    title="IrrOnDeath" />

    <header name="InforceLives"
                    title="YearEndInforceLives" />

    <header name="ClaimsPaid" basis="run_curr_basis"
                    title="ClaimsPaid" />
    <header name="NetClaims" basis="run_curr_basis"
                    title="NetClaims" />
    <header name="ExperienceReserve" basis="run_curr_basis"
                    title="ExperienceReserve" />
    <header name="ProjectedCoiCharge" basis="run_curr_basis"
                    title="ProjectedMortalityCharge" />
    <header name="KFactor" basis="run_curr_basis"
                    title="KFactor" />

    <header name="NetCOICharge" basis="run_curr_basis_sa_zero"
                    title="NetMortalityCharge0Int" />
    <header name="NetClaims" basis="run_curr_basis_sa_zero"
                    title="NetClaims0Int" />
    <header name="ExperienceReserve" basis="run_curr_basis_sa_zero"
                    title="ExperienceReserve0Int" />
    <header name="ProjectedCoiCharge" basis="run_curr_basis_sa_zero"
                    title="ProjectedMortalityCharge0Int" />
    <header name="KFactor" basis="run_curr_basis_sa_zero"
                    title="KFactor0Int" />

    <header name="ProducerCompensation" title="ProducerCompensation" />
</xsl:variable>

<!--
    Extract headers from $headers_xml variable.
    For that we are going to extract xml data from this stylesheet template
    taken as an xml file.
    XPath construct "document('')" references the stylesheet itself taken as XML file.
-->
<xsl:variable name="headers" select="document('tab_delimited.xsl')/xsl:stylesheet/xsl:variable[@name='headers_xml']/header" />

<!-- look below for some comments on $illustration -->
<xsl:template name="vector_item"
    ><xsl:param name="illustration"
   /><xsl:param name="position"
   /><xsl:param name="name"
   /><xsl:param name="basis"
   /><xsl:choose
        ><xsl:when test="$basis"
            ><xsl:choose
                ><xsl:when test="count($illustration/double_vector[@name=$name][@basis=$basis]/duration) != 0"
                    ><xsl:value-of select="$illustration/double_vector[@name=$name][@basis=$basis]/duration[$position]"
                /></xsl:when
                ><xsl:otherwise
                    ><xsl:value-of select="$illustration/string_vector[@name=$name][@basis=$basis]/duration[$position]"
                /></xsl:otherwise
            ></xsl:choose
        ></xsl:when
        ><xsl:otherwise
            ><xsl:choose
                ><xsl:when test="count($illustration/double_vector[@name=$name]/duration) != 0"
                    ><xsl:value-of select="$illustration/double_vector[@name=$name]/duration[$position]"
                /></xsl:when
                ><xsl:otherwise
                    ><xsl:value-of select="$illustration/string_vector[@name=$name]/duration[$position]"
                /></xsl:otherwise
            ></xsl:choose
        ></xsl:otherwise
    ></xsl:choose
></xsl:template>

<xsl:template match="/illustration"
    ><xsl:text>&nl;</xsl:text><xsl:text>&nl;</xsl:text


    >FOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.<xsl:text>&nl;</xsl:text><xsl:text>&nl;</xsl:text

    >ProducerName<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='ProducerName']" /><xsl:text>&nl;</xsl:text

    >ProducerStreet<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='ProducerStreet']" /><xsl:text>&nl;</xsl:text

    >ProducerCity<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='ProducerCity']" /><xsl:text>&nl;</xsl:text

    >CorpName<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='CorpName']" /><xsl:text>&nl;</xsl:text

    >Insured1<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='Insured1']" /><xsl:text>&nl;</xsl:text

    >Gender<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='Gender']" /><xsl:text>&nl;</xsl:text

    >Smoker<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='Smoker']" /><xsl:text>&nl;</xsl:text

    >IssueAge<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="double_scalar[@name='Age']" /><xsl:text>&nl;</xsl:text

    >InitBaseSpecAmt<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']" /><xsl:text>&nl;</xsl:text

    >InitTermSpecAmt<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="double_scalar[@name='InitTermSpecAmt']" /><xsl:text>&nl;</xsl:text

    ><!-- InitTotalSA = InitBaseSpecAmt + InitTermSpecAmt
  -->Total:<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="double_scalar[@name='InitTotalSA']" /><xsl:text>&nl;</xsl:text

    >PolicyMktgName<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='PolicyMktgName']" /><xsl:text>&nl;</xsl:text

    >PolicyLegalName<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='PolicyLegalName']" /><xsl:text>&nl;</xsl:text

    >PolicyForm<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='PolicyForm']" /><xsl:text>&nl;</xsl:text

    >UWClass<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='UWClass']" /><xsl:text>&nl;</xsl:text

    >UWType<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    ><xsl:value-of select="string_scalar[@name='UWType']" /><xsl:text>&nl;</xsl:text

    >DatePrepared<xsl:text>&tab;</xsl:text><xsl:text>&tab;</xsl:text
    >'<xsl:value-of select="string_scalar[@name='PrepMonth']" /> <xsl:value-of select="string_scalar[@name='PrepDay']" />, <xsl:value-of select="string_scalar[@name='PrepYear']" />'<xsl:text>&nl;</xsl:text

    ><xsl:variable name="loop_variable" select="'Outlay'"></xsl:variable
    ><xsl:variable name="start_age" select="number(double_scalar[@name='Age'])"></xsl:variable


    >PolicyYear<xsl:text>&tab;</xsl:text
    >AttainedAge<xsl:text>&tab;</xsl:text

    ><xsl:for-each select="$headers"
        ><xsl:value-of select="@title" /><xsl:text>&tab;</xsl:text
        ><!--
         TODO Prefer calling the title template instead of using hardcoded title names
         <xsl:call-template name="title"
            ><xsl:with-param name="name" select="@name"
           /><xsl:with-param name="basis" select="5"
       /></xsl:call-template
    >
  --></xsl:for-each
    ><xsl:text>&nl;</xsl:text

    ><!--
         In an inner loop below we are iterating over a node set from some other document.
         For some reason the root node '/' becomes bound to that other document
         and we cannot use '/illustration' no longer to acces our xml data.
         Therefore bind '/illustration' to a variable '$illustration' and pass it
         as a parameter to a template.
  --><xsl:variable name="illustration" select="/illustration"></xsl:variable

    ><xsl:for-each select="double_vector[@name=$loop_variable]/duration"
        ><xsl:variable name="position" select="number(position())"></xsl:variable

        ><xsl:value-of select="$position + 1" /><xsl:text>&tab;</xsl:text
        ><xsl:value-of select="$start_age + $position" /><xsl:text>&tab;</xsl:text

        ><xsl:for-each select="$headers"
            ><!--
                 deal with uncommon column cases in here
          --><xsl:choose
                ><xsl:when test="(@name='IrrOnSurrender' or @name='IrrOnDeath') and number($illustration/double_scalar[@name='IsInforce'])!=0"
                    ><xsl:text>(inforce)</xsl:text
                ></xsl:when

                ><xsl:when test="@title='InforceLives'"
                   ><xsl:value-of select="number($illustration/double_vector[@name='InforceLives']/duration[$position + 1])"
               /></xsl:when

                ><!--xsl:when test="@title=''"
// Show experience-rating columns for current-expense, zero-
// interest basis if used, to support testing.
std::vector<e_run_basis> const& bases(ledger_values.GetRunBases());
if
   (   bases.end()
   !=  std::find(bases.begin(), bases.end(), e_run_curr_basis_sa_zero)
   )
   {
   LedgerVariant const& Curr0 = ledger_values.GetCurrZero();
   os << Curr0.value_str("NetCOICharge"          ,j) << '\t';
   os << Curr0.value_str("NetClaims"             ,j) << '\t';
   os << Curr0.value_str("ExperienceReserve"     ,j) << '\t';
   os << Curr0.value_str("ProjectedCoiCharge"    ,j) << '\t';
   os << Curr0.value_str("KFactor"               ,j) << '\t';
   }
else
   {
   os << "0\t";
   os << "0\t";
   os << "0\t";
   os << "0\t";
   os << "0\t";
   }
                ></xsl:when
              --><xsl:otherwise
                    ><xsl:call-template name="vector_item"
                        ><xsl:with-param name="illustration" select="$illustration"
                       /><xsl:with-param name="name" select="@name"
                       /><xsl:with-param name="basis" select="@basis"
                       /><xsl:with-param name="position" select="$position"
                   /></xsl:call-template
                ></xsl:otherwise
            ></xsl:choose
            ><xsl:text>&tab;</xsl:text
        ></xsl:for-each
        ><xsl:text>&nl;</xsl:text
      ></xsl:for-each
></xsl:template>

</xsl:stylesheet>
