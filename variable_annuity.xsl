<?xml version="1.0" encoding="UTF-8"?>
<!--
    Annuity illustrations.

    Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017 Gregory W. Chicares.

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
    email: <gchicares@sbcglobal.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
-->
<!DOCTYPE stylesheet [
<!ENTITY nbsp "&#xA0;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
  <xsl:import href="fo_common.xsl"/>
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the Cover page. -->
        <fo:simple-page-master master-name="cover-page">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1.4in 0 1.2in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration Current page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-current">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.7in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="2.7in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration CurrentZero page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-current-zero">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.7in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="2.7in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration Guaranteed page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-guaranteed">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.7in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="2.7in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Hypothetical Illustration GuaranteedZero page. -->
        <fo:simple-page-master master-name="hypothetical-illustration-guaranteed-zero">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.7in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="2.7in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 1. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-1">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1.5in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="1.4in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 2. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-2">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1.5in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="1.4in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Certification Statements page. -->
        <fo:simple-page-master master-name="certification-statements">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.0in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="2.0in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="2.7in 0 1.2in"/>
            <!-- Header -->
            <fo:region-before extent="2.7in"/>
            <!-- Footer -->
            <fo:region-after extent="1.1in"/>
          </fo:simple-page-master>
        </xsl:if>

      </fo:layout-master-set>

      <!-- Cover Page -->

      <fo:page-sequence master-reference="cover-page">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center">
            <fo:block font-size="18pt" padding="1em 0 25pt">
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>
            </fo:block>
            <fo:block font-size="12pt" padding-after="45pt">
              Hypothetical Annuity Illustration - Accumulation Phase
            </fo:block>
            <fo:block padding-after="25pt">
              Prepared by: <xsl:value-of select="$scalars/InsCoShortName"/>
            </fo:block>
            <fo:block padding-after="25pt">
              Prepared for:
              <!--
              Properly adjust for long user input strings: limit output
              to 140 characters for approximately two lines.
              -->
              <xsl:call-template name="limitstring">
                <xsl:with-param name="length" select="140"/>
                <xsl:with-param name="passString">
                  <xsl:choose>
                    <xsl:when test="not($is_composite)">
                      <xsl:value-of select="$scalars/Insured1"/>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$scalars/CorpName"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
              </xsl:call-template>
            </fo:block>
            <fo:block padding-after="150.0pt">
              Date Prepared: <xsl:call-template name="date-prepared"/>
            </fo:block>
          </fo:block>
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block padding-after="10.0pt">
              The purpose of this illustration is to show how hypothetical rates
              of return will affect Contract Value. These hypothetical returns
              do not reflect past performance and are not predictive
              of future results. Actual results could be less than or greater
              than the hypothetical rates and in all likelihood will vary
              from year to year.
            </fo:block>
            <fo:block font-weight="bold" padding-after="10.0pt">
              This material must be preceded or accompanied by the current
              Confidential Private Placement Memorandum for
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>
              and the current prospectuses and private placement memoranda
              for its underlying investment choices. Investors should
              carefully consider the investment objectives, risks,
              charges and expenses of the variable annuity Contract and its
              underlying investment choices. Please read the prospectuses
              and private placement memoranda carefully before investing
              or sending money.
            </fo:block>
            <fo:block padding-after="10.0pt">
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>
              is an individual variable deferred annuity contract with flexible
              purchase payments issued by <xsl:value-of select="$scalars/InsCoName"/>
              (<xsl:value-of select="$scalars/InsCoShortName"/>),
              <xsl:value-of select="$scalars/InsCoStreet"/>.
            </fo:block>
            <fo:block padding-after="10.0pt">
              Placement Agents:
              <xsl:value-of select="$scalars/CoUnderwriter"/> serves as the
              placement agent for contracts sold by its registered
              representatives.
              &nbsp;<xsl:value-of select="$scalars/MainUnderwriter"/>
              serves as the placement agent for contracts sold by registered
              representatives of other broker-dealers that have entered into
              distribution agreements with
              &nbsp;<xsl:value-of select="$scalars/MainUnderwriter"/>.
            </fo:block>
            <fo:block padding-after="10.0pt">
              <xsl:value-of select="$scalars/CoUnderwriter"/> and
              &nbsp;<xsl:value-of select="$scalars/MainUnderwriter"/> are
              subsidiaries of <xsl:value-of select="$scalars/InsCoName"/>
              (<xsl:value-of select="$scalars/InsCoShortName"/>) and are
              located at <xsl:value-of select="$scalars/InsCoStreet"/>.
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/InsCoShortName"/> Financial Group
              is a marketing designation
              for <xsl:value-of select="$scalars/InsCoName"/>
              (<xsl:value-of select="$scalars/InsCoShortName"/>)
              and its subsidiaries.
            </fo:block>
          </fo:block>
        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration Current -->

      <fo:page-sequence master-reference="hypothetical-illustration-current">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'Current'"/>
          </xsl:call-template>
          <xsl:call-template name="titles">
            <xsl:with-param name="basis" select="'Current'"/>
          </xsl:call-template>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="basis" select="'Current'"/>
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
          <xsl:call-template name="illustration-table">
            <xsl:with-param name="basis" select="'Current'"/>
            <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
          </xsl:call-template>
        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration CurrentZero -->

      <fo:page-sequence master-reference="hypothetical-illustration-current-zero">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
          </xsl:call-template>
          <xsl:call-template name="titles">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
          </xsl:call-template>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
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
          <xsl:call-template name="illustration-table">
            <xsl:with-param name="basis" select="'CurrentZero'"/>
            <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
          </xsl:call-template>
        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration Guaranteed -->

      <fo:page-sequence master-reference="hypothetical-illustration-guaranteed">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
          </xsl:call-template>
          <xsl:call-template name="titles">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
          </xsl:call-template>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
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
          <xsl:call-template name="illustration-table">
            <xsl:with-param name="basis" select="'Guaranteed'"/>
            <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
          </xsl:call-template>
        </fo:flow>

      </fo:page-sequence>

      <!-- Hypothetical Illustration GuaranteedZero -->

      <fo:page-sequence master-reference="hypothetical-illustration-guaranteed-zero">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <xsl:call-template name="scalar-header">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
          </xsl:call-template>
          <xsl:call-template name="titles">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
          </xsl:call-template>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
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
          <xsl:call-template name="illustration-table">
            <xsl:with-param name="basis" select="'GuaranteedZero'"/>
            <xsl:with-param name="basis-lapse-year" select="$basis-lapse-year"/>
          </xsl:call-template>
        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 1 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-1">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center" padding-top=".8em">
            Explanation and Footnotes
          </fo:block>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10pt" font-weight="bold" text-decoration="underline">
              The Illustration
            </fo:block>
            <fo:block padding-top="1em">
              The hypothetical Net Rate of Return is the rate of return net
              of investment management, custody and other expenses charged
              by the investment manager of the underlying fund(s).
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              The information provided in this illustration is not written
              or intended as tax or legal advice and may not be relied on
              for purposes of avoiding any Federal tax penalties.
              <xsl:value-of select="$scalars/InsCoShortName"/>, its employees
              and representatives are not authorized to give tax
              or legal advice. Individuals are encouraged to seek advice
              from their own tax or legal counsel.
            </fo:block>
            <fo:block padding-top="1em">
              This illustration is not a Contract or an offer or solicitation
              to enter into a Contract. Offers are made only through
              the Confidential Private Placement Memorandum for
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>.
            </fo:block>
          </fo:block>

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              The Contract
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Contract Requirements:</fo:inline>
              This Contract is only available to persons who are deemed
              accredited investors and qualified purchasers under applicable
              federal securities laws. The minimum initial premium is $500,000.
              You must be able to bear the risk of loss of your entire investment
              in the Contract. You will be required to represent to
              <xsl:value-of select="$scalars/InsCoShortName"/> that you are
              familiar with and understand the fundamental risks and financial
              hazards of investing in the Contract.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Purchase Payment Charges:</fo:inline>
              The sum of the negotiated Purchase Payment Distribution Charge
              and Purchase Payment Expense Charge plus state premium tax,
              if applicable.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Monthly Charges:</fo:inline>
              Monthly Charges will be deducted from the Contract Value.
              The Company may assess Monthly Charges of three (3) types:
              Mortality and Expense Risk Charges, Asset Based
              Distribution Charges and Administrative Charges. The Monthly
              Charges will be taken from the Money Market Sub-Account.
            </fo:block>
            <fo:block padding-top="1em">
              Due to the limitations on withdrawals from investment options
              offered under the Contract, we reserve the right to (i) allocate
              a portion of each purchase payment to the Money Market Sub-Account
              and (ii) transfer Contract Value to the Money Market Sub-Account
              from time to time to ensure that monthly charges can be deducted
              from the Money Market Sub-Account when due. The illustrated
              values do not reflect any amounts that may be allocated
              to meet the minimum Money Market requirement.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Mortality and Expense Charge ("M&amp;E"):</fo:inline>
              The current and guaranteed maximum charges are described
              in the Table of Charges of the Confidential Private Placement
              Memorandum for <xsl:value-of select="$scalars/PolicyMktgName"/>
              <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Administrative Charge:</fo:inline>
              <xsl:value-of select="$scalars/InterestDisclaimer"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Asset Based Distribution Charge:</fo:inline>
              The current and guaranteed charges are equal to annual negotiated
              asset based compensation, if any.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Withdrawals:</fo:inline>
              Withdrawals are taxed to the extent of any gain in the contract.
              In some cases, an additional 10% federal income tax penalty
              will apply to the taxable portion of the withdrawal.
              This illustration does not reflect the tax consequences
              of withdrawals.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Illiquid Sub-Accounts:</fo:inline>
              We have also made available under the Contract the ability
              to allocate purchase payments and Contract Value to sub-accounts
              that will provide for investment in certain alternative
              investments including hedge funds. Because of the liquidity
              restrictions imposed by these alternative investment funds,
              these sub-accounts are referred to in the Contract
              and in the private placement memorandum as illiquid sub-accounts.
              Contract owners must understand, and be willing to bear, the risks
              associated with such investments. If Contract Value is allocated
              to illiquid sub-accounts, the Owner will not be able to transfer
              Contract Value or effect a withdrawal or surrender
              of the Contract until specified dates. Refer to the Confidential
              Private Placement Memorandum for details.
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              Note: Please refer to the Contract for complete explanation
              of benefits, rights and obligations. In the event of a conflict
              between the illustration and the Contract, the terms
              of the Contract will control.
            </fo:block>
          </fo:block>
        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 2 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-2">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standard-header">
            <xsl:with-param name="logo_only" select="0"/>
          </xsl:call-template>
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center" padding-top=".8em">
            Explanation and Footnotes
          </fo:block>
        </fo:static-content>

        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standard-footer">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10.0pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              Column Definitions
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Year:</fo:inline>
              The annual period beginning on the hypothetical Contract
              issue date.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Purchase Payment:</fo:inline>
              A payment made by or on behalf of a Contract Owner during
              the Accumulation Period of the Contract.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Purchase Payment Charges:</fo:inline>
              The sum of the negotiated Purchase Payment Distribution Charge
              and Purchase Payment Expense Charge, if any, and state premium tax
              charged on an upfront basis, if any.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Withdrawal(s):</fo:inline>
              The dollar amount redeemed from the Contract. Withdrawals
              are reflected at the beginning of each year.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Monthly Charges:</fo:inline>
              The annual M&amp;E charge and administrative charge, plus
              any asset based distribution charge, which is subject
              to negotiation prior to contract issuance, deducted from
              Contract Value.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Investment Experience:</fo:inline>
              The annual investment income credited to the Contract
              sub-accounts using the hypothetical Net Rate of Return.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">End of Year Contract Value:</fo:inline>
              The value of the Contract sub-account(s) after applying
              any Purchase Payments (less Purchase Payment Charges),
              deducting any Withdrawals and crediting annual
              hypothetical Investment Experience.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Cumulative Contract Value IRR:</fo:inline>
              The Contract Value Internal Rate of Return ("IRR")
              is an interest rate at which the Net Purchase Payment demonstrated
              would have to be invested outside the Contract to generate
              the Contract Value. The IRR is illustrative only and does
              not predict future results.
            </fo:block>
          </fo:block>

          <!-- Certification Statements -->

          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center" padding-top="2em">
            Certification Statements
          </fo:block>
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left" padding-top="1em">
            <fo:block padding-top="1em">
              CONTRACT OWNER / APPLICANT
            </fo:block>
            <fo:block padding-bottom="1em">
              I have received a copy of this illustration, and I understand
              that any non-guaranteed charges illustrated are subject
              to change and could be either higher or lower. Additionally,
              I have been informed by my agent that these values
              are not guaranteed.
            </fo:block>
            <fo:block text-decoration="overline" padding="2em">
              CONTRACT OWNER OR APPLICANT SIGNATURE &nbsp;&nbsp;&nbsp;
              <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>DATE
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
            </fo:block>
            <fo:block padding-top="2em">
              AGENT / AUTHORIZED REPRESENTATIVE
            </fo:block>
            <fo:block padding-bottom="1em">
              I certify that this illustration has been presented
              to the applicant, and that I have explained that any
              non-guaranteed charges illustrated are subject to change. I have
              made no statements that are inconsistent with the illustration.
            </fo:block>
            <fo:block text-decoration="overline" padding="2em">
              AGENT OR AUTHORIZED REPRESENTATIVE
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
              <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>DATE
              &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
            </fo:block>
          </fo:block>
          <xsl:if test="not($has_supplemental_report)">
            <fo:block id="endofdoc"/>
          </xsl:if>
        </fo:flow>

      </fo:page-sequence>

      <!-- Supplemental Report -->

      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <xsl:call-template name="standard-header">
              <xsl:with-param name="logo_only" select="0"/>
            </xsl:call-template>
            <xsl:call-template name="scalar-header">
              <xsl:with-param name="basis" select="NoBasis"/>
            </xsl:call-template>
            <fo:block text-align="center" border-bottom="1pt solid black">
              <fo:block padding="2.5pt 0" font-weight="bold" font-size="13pt" font-family="sans-serif">
                SUPPLEMENTAL REPORT
              </fo:block>
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
          <xsl:call-template name="supplemental-report-body"/>
        </fo:page-sequence>
      </xsl:if>

    </fo:root>
  </xsl:template>

  <xsl:template name="standard-header">
    <xsl:param name="logo_only"/>
    <fo:block padding-after="2.5pt">
      <fo:table table-layout="fixed" width="100%" font-weight="bold" font-size="13.0pt" font-family="sans-serif">
        <fo:table-column column-width="50mm"/>
        <fo:table-column column-width="90mm"/>
        <fo:table-column column-width="50mm"/>
        <fo:table-body>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:call-template name="company-logo"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block/>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block/>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
    <xsl:if test="not($logo_only)">
      <fo:block font-weight="bold" font-size="14pt" font-family="sans-serif" text-align="center" padding-top="1em">
        <xsl:value-of select="$scalars/PolicyMktgName"/>
        <fo:inline vertical-align="super" font-size="7pt">SM</fo:inline>
      </fo:block>
      <fo:block font-weight="normal" font-size="11pt" font-family="sans-serif" text-align="center" border-bottom="1pt solid black" padding-bottom="8.0pt">
        <xsl:value-of select="$scalars/PolicyLegalName"/>
      </fo:block>
    </xsl:if>
  </xsl:template>

  <xsl:template name="scalar-header">
    <xsl:param name="basis"/>
    <fo:table table-layout="fixed" width="100%" padding="2.5pt 0" font-weight="normal" font-size="9.5pt" font-family="sans-serif" border-bottom="1pt solid black">
      <fo:table-column column-width="135mm"/>
      <fo:table-column column-width="55mm"/>
      <fo:table-body>
        <fo:table-row>
          <fo:table-cell padding-right="10mm">
            <fo:block text-align="left">
              Prepared for:
              <!--
              Properly adjust for long user input strings: limit output
              to 140 characters for approximately two lines.
              -->
              <xsl:call-template name="limitstring">
                <xsl:with-param name="length" select="140"/>
                <xsl:with-param name="passString">
                  <xsl:choose>
                    <xsl:when test="not($is_composite)">
                      <xsl:value-of select="$scalars/Insured1"/>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$scalars/CorpName"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
              </xsl:call-template>
            </fo:block>
            <xsl:if test="not($is_composite)">
              <fo:block text-align="left">
                Annuitant Age at Issue: <xsl:value-of select="$scalars/Age"/>
              </fo:block>
            </xsl:if>
            <xsl:call-template name="print-mastercontractnumber-and-contractnumber"/>
          </fo:table-cell>
          <fo:table-cell text-align="left">
            <fo:block>
              Issue State: <xsl:value-of select="$scalars/StatePostalAbbrev"/>
            </fo:block>
            <xsl:choose>
              <xsl:when test="($basis='Current') or ($basis='CurrentZero') or ($basis='Guaranteed') or ($basis='GuaranteedZero')">
                <fo:block>
                  Net Rate of Return:
                  <xsl:value-of select="$scalars/*[name()=concat('InitAnnSepAcctNetInt_', $basis)]"/>
                </fo:block>
              </xsl:when>
              <xsl:when test="$basis='NoBasis'"/>
            </xsl:choose>
          </fo:table-cell>
        </fo:table-row>
      </fo:table-body>
    </fo:table>
  </xsl:template>

  <xsl:template name="titles">
    <xsl:param name="basis"/>
    <fo:block font-weight="bold" font-size="13pt" font-family="sans-serif" text-align="center" padding-top=".5em">
      HYPOTHETICAL ILLUSTRATION
    </fo:block>
    <fo:block font-weight="bold" font-size="11.0pt" font-family="sans-serif" text-align="center" border-bottom="1pt solid black" padding-after="2.5pt">
      Accumulation Phase -
      <xsl:choose>
        <xsl:when test="($basis='Current') or ($basis='CurrentZero')">
          Current Charges
        </xsl:when>
        <xsl:when test="($basis='Guaranteed') or ($basis='GuaranteedZero')">
          Guaranteed Charges
        </xsl:when>
        <xsl:when test="$basis='NoBasis'"/>
      </xsl:choose>
    </fo:block>
  </xsl:template>

  <xsl:template name="illustration-table">
    <xsl:param name="basis"/>
    <xsl:param name="basis-lapse-year"/>
    <fo:table table-layout="fixed" width="100%" font-weight="normal" font-size="9pt" font-family="serif">
      <xsl:variable name="column-values-columns">
        <column name="PolicyYear">Year</column>
        <column name="GrossPmt">Purchase _Payment</column>
        <column special="SpecialPmt">Purchase _Payment _Charges</column>
        <column name="NetWD">Withdrawal(s)</column>
        <column name="SepAcctCharges_Current">Monthly _Charges</column>
        <column special="SpecialGrossIntCredited">Investment _Experience</column>
        <column special="SpecialAcctVal">End of Year _Contract Value</column>
        <column special="SpecialIrrCsv">Cumulative _Contract Value _IRR</column>
      </xsl:variable>
      <xsl:variable name="columns" select="document('')//xsl:variable[@name='column-values-columns']/column"/>

      <xsl:call-template name="generate-table-columns">
        <xsl:with-param name="columns" select="$columns"/>
      </xsl:call-template>

      <fo:table-header>
        <xsl:call-template name="generate-table-headers">
          <xsl:with-param name="columns" select="$columns"/>
        </xsl:call-template>
      </fo:table-header>

      <fo:table-body>
        <xsl:call-template name="generate-table-values">
          <xsl:with-param name="columns" select="$columns"/>
          <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
          <xsl:with-param name="max-counter" select="$basis-lapse-year"/>
          <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
          <xsl:with-param name="special-param" select="$basis"/>
        </xsl:call-template>
      </fo:table-body>
    </fo:table>
  </xsl:template>

  <xsl:template name="get-special-column-value">
    <xsl:param name="special"/>
    <xsl:param name="column"/>
    <xsl:param name="counter"/>
    <xsl:param name="special-param"/>
    <xsl:variable name="basis" select="$special-param"/>
    <xsl:variable name="name_suffix" select="concat('_', $basis)"/>
    <xsl:choose>
      <xsl:when test="$special='SpecialPmt'">
        <xsl:variable name="GrossPmt" select="$vectors[@name='GrossPmt']/duration[$counter]/@column_value"/>
        <xsl:variable name="NetPmt" select="$vectors[@name=concat('NetPmt', $name_suffix)]/duration[$counter]/@column_value"/>
        <xsl:value-of select="format-number(translate($GrossPmt,$numberswc,$numberswoc) - translate($NetPmt,$numberswc,$numberswoc), '###,###,###')"/>
      </xsl:when>
      <xsl:when test="$special='SpecialGrossIntCredited'">
        <xsl:value-of select="$vectors[@name=concat('GrossIntCredited', $name_suffix)]/duration[$counter]/@column_value"/>
      </xsl:when>
      <xsl:when test="$special='SpecialAcctVal'">
        <xsl:value-of select="$vectors[@name=concat('AcctVal', $name_suffix)]/duration[$counter]/@column_value"/>
      </xsl:when>
      <xsl:when test="$special='SpecialIrrCsv'">
        <xsl:value-of select="$vectors[@name=concat('IrrCsv', $name_suffix)]/duration[$counter]/@column_value"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="ERROR_Unknown_Special_Column_Name_Specified"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="standard-footer">
    <xsl:param name="basis"/>
    <xsl:param name="displaypagenumber"/>
    <fo:block font-size="8.5pt" font-family="sans-serif" text-align="left">
    <fo:block padding-after="1em">
      <xsl:choose>
        <xsl:when test="($basis='Current') or ($basis='CurrentZero')">
          Contract values are net of CURRENT monthly charges which
          are not guaranteed. See the Explanation and Footnotes
          for important Contract information.
        </xsl:when>
        <xsl:when test="($basis='Guaranteed') or ($basis='GuaranteedZero')">
          Contract values are net of GUARANTEED monthly charges.
          See the Explanation and Footnotes for important Contract information.
        </xsl:when>
        <xsl:otherwise>
          <fo:block padding=".5em"/>
        </xsl:otherwise>
      </xsl:choose>
    </fo:block>
    <fo:block padding="1em 0 .5em" border-top="1pt solid blue">
      This illustration is not complete unless all pages as noted below
      are included.
    </fo:block>
    <fo:table table-layout="fixed" width="100%" font-size="9pt">
      <fo:table-column column-width="proportional-column-width(1)"/>
      <fo:table-column column-width="proportional-column-width(1)"/>
      <fo:table-column column-width="proportional-column-width(1)"/>
      <fo:table-body>
        <fo:table-row>
          <fo:table-cell text-align="left" display-align="after">
            <fo:block>
              Date Prepared: <xsl:call-template name="date-prepared"/>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell text-align="center" display-align="after">
            <fo:block>
              <xsl:if test="$displaypagenumber=1">
                <xsl:call-template name="page-of"/>
              </xsl:if>
            </fo:block>
          </fo:table-cell>
          <fo:table-cell text-align="right" display-align="after">
            <fo:block>
              <xsl:if test="$compliance_tracking_number">
                <fo:block>
                  <xsl:value-of select="$compliance_tracking_number"/>
                </fo:block>
              </xsl:if>
              <xsl:if test="$scalars/LmiVersion!=''">
                <fo:block>
                  System Version: <xsl:value-of select="$scalars/LmiVersion"/>
                </fo:block>
              </xsl:if>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
      </fo:table-body>
    </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="get-basis-lapse-year">
    <xsl:param name="basis"/>
    <xsl:choose>
      <xsl:when test="$basis='Current' or $basis='CurrentZero' or $basis='Guaranteed' or $basis='GuaranteedZero'">
        <xsl:value-of select="$scalars/*[name()=concat('LapseYear_', $basis)]/text()"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="ERROR_Unknown_basis_value"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
