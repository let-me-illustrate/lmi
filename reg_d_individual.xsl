<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014 Gregory W. Chicares.

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

    $Id$
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

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1.4in 0 1.2in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the IRR (Guaranteed Charges) Illustration page. -->
        <fo:simple-page-master master-name="irr-guaranteed-illustration">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the IRR (Current Charges) Illustration page -->
        <fo:simple-page-master master-name="irr-current-illustration">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the Current Values Illustration page -->
        <fo:simple-page-master master-name="current-illustration">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 1. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-1">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="1in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the Explanation and Footnotes page 2. -->
        <fo:simple-page-master master-name="explanation-and-footnotes-2">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="1in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the Certification Statements page. -->
        <fo:simple-page-master master-name="certification-statements">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="1.6in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="1in"/>
          <!-- Footer -->
          <fo:region-after extent=".75in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Illustration page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="2.6in 0 1.5in"/>
            <!-- Header -->
            <fo:region-before extent="3in"/>
            <!-- Footer -->
            <fo:region-after extent=".75in"/>
          </fo:simple-page-master>
        </xsl:if>
      </fo:layout-master-set>

      <!-- Cover Page -->
      <!-- Body page -->
      <fo:page-sequence master-reference="cover" force-page-count="no-force">

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="omit-disclaimer" select="1"/>
        </xsl:call-template>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center">
            <fo:block font-size="18pt" padding="1em 0 25pt">
              <xsl:value-of select="$scalars/PolicyMktgName"/>
            </fo:block>
            <fo:block font-size="12pt" padding-after="45pt">
              Hypothetical Life Insurance Illustration
            </fo:block>
            <fo:block padding-after="25pt">
              Prepared by: <xsl:value-of select="$scalars/InsCoShortName"/>
            </fo:block>
            <fo:block padding-after="25pt">
              Prepared for:
              <!--
              Properly adjust for long user input strings limit output
              to 140 characters for appox. 2 lines
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
            <fo:block padding-after="120.0pt">
              Date Prepared: <xsl:call-template name="date-prepared"/>
            </fo:block>
          </fo:block>
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block padding-after="10.0pt">
              The purpose of this illustration is to show how hypothetical rates
              of return will affect policy account value. These hypothetical
              returns do not reflect past performance and are not predictive
              of future results. Actual results could be less than or greater
              than the hypothetical rates and in all likelihood will vary from
              year to year.
            </fo:block>
            <fo:block font-weight="bold" padding-after="10.0pt">
              This material must be preceded or accompanied by the current
              Confidential Private Placement Memorandum for
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              and the current prospectuses and private placement memoranda
              for its underlying investment choices. Investors should
              carefully consider the investment objectives, risks,
              charges and expenses of the policy and its underlying investment
              choices. Please read the prospectuses and private placement
              memoranda carefully before investing or sending money.
            </fo:block>
            <fo:block padding-after="10.0pt">
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              is a flexible premium variable adjustable life insurance policy
              issued by <xsl:value-of select="$scalars/InsCoName"/>
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
            <fo:block padding-after="10.0pt">
              <xsl:value-of select="$scalars/InsCoShortName"/> Financial Group
              is a marketing designation
              for <xsl:value-of select="$scalars/InsCoName"/>
              (<xsl:value-of select="$scalars/InsCoShortName"/>)
              and its affiliate companies and sales representatives.
            </fo:block>
            <fo:block>
              Compliance tracking number:
              <xsl:value-of select="$compliance_tracking_number"/>
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- IRR (Guaranteed Charges) Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="irr-guaranteed-illustration">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standardheader">
            <xsl:with-param name="displaycontractlanguage" select="1"/>
            <xsl:with-param name="displaydisclaimer" select="1"/>
          </xsl:call-template>
          <fo:block text-align="center" font-size="9pt" font-family="serif" padding-top="1em">
            End of Year Policy Values using Guaranteed Charges
            <xsl:call-template name="dollar-units"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <xsl:call-template name="irr-guaranteed-illustration-report"/>

      </fo:page-sequence>

      <!-- IRR (Current Charges) Illustration page. -->
      <!-- Body page -->
      <fo:page-sequence master-reference="irr-current-illustration">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standardheader">
            <xsl:with-param name="displaycontractlanguage" select="1"/>
            <xsl:with-param name="displaydisclaimer" select="1"/>
          </xsl:call-template>
          <fo:block text-align="center" font-size="9pt" font-family="serif" padding-top="1em">
            End of Year Policy Values using Current Charges
            <xsl:call-template name="dollar-units"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <xsl:call-template name="irr-current-illustration-report"/>

      </fo:page-sequence>

      <!-- Current Values Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="current-illustration">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standardheader">
            <xsl:with-param name="displaycontractlanguage" select="1"/>
            <xsl:with-param name="displaydisclaimer" select="1"/>
          </xsl:call-template>
          <fo:block text-align="center" font-size="9pt" font-family="serif" padding-top="1em">
            End of Year Policy Values using Current Charges
            <xsl:call-template name="dollar-units"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <xsl:call-template name="current-illustration-report"/>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 1 -->
      <fo:page-sequence master-reference="explanation-and-footnotes-1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="plain-header"/>
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center">
            Explanatory Notes
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <fo:flow flow-name="xsl-region-body">

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10pt" font-weight="bold" text-decoration="underline">
              The Illustration
            </fo:block>
            <fo:block padding-top="1em">
              This illustration is not a policy or an offer or solicitation
              to purchase a policy. Offers are made only through the
              Confidential Private Placement Memorandum ("the PPM") for
              <xsl:value-of select="$scalars/PolicyMktgName"/>.
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/ProductDescription"/>.
            </fo:block>
          </fo:block>

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              The Policy
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Policy Requirements:</fo:inline>
              This policy is only available to persons who are deemed
              accredited investors and qualified purchasers under applicable
              federal securities laws. The policy owner must be able to bear
              the risk of loss of the entire investment in the policy. The
              policy owner must be familiar with and understand the fundamental
              risks and financial hazards of investing in the policy, and be
              willing to represent as such to
              <xsl:value-of select="$scalars/InsCoShortName"/>.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Maximum Net Amount at Risk Limitation: </fo:inline>
              <xsl:value-of select="$scalars/MaxNaarFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Monthly Charges:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/MonthlyChargesFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Surrender Charges:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/PremTaxSurrChgFootnote"/>
            </fo:block>
          </fo:block>
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10.0pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              Column Definitions
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Account Value:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/AccountValueFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Administrative Charge:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/PolicyFeeFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Asset Charges:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/AssetChargeFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Cash Surrender Value:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/CashSurrValueFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Death Benefit:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/DeathBenefitFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">End of Year Age:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/AttainedAgeFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Investment Income:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/InvestmentIncomeFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">IRR on Death Benefit:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/IrrDbFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">IRR on Surrender Value:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/IrrCsvFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Mortality Charges:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/MortalityChargesFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Policy Year:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/PolicyYearFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Premium Loads:</fo:inline>
              The applicable state premium tax, deferred acquisition cost tax
              charge, sales load (if any), separate account administrative
              charge (if any), and other charges incurred as a result of
              retaining an unaffiliated money manager (if any).
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">Premium Outlay:</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/OutlayFootnote"/>
            </fo:block>
          </fo:block>

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10.0pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              Separate Account
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/HypotheticalRatesFootnote"/>
            </fo:block>
          </fo:block>

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10.0pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              Withdrawals and Policy Loans
            </fo:block>
            <fo:block padding-top="1em">
              If applicable, withdrawals and policy loans will appear on a
              supplemental report at the end of the illustration. Withdrawals
              and policy loans are assumed to be taken at the beginning of the
              applicable policy year. The illustration and supplemental report
              reflect a fixed policy loan interest rate of <xsl:value-of select="$scalars/InitAnnLoanDueRate"/>.
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/LoanAndWithdrawalFootnote"/>
            </fo:block>
          </fo:block>
        </fo:flow>

      </fo:page-sequence>

      <!-- Explanation and Footnotes page 2 -->

      <fo:page-sequence master-reference="explanation-and-footnotes-2">

        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="plain-header"/>
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center">
            Explanatory Notes
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <fo:flow flow-name="xsl-region-body">

          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left">
            <fo:block font-size="10.0pt" font-weight="bold" padding-top="2em" text-decoration="underline">
              Important Tax Disclosures
            </fo:block>
            <fo:block>
              The definition of life insurance elected for this policy is
              <xsl:choose>
                <xsl:when test="$scalars/DefnLifeIns='GPT'">
                  the guideline premium test. The guideline single premium
                  is $<xsl:value-of select="$scalars/InitGSP"/>
                  and the guideline level premium
                  is $<xsl:value-of select="$scalars/InitGLP"/>.
                </xsl:when>
                <xsl:otherwise>
                  the cash value accumulation test.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              The initial 7-pay premium limit
              is $<xsl:value-of select="$scalars/InitSevenPayPrem"/>.
              As illustrated, this policy
              <xsl:choose>
                <xsl:when test="$scalars/IsMec='1'">
                  fails
                </xsl:when>
                <xsl:otherwise>
                  passes
                </xsl:otherwise>
              </xsl:choose>
              the seven-pay test defined in Section 7702A
              of the Internal Revenue Code and therefore
              <xsl:choose>
                <xsl:when test="$scalars/IsMec='1'">
                  becomes a Modified Endowment Contract (MEC)
                  in policy year <xsl:value-of select="$scalars/MecYear+1"/>.
                </xsl:when>
                <xsl:otherwise>
                  is not a Modified Endowment Contract (MEC). Subsequent changes
                  to the policy, including but not limited to increases
                  and decreases in premiums or benefits, may cause the policy
                  to be retested and may result in the policy becoming a MEC.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              If a policy is a MEC, withdrawals and policy loans are taxed to
              the extent of any gain in the policy, and an additional penalty
              tax may apply to the taxable portion of the distribution.
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              The information provided in this illustration is not written or
              intended as tax or legal advice and may not be relied on for
              purposes of avoiding any federal tax penalties.
              <xsl:value-of select="$scalars/InsCoShortName"/>, its employees
              and representatives are not authorized to give tax or legal advice.
              Individuals are encouraged to seek advice from their own tax or
              legal counsel.
            </fo:block>
          </fo:block>

          <!-- Certification Statements -->
          <fo:block font-weight="bold" font-size="11pt" font-family="sans-serif" text-align="center" padding-top="2em">
            Certification Statements
          </fo:block>
          <fo:block font-weight="normal" font-size="9pt" font-family="sans-serif" text-align="left" padding-top="1em">
            <fo:block padding-top="1em">
              POLICY OWNER / APPLICANT
            </fo:block>
            <fo:block padding-bottom="1em">
              I have received a copy of this illustration, and I understand
              that any non-guaranteed charges illustrated are subject
              to change and could be either higher or lower. Additionally,
              I have been informed by my agent that these values
              are not guaranteed.
            </fo:block>
            <fo:block text-decoration="overline" padding="2em">
              POLICY OWNER OR APPLICANT SIGNATURE &nbsp;&nbsp;&nbsp;
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

      <!-- Supplemental Illustration -->
      <!-- Body page -->
      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <xsl:call-template name="standardheader">
              <xsl:with-param name="displaycontractlanguage" select="1"/>
              <xsl:with-param name="displaydisclaimer" select="1"/>
            </xsl:call-template>
            <fo:block text-align="center" font-size="9pt" font-family="serif" padding-top="1em">
              <xsl:value-of select="illustration/supplementalreport/title"/>
              <xsl:call-template name="dollar-units"/>
            </fo:block>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter"/>

          <!-- Supplemental report body -->
          <xsl:call-template name="supplemental-report-body"/>
        </fo:page-sequence>
      </xsl:if>
    </fo:root>
  </xsl:template>

  <xsl:template name="standardheader">
    <xsl:param name="logo_only"/>
    <fo:table table-layout="fixed" padding-after="2.5pt" font-weight="bold" font-size="13.0pt" font-family="sans-serif">
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
    <xsl:param name="displaycontractlanguage"/>
    <xsl:param name="displaydisclaimer"/>
    <fo:block text-align="center" font-size="9pt">
      <xsl:if test="$displaycontractlanguage=1">
        <fo:block padding-top="1em">
          <xsl:choose>
            <xsl:when test="$scalars/IsInforce!='1'">
              Illustration for Flexible Premium Variable Adjustable
              Life Insurance Contract.
            </xsl:when>
            <xsl:otherwise>
              In Force Illustration for Flexible Premium Variable Adjustable
              Life Insurance Contract.
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </xsl:if>
      <xsl:if test="$displaydisclaimer=1">
        <fo:block padding-top="1em">
          The purpose of the Illustration is to show how the performance of the
          underlying separate account divisions could affect the policy's
          account values and death benefits. This Illustration is hypothetical
          and may not be used to project or predict investment results.
        </fo:block>
      </xsl:if>
    </fo:block>

    <fo:table table-layout="fixed" width="100%" padding-top="1em">
      <fo:table-column column-width="125mm"/>
      <fo:table-column column-width="proportional-column-width(1)"/>
      <fo:table-body>
        <fo:table-row>
          <fo:table-cell padding-right="2mm">
            <fo:block text-align="left" font-size="9pt">
              <fo:block>
                Date Prepared: <xsl:call-template name="date-prepared"/>
              </fo:block>
              <fo:block>
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    Composite of individuals
                  </xsl:when>
                  <xsl:otherwise>
                    Prepared for:
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="$scalars/Insured1"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Age: <xsl:value-of select="$scalars/Age"/>
                </fo:block>
              </xsl:if>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Issue State: <xsl:value-of select="$scalars/StatePostalAbbrev"/>
                </fo:block>
              </xsl:if>
              <fo:block>
                Selected Face Amount:
                $<xsl:value-of select="$scalars/InitTotalSA"/>
              </fo:block>
              <fo:block>
                Initial Death Benefit Option:
                <xsl:value-of select="$scalars/DBOptInitInteger+1"/>
              </fo:block>
            </fo:block>
          </fo:table-cell>

          <fo:table-cell>
            <fo:block text-align="left" font-size="9pt">
              <fo:block>
                Policy: <xsl:value-of select="$scalars/PolicyMktgName"/>
              </fo:block>
              <fo:block/>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Underwriting Type:
                  <xsl:choose>
                    <xsl:when test="$scalars/UWType='Medical'">
                      Fully underwritten
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$scalars/UWType"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </fo:block>
                <fo:block>
                  Rate Classification: <xsl:value-of select="$scalars/Gender"/>,
                  <xsl:value-of select="$scalars/Smoker"/>,
                  <xsl:value-of select="$scalars/UWClass"/>
                </fo:block>
                <xsl:if test="$scalars/UWClass='Rated'">
                  <fo:block padding-left="3em">
                    <fo:block>
                      Table Rating:
                      <xsl:value-of select="$scalars/SubstandardTable"/>
                    </fo:block>
                    <fo:block>
                      Initial Annual Flat Extra:
                      <xsl:value-of select="$vectors[@name='MonthlyFlatExtra']/duration[1]/@column_value"/>
                      per 1,000
                    </fo:block>
                  </fo:block>
                </xsl:if>
              </xsl:if>
            </fo:block>
          </fo:table-cell>
        </fo:table-row>
      </fo:table-body>
    </fo:table>

    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
      <xsl:call-template name="print-mastercontractnumber-and-contractnumber"/>
    </fo:block>
  </xsl:template>

  <xsl:template name="plain-header">
    <xsl:param name="logo_only"/>
    <fo:table table-layout="fixed" padding-after="2.5pt" font-weight="bold" font-size="13.0pt" font-family="sans-serif">
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
  </xsl:template>

  <xsl:template name="irr-guaranteed-illustration-report">
    <xsl:variable name="irr_guaranteed_illustration_columns_raw">
      <column name="PolicyYear">Policy _Year</column>
      <column composite="1"/>
      <column composite="0" name="AttainedAge">End of _Year Age</column>
      <column name="GrossPmt">Premium _Outlay</column>
      <column name="CSVNet_GuaranteedZero">Cash _Surr Value</column>
      <column name="EOYDeathBft_GuaranteedZero">Death _Benefit</column>
      <column name="IrrCsv_GuaranteedZero">IRR on _Surr Value</column>
      <column name="IrrDb_GuaranteedZero">IRR on _Death Bft</column>
      <column/>
      <column name="CSVNet_Guaranteed">Cash _Surr Value</column>
      <column name="EOYDeathBft_Guaranteed">Death _Benefit</column>
      <column name="IrrCsv_Guaranteed">IRR on _Surr Value</column>
      <column name="IrrDb_Guaranteed">IRR on _Death Bft</column>
    </xsl:variable>
    <xsl:variable name="columns_raw" select="document('')//xsl:variable[@name='irr_guaranteed_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <!-- Custom part of the table header -->
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_GuaranteedZero"/>
                  Hypothetical Rate of Return*
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Guaranteed"/>
                  Hypothetical Rate of Return*
                </fo:block>
              </fo:table-cell>
            </fo:table-row>

            <!-- Generic part of the table header -->
            <xsl:call-template name="generate-table-headers">
              <xsl:with-param name="columns" select="$columns"/>
            </xsl:call-template>
          </fo:table-header>

          <fo:table-body>
            <xsl:call-template name="generate-table-values">
              <xsl:with-param name="columns" select="$columns"/>
              <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="irr-current-illustration-report">
    <xsl:variable name="irr_current_illustration_columns_raw">
      <column name="PolicyYear">Policy _Year</column>
      <column composite="1"/>
      <column composite="0" name="AttainedAge">End of _Year Age</column>
      <column name="GrossPmt">Premium _Outlay</column>
      <column name="CSVNet_CurrentZero">Cash _Surr Value</column>
      <column name="EOYDeathBft_CurrentZero">Death _ Benefit</column>
      <column name="IrrCsv_CurrentZero">IRR on _Surr Value</column>
      <column name="IrrDb_CurrentZero">IRR on _Death Bft</column>
      <column/>
      <column name="CSVNet_Current">Cash _Surr Value</column>
      <column name="EOYDeathBft_Current">Death _Benefit</column>
      <column name="IrrCsv_Current">IRR on _Surr Value</column>
      <column name="IrrDb_Current">IRR on _Death Bft</column>
    </xsl:variable>
    <xsl:variable name="columns_raw" select="document('')//xsl:variable[@name='irr_current_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <!-- Custom part of the table header -->
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_CurrentZero"/>
                  Hypothetical Rate of Return*
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>
                  Hypothetical Rate of Return*
                </fo:block>
              </fo:table-cell>
            </fo:table-row>

            <!-- Generic part of the table header -->
            <xsl:call-template name="generate-table-headers">
              <xsl:with-param name="columns" select="$columns"/>
            </xsl:call-template>
          </fo:table-header>

          <fo:table-body>
            <xsl:call-template name="generate-table-values">
              <xsl:with-param name="columns" select="$columns"/>
              <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="current-illustration-report">
    <xsl:variable name="current_illustration_columns_raw">
      <column name="PolicyYear">Policy _Year</column>
      <column composite="1"/>
      <column composite="0" name="AttainedAge">End of _Year Age</column>
      <column name="GrossPmt">Premium _Outlay</column>
      <column special="Premium_Loads">Premium _Loads</column>
      <column special="Admin_Charges">Admin _Charges</column>
      <column name="COICharge_Current">Mortality _Charges</column>
      <column name="SepAcctCharges_Current">Asset _Charges</column>
      <column name="GrossIntCredited_Current">Investment _Income</column>
      <column name="AcctVal_Current">Account _Value</column>
      <column name="CSVNet_Current">Cash _Surr Value</column>
      <column name="EOYDeathBft_Current">Death_ Benefit</column>
    </xsl:variable>
    <xsl:variable name="columns_raw" select="document('')//xsl:variable[@name='current_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <!-- Custom part of the table header -->
            <fo:table-row>
              <fo:table-cell number-columns-spanned="7">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="7">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Guaranteed"/>
                  Hypothetical Rate of Return*
                </fo:block>
              </fo:table-cell>
            </fo:table-row>

            <!-- Generic part of the table header -->
            <xsl:call-template name="generate-table-headers">
              <xsl:with-param name="columns" select="$columns"/>
            </xsl:call-template>
          </fo:table-header>

          <fo:table-body>
            <xsl:call-template name="generate-table-values">
              <xsl:with-param name="columns" select="$columns"/>
              <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="get-special-column-value">
    <xsl:param name="special"/>
    <xsl:param name="column"/>
    <xsl:param name="counter"/>
    <xsl:param name="special-param"/>
    <xsl:choose>
      <xsl:when test="$special='Premium_Loads'">
        <xsl:variable name="GrossPmt" select="$vectors[@name='GrossPmt']/duration[$counter]/@column_value"/>
        <xsl:variable name="NetPmt_Current" select="$vectors[@name='NetPmt_Current']/duration[$counter]/@column_value"/>
        <xsl:value-of select="format-number(translate($GrossPmt,$numberswc,$numberswoc) - translate($NetPmt_Current,$numberswc,$numberswoc),'###,###,###')"/>
      </xsl:when>
      <xsl:when test="$special='Admin_Charges'">
        <xsl:variable name="SpecAmtLoad_Current" select="$vectors[@name='SpecAmtLoad_Current']/duration[$counter]/@column_value"/>
        <xsl:variable name="PolicyFee_Current" select="$vectors[@name='PolicyFee_Current']/duration[$counter]/@column_value"/>
        <xsl:value-of select="format-number(translate($SpecAmtLoad_Current,$numberswc,$numberswoc) + translate($PolicyFee_Current,$numberswc,$numberswoc),'###,###,###')"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="ERROR_Unknown_Special_Column_Name_Specified"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="standardfooter">
    <xsl:param name="omit-pagenumber" select="boolean(0)"/>
    <xsl:param name="omit-disclaimer" select="boolean(0)"/>
    <xsl:call-template name="generic-footer">
      <xsl:with-param name="top-block">
        <fo:block>
          This illustration is not complete unless all pages as noted
          below are included.
        </fo:block>
      </xsl:with-param>
      <xsl:with-param name="left-block">
        <fo:block><xsl:value-of select="$scalars/InsCoName"/></fo:block>
        <fo:block><xsl:value-of select="$scalars/InsCoAddr"/></fo:block>
      </xsl:with-param>
      <xsl:with-param name="center-block">
        <xsl:if test="$scalars/LmiVersion!=''">
          <fo:block>
            System Version:
            <xsl:value-of select="$scalars/LmiVersion"/>
          </fo:block>
        </xsl:if>
      </xsl:with-param>
      <xsl:with-param name="right-block">
        <fo:block>
          Policy Form:
          <xsl:value-of select="$scalars/PolicyForm"/>
        </fo:block>
        <fo:block>
          <xsl:choose>
            <xsl:when test="not($omit-pagenumber)">
              <xsl:call-template name="page-of"/>
            </xsl:when>
            <xsl:otherwise>
              Attachment
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

</xsl:stylesheet>
