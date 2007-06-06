<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

    Copyright (C) 2004, 2005, 2006, 2007 Gregory W. Chicares.

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

    $Id: group_private_placement.xsl,v 1.19 2007-06-06 14:03:23 etarassov Exp $
-->
<!DOCTYPE stylesheet [
<!ENTITY nbsp "&#xA0;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
  <xsl:include href="fo_common.xsl"/>
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:variable name="counter" select="1"/>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.1in" margin-bottom="0in" margin-left="0.35in" margin-right="0.35in">
          <fo:region-body margin-top="0.25in" margin-bottom="0.10in"/>
        </fo:simple-page-master>

        <!-- Define the Column Definitions page. -->
        <fo:simple-page-master master-name="column-definitions" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.00in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.50in" margin-bottom="1.25in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary-separate-account" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="2" margin-top="2.25in" margin-bottom="3.0in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the Basic Illustration page. -->
        <fo:simple-page-master master-name="basic-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.00in" margin-bottom="1.25in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Illustration page. -->
        <fo:simple-page-master master-name="supplemental-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.00in" margin-bottom="1.00in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".9in"/>
        </fo:simple-page-master>

        <!-- Define the Illustration Assumption Detail page. -->
        <fo:simple-page-master master-name="illustration-assumption" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.80in" margin-bottom="1.00in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".9in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
            <!-- Central part of page -->
            <fo:region-body column-count="1" margin-top="2.80in" margin-bottom="1.10in"/>
            <!-- Header -->
            <fo:region-before extent="3in"/>
            <!-- Footer -->
            <fo:region-after extent="1in"/>
          </fo:simple-page-master>
        </xsl:if>

      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->

      <fo:page-sequence master-reference="cover" force-page-count="no-force">
        <fo:flow flow-name="xsl-region-body">
          <fo:block border="2pt solid blue" font-size="14.0pt" text-align="center" font-family="sans-serif">
            <fo:block font-size="20.0pt" font-weight="bold" padding-top="5em">
              <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
            </fo:block>
            <xsl:choose>
              <xsl:when test="illustration/scalar/IsInforce!='1'">
                <fo:block font-size="20.0pt" font-weight="bold">
                  <xsl:text>Life Insurance Illustration</xsl:text>
                </fo:block>
              </xsl:when>
              <xsl:otherwise>
                <fo:block font-size="20.0pt" font-weight="bold">
                  <xsl:text>In Force Life Insurance Illustration</xsl:text>
                </fo:block>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block text-align="center" font-weight="bold" padding-top="10em">
              Prepared for:
            </fo:block>
            <fo:block margin-left="0.15in" margin-right="0.15in" padding-top="1em">
              <xsl:choose>
                <xsl:when test="not($is_composite)">
                  <!-- Properly adjust for long user input strings limit output to 140 characters for appox. 2 lines -->
                  <xsl:call-template name="limitstring">
                    <xsl:with-param name="passString" select="illustration/scalar/Insured1"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                  <xsl:if test="string-length(illustration/scalar/Insured1) &lt; 70">
                    <fo:block padding=".7em"/>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:call-template name="limitstring">
                    <xsl:with-param name="passString" select="illustration/scalar/CorpName"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                  <xsl:if test="string-length(illustration/scalar/CorpName) &lt; 70">
                    <fo:block padding=".7em"/>
                  </xsl:if>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block text-align="center" font-weight="bold" padding-top="3em">
              Presented by:
            </fo:block>
            <fo:block text-align="center" padding-top="1em">
              <xsl:value-of select="illustration/scalar/ProducerName"/>
            </fo:block>
            <fo:block text-align="center">
              <xsl:value-of select="illustration/scalar/ProducerStreet"/>
            </fo:block>
            <fo:block text-align="center">
              <xsl:value-of select="illustration/scalar/ProducerCity"/>
            </fo:block>
            <fo:block text-align="center" padding-top="2em">
              <xsl:value-of select="illustration/scalar/PrepMonth"/>
              <xsl:text> </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepDay"/>
              <xsl:text>, </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepYear"/>
            </fo:block>
            <fo:block padding-top="10em">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:value-of select="illustration/scalar/InsCoName"/>
            </fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/InsCoAddr"/>
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- Basic Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="basic-illustration" initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
        <fo:block text-align="left">
          <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
        </fo:block>
        <xsl:choose>
          <xsl:when test="illustration/scalar/IsInforce!='1'">
            <xsl:call-template name="standardheader">
              <xsl:with-param name="reporttitle" select="'Basic Illustration'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="standardheader">
              <xsl:with-param name="reporttitle" select="'In Force Basic Illustration'"/>
            </xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
            <xsl:text> </xsl:text>
          </fo:block>
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <xsl:call-template name="basic-illustration-report"/>

      </fo:page-sequence>

      <!-- Supplemental Illustration -->
      <!-- narrative summary - begins here -->
      <fo:page-sequence master-reference="column-definitions">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Column Definitions Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="10pt" font-family="sans-serif">
            <fo:block font-weight="bold" text-align="center" padding-top="1em">
              Column Definitions
            </fo:block>
            <fo:block font-weight="bold" padding-top="3em">
              <xsl:text>Policy Year</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The number of years the policy is assumed to have been in force.</xsl:text>
            </fo:block>
            <xsl:if test="not($is_composite)">
              <fo:block font-weight="bold" padding-top="2em">
                <xsl:text>End of Year Age</xsl:text>
              </fo:block>
              <fo:block>
                <xsl:text>The issue age of the insured plus the number of completed policy years since the policy date.</xsl:text>
              </fo:block>
            </xsl:if>
            <fo:block font-weight="bold" padding-top="2em">
              <xsl:text>Premium Outlay</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The total amount the premium payor plans to pay each year, assuming that each payment is made at the beginning of the premium paying period.</xsl:text>
            </fo:block>
            <fo:block font-weight="bold" padding-top="2em">
              <xsl:text>Cash Surrender Value</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The Cash Surrender Value is the amount payable to a policy owner upon surrender of the policy.  It is equal to the Account Value less any surrender charges and less any policy debt and any unpaid monthly charges.</xsl:text>
            </fo:block>
            <fo:block font-weight="bold" padding-top="2em">
              <xsl:text>Death Benefit</xsl:text>
            </fo:block>
            <fo:block padding-bottom="4em">
              <xsl:text>The Death Benefit is the net amount paid to a beneficiary following receipt of due proof of death.  The Death Benefit is equal to the benefit provided by the death benefit option in effect on the date of death less any policy debt and any unpaid monthly charges.</xsl:text>
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- narrative summary - begins here -->
      <fo:page-sequence master-reference="narrative-summary">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader">
            <xsl:with-param name="reporttitle" select="'Narrative Summary'"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- narrative summary Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="10pt" font-family="sans-serif">
            <fo:block>
              <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
              <xsl:text> is a </xsl:text>
              <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
              <xsl:text> issued by </xsl:text>
              <xsl:value-of select="illustration/scalar/InsCoName"/>
              <xsl:text>. The policy provides lifetime insurance protection for as long as it remains in force. The policy is available at issue with at least two death benefit options: Option 1 (death benefit equal to the greater of (a) the selected face amount in effect on the date of death or (b) the minimum face amount in effect on the date of death); and Option 2 (death benefit equal to the greater of (a) the selected face amount in effect on the date of death plus account value on that date or (b) the minimum face amount in effect on the date of death).  A policy with the stable value endorsement requires Death Benefit Option 1.</xsl:text>
            </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>This illustration assumes Initial Death Benefit Option </xsl:text>
            <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
            <xsl:text>.</xsl:text>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>This policy is only available to entities that meet the definition of "accredited investor" or "qualified purchaser" under applicable Federal securities laws.  Purchase of this policy is suitable only for entities of substantial economic means.  Each prospective purchaser will be required to represent that it is familiar with and understands the fundamental risks and financial hazards of purchasing the policy.  Each prospective purchaser must also represent that it meets minimum financial and other suitability standards.</xsl:text>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>Account values may be used to pay contract charges. Contract charges are due during the life of the insured; depending on actual results, the premium payer may need to continue or resume premium outlays.</xsl:text>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>The definition of life insurance elected for this contract is the </xsl:text>
            <xsl:choose>
              <xsl:when test="illustration/scalar/DefnLifeIns='GPT'">
                <xsl:text>guideline premium test. The guideline single premium is $</xsl:text>
                <xsl:value-of select="illustration/scalar/InitGSP"/>
                <xsl:text> and the guideline level premium is $</xsl:text>
                <xsl:value-of select="illustration/scalar/InitGLP"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>cash value accumulation test.</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:value-of select="illustration/scalar/InsCoName"/>
            <xsl:text> has the right to promptly return any amount of premium paid if it will increase the net amount at risk </xsl:text>
            <xsl:text>(referred to in the contract as the Amount of Insurance that Requires a Charge).</xsl:text>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>Account values, cash surrender values, and death benefits are illustrated as of the end of the year.</xsl:text>
          </fo:block>
          <fo:block padding-top="1em">
            <xsl:text>PLEASE READ THE FOLLOWING IMPORTANT TAX DISCLOSURE</xsl:text>
          </fo:block>
          <fo:block>
            <xsl:text>The initial 7-pay premium limit is $</xsl:text>
            <xsl:value-of select="illustration/scalar/InitSevenPayPrem"/>
            <xsl:text>. As illustrated, this contract </xsl:text>
            <xsl:choose>
              <xsl:when test="illustration/scalar/IsMec='1'">
                <xsl:text>fails </xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>passes </xsl:text>
              </xsl:otherwise>
            </xsl:choose>
            <xsl:text>the seven-pay test defined in Section 7702A of the Internal Revenue Code and therefore </xsl:text>
            <xsl:choose>
              <xsl:when test="illustration/scalar/IsMec='1'">
                <xsl:choose>
                  <xsl:when test="illustration/scalar/MecYear!='0'">
                    <xsl:text>becomes a Modified Endowment Contract (MEC) </xsl:text>
                    <xsl:text>in policy year </xsl:text>
                    <xsl:value-of select="illustration/scalar/MecYear+1"/>
                    <xsl:text>.  As a MEC, any loans or withdrawals are taxed to the extent of any gain in the contract, and an additional 10% penalty tax will apply to the taxable portion of the loan or withdrawal.</xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>is a Modified Endowment Contract (MEC)</xsl:text>
                    <xsl:text>.  As a MEC, any loans or withdrawals are taxed to the extent of any gain in the contract, and an additional 10% penalty tax will apply to the taxable portion of the loan or withdrawal.</xsl:text>
                  </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>is not a Modified Endowment Contract (MEC)</xsl:text>
                <xsl:text>.  Subsequent changes in the contract, including but not limited to increases and decreases in premiums or benefits, may cause the contract to be retested and may result in the contract becoming a MEC.</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              <xsl:text>This illustration is not written or intended as tax or legal advice and may not be relied on for purposes of avoiding any federal tax penalties.  For more information pertaining to the tax consequences of purchasing or owning this policy, consult with your own independent tax or legal counsel.</xsl:text>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>This contract has a guaranteed maximum cost of insurance (based on 1980 CSO mortality tables) and maximum administrative charges. The actual current charges are lower than these and are reflected in the values. However, these current charges are subject to change.</xsl:text>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>The loan interest rate may be fixed or adjustable as elected by the policy owner.</xsl:text>
            </fo:block>
            <fo:block padding-top="1em" padding-bottom="1em">
              <xsl:choose>
                <xsl:when test="illustration/scalar/UseExperienceRating='1'">
                  <xsl:text>The illustration assumes mortality experience rating.  To the extent that other clients participate in the pool your results will be affected by the experience of the entire pool.</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>The illustration does not assume any mortality experience rating.</xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>

            <!-- Force new page -->
            <fo:block break-after="page"/>

            <fo:block font-style="italic">
              <xsl:text>This illustration may only be used when preceded or accompanied by the offering memorandum for </xsl:text>
              <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
              <xsl:text> (</xsl:text>
              <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
              <xsl:text>)</xsl:text>
              <xsl:text> insurance contract and its underlying investment choices. For a complete listing of the underlying investment choices, please refer to the offering memorandum.  Before purchasing a variable life insurance contract, investors should carefully consider the investment objectives, risks, charges and expenses of the variable life insurance contract and its underlying investment choices. Please read the offering memorandum carefully before investing or sending money.</xsl:text>
            </fo:block>
          </fo:block>

          <fo:block text-align="left" font-size="10pt" font-family="sans-serif" padding-top="1em">
            <xsl:if test="illustration/scalar/HasTerm='1'">
              <fo:block>
                <xsl:text>A Term Rider is available for attachment to this policy. The Term Rider provides the option to purchase monthly term insurance on the life of the insured. </xsl:text>
                <xsl:text>The term rider selected face amount supplements the selected face amount of the contract. If the Term Rider is attached, the policy to which it is attached may have a lower annual cutoff premium and, as a result, the lower overall sales loads paid may be lower than a contract having the same total face amount, but with no Term Rider. Also, the lapse protection feature of the contract's</xsl:text>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text> does not apply to the Term Rider's selected face amount.</xsl:text>
              </fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasWP='1'">
              <fo:block padding-top="1em">
                <xsl:text>A Waiver of Monthly Charges rider is available for attachment to this policy for insureds with ages 20-64. The Waiver of Monthly Charges Rider provides that in the event of the disability of the insured that begins before attained age 65 and continues for at least 6 months, </xsl:text>
                <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                <xsl:text> will waive certain monthly charges up to age 65, but not less than two years, while the insured remains totally disabled. An additional charge is associated with this rider, if elected. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/UsePartialMort='1'">
              <fo:block padding-top="1em">
                <xsl:text>This illustration reflects an initial mortality assumption of </xsl:text>
                <xsl:value-of select="illustration/data/newcolumn/column[@name='PartMortTableMult']/duration[1]/@column_value"/>
                <xsl:text> of the </xsl:text>
                <xsl:value-of select="illustration/scalar/PartMortTableName"/>
                <xsl:text> table with all deaths at the end of the year.</xsl:text>
              </fo:block>
            </xsl:if>
            <xsl:choose>
              <xsl:when test="illustration/scalar/GenderBlended='1'">
                <fo:block padding-top="1em">
                  <xsl:text>Custom blending of cost of insurance charges is based on the plan's assumed distribution of initial selected face amount by gender and tobacco use.  This illustration assumes that the distribution remains constant in future years. Custom blending is available only on plans of 100 or more lives. Custom blend assumptions: </xsl:text>
                  <xsl:choose>
                    <xsl:when test="illustration/scalar/SmokerBlended='1'">
                      <xsl:text>tobacco = </xsl:text>
                      <xsl:value-of select="illustration/scalar/SmokerPct"/>
                      <xsl:text>; </xsl:text>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:text>no blending by tobacco use; </xsl:text>
                    </xsl:otherwise>
                  </xsl:choose>
                  <xsl:text> male = </xsl:text>
                  <xsl:value-of select="illustration/scalar/MalePct"/>
                  <xsl:text>.</xsl:text>
                </fo:block>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="illustration/scalar/SmokerBlended='1'">
                  <fo:block padding-top="1em">
                    <xsl:text>Custom blending of cost of insurance charges is based on the plan's assumed distribution of initial selected face amount by gender and tobacco use.  This illustration assumes that the distribution remains constant in future years. Custom blending is available only on plans of 100 or more lives. Custom blend assumptions: </xsl:text>
                    <xsl:text>tobacco = </xsl:text>
                    <xsl:value-of select="illustration/scalar/SmokerPct"/>
                    <xsl:text>; </xsl:text>
                    <xsl:choose>
                      <xsl:when test="illustration/scalar/GenderBlended='1'">
                        <xsl:text> male = </xsl:text>
                        <xsl:value-of select="illustration/scalar/MalePct"/>
                      </xsl:when>
                      <xsl:otherwise>
                        <xsl:text> no blending by gender.</xsl:text>
                      </xsl:otherwise>
                    </xsl:choose>
                  </fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block padding-top="1em">
              <xsl:text>The state of issue is </xsl:text>
              <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
              <xsl:text>.</xsl:text>
            </fo:block>
            <xsl:choose>
              <xsl:when test="illustration/scalar/IsInforce!='1'">
                <xsl:if test="string-length(illustration/scalar/InsCoPhone) &gt; 14">
                  <fo:block padding-top="1em">
                    <xsl:text>Compliance tracking number: </xsl:text>
                    <xsl:value-of select="substring(illustration/scalar/InsCoPhone, 1, 15)"/>
                  </fo:block>
                </xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="string-length(illustration/scalar/InsCoPhone) &gt; 16">
                  <fo:block padding-top="1em">
                    <xsl:text>Compliance Tracking Number: </xsl:text>
                    <xsl:value-of select="substring(illustration/scalar/InsCoPhone, 16)"/>
                  </fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block padding-top="1em">
              <xsl:text>SEPARATE ACCOUNT</xsl:text>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>This illustration shows how the death benefit could vary over an extend period of time, assuming hypothetical gross rates of investment return for the funds.  </xsl:text>
              <xsl:text> These hypothetical returns do not reflect past performance and are not predictive of future results.  </xsl:text>
              <xsl:text>Actual results could be less than or greater than the hypothetical results and in all likelihood will vary </xsl:text>
              <xsl:text>year to year. Timing of premium payments, investment allocations and withdrawals or loans, if taken, may impact investment results. </xsl:text>
              <xsl:text>Separate Account Charges are deducted from the gross investment rate to determine the net investment rate. These charges include a mortality </xsl:text>
              <xsl:text>and expense charge, and an investment management fee and other fund operating expenses. The total fund operating expenses charged vary </xsl:text>
              <xsl:text>by fund. No tax charge is made to the Separate Account. However, such a charge may be made in the future.  SEE OFFERING MEMORANDUM.</xsl:text>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>This illustration is based on total Separate Account fund expenses of </xsl:text>
              <xsl:value-of select="illustration/data/newcolumn/column[@name='TotalIMF']/duration[1]/@column_value"/>
              <xsl:text>.</xsl:text>
            </fo:block>
            <fo:block font-size="10.5pt" font-weight="bold" padding-top="1em">
              <xsl:text>This illustration may not reflect your actual tax and accounting consequences and is not intended as tax advice nor may it be relied on for purposes of avoiding any federal tax penalties. Consult professional tax advisors for tax advice.</xsl:text>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>Placement Agent: </xsl:text>
              <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
              <xsl:text> (a registered broker-dealer)</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/MainUnderwriterAddress"/>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:text>Wholly owned subsidiary of </xsl:text>
              <xsl:value-of select="illustration/scalar/InsCoName"/>
            </fo:block>
          </fo:block>
          <xsl:choose>
            <xsl:when test="$has_supplemental_report">
            </xsl:when>
            <xsl:otherwise>
              <fo:block id="endofdoc"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:flow>
      </fo:page-sequence>

      <!-- Supplemental Report -->
      <!-- Body page -->
      <xsl:if test="$has_supplemental_report">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <xsl:call-template name="standardheader">
              <xsl:with-param name="reporttitle" select="'Supplemental Report'"/>
            </xsl:call-template>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <fo:static-content flow-name="xsl-region-after">
            <fo:block>
              <xsl:call-template name="standardfooter">
                <xsl:with-param name="displaypagenumber" select="1"/>
              </xsl:call-template>
            </fo:block>
          </fo:static-content>

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
        The purpose of this illustration is to show how the performance of the underlying separate account divisions could affect the contract account value and death benefit. <fo:inline font-weight="bold">The hypothetical returns do not reflect past performance and are not predictive of future results.  Actual results could be less than or greater than the hypothetical results and in all likelihood will vary year to year.</fo:inline>
      </fo:block>
      <fo:block padding-top="1em">
        <xsl:value-of select="$reporttitle"/>
      </fo:block>
    </fo:block>
    <xsl:variable name="header-width" select="33"/>
    <xsl:variable name="header-field-width">
      <xsl:value-of select="$header-width * 0.44"/>
      <xsl:text>pc</xsl:text>
    </xsl:variable>
    <fo:block padding-before="0pt" font-size="10.0pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="125mm"/>
        <fo:table-column column-width="2mm"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body padding-before="2pt">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/CorpName"/>
                      <xsl:with-param name="length" select="60"/>
                    </xsl:call-template>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/Insured1='  '"/>
                  <xsl:otherwise>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/Insured1"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="illustration/scalar/Gender"/>
                    <xsl:text>&nbsp;</xsl:text>
                    <xsl:value-of select="illustration/scalar/Smoker"/>
                    <xsl:text> rates, Age </xsl:text>
                    <xsl:value-of select="illustration/scalar/Age"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:text>Composite</xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>Contract: </xsl:text>
                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:text>Contract: </xsl:text>
                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text>Initial Premium: $</xsl:text>
                    <xsl:value-of select="illustration/scalar/InitPrem"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:text>Initial Premium: $</xsl:text>
                    <xsl:value-of select="illustration/scalar/InitPrem"/>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/UWType='Medical'">
                    <xsl:text>Fully underwritten</xsl:text>
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="illustration/scalar/UWClass"/>
                    <xsl:text>, Initial Death Benefit Option: </xsl:text>
                    <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/StatePostalAbbrev='TX'">
                    <xsl:choose>
                      <xsl:when test="illustration/scalar/UWType='Guaranteed issue'">
                        <xsl:text>Substandard *</xsl:text>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="illustration/scalar/UWClass"/>
                        <xsl:text>, Initial Death Benefit Option: </xsl:text>
                        <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                      </xsl:when>
                      <xsl:otherwise>
                        <xsl:value-of select="illustration/scalar/UWType"/>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="illustration/scalar/UWClass"/>
                        <xsl:text>, Initial Death Benefit Option: </xsl:text>
                        <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                      </xsl:otherwise>
                    </xsl:choose>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:value-of select="illustration/scalar/UWType"/>
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="illustration/scalar/UWClass"/>
                    <xsl:text>, Initial Death Benefit Option: </xsl:text>
                    <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:if test="illustration/scalar/HasWP='1'">
                  <xsl:text>Waiver of Monthly Charges Rider elected.</xsl:text>
                </xsl:if>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    <xsl:if test="illustration/scalar/Franchise!=''">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limitstring">
                        <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/Franchise!='' and illustration/scalar/PolicyNumber!=''">
                    <xsl:text>Master contract: </xsl:text>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                      <xsl:with-param name="length" select="15"/>
                    </xsl:call-template>
                    <xsl:text>&nbsp;&nbsp;&nbsp;Contract number: </xsl:text>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/PolicyNumber"/>
                      <xsl:with-param name="length" select="15"/>
                    </xsl:call-template>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/Franchise!=''">
                    <xsl:text>Master contract: </xsl:text>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                  </xsl:when>
                  <xsl:when test="illustration/scalar/PolicyNumber!=''">
                    <xsl:text>Contract number: </xsl:text>
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="illustration/scalar/PolicyNumber"/>
                      <xsl:with-param name="length" select="30"/>
                    </xsl:call-template>
                  </xsl:when>
                </xsl:choose>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Assumed Gross Rate: </xsl:text>
                <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                <xsl:text> (</xsl:text>
                <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                <xsl:text> Net)*</xsl:text>
              </fo:block>
              <fo:block text-align="left">
                <xsl:text>Initial </xsl:text>
                <xsl:if test="illustration/scalar/HasTerm!='0'">
                  <xsl:text> Total</xsl:text>
                </xsl:if>
                <xsl:text> Face Amount: $</xsl:text>
                <xsl:value-of select="illustration/scalar/InitTotalSA"/>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="illustration/scalar/HasTerm!='0'">
                    <xsl:text>Initial Base Face Amount: $</xsl:text>
                    <xsl:value-of select="illustration/scalar/InitBaseSpecAmt"/>
                  </xsl:when>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="illustration/scalar/HasTerm!='0'">
                    <xsl:text>Initial Term Face Amount: $</xsl:text>
                    <xsl:value-of select="illustration/scalar/InitTermSpecAmt"/>
                  </xsl:when>
                </xsl:choose>
              </fo:block>
              <fo:block text-align="left">
                <xsl:if test="not($is_composite) and (illustration/scalar/UWClass='Rated')">
                  <xsl:text>Table Rating: </xsl:text>
                  <xsl:value-of select="illustration/scalar/SubstandardTable"/>
                </xsl:if>
              </fo:block>
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
    <xsl:variable name="basic_illustration_columns" select="document('')/xsl:stylesheet/xsl:template[@name='basic-illustration-report']/xsl:variable[@name='basic_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$basic_illustration_columns[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
            <!-- Custom part (the biggest) of the table header -->
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3" padding="0pt"/>
              <fo:table-cell number-columns-spanned="5" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">Using guaranteed charges</fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="5" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">Using current charges</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block/>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt">
                <fo:block text-align="center">
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_GuaranteedZero"/>
                  Hypothetical Gross
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt">
                <fo:block text-align="center">
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Guaranteed"/>
                  Hypothetical Gross
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt">
                <fo:block text-align="center">
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_CurrentZero"/>
                  Hypothetical Gross
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt">
                <fo:block text-align="center">
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                  Hypothetical Gross
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_GuaranteedZero"/>
                  <xsl:text>&nbsp;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Guaranteed"/>
                  <xsl:text>&nbsp;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_CurrentZero"/>
                  <xsl:text>&nbsp;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                  <xsl:text>&nbsp;net)</xsl:text>
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
              <xsl:with-param name="counter" select="$illustration/scalar/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $illustration/scalar/InforceYear"/>
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
    <xsl:variable name="supplemental_illustration_columns" select="document('')/xsl:stylesheet/xsl:template[@name='supplemental-illustration']/xsl:variable[@name='supplemental_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$supplemental_illustration_columns[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
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
              <xsl:with-param name="counter" select="$illustration/scalar/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $illustration/scalar/InforceYear"/>
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
    <xsl:variable name="illustration_assumption_columns" select="document('')/xsl:stylesheet/xsl:template[@name='illustration-assumption']/xsl:variable[@name='illustration_assumption_columns_raw']/column"/>
    <xsl:variable name="columns" select="$illustration_assumption_columns[not(@composite) or (boolean(@composite='1')=$is_composite)]"/>

    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
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
              <xsl:with-param name="counter" select="$illustration/scalar/InforceYear + 1"/>
              <xsl:with-param name="max-counter" select="$max-lapse-year"/>
              <xsl:with-param name="inforceyear" select="0 - $illustration/scalar/InforceYear"/>
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
    <xsl:if test="illustration/data/newcolumn/column[@name='FundNames']/duration[$counter]/@column_value!=''">
      <fo:block>
        <xsl:value-of select="illustration/data/newcolumn/column[@name='FundAllocations']/duration[$counter]/@column_value"/>
          <xsl:text> </xsl:text>
          <xsl:value-of select="illustration/data/newcolumn/column[@name='FundNames']/duration[$counter]/@column_value"/>
      </fo:block>
      <xsl:call-template name="list-fund-allocation">
        <xsl:with-param name="counter" select="$counter + 1"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="standardfooter">
    <xsl:param name="displaypagenumber"/>
    <fo:block text-align="left" font-size="8.5pt" font-family="sans-serif">
      <fo:block text-align="left">
        <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
      </fo:block>
      <fo:block text-align="left">
        <xsl:text>See the Narrative Summary for important policy information. This illustration is not complete without all pages.</xsl:text>
      </fo:block>
    </fo:block>
    <fo:block padding-before="5pt" font-size="8.5pt" font-family="sans-serif" padding-top="1em">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body padding-before="2pt" border-top-style="solid" border-top-width="1pt" border-top-color="blue">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:value-of select="illustration/scalar/InsCoName"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="center">
                <xsl:text>Date Prepared: </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepMonth"/>
                <xsl:text>&nbsp;</xsl:text>
                <xsl:value-of select="illustration/scalar/PrepDay"/>
                <xsl:text>, </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepYear"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:text>Policy Form: </xsl:text>
                <xsl:value-of select="illustration/scalar/PolicyForm"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:value-of select="illustration/scalar/InsCoAddr"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <xsl:if test="illustration/scalar/LmiVersion!=''">
                <fo:block text-align="center">System Version:
                  <xsl:value-of select="illustration/scalar/LmiVersion"/>
                </fo:block>
              </xsl:if>
            </fo:table-cell>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="$displaypagenumber=1">
                  <fo:block text-align="right">
                    <xsl:text>Page </xsl:text>
                    <fo:page-number/>
                    <xsl:text> of </xsl:text>
                    <fo:page-number-citation ref-id="endofdoc"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="right">
                    <xsl:text>Attachment</xsl:text>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="footer-with-underwriter">
    <xsl:param name="displaypagenumber"/>
    <fo:block text-align="left" font-size="8.5pt" font-family="sans-serif">
      <fo:block span="all">
        <fo:block padding-top="1em">
          <xsl:text>The Principal Underwriter is </xsl:text>
          <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
          <xsl:text>, a wholly owned subsidiary of </xsl:text>
          <xsl:value-of select="illustration/scalar/InsCoShortName"/>
          <xsl:text> and is located at </xsl:text>
        </fo:block>
        <fo:block>
          <xsl:value-of select="illustration/scalar/MainUnderwriterAddress"/>
        </fo:block>
      </fo:block>
      <fo:block text-align="left" padding-top="1em">
        <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
      </fo:block>
      <fo:block text-align="left">
        <xsl:text>See the Narrative Summary for important policy information. This illustration is not complete without all pages.</xsl:text>
      </fo:block>
    </fo:block>
    <fo:block padding-before="5pt" font-size="8.0pt" font-family="sans-serif" padding-top="1em">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body padding-before="2pt" border-top-style="solid" border-top-width="1pt" border-top-color="blue">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:value-of select="illustration/scalar/InsCoName"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block/>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:text>Policy Form: </xsl:text>
                <xsl:value-of select="illustration/scalar/PolicyForm"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:value-of select="illustration/scalar/InsCoAddr"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <xsl:if test="illustration/scalar/VersionNumber!=''">
                <fo:block text-align="center">Version Number:
                   <xsl:value-of select="illustration/scalar/VersionNumber"/>
                </fo:block>
              </xsl:if>
            </fo:table-cell>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="$displaypagenumber=1">
                  <fo:block text-align="right">
                    <xsl:text>Page </xsl:text>
                    <fo:page-number/>
                    <xsl:text> of </xsl:text>
                    <fo:page-number-citation ref-id="endofdoc"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="right">
                    <xsl:text>Attachment</xsl:text>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

</xsl:stylesheet>
