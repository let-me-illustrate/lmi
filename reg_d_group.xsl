<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Gregory W. Chicares.

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
          <fo:region-body margin=".1in .1in 0in"/>
        </fo:simple-page-master>

        <!-- Define the Column Definitions page. -->
        <fo:simple-page-master master-name="column-definitions">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.25in 0 1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.5in 0 1.25in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary-separate-account">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.25in 0 3in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the Basic Illustration page. -->
        <fo:simple-page-master master-name="basic-illustration">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3in 0 1.25in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Illustration page. -->
        <fo:simple-page-master master-name="supplemental-illustration">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3in 0 1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".9in"/>
        </fo:simple-page-master>

        <!-- Define the Illustration Assumption Detail page. -->
        <fo:simple-page-master master-name="illustration-assumption">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.8in 0 1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".9in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="2.8in 0 1.1in"/>
            <!-- Header -->
            <fo:region-before extent="3in"/>
            <!-- Footer -->
            <fo:region-after extent="1in"/>
          </fo:simple-page-master>
        </xsl:if>

      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->
      <xsl:call-template name="generic-cover"/>

      <!-- Basic Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="basic-illustration" initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
        <fo:block text-align="left">
          <xsl:call-template name="company-logo"/>
        </fo:block>
        <xsl:call-template name="standardheader">
          <xsl:with-param name="reporttitle">
            <xsl:choose>
              <xsl:when test="$scalars/IsInforce!='1'">
                Basic Illustration
              </xsl:when>
              <xsl:otherwise>
                In Force Basic Illustration
              </xsl:otherwise>
            </xsl:choose>
          </xsl:with-param>
        </xsl:call-template>
        <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <xsl:call-template name="basic-illustration-report"/>

      </fo:page-sequence>

      <!-- Supplemental Illustration -->
      <!-- narrative summary - begins here -->
      <fo:page-sequence master-reference="column-definitions">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- Column Definitions Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="10pt" font-family="sans-serif">
            <fo:block font-weight="bold" text-align="center" padding-bottom="2em">
              Column Definitions
            </fo:block>

            <fo:block font-weight="bold" padding-top="2em">
              Policy Year
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/PolicyYearFootnote"/>
            </fo:block>

            <xsl:if test="not($is_composite)">
              <fo:block font-weight="bold" padding-top="2em">
                End of Year Age
              </fo:block>
              <fo:block>
              <xsl:value-of select="$scalars/AttainedAgeFootnote"/>
              </fo:block>
            </xsl:if>

            <fo:block font-weight="bold" padding-top="2em">
              Premium Outlay
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/OutlayFootnote"/>
            </fo:block>

            <fo:block font-weight="bold" padding-top="2em">
              Cash Surrender Value
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/CashSurrValueFootnote"/>
            </fo:block>

            <fo:block font-weight="bold" padding-top="2em">
              Death Benefit
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/DeathBenefitFootnote"/>
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- narrative summary - begins here -->
      <fo:page-sequence master-reference="narrative-summary">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader">
            <xsl:with-param name="reporttitle" select="'Narrative Summary'"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- narrative summary Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="10pt" font-family="sans-serif">
            <fo:block>
              <xsl:value-of select="$scalars/PolicyMktgName"/>
              is a <xsl:value-of select="$scalars/PolicyLegalName"/>
              issued by <xsl:value-of select="$scalars/InsCoName"/>.
              <xsl:value-of select="$scalars/ProductDescription"/>
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline text-decoration="underline">Stable Value Feature</fo:inline>
              &nbsp;<xsl:value-of select="$scalars/StableValueFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              This illustration assumes Initial Death Benefit Option
              <xsl:value-of select="$scalars/DBOptInitInteger+1"/>.
            </fo:block>
            <fo:block padding-top="1em">
              This policy is only available to entities that meet the definition
              of "accredited investor" or "qualified purchaser" under applicable
              Federal securities laws.  Purchase of this policy is suitable
              only for entities of substantial economic means. Each prospective
              purchaser will be required to represent that it is familiar
              with and understands the fundamental risks and financial hazards
              of purchasing the policy. Each prospective purchaser must
              also represent that it meets minimum financial and other
              suitability standards.
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/NoVanishPremiumFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              The definition of life insurance elected for this contract
              is the
              <xsl:choose>
                <xsl:when test="$scalars/DefnLifeIns='GPT'">
                  guideline premium test. The guideline single premium
                  is $<xsl:value-of select="$scalars/InitGSP"/>
                  and the guideline level premium
                  is $<xsl:value-of select="$scalars/InitGLP"/>.
                </xsl:when>
                <xsl:otherwise>
                  cash value accumulation test.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/InsCoName"/>
              <xsl:value-of select="$scalars/RejectPremiumFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              Account values, cash surrender values, and death benefits
              are illustrated as of the end of the year.
            </fo:block>
            <fo:block padding-top="1em">
              PLEASE READ THE FOLLOWING IMPORTANT TAX DISCLOSURE
            </fo:block>
            <fo:block>
              The initial 7-pay premium limit is
              $<xsl:value-of select="$scalars/InitSevenPayPrem"/>.
              As illustrated, this contract
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
                  <xsl:choose>
                    <xsl:when test="$scalars/MecYear!='0'">
                      becomes a Modified Endowment Contract (MEC) in policy year
                      <xsl:value-of select="$scalars/MecYear+1"/>.
                    </xsl:when>
                    <xsl:otherwise>
                      is a Modified Endowment Contract (MEC).
                    </xsl:otherwise>
                  </xsl:choose>
                  As a MEC, any loans or withdrawals are taxed to the extent
                  of any gain in the contract, and an additional 10% penalty
                  tax will apply to the taxable portion of the loan
                  or withdrawal.
                </xsl:when>
                <xsl:otherwise>
                  is not a Modified Endowment Contract (MEC).
                  Subsequent changes in the contract, including but not limited
                  to increases and decreases in premiums or benefits, may cause
                  the contract to be retested and may result in the contract
                  becoming a MEC.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              This illustration is not written or intended as tax
              or legal advice and may not be relied on for purposes
              of avoiding any federal tax penalties. For more information
              pertaining to the tax consequences of purchasing
              or owning this policy, consult with your own independent tax
              or legal counsel.
            </fo:block>

            <!-- Force new page -->
            <fo:block break-after="page"/>

              <fo:block padding-top="1em">
                <xsl:value-of select="$scalars/GuarMortalityFootnote"/>
              </fo:block>
              <fo:block padding-top="1em">
                The loan interest rate may be fixed or adjustable as elected
                by the policy owner.
              </fo:block>
              <fo:block padding-top="1em">
                <xsl:choose>
                  <xsl:when test="$scalars/UseExperienceRating='1'">
                  <xsl:value-of select="$scalars/ExpRatingFootnote"/>
                  </xsl:when>
                  <xsl:otherwise>
                    The illustration does not assume any mortality
                    experience rating.
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
            </fo:block>
            <fo:block padding-top="1em" font-size="10pt">
              <fo:block font-weight="bold">
              This illustration may only be used when preceded or accompanied
              by the offering memorandum for
              <xsl:value-of select="$scalars/PolicyLegalName"/>
              (<xsl:value-of select="$scalars/PolicyMktgName"/>)
              insurance contract and its underlying investment choices.
              For a complete listing of the underlying investment choices,
              please refer to the offering memorandum. Before purchasing
              a variable life insurance contract, investors should
              carefully consider the investment objectives, risks, charges
              and expenses of the variable life insurance contract
              and its underlying investment choices. Please read
              the offering memorandum carefully before investing
              or sending money.
            </fo:block>

            <xsl:if test="$scalars/HasTerm='1'">
              <fo:block padding-top="1em">
                <xsl:value-of select="$scalars/TermFootnote"/>
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasWP='1'">
              <fo:block padding-top="1em">
                <xsl:value-of select="$scalars/WaiverFootnote"/>
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/UsePartialMort='1'">
              <fo:block padding-top="1em">
                This illustration reflects an initial mortality assumption
                of <xsl:value-of select="$vectors[@name='PartMortTableMult']/duration[1]/@column_value"/>
                of the <xsl:value-of select="$scalars/PartMortTableName"/> table
                with all deaths at the end of the year.
              </fo:block>
            </xsl:if>
            <xsl:choose>
              <xsl:when test="$scalars/GenderBlended='1'">
                <fo:block padding-top="1em">
                  <xsl:value-of select="$scalars/MortalityBlendFootnote"/>
                  Custom blend assumptions:
                  <xsl:choose>
                    <xsl:when test="$scalars/SmokerBlended='1'">
                      tobacco = <xsl:value-of select="$scalars/SmokerPct"/>;
                    </xsl:when>
                    <xsl:otherwise>
                      no blending by tobacco use;
                    </xsl:otherwise>
                  </xsl:choose>
                  male = <xsl:value-of select="$scalars/MalePct"/>.
                </fo:block>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="$scalars/SmokerBlended='1'">
                  <fo:block padding-top="1em">
                    <xsl:value-of select="$scalars/MortalityBlendFootnote"/>
                    Custom blend assumptions:
                    tobacco = <xsl:value-of select="$scalars/SmokerPct"/>;
                    <xsl:choose>
                      <xsl:when test="$scalars/GenderBlended='1'">
                        male = <xsl:value-of select="$scalars/MalePct"/>.
                      </xsl:when>
                      <xsl:otherwise>
                        no blending by gender.
                      </xsl:otherwise>
                    </xsl:choose>
                  </fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block padding-top="1em">
              The state of issue
              is <xsl:value-of select="$scalars/StatePostalAbbrev"/>.
            </fo:block>
            <xsl:if test="$compliance_tracking_number">
              <fo:block padding-top="1em">
                Compliance tracking number:
                <xsl:value-of select="$compliance_tracking_number"/>
              </fo:block>
            </xsl:if>

            <fo:block padding-top="1em">
              SEPARATE ACCOUNT
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/HypotheticalRatesFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              This illustration is based on total Separate Account fund expenses
              of <xsl:value-of select="$vectors[@name='TotalIMF']/duration[1]/@column_value"/>.
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              This illustration may not reflect your actual tax
              and accounting consequences and is not intended as tax advice
              nor may it be relied on for purposes of avoiding any federal
              tax penalties. Consult professional tax advisors for tax advice.
            </fo:block>
            <fo:block padding-top="1em">
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
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/CoUnderwriter"/> and
              &nbsp;<xsl:value-of select="$scalars/MainUnderwriter"/> are
              subsidiaries of <xsl:value-of select="$scalars/InsCoName"/>
              (<xsl:value-of select="$scalars/InsCoShortName"/>) and are
              located at <xsl:value-of select="$scalars/InsCoStreet"/>.
            </fo:block>
          </fo:block>
          <xsl:if test="not($has_supplemental_report)">
            <fo:block id="endofdoc"/>
          </xsl:if>
        </fo:flow>
      </fo:page-sequence>

      <!-- Supplemental Report -->
      <!-- Body page -->
      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader">
              <xsl:with-param name="reporttitle" select="'Supplemental Report'"/>
            </xsl:call-template>
            <xsl:call-template name="dollar-units"/>
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
    <xsl:param name="reporttitle"/>
    <fo:block text-align="center" font-size="9.75pt">
      <fo:block padding-top="1em">
        The purpose of this illustration is to show how the performance
        of the underlying separate account divisions could affect
        the contract account value and death benefit.
        <fo:inline font-weight="bold">
          The hypothetical returns do not reflect past performance
          and are not predictive of future results. Actual results
          could be less than or greater than the hypothetical results
          and in all likelihood will vary year to year.
        </fo:inline>
      </fo:block>
      <fo:block padding-top="1em">
        <xsl:value-of select="$reporttitle"/>
      </fo:block>
    </fo:block>
    <xsl:variable name="header-width" select="33"/>
    <xsl:variable name="header-field-width">
      <xsl:value-of select="$header-width * 0.44"/>pc
    </xsl:variable>
    <fo:block font-size="10pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="125mm"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body>
          <fo:table-row>
            <fo:table-cell padding-right="2mm">
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="$scalars/CorpName"/>
                      <xsl:with-param name="length" select="60"/>
                    </xsl:call-template>
                  </xsl:when>
                  <xsl:when test="not($scalars/Insured1='  ')">
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="$scalars/Insured1"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>,
                    <xsl:value-of select="$scalars/Gender"/>
                    <!-- xsl:text here prevents stripping of spaces by XSLT -->
                    <xsl:text> </xsl:text>
                    <xsl:value-of select="$scalars/Smoker"/> rates,
                    Age <xsl:value-of select="$scalars/Age"/>
                  </xsl:when>
                </xsl:choose>
              </fo:block>
              <xsl:if test="$is_composite">
                <fo:block text-align="left">
                  Composite
                </fo:block>
              </xsl:if>
              <fo:block text-align="left">
                Contract: <xsl:value-of select="$scalars/PolicyMktgName"/>
              </fo:block>
              <fo:block text-align="left">
                Initial Premium: $<xsl:value-of select="$scalars/InitPrem"/>
              </fo:block>
              <xsl:if test="not($is_composite)">
                <fo:block text-align="left">
                  <xsl:choose>
                    <xsl:when test="$scalars/UWType='Medical'">
                      Fully underwritten,
                    </xsl:when>
                    <xsl:when test="$scalars/StatePostalAbbrev='TX' and $scalars/UWType='Guaranteed issue'">
                      Substandard *,
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$scalars/UWType"/>,
                    </xsl:otherwise>
                  </xsl:choose>
                  <xsl:value-of select="$scalars/UWClass"/>,
                  Initial Death Benefit Option:
                  <xsl:value-of select="$scalars/DBOptInitInteger+1"/>
                </fo:block>
              </xsl:if>
              <xsl:if test="$scalars/HasWP='1'">
                <fo:block text-align="left">
                  Waiver of Monthly Charges Rider elected.
                </fo:block>
              </xsl:if>
              <xsl:call-template name="print-franchise-and-policynumber"/>
            </fo:table-cell>

            <fo:table-cell>
              <fo:block text-align="left">
                Assumed Gross Rate:
                <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>
                (<xsl:value-of select="$scalars/InitAnnSepAcctNetInt_Current"/>
                Net)*
              </fo:block>
              <fo:block text-align="left">
                Initial
                <xsl:if test="$scalars/HasTerm!='0'">
                  Total
                </xsl:if>
                Face Amount: $<xsl:value-of select="$scalars/InitTotalSA"/>
              </fo:block>
              <xsl:if test="$scalars/HasTerm!='0'">
                <fo:block text-align="left">
                  Initial Base Face Amount:
                  $<xsl:value-of select="$scalars/InitBaseSpecAmt"/>
                </fo:block>
                <fo:block text-align="left">
                  Initial Term Face Amount:
                  $<xsl:value-of select="$scalars/InitTermSpecAmt"/>
                </fo:block>
              </xsl:if>
              <xsl:if test="not($is_composite) and ($scalars/UWClass='Rated')">
                <fo:block text-align="left">
                  Table Rating:
                  <xsl:value-of select="$scalars/SubstandardTable"/>
                </fo:block>
              </xsl:if>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="basic-illustration-report">
    <xsl:variable name="basic_illustration_columns_raw">
      <column name="PolicyYear">Policy _Year</column>
      <column composite="0" name="AttainedAge">End of _Year Age</column>
      <column name="GrossPmt">Premium _Outlay</column>
      <column composite="1"/>
      <column name="CSVNet_GuaranteedZero">Cash Surr _Value</column>
      <column name="EOYDeathBft_GuaranteedZero">Death _Benefit</column>
      <column/>
      <column name="CSVNet_Guaranteed">Cash Surr Value</column>
      <column name="EOYDeathBft_Guaranteed">Death _Benefit</column>
      <column/>
      <column name="CSVNet_CurrentZero">Cash Surr Value</column>
      <column name="EOYDeathBft_CurrentZero">Death _Benefit</column>
      <column/>
      <column name="CSVNet_Current">Cash Surr Value</column>
      <column name="EOYDeathBft_Current">Death _Benefit</column>
    </xsl:variable>
    <xsl:variable name="basic_illustration_columns" select="document('')//xsl:variable[@name='basic_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$basic_illustration_columns[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <!-- Custom part (the biggest) of the table header -->
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="5">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  Using guaranteed charges
                </fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="5">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  Using current charges
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_GuaranteedZero"/>
                  Hypothetical Gross
                </fo:block>
                <fo:block text-align="center">
                  Return
                  (<xsl:value-of select="$scalars/InitAnnSepAcctNetInt_GuaranteedZero"/>
                  net)
                </fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Guaranteed"/>
                  Hypothetical Gross
                </fo:block>
                <fo:block text-align="center">
                  Return
                  (<xsl:value-of select="$scalars/InitAnnSepAcctNetInt_Guaranteed"/>
                  net)
                </fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_CurrentZero"/>
                  Hypothetical Gross
                </fo:block>
                <fo:block text-align="center">
                  Return
                  (<xsl:value-of select="$scalars/InitAnnSepAcctNetInt_CurrentZero"/>
                  net)
                </fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>
                  Hypothetical Gross
                </fo:block>
                <fo:block text-align="center">
                  Return
                  (<xsl:value-of select="$scalars/InitAnnSepAcctNetInt_Current"/>
                  net)
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

  <xsl:template name="supplemental-illustration">
    <xsl:variable name="supplemental_illustration_columns_raw">
      <column name="PolicyYear">Policy _Year</column>
      <column composite="0" name="AttainedAge">End of _Year Age</column>
      <column name="Outlay">Premium _Outlay</column>
      <column composite="1"/>
      <column name="PolicyFee_Current">Admin _Charge</column>
      <column name="PremTaxLoad_Current">Premium _Tax Load</column>
      <column name="DacTaxLoad_Current">DAC _Tax Load</column>
      <column name="NetPmt_Current">Net _Premium</column>
      <column name="COICharge_Current">Mortality _Charge</column>
      <column name="AcctVal_Current">Account _Value</column>
      <column name="CSVNet_Current">Cash Surr _Value</column>
      <column name="EOYDeathBft_Current">Death _Benefit</column>
    </xsl:variable>
    <xsl:variable name="supplemental_illustration_columns" select="document('')//xsl:variable[@name='supplemental_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$supplemental_illustration_columns[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
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
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="illustration-assumption">
    <xsl:variable name="illustration_assumption_columns_raw">
      <column name="PolicyYear">|Policy _Year</column>
      <column composite="0" name="AttainedAge">|End of _Year Age</column>
      <column composite="0" name="AnnSAIntRate_Current">Net Crediting Rate|Sep Acct</column>
      <column composite="0" name="AnnGAIntRate_Current">Net Crediting Rate|Gen Acct</column>
      <column composite="0" name="CurrMandE">|M&amp;E</column>
      <column name="NetWD">|Withdrawal</column>
      <column name="NewCashLoan">|Loan</column>
      <column composite="0" name="EeMode">|Indiv _Pmt Mode</column>
      <column composite="0" name="ErMode">|Corp _Pmt Mode</column>
      <column composite="0" scalar="InitAnnLoanDueRate">|Assumed _Loan Interest</column>
      <column composite="0" name="MonthlyFlatExtra">|Flat Extra _Per 1,000</column>
    </xsl:variable>
    <xsl:variable name="illustration_assumption_columns" select="document('')//xsl:variable[@name='illustration_assumption_columns_raw']/column"/>
    <xsl:variable name="columns" select="$illustration_assumption_columns[not(@composite) or (boolean(@composite='1')=$is_composite)]"/>

    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9pt serif">
        <fo:table table-layout="fixed" width="100%">
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
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $scalars/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
      <xsl:if test="not($has_supplemental_report)">
        <fo:block id="endofdoc"/>
      </xsl:if>
    </fo:flow>
  </xsl:template>

  <xsl:template name="list-fund-allocation">
    <xsl:param name="counter"/>
    <xsl:if test="$vectors[@name='FundNames']/duration[$counter]/@column_value!=''">
      <fo:block>
        <xsl:value-of select="$vectors[@name='FundAllocations']/duration[$counter]/@column_value"/>
        <xsl:value-of select="$vectors[@name='FundNames']/duration[$counter]/@column_value"/>
      </fo:block>
      <xsl:call-template name="list-fund-allocation">
        <xsl:with-param name="counter" select="$counter + 1"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="standardfooter">
    <xsl:param name="omit-pagenumber" select="boolean(0)"/>
    <xsl:call-template name="generic-footer">
      <xsl:with-param name="top-block">
        <fo:block>
          * This illustration is based on the assumed Gross Rate shown.
          The Net Rate is provided for information purposes only.
        </fo:block>
        <fo:block>
          See the Narrative Summary for important policy information.
          This illustration is not complete without all pages.
        </fo:block>
      </xsl:with-param>
      <xsl:with-param name="left-block">
        <fo:block><xsl:value-of select="$scalars/InsCoName"/></fo:block>
        <fo:block><xsl:value-of select="$scalars/InsCoAddr"/></fo:block>
      </xsl:with-param>
      <xsl:with-param name="center-block">
        <fo:block>
          Date Prepared:
          <xsl:call-template name="date-prepared"/>
        </fo:block>
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
