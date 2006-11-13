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
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA

    http://savannah.nongnu.org/projects/lmi
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: tab_delimited.xsl,v 1.1.2.13 2006-11-13 12:45:11 etarassov Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<!DOCTYPE xsl:stylesheet [
<!ENTITY tab "&#x9;">
<!ENTITY nl "&#xA;">
]>
<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xsi:schemaLocation="http://savannah.nongnu.org/projects/lmi schema.xsd">

<xsl:include href="tsv_common.xsl"/>

<xsl:variable name="basic_columns_xml">
    <column name="DBOpt">DeathBenefitOption</column>
    <column name="EeGrossPmt">EmployeeGrossPremium</column>
    <column name="ErGrossPmt">CorporationGrossPremium</column>
    <column name="NetWD">GrossWithdrawal</column>
    <column name="NewCashLoan">NewCashLoan</column>
    <column name="TotalLoanBalance" basis="run_curr_basis">LoanBalance</column>
    <column name="Outlay">Outlay</column>

    <column name="NetPmt" basis="run_curr_basis">NetPremium</column>

    <column name="PremTaxLoad" basis="run_curr_basis">PremiumTaxLoad</column>
    <column name="DacTaxLoad" basis="run_curr_basis">DacTaxLoad</column>
    <column name="PolicyFee" basis="run_curr_basis">PolicyFee</column>
    <column name="SpecAmtLoad" basis="run_curr_basis">SpecifiedAmountLoad</column>
    <column name="MonthlyFlatExtra">MonthlyFlatExtra</column>
    <column name="COICharge" basis="run_curr_basis">MortalityCharge</column>
    <column name="NetCOICharge" basis="run_curr_basis">NetMortalityCharge</column>
    <column name="SepAcctLoad" basis="run_curr_basis">AccountValueLoadAfterMonthlyDeduction</column>

    <column name="AnnSAIntRate" basis="run_curr_basis">CurrentSeparateAccountInterestRate</column>
    <column name="AnnGAIntRate" basis="run_curr_basis">CurrentGeneralAccountInterestRate</column>
    <column name="GrossIntCredited" basis="run_curr_basis">CurrentGrossInterestCredited</column>
    <column name="NetIntCredited" basis="run_curr_basis">CurrentNetInterestCredited</column>

    <column name="AcctVal" basis="run_guar_basis">GuaranteedAccountValue</column>
    <column name="CSVNet" basis="run_guar_basis">GuaranteedNetCashSurrenderValue</column>
    <column name="EOYDeathBft" basis="run_guar_basis">GuaranteedYearEndDeathBenefit</column>
    <column name="AcctVal" basis="run_curr_basis">CurrentAccountValue</column>
    <column name="CSVNet" basis="run_curr_basis">CurrentNetCashSurrenderValue</column>
    <column name="EOYDeathBft" basis="run_curr_basis">CurrentYearEndDeathBenefit</column>

    <column name="IrrOnSurrender">IrrOnSurrender</column>
    <column name="IrrOnDeath">IrrOnDeath</column>

    <column name="InforceLives">YearEndInforceLives</column>

    <column name="ClaimsPaid" basis="run_curr_basis">ClaimsPaid</column>
    <column name="NetClaims" basis="run_curr_basis">NetClaims</column>
    <column name="ExperienceReserve" basis="run_curr_basis">ExperienceReserve</column>
    <column name="ProjectedCoiCharge" basis="run_curr_basis">ProjectedMortalityCharge</column>
    <column name="KFactor" basis="run_curr_basis">KFactor</column>

    <column name="NetCOICharge" basis="run_curr_basis_sa_zero">NetMortalityCharge0Int</column>
    <column name="NetClaims" basis="run_curr_basis_sa_zero">NetClaims0Int</column>
    <column name="ExperienceReserve" basis="run_curr_basis_sa_zero">ExperienceReserve0Int</column>
    <column name="ProjectedCoiCharge" basis="run_curr_basis_sa_zero">ProjectedMortalityCharge0Int</column>
    <column name="KFactor" basis="run_curr_basis_sa_zero">KFactor0Int</column>

    <column name="ProducerCompensation">ProducerCompensation</column>
</xsl:variable>

<!--
    Extract headers from $headers_xml variable.
    For that we are going to extract xml data from this stylesheet template
    taken as an xml file.
    XPath construct "document('')" references the stylesheet itself taken as XML file.
-->
<xsl:variable name="basic_columns" select="document('tab_delimited.xsl')/xsl:stylesheet/xsl:variable[@name='basic_columns_xml']/column"/>

<xsl:variable name="all_columns" select="$basic_columns | $calculation_summary_columns"/>

<xsl:template match="/illustration">
    <xsl:text>&nl;</xsl:text>
    <xsl:text>&nl;</xsl:text>
    <xsl:text>FOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.&nl;</xsl:text>
    <xsl:text>&nl;</xsl:text>
    <xsl:text>ProducerName&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='ProducerName']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>ProducerStreet&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='ProducerStreet']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>ProducerCity&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='ProducerCity']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>CorpName&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='CorpName']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>Insured1&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='Insured1']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>Gender&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='Gender']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>Smoker&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='Smoker']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>IssueAge&tab;&tab;</xsl:text>
    <xsl:value-of select="double_scalar[@name='Age']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>InitBaseSpecAmt&tab;&tab;</xsl:text>
    <xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>InitTermSpecAmt&tab;&tab;</xsl:text>
    <xsl:value-of select="double_scalar[@name='InitTermSpecAmt']"/><xsl:text>&nl;</xsl:text>
    <!-- InitTotalSA = InitBaseSpecAmt + InitTermSpecAmt -->
    <xsl:text>Total:&tab;&tab;</xsl:text>
    <xsl:value-of select="double_scalar[@name='InitTotalSA']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>PolicyMktgName&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='PolicyMktgName']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>PolicyLegalName&tab;&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='PolicyLegalName']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>PolicyForm&tab;</xsl:text><xsl:text>&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='PolicyForm']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>UWClass&tab;</xsl:text><xsl:text>&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='UWClass']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>UWType&tab;</xsl:text><xsl:text>&tab;</xsl:text>
    <xsl:value-of select="string_scalar[@name='UWType']"/><xsl:text>&nl;</xsl:text>
    <xsl:text>DatePrepared&tab;</xsl:text><xsl:text>&tab;</xsl:text>
    <xsl:text>'</xsl:text>
        <xsl:value-of select="string_scalar[@name='PrepMonth']"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="string_scalar[@name='PrepDay']"/>
        <xsl:text>, </xsl:text>
        <xsl:value-of select="string_scalar[@name='PrepYear']"/>
    <xsl:text>'</xsl:text><xsl:text>&nl;</xsl:text>
    <xsl:call-template name="data_table">
        <xsl:with-param name="pos" select="1"/>
        <xsl:with-param name="columns" select="$all_columns"/>
        <xsl:with-param name="headers" select="$empty_nodeset"/>
        <xsl:with-param name="vectors" select="$empty_nodeset"/>
    </xsl:call-template>
</xsl:template>

<!--
    Templates to be called from 'do_data_table' for every row in a table.
    The purpose is to generate some static columns in the table.
-->
<xsl:template name="do_data_table_pre_headers">
    <xsl:text>PolicyYear&tab;</xsl:text>
</xsl:template>
<xsl:template name="do_data_table_pre_data">
    <xsl:param name="position"/>
    <xsl:value-of select="$policy_year/duration[$position]"/>
    <xsl:text>&tab;</xsl:text>
</xsl:template>

</xsl:stylesheet>
