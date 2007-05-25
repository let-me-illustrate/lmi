<?xml version="1.0" encoding="UTF-8"?>
<!--
    Annuity illustrations.

    Copyright (C) 2007 Gregory W. Chicares.

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

    $Id: variable_annuity.xsl,v 1.3.2.9 2007-05-25 08:29:57 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <xsl:include href="illustrations_common.xsl" />
  <xsl:output method="xml" encoding="UTF-8" indent="yes" />
  <xsl:variable name="counter" select="1"/>
  <xsl:variable name="numberswoc">0123456789</xsl:variable>
  <xsl:variable name="numberswc">0123456789,</xsl:variable>
  <xsl:variable name="max-years">100</xsl:variable>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the Hypothetical Illustration Current page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-current" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.55in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.3in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration CurrentZero page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-current-zero" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.55in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.3in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration Guaranteed page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-guaranteed" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.55in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.3in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration GuaranteedZero page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-guaranteed-zero" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.55in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.3in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 1. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-1" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="1.5in" margin-bottom="1.0in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.0in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 2. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-2" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="1.5in" margin-bottom="1.0in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.0in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 3. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-3" page-height="11in" page-width="8.5in"
          margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="1.5in" margin-bottom="1.0in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.0in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in"
            margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
            <!-- Central part of page -->
            <fo:region-body column-count="1" margin-top="2.5in" margin-bottom="1.55in"/>
            <!-- Header -->
            <fo:region-before border-after-style="solid" extent="3in"/>
            <!-- Footer -->
            <fo:region-after border-before-style="solid" extent="1in"/>
          </fo:simple-page-master>
        </xsl:if>

      </fo:layout-master-set>

      <!-- Hypothetical Illustration Current -->

      <fo:page-sequence master-reference="hypothetical-illustration-current">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'Current'"/>
          </xsl:call-template>
          <xsl:call-template name="titles"/>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="preliminary-footer">
            <xsl:with-param name="basis" select="'Current'"/>
          </xsl:call-template>
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">

          <xsl:variable name="basis-lapse-year-text">
            <xsl:call-template name="get-basis-lapse-year">
              <xsl:with-param name="basis" select="'Current'"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="basis-lapse-year" select="number($basis-lapse-year-text)"/>

          <!-- Illustration Values -->
          <fo:table table-layout="fixed" width="100%"
            font-size="9.0pt" font-family="serif" font-weight="normal">
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <!-- Column Headings -->
            <xsl:call-template name="column-headings"/>
            <!-- Column Values -->
            <!-- make inforce illustration start in the inforce year -->
            <fo:table-body padding-before="5.0pt" border-top-style="solid"
              border-top-width="1pt" border-top-color="blue">
              <xsl:call-template name="column-values">
                <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                <xsl:with-param name="inforceyear" select="0 - illustration/scalar/InforceYear"/>
                <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
                <xsl:with-param name="basis" select="'Current'"/>
              </xsl:call-template>
            </fo:table-body>
          </fo:table>

        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration CurrentZero -->

      <fo:page-sequence master-reference="hypothetical-illustration-current-zero">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
          </xsl:call-template>
          <xsl:call-template name="titles"/>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="preliminary-footer">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
          </xsl:call-template>
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">

          <xsl:variable name="basis-lapse-year-text">
            <xsl:call-template name="get-basis-lapse-year">
              <xsl:with-param name="basis" select="'CurrentZero'"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="basis-lapse-year" select="number($basis-lapse-year-text)"/>

          <!-- Illustration Values -->
          <fo:table table-layout="fixed" width="100%"
            font-size="9.0pt" font-family="serif" font-weight="normal">
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <!-- Column Headings -->
            <xsl:call-template name="column-headings"/>
            <!-- Column Values -->
            <!-- make inforce illustration start in the inforce year -->
            <fo:table-body padding-before="5.0pt" border-top-style="solid"
              border-top-width="1pt" border-top-color="blue">
              <xsl:call-template name="column-values">
                <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                <xsl:with-param name="inforceyear" select="0 - illustration/scalar/InforceYear"/>
                <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
                <xsl:with-param name="basis" select="'CurrentZero'"/>
              </xsl:call-template>
            </fo:table-body>
          </fo:table>

        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration Guaranteed -->

      <fo:page-sequence master-reference="hypothetical-illustration-guaranteed">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
          </xsl:call-template>
          <xsl:call-template name="titles"/>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="preliminary-footer">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
          </xsl:call-template>
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">

          <xsl:variable name="basis-lapse-year-text">
            <xsl:call-template name="get-basis-lapse-year">
              <xsl:with-param name="basis" select="'Guaranteed'"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="basis-lapse-year" select="number($basis-lapse-year-text)"/>

          <!-- Illustration Values -->
          <fo:table table-layout="fixed" width="100%"
            font-size="9.0pt" font-family="serif" font-weight="normal">
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <!-- Column Headings -->
            <xsl:call-template name="column-headings"/>
            <!-- Column Values -->
            <!-- make inforce illustration start in the inforce year -->
            <fo:table-body padding-before="5.0pt" border-top-style="solid"
              border-top-width="1pt" border-top-color="blue">
              <xsl:call-template name="column-values">
                <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                <xsl:with-param name="inforceyear" select="0 - illustration/scalar/InforceYear"/>
                <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
                <xsl:with-param name="basis" select="'Guaranteed'"/>
              </xsl:call-template>
            </fo:table-body>
          </fo:table>

        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration GuaranteedZero -->

      <fo:page-sequence master-reference="hypothetical-illustration-guaranteed-zero">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
          </xsl:call-template>
          <xsl:call-template name="titles"/>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="preliminary-footer">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
          </xsl:call-template>
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">

          <xsl:variable name="basis-lapse-year-text">
            <xsl:call-template name="get-basis-lapse-year">
              <xsl:with-param name="basis" select="'GuaranteedZero'"/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="basis-lapse-year" select="number($basis-lapse-year-text)"/>

          <!-- Illustration Values -->
          <fo:table table-layout="fixed" width="100%"
            font-size="9.0pt" font-family="serif" font-weight="normal">
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <fo:table-column/>
            <!-- Column Headings -->
            <xsl:call-template name="column-headings"/>
            <!-- Column Values -->
            <!-- make inforce illustration start in the inforce year -->
            <fo:table-body padding-before="5.0pt" border-top-style="solid"
              border-top-width="1pt" border-top-color="blue">
              <xsl:call-template name="column-values">
                <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                <xsl:with-param name="inforceyear" select="0 - illustration/scalar/InforceYear"/>
                <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
                <xsl:with-param name="basis" select="'GuaranteedZero'"/>
              </xsl:call-template>
            </fo:table-body>
          </fo:table>

        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 1 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-1">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
            <xsl:text>Explanation and Footnotes</xsl:text>
          </fo:block>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-size="10.0pt" text-align="left" font-family="sans-serif" font-weight="bold"
            text-decoration="underline">
            <xsl:text>The Illustration</xsl:text>
          </fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>The purpose of this illustration is to show how hypothetical rates of return will</xsl:text>
            <xsl:text> affect Contract Value. These hypothetical returns do not reflect past performance</xsl:text>
            <xsl:text> and are not predictive of future results. Actual results could be less than or</xsl:text>
            <xsl:text> greater than the hypothetical rates and in all likelihood will vary from year to</xsl:text>
            <xsl:text> year. The maximum rate illustrated must be reasonable considering market conditions</xsl:text>
            <xsl:text> and the available investment choices.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>The hypothetical Net Rate of Return is the rate of return net of investment</xsl:text>
            <xsl:text> management, custody and other expenses charged by the investment manager of the</xsl:text>
            <xsl:text> underlying fund(s).</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>This illustration does not reflect tax and accounting consequences and is not</xsl:text>
            <xsl:text> intended as tax advice nor may be relied on for purposes of avoiding any federal</xsl:text>
            <xsl:text> tax penalties. Consult professional tax advisors for tax advice.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>This illustration is not a Contact or an offer or solicitation to enter into a</xsl:text>
            <xsl:text> Contract. Offers are made only through the Confidential Private Placement</xsl:text>
            <xsl:text> Memorandum for </xsl:text>
            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
            <fo:inline vertical-align="super" font-size="7.5pt">SM</fo:inline>
            <xsl:text>.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>This material must be preceded or accompanied by the current Confidential Private</xsl:text>
            <xsl:text> Placement Memorandum for </xsl:text>
            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
            <fo:inline vertical-align="super" font-size="7.5pt">SM</fo:inline>
            <xsl:text> and the current prospectuses and private placement memoranda for its underlying</xsl:text>
            <xsl:text> underlying investment choices. Investors should carefully consider the investment</xsl:text>
            <xsl:text> objectives, risks, charges and expenses of the variable annuity Contract and its</xsl:text>
            <xsl:text> underlying investment choices. Please read the prospectuses and private placement</xsl:text>
            <xsl:text> memoranda carefully before investing or sending money.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
            <xsl:text> Financial Group is a marketing designation for </xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoName"/>
            <xsl:text> (</xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
            <xsl:text>) and its subsidiaries.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="10.0pt" text-align="left" font-family="sans-serif" font-weight="bold"
            text-decoration="underline">
            <xsl:text>The Contract</xsl:text>
          </fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
            <fo:inline vertical-align="super" font-size="7.5pt">SM</fo:inline>
            <xsl:text> is an </xsl:text>
            <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
            <xsl:text> issued by </xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoName"/>
            <xsl:text> (</xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
            <xsl:text>), </xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoStreet"/>
            <xsl:text>.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Contract Requirements: </fo:inline>
            <xsl:text> This Contract is only available to persons who are deemed accredited investors</xsl:text>
            <xsl:text> and qualified purchasers under applicable federal securities laws. The minimum</xsl:text>
            <xsl:text> initial premium is $500,000. You must be able to bear the risk of loss of your</xsl:text>
            <xsl:text> entire investment in the Contract. You will be required to represent to </xsl:text>
            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
            <xsl:text> that you are familiar with and understand the fundamental risks and financial</xsl:text>
            <xsl:text> hazards of investing in the Contract.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Purchase Payment Charges: </fo:inline>
            <xsl:text> The sum of the Purchase Payment Distribution Charges, if any, and any Purchase</xsl:text>
            <xsl:text> Payment Expense Charges including state premium tax, if applicable.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Monthly Charges: </fo:inline>
            <xsl:text> Monthly Charges will be deducted from the Contract Value. The Company may assess</xsl:text>
            <xsl:text> Monthly Charges of three (3) types:  Mortality and Expense Risk Charges, Asset</xsl:text>
            <xsl:text> Based Distribution Charges and Administrative Charges. The Monthly Charges will</xsl:text>
            <xsl:text> be taken from the Money Market Sub-Account.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <xsl:text>Due to the limitations on withdrawals from investment options offered under the</xsl:text>
            <xsl:text> Contract, we reserve the right to (i) allocate a portion of each purchase</xsl:text>
            <xsl:text> payment to the Money Market Sub-Account and (ii) transfer Contract Value to</xsl:text>
            <xsl:text> the Money Market Sub-Account from time to time to ensure that monthly charges</xsl:text>
            <xsl:text> can be deducted from the Money Market Sub-Account when due. The illustrated</xsl:text>
            <xsl:text> values do not reflect any amounts that may be allocated to meet the minimum</xsl:text>
            <xsl:text> money market requirement.</xsl:text>
          </fo:block>
        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 2 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-2">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
            <xsl:text>Explanation and Footnotes</xsl:text>
          </fo:block>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Mortality and Expense Charge ("M&amp;E"): </fo:inline>
            <xsl:text> The current charge as described in the Table of Charges of the Confidential</xsl:text>
            <xsl:text> Private Placement Memorandum for </xsl:text>
            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
            <fo:inline vertical-align="super" font-size="7.5pt">SM</fo:inline>
            <xsl:text>.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Administrative Charge: </fo:inline>
            <xsl:text> The current charge is equal on an annual basis to .20% of the Contract Value,</xsl:text>
            <xsl:text> if on any monthly calculation date, the Contract Value is less than $1,000,000.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Asset Based Distribution Charge: </fo:inline>
            <xsl:text> The current charge is equal to annual negotiated asset based compensation, if any.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Withdrawals: </fo:inline>
            <xsl:text> Early withdrawals may result in tax consequences on the amount being withdrawn</xsl:text>
            <xsl:text> and, in some cases, an early withdrawal penalty tax. As a result, you may receive</xsl:text>
            <xsl:text> less than the original amount invested. Any withdrawals are taxed to the extent of</xsl:text>
            <xsl:text> any gain in the contract and an additional 10% penalty tax will apply to the</xsl:text>
            <xsl:text> taxable portion of the withdrawal.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Illiquid Sub-Accounts: </fo:inline>
            <xsl:text> We have also made available under the Contract the ability to allocate purchase</xsl:text>
            <xsl:text> payments and Contract Value to sub-accounts that will provide for investment in</xsl:text>
            <xsl:text> certain alternative investments including hedge funds. Because of the liquidity</xsl:text>
            <xsl:text> restrictions imposed by these alternative investment funds, these sub-accounts</xsl:text>
            <xsl:text> are referred to in the Contract and in the private placement memorandum as illiquid</xsl:text>
            <xsl:text> sub-accounts. Contract owners must understand, and be willing to bear, the risks</xsl:text>
            <xsl:text> associated with such investments. If Contract Value is allocated to illiquid</xsl:text>
            <xsl:text> sub-accounts, the Owner will not be able to transfer Contract Value or effect</xsl:text>
            <xsl:text> a withdrawal or surrender of the Contract until specified dates. Refer to the</xsl:text>
            <xsl:text> Confidential Private Placement Memorandum for details.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="bold">
            <xsl:text>Note:  Please refer to the Contract for complete explanation of benefits, rights</xsl:text>
            <xsl:text> and obligations. In the event of a conflict between the illustration and the</xsl:text>
            <xsl:text> Contract, the terms of the Contract will control.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="10.0pt" text-align="left" font-family="sans-serif" font-weight="bold"
            text-decoration="underline">
            <xsl:text>Column Definitions</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Year: </fo:inline>
            <xsl:text> The annual period beginning on the hypothetical Contract issue date.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Purchase Payment: </fo:inline>
            <xsl:text> A payment made by or on behalf of a Contract Owner during the Accumulation</xsl:text>
            <xsl:text> Period of the Contract.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Purchase Payment Charges: </fo:inline>
            <xsl:text> The sum of the Purchase Payment Distribution Charge and applicable state premium</xsl:text>
            <xsl:text> tax charged on an upfront basis, if any.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Withdrawal(s): </fo:inline>
            <xsl:text> The net dollar amount redeemed from the Contract. Withdrawals are reflected</xsl:text>
            <xsl:text> at the beginning of each year.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Asset Charges: </fo:inline>
            <xsl:text> The annual M&amp;E charges, asset based distribution charges and administrative</xsl:text>
            <xsl:text> charges deducted from Contract Value.</xsl:text>
          </fo:block>
        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 3 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-3">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header"/>
          <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
            <xsl:text>Explanation and Footnotes</xsl:text>
          </fo:block>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Investment Experience: </fo:inline>
            <xsl:text> The annual investment income credited to the Contract sub-accounts using</xsl:text>
            <xsl:text> the hypothetical Net Rate of Return.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">End of Year Contract Value: </fo:inline>
            <xsl:text> The value of the Contract sub-account(s) after applying any Purchase Payments</xsl:text>
            <xsl:text> (less Purchase Payment Charges), deducting any Withdrawals and crediting annual</xsl:text>
            <xsl:text> hypothetical Investment Experience.</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block font-size="9.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
            <fo:inline font-weight="bold">Cumulative Contract Value IRR: </fo:inline>
            <xsl:text> The Contract Value Internal Rate of Return ("IRR") is an interest rate at which</xsl:text>
            <xsl:text> the Net Purchase Payment demonstrated would have to be invested outside the</xsl:text>
            <xsl:text> Contract to generate the Contract Value. The IRR is illustrative only and does</xsl:text>
            <xsl:text> not predict future results.</xsl:text>
          </fo:block>
          <xsl:choose>
            <xsl:when test="$has_supplemental_report">
            </xsl:when>
            <xsl:otherwise>
              <fo:block id="endofdoc"></fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </fo:flow>

      </fo:page-sequence>

      <!-- Supplemental Report -->

      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <xsl:call-template name="standard-header"/>
            <xsl:call-template name="scalar-header">
              <xsl:with-param name="basis" select="NoBasis"/>
            </xsl:call-template>
            <fo:block font-size="13.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
              <xsl:text>SUPPLEMENTAL REPORT</xsl:text>
            </fo:block>
            <fo:block font-size="10.0pt" text-align="center" font-family="sans-serif" font-weight="normal"
              border-top-style="solid" border-top-width="1pt" border-top-color="black" padding-after="2.5pt">
              <fo:block><xsl:text></xsl:text></fo:block>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <fo:static-content flow-name="xsl-region-after">
            <xsl:call-template name="standard-footer">
              <xsl:with-param name="displaypagenumber" select="1"/>
            </xsl:call-template>
          </fo:static-content>

          <!-- Supplemental report body -->
          <xsl:call-template name="supplemental-report-body" />
        </fo:page-sequence>
      </xsl:if>

    </fo:root>
  </xsl:template>

  <xsl:template name="standard-header">
    <fo:table table-layout="fixed" width="100%" padding-after="2.5pt"
      font-size="13.0pt" font-family="sans-serif" font-weight="bold">
      <fo:table-column column-width="50mm"/>
      <fo:table-column column-width="90mm"/>
      <fo:table-column column-width="50mm"/>
      <fo:table-body padding-before="2pt">
        <fo:table-row>
          <fo:table-cell>
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="center">
              <xsl:text>FOR BROKER-DEALER USE ONLY</xsl:text>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="center">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
      </fo:table-body>
    </fo:table>
    <fo:block><fo:leader></fo:leader></fo:block>
    <fo:block font-size="14.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
      <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
      <fo:inline vertical-align="super" font-size="7.0pt">SM</fo:inline>
    </fo:block>
    <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="normal">
      <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
    </fo:block>
    <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="normal"
      border-top-style="solid" border-top-width="1pt" border-top-color="black" padding-after="8.0pt">
      <fo:block><xsl:text></xsl:text></fo:block>
    </fo:block>
  </xsl:template>

  <xsl:template name="scalar-header">
    <xsl:param name="basis"/>
    <fo:table table-layout="fixed" width="100%" padding-after="2.5pt"
      font-size="9.5pt" font-family="sans-serif" font-weight="normal">
      <fo:table-column column-width="125mm"/>
      <fo:table-column column-width="10mm"/>
      <fo:table-column column-width="45mm"/>
      <fo:table-column column-width="10mm"/>
      <fo:table-body padding-before="2pt">
        <fo:table-row>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text>Date Prepared: </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepMonth"/>
              <xsl:text>&#xA0;</xsl:text>
              <xsl:value-of select="illustration/scalar/PrepDay"/>
              <xsl:text>, </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepYear"/>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text>Issue State: </xsl:text>
              <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
        <fo:table-row>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text>Prepared For: </xsl:text>
              <xsl:choose>
                <xsl:when test="not($is_composite)">
                  <!-- Properly adjust for long user input strings limit output to 140 characters for appox. 2 lines -->
                  <xsl:call-template name="limit-string" >
                    <xsl:with-param name="pass-string" select="illustration/scalar/Insured1"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:call-template name="limit-string" >
                    <xsl:with-param name="pass-string" select="illustration/scalar/CorpName"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:choose>
                <xsl:when test="$basis='Current'">
                  <xsl:text>Net Rate of Return: </xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                </xsl:when>
                <xsl:when test="$basis='CurrentZero'">
                  <xsl:text>Net Rate of Return: </xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_CurrentZero"/>
                </xsl:when>
                <xsl:when test="$basis='Guaranteed'">
                  <xsl:text>Net Rate of Return: </xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Guaranteed"/>
                </xsl:when>
                <xsl:when test="$basis='GuaranteedZero'">
                  <xsl:text>Net Rate of Return: </xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_GuaranteedZero"/>
                </xsl:when>
                <xsl:when test="$basis='NoBasis'">
                  <xsl:text></xsl:text>
                </xsl:when>
               <xsl:otherwise>
                  <xsl:text></xsl:text>
               </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
        <fo:table-row>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:choose>
                <xsl:when test="not($is_composite)">
                  <xsl:text>Annuitant Age at Issue: </xsl:text>
                  <xsl:value-of select="illustration/scalar/Age"/>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:if test="illustration/scalar/Franchise!=''">
                    <fo:block text-align="left">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limit-string" >
                        <xsl:with-param name="pass-string" select="illustration/scalar/Franchise"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </fo:block>
                  </xsl:if>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:choose>
                <xsl:when test="$basis='Current'">
                  <xsl:text>Current Charges</xsl:text>
                </xsl:when>
                <xsl:when test="$basis='CurrentZero'">
                  <xsl:text>Current Charges</xsl:text>
                </xsl:when>
                <xsl:when test="$basis='Guaranteed'">
                  <xsl:text>Guaranteed Charges</xsl:text>
                </xsl:when>
                <xsl:when test="$basis='GuaranteedZero'">
                  <xsl:text>Guaranteed Charges</xsl:text>
                </xsl:when>
                <xsl:when test="$basis='NoBasis'">
                  <xsl:text></xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text></xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell>
            <fo:block text-align="left">
              <xsl:text></xsl:text>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
        <xsl:if test="not($is_composite)">
          <xsl:choose>
            <xsl:when test="illustration/scalar/Franchise!='' and illustration/scalar/PolicyNumber!=''">
              <fo:table-row>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Master contract: </xsl:text>
                    <xsl:call-template name="limit-string" >
                      <xsl:with-param name="pass-string" select="illustration/scalar/Franchise"/>
                      <xsl:with-param name="length" select="15"/>
                    </xsl:call-template>
                    <xsl:text>&#xA0;&#xA0;&#xA0;Contract number: </xsl:text>
                    <xsl:call-template name="limit-string" >
                      <xsl:with-param name="pass-string" select="illustration/scalar/PolicyNumber"/>
                      <xsl:with-param name="length" select="15"/>
                    </xsl:call-template>
                  </fo:block>
                </fo:table-cell>
              </fo:table-row>
            </xsl:when>
            <xsl:when test="illustration/scalar/Franchise!=''">
              <fo:table-row>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Master contract: </xsl:text>
                    <xsl:call-template name="limit-string" >
                      <xsl:with-param name="pass-string" select="illustration/scalar/Franchise"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                  </fo:block>
                </fo:table-cell>
              </fo:table-row>
            </xsl:when>
            <xsl:when test="illustration/scalar/PolicyNumber!=''">
              <fo:table-row>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Contract number: </xsl:text>
                    <xsl:call-template name="limit-string" >
                      <xsl:with-param name="pass-string" select="illustration/scalar/PolicyNumber"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                  </fo:block>
                </fo:table-cell>
              </fo:table-row>
            </xsl:when>
          </xsl:choose>
        </xsl:if>
      </fo:table-body>
    </fo:table>
    <fo:block font-size="10.0pt" text-align="center" font-family="sans-serif" font-weight="normal"
      border-top-style="solid" border-top-width="1pt" border-top-color="black" padding-after="2.5pt">
      <fo:block><xsl:text></xsl:text></fo:block>
    </fo:block>
  </xsl:template>

  <xsl:template name="titles">
    <fo:block font-size="13.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
      <xsl:text>HYPOTHETICAL ILLUSTRATION</xsl:text>
    </fo:block>
    <fo:block font-size="11.0pt" text-align="center" font-family="sans-serif" font-weight="bold">
      <xsl:text>Accumulation Phase</xsl:text>
    </fo:block>
    <fo:block font-size="10.0pt" text-align="center" font-family="sans-serif" font-weight="normal"
      border-top-style="solid" border-top-width="1pt" border-top-color="black" padding-after="2.5pt">
      <fo:block><xsl:text></xsl:text></fo:block>
    </fo:block>
  </xsl:template>

  <xsl:template name="column-headings">
    <fo:table-header padding-after="5.0pt">
      <fo:table-row>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>End of</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <fo:table-row>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Purchase</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Year</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Cumulative</xsl:text>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <fo:table-row>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Purchase</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Payment</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text></xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Asset</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Investment</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Contract</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Contract</xsl:text>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <fo:table-row>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Year</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Payment</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Charges</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Withdrawal(s)</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Charges</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Experience</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Value</xsl:text>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:text>Value IRR</xsl:text>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
    </fo:table-header>
  </xsl:template>

  <xsl:template name="column-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:param name="basis-lapse-year"/>
    <xsl:param name="basis"/>
    <xsl:if test="$counter &lt;= $basis-lapse-year">
      <fo:table-row>
        <fo:table-cell padding="0.2pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="$basis='Current'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetPmt_Current']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='CurrentZero'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetPmt_CurrentZero']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='Guaranteed'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetPmt_Guaranteed']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='GuaranteedZero'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetPmt_GuaranteedZero']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text></xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='NetWD']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="$basis='Current'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossIntCredited_Current']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetIntCredited_Current']/duration[$counter]/@column_value,$numberswc,$numberswoc)+translate(illustration/data/newcolumn/column[@name='SepAcctLoad_Current']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='CurrentZero'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossIntCredited_CurrentZero']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetIntCredited_CurrentZero']/duration[$counter]/@column_value,$numberswc,$numberswoc)+translate(illustration/data/newcolumn/column[@name='SepAcctLoad_CurrentZero']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='Guaranteed'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossIntCredited_Guaranteed']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetIntCredited_Guaranteed']/duration[$counter]/@column_value,$numberswc,$numberswoc)+translate(illustration/data/newcolumn/column[@name='SepAcctLoad_Guaranteed']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:when test="$basis='GuaranteedZero'">
                <xsl:value-of select="format-number(translate(illustration/data/newcolumn/column[@name='GrossIntCredited_GuaranteedZero']/duration[$counter]/@column_value,$numberswc,$numberswoc)-translate(illustration/data/newcolumn/column[@name='NetIntCredited_GuaranteedZero']/duration[$counter]/@column_value,$numberswc,$numberswoc)+translate(illustration/data/newcolumn/column[@name='SepAcctLoad_GuaranteedZero']/duration[$counter]/@column_value,$numberswc,$numberswoc),'###,###,###')"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text></xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="$basis='Current'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossIntCredited_Current']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='CurrentZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossIntCredited_CurrentZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='Guaranteed'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossIntCredited_Guaranteed']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='GuaranteedZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossIntCredited_GuaranteedZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text></xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="$basis='Current'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Current']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='CurrentZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_CurrentZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='Guaranteed'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Guaranteed']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='GuaranteedZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_GuaranteedZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text></xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="$basis='Current'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='IrrCsv_Current']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='CurrentZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='IrrCsv_CurrentZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='Guaranteed'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='IrrCsv_Guaranteed']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="$basis='GuaranteedZero'">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='IrrCsv_GuaranteedZero']/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text></xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="column-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
        <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
        <xsl:with-param name="basis" select="$basis"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="preliminary-footer">
    <xsl:param name="basis"/>
    <xsl:choose>
      <xsl:when test="$basis='Current'">
        <fo:block font-size="8.5pt" text-align="left" font-family="sans-serif" font-weight="normal">
          <xsl:text>Contract values are net of Current contract charges which are not guaranteed.</xsl:text>
          <xsl:text> See the Explanation and Footnotes for important Contract information.</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:when test="$basis='CurrentZero'">
        <fo:block font-size="8.5pt" text-align="left" font-family="sans-serif" font-weight="normal">
          <xsl:text>Contract values are net of Current contract charges which are not guaranteed.</xsl:text>
          <xsl:text> See the Explanation and Footnotes for important Contract information.</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:when test="$basis='Guaranteed'">
        <fo:block font-size="8.5pt" text-align="left" font-family="sans-serif" font-weight="normal">
          <xsl:text>Contract values are net of Guaranteed contract charges.</xsl:text>
          <xsl:text> See the Explanation and Footnotes for important Contract information.</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:when test="$basis='GuaranteedZero'">
        <fo:block font-size="8.5pt" text-align="left" font-family="sans-serif" font-weight="normal">
          <xsl:text>Contract values are net of Guaranteed contract charges.</xsl:text>
          <xsl:text> See the Explanation and Footnotes for important Contract information.</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:otherwise>
        <fo:block font-size="8.0pt" text-align="left" font-family="sans-serif" font-weight="normal">
          <xsl:text></xsl:text>
        </fo:block>
      </xsl:otherwise>
    </xsl:choose>
    <fo:block><fo:leader></fo:leader></fo:block>
  </xsl:template>

  <xsl:template name="standard-footer">
    <xsl:param name="displaypagenumber"/>
    <fo:block padding-before="5pt" font-size="9.0pt" font-family="sans-serif"
      border-top-style="solid" border-top-width="1pt" border-top-color="blue">
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block font-size="8.5pt" text-align="left" font-family="sans-serif" font-weight="normal">
        <xsl:text>Principal Underwriter: </xsl:text>
        <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
        <xsl:text>, </xsl:text>
        <xsl:value-of select="illustration/scalar/MainUnderwriterAddress"/>
        <xsl:text>. </xsl:text>
        <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
        <xsl:text> is a wholly owned subsidiary of </xsl:text>
        <xsl:value-of select="illustration/scalar/InsCoName"/>
        <xsl:text>.</xsl:text>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-body padding-before="2pt">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Date Prepared: </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepMonth"/>
                <xsl:text>&#xA0;</xsl:text>
                <xsl:value-of select="illustration/scalar/PrepDay"/>
                <xsl:text>, </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepYear"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="$displaypagenumber=1">
                  <fo:block text-align="center">
                    <xsl:text>Page: </xsl:text>
                    <fo:page-number/>
                    <xsl:text> of </xsl:text>
                    <fo:page-number-citation ref-id="endofdoc"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="right">
                    <xsl:text></xsl:text>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:if test="illustration/scalar/LmiVersion!=''">
                  <fo:block text-align="right">System Version:
                    <xsl:value-of select="illustration/scalar/LmiVersion"/>
                  </fo:block>
                </xsl:if>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="limit-string">
    <xsl:param name="pass-string"/>
    <xsl:param name="length"/>
    <xsl:choose>
      <xsl:when test="string-length($pass-string) &gt; $length">
        <xsl:value-of select="substring($pass-string,1, $length)"/>
        <xsl:text>...</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$pass-string"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="get-basis-lapse-year">
    <xsl:param name="basis"/>
    <xsl:choose>
      <xsl:when test="$basis='Current'">
        <xsl:value-of select="illustration/scalar/LapseYear_Current"/>
      </xsl:when>
      <xsl:when test="$basis='CurrentZero'">
        <xsl:value-of select="illustration/scalar/LapseYear_CurrentZero"/>
      </xsl:when>
      <xsl:when test="$basis='Guaranteed'">
        <xsl:value-of select="illustration/scalar/LapseYear_Guaranteed"/>
      </xsl:when>
      <xsl:when test="$basis='GuaranteedZero'">
        <xsl:value-of select="illustration/scalar/LapseYear_GuaranteedZero"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="number($max-years)"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
