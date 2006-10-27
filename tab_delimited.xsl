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

    $Id: tab_delimited.xsl,v 1.1.2.2 2006-10-27 15:52:19 etarassov Exp $

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

<xsl:variable name="basic_columns_xml">
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
<xsl:variable name="basic_columns" select="document('tab_delimited.xsl')/xsl:stylesheet/xsl:variable[@name='basic_columns_xml']/header" />

<xsl:variable name="all_columns" select="$basic_columns | $supplemental_columns" />

<!-- Print a non-empty value, and puts '0' for an empty (non-existing) value. -->
<xsl:template name="print_value"
   ><xsl:param name="value"

  /><xsl:choose
       ><xsl:when test="not($value)"
           ><xsl:text>0</xsl:text
       ></xsl:when
       ><xsl:otherwise
           ><xsl:value-of select="$value"
      /></xsl:otherwise
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

    ><xsl:call-template name="data_table"
        ><xsl:with-param name="pos" select="1"
       /><xsl:with-param name="columns" select="$all_columns"
       /><xsl:with-param name="headers" select="$empty_nodeset"
       /><xsl:with-param name="vectors" select="$empty_nodeset"
   /></xsl:call-template

></xsl:template>

<!--
    The template to be called from 'data_table'. See 'data_table' for parameter
    description.
    It generates the tab delimited table of values.
-->
<xsl:template name="do_data_table"
   ><xsl:param name="headers"
  /><xsl:param name="vectors"

  /><xsl:variable name="start_age" select="number($illustration/double_scalar[@name='Age'])"></xsl:variable

   ><!--
    Table: Headers
    ==============
 -->PolicyYear<xsl:text>&tab;</xsl:text
   >AttainedAge<xsl:text>&tab;</xsl:text

   ><xsl:for-each select="$headers"
       ><xsl:choose
           ><!-- a spacer
         --><xsl:when test="./@name"
               ><xsl:text><!-- leave the cell empty for a spacer column --></xsl:text
           ></xsl:when

           ><!-- if the title is specified directly, then use it
         --><xsl:when test="./@title"
               ><xsl:value-of select="./@title"
          /></xsl:when

           ><!-- otherwise get it from 'format.xml'
         --><xsl:otherwise
               ><xsl:call-template name="title"
                   ><xsl:with-param name="name" select="@name"
                  /><xsl:with-param name="basis" select="@basis"
                  /><xsl:with-param name="column" select="."
              /></xsl:call-template
           ></xsl:otherwise
       ></xsl:choose
   ><xsl:text>&tab;</xsl:text
   ></xsl:for-each
   ><xsl:text>&nl;</xsl:text

   ><!--
    Table: Data
    ===========
 --><xsl:variable name="is_inforce" select="number($illustration/double_scalar[@name='IsInforce'])"
  /><xsl:for-each select="$vectors[1]/duration"
       ><xsl:variable name="position" select="number(position())"></xsl:variable

       ><xsl:value-of select="$position + 1" /><xsl:text>&tab;</xsl:text
       ><xsl:value-of select="$start_age + $position" /><xsl:text>&tab;</xsl:text

       ><xsl:for-each select="$vectors"
           ><xsl:variable name="name" select="@name"
          /><xsl:variable name="basis" select="@basis"

          /><xsl:choose
               ><!--
                 deal with uncommon column cases in here
             --><xsl:when test="$is_inforce > 0 and ($name='IrrOnSurrender' or $name='IrrOnDeath')"
                   ><xsl:text>(inforce)</xsl:text
               ></xsl:when
               ><xsl:when test="$name='InforceLives'"
                   ><xsl:value-of select="$vectors[@name=$name]/duration[$position + 1]"
              /></xsl:when

               ><!--
                 the general case
             --><xsl:otherwise
                   ><xsl:choose
                       ><xsl:when test="not($name)"
                           ><xsl:text><!-- just an empty cell for a spacer column --></xsl:text
                       ></xsl:when
                       ><xsl:when test="not($basis)"
                           ><xsl:call-template name="print_value"
                               ><xsl:with-param name="value" select="$vectors[@name=$name]/duration[$position]"
                          /></xsl:call-template
                       ></xsl:when
                       ><xsl:otherwise
                           ><xsl:call-template name="print_value"
                               ><xsl:with-param name="value" select="$vectors[@name=$name][@basis=$basis]/duration[$position]"
                          /></xsl:call-template
                       ></xsl:otherwise
                   ></xsl:choose
               ></xsl:otherwise
            ></xsl:choose

            ><xsl:text>&tab;</xsl:text

        ></xsl:for-each

        ><xsl:text>&nl;</xsl:text

      ></xsl:for-each
></xsl:template>

</xsl:stylesheet>
