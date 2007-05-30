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

    $Id: nasd.xsl,v 1.16 2007-05-30 15:30:31 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <xsl:include href="fo_common.xsl" />
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:variable name="counter" select="1"/>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.1in" margin-bottom="0in" margin-left="0.35in" margin-right="0.35in" >
          <fo:region-body margin-top="0.25in" margin-bottom="0.10in"/>
        </fo:simple-page-master>

        <!-- Define the Column Definitions page. -->
        <fo:simple-page-master master-name="column-definitions" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="0.91in"/>
        </fo:simple-page-master>

        <!-- Define the explanatory notes page. -->
        <fo:simple-page-master master-name="explanatory-notes" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="2.35in" margin-bottom="0.90in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.0in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="0.91in"/>
        </fo:simple-page-master>

        <!-- Define the explanatory notes page. -->
        <fo:simple-page-master master-name="explanatory-notes-separate-account" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <!--<fo:region-body column-count="2" margin-top="2.25in" margin-bottom="2.70in"/> -->
          <fo:region-body column-count="2" margin-top="2.35in" margin-bottom="1.3in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.3in"/>
        </fo:simple-page-master>

        <!-- Define the Basic Illustration page. -->
        <fo:simple-page-master master-name="basic-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.00in" margin-bottom="1.25in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent=".91in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Illustration page. -->
        <fo:simple-page-master master-name="supplemental-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.00in" margin-bottom="1.00in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent=".91in"/>
        </fo:simple-page-master>

        <!-- Define the Illustration Assumption Detail page. -->
        <xsl:if test="illustration/scalar/Composite!='1'">
          <fo:simple-page-master master-name="illustration-assumption" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
            <!-- Central part of page -->
            <fo:region-body column-count="1" margin-top="2.80in" margin-bottom="1.00in"/>
            <!-- Header -->
            <fo:region-before border-after-style="solid" extent="3in"/>
            <!-- Footer -->
            <fo:region-after border-before-style="solid" extent=".91in"/>
          </fo:simple-page-master>
        </xsl:if>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
            <!-- Central part of page -->
            <fo:region-body column-count="1" margin-top="2.80in" margin-bottom="1.10in"/>
            <!-- Header -->
            <fo:region-before border-after-style="solid" extent="3in"/>
            <!-- Footer -->
            <fo:region-after border-before-style="solid" extent=".96in"/>
          </fo:simple-page-master>
        </xsl:if>
      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->
      <fo:page-sequence master-reference="cover" force-page-count="no-force">
        <fo:flow flow-name="xsl-region-body">
          <fo:block border-top-color="blue"
            border-top-style="solid"
            border-top-width="thick"
            border-bottom-color="blue"
            border-bottom-style="solid"
            border-bottom-width="thick"
            border-left-color="blue"
            border-left-width="thick"
            border-left-style="solid"
            border-right-color="blue"
            border-right-width="thick"
            border-right-style="solid"
            font-size="14.0pt"
            text-align="center"
            font-family="sans-serif">
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-size="20.0pt" font-weight="bold">
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
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block text-align="center" font-weight="bold">
                Prepared for:
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block margin-left="0.15in" margin-right="0.15in">
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite!='1'">
                  <!-- Properly adjust for long user input strings limit output to 140 characters for appox. 2 lines -->
                  <xsl:call-template name="limitstring" >
                    <xsl:with-param name="passString" select="illustration/scalar/Insured1"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                  <xsl:if test="string-length(illustration/scalar/Insured1) &lt; 70">
                    <fo:block><fo:leader></fo:leader></fo:block>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:call-template name="limitstring" >
                    <xsl:with-param name="passString" select="illustration/scalar/CorpName"/>
                    <xsl:with-param name="length" select="140"/>
                  </xsl:call-template>
                  <xsl:if test="string-length(illustration/scalar/CorpName) &lt; 70">
                    <fo:block><fo:leader></fo:leader></fo:block>
                  </xsl:if>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block text-align="center" font-weight="bold">
                Presented by:
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block text-align="center" >
              <xsl:value-of select="illustration/scalar/ProducerName"/>
            </fo:block>
            <fo:block text-align="center" >
              <xsl:value-of select="illustration/scalar/ProducerStreet"/>
            </fo:block>
            <fo:block text-align="center" >
              <xsl:value-of select="illustration/scalar/ProducerCity"/>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block text-align="center" >
              <xsl:value-of select="illustration/scalar/PrepMonth"/>
              <xsl:text> </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepDay"/>
              <xsl:text>, </xsl:text>
              <xsl:value-of select="illustration/scalar/PrepYear"/>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/InsCoName"/>
            </fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/InsCoAddr"/>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- Basic Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="basic-illustration"  initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:choose>
            <xsl:when test="illustration/scalar/IsInforce!='1'">
              <xsl:call-template name="standardheader">
                <xsl:with-param name="reporttitle" select="'Basic Life Illustration'"/>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name="standardheader">
                <xsl:with-param name="reporttitle" select="'In Force Basic Illustration'"/>
              </xsl:call-template>
            </xsl:otherwise>
          </xsl:choose>
          <fo:block><fo:leader></fo:leader></fo:block>
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
      <!-- Body page -->
      <fo:page-sequence master-reference="supplemental-illustration">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader">
            <xsl:with-param name="reporttitle" select="'Supplemental Illustration'"/>
          </xsl:call-template>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block text-align="center" font-size="9pt" font-family="sans-serif">
            <xsl:text>Using Current charges</xsl:text>
          </fo:block>
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

        <xsl:call-template name="supplemental-illustration-report"/>

      </fo:page-sequence>

      <!-- Column Definition Page - begins here -->
      <fo:page-sequence master-reference="column-definitions">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader">
          </xsl:call-template>
          <fo:block><fo:leader></fo:leader></fo:block>
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
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold" text-align="center">
              Column Definitions
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold">
              <xsl:text>Policy Year</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The number of years the policy is assumed to have been in force.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/Composite!='1'">
              <fo:block font-weight="bold">
                <xsl:text>End of Year Age</xsl:text>
              </fo:block>
              <fo:block>
                <xsl:text>The issue age of the insured plus the number of completed policy years since the policy date.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block font-weight="bold">
              <xsl:text>Premium Outlay</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The total amount the premium payor plans to pay each year, assuming that each payment is made at the beginning of the premium paying period.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold">
              <xsl:text>Net Premium</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The Net Premium is the Premium Outlay less any premium loads.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold">
              <xsl:text>Cash Surrender Value</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The Cash Surrender Value is the amount payable to a policy owner upon surrender of the policy.  It is equal to the Account Value less any surrender charges, if applicable, and less any policy debt and any unpaid monthly charges.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold">
              <xsl:text>Death Benefit</xsl:text>
            </fo:block>
            <fo:block>
              <xsl:text>The Death Benefit is the net amount paid to a beneficiary following receipt of due proof of death.  The Death Benefit is equal to the benefit provided by the death benefit option in effect on the date of death less any policy debt and any unpaid monthly charges.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- EXPLANATORY NOTES - begins here -->
      <fo:page-sequence master-reference="explanatory-notes">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader">
          </xsl:call-template>
          <fo:block><fo:leader></fo:leader></fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Explanatory Notes Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="9.75pt" font-family="sans-serif">
            <fo:block font-weight="bold" text-align="center">
              <xsl:text>IMPORTANT TAX DISCLOSURE</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>As illustrated, this contract </xsl:text>
              <xsl:choose>
                <xsl:when test="illustration/scalar/IsMec='1'">
                  <xsl:text>becomes </xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>would not become </xsl:text>
                </xsl:otherwise>
              </xsl:choose>
              <xsl:text>a Modified Endowment Contract (MEC) under the Internal Revenue Code</xsl:text>
              <xsl:choose>
                <xsl:when test="illustration/scalar/IsMec='1'">
                  <xsl:text> in year </xsl:text>
                  <xsl:value-of select="illustration/scalar/MecYear+1"/>
                  <xsl:text>. </xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>. </xsl:text>
                </xsl:otherwise>
              </xsl:choose>
              <xsl:text>To the extent of gain in the contract, loans, distributions and withdrawals from a MEC are subject to income tax and may also trigger a penalty tax.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/IsInforce!='1'">
              <fo:block>
                <xsl:text>The initial 7-pay premium limit is $</xsl:text>
                <xsl:value-of select="illustration/scalar/InitSevenPayPrem"/>
                <xsl:text>. </xsl:text>
              </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <xsl:text>No tax charge is made to the Separate Account. However, such a charge may be made in the future.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold">
              <xsl:text>The information contained in this illustration is not written or intended as tax or legal advice, and may not be relied upon for purposes of avoiding any federal tax penalties. Neither </xsl:text>
                  <xsl:value-of select="illustration/scalar/InsCoShortName"/>
              <xsl:text> nor any of its employees or representatives are authorized to give tax or legal advice.  For more information pertaining to the tax consequences of purchasing or owning this policy, consult with your own independent tax or legal counsel.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold" text-align="center">
              <xsl:text>EXPLANATORY NOTES</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
              <xsl:text> is a </xsl:text>
              <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
              <xsl:text> issued by </xsl:text>
              <xsl:value-of select="illustration/scalar/InsCoName"/>
              <xsl:text>. The policy provides lifetime insurance protection for as long as it remains in force. The policy is available at issue with at least two death benefit options: Option 1 (death benefit equal to the greater of (a) the selected face amount at death or (b) the minimum face amount at death); and Option 2 (death benefit equal to the greater of (a) the selected face amount plus account value at death or (b) the minimum face amount at death). If available under the policy, Option 3 is a death benefit option equal to the greatest of (a) the selected face amount at death plus the sum of premiums paid less withdrawals; or (b) selected face amount at death; or (c) minimum face amount at death.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>Account values may be used to pay contract charges. Contract charges are due during the life of the insured; depending on actual results, the premium payer may need to continue or resume premium outlays.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/NoLapse='1' and illustration/scalar/StatePostalAbbrev!='NY'">
              <fo:block>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text>: The </xsl:text>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text> is a lapse protection feature. If met, this test allows your contract to stay in force for a period of time even if there is insufficient </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value to cover the </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value charges. Refer to your policy for specific requirements of meeting the </xsl:text>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text>.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/NoLapseAlwaysActive='1'">
              <fo:block>
                <xsl:text>No-Lapse Guarantee: The contract will remain in force after the first premium has been paid, even if there is insufficient </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value to cover the monthly charges provided that the insured is not in a substandard rating class and the policy debt does not exceed </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
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
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/InsCoName"/>
              <xsl:text> has the right to promptly refund any amount of premium paid if it will increase the net amount at risk </xsl:text>
              <xsl:text>(referred to in the contract as the Amount of Insurance that Requires a Charge).</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>Premium payments are assumed to be made at the beginning of the year. Account values, cash surrender values, and death benefits are illustrated as of the end of the year.</xsl:text>
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!--  EXPLANATORY NOTES page 2 - begins here -->
      <fo:page-sequence master-reference="explanatory-notes-separate-account">
        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader">
          </xsl:call-template>
          <fo:block><fo:leader></fo:leader></fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="footer-with-underwriter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Explanatory Notes Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block span="all" text-align="left" font-size="9.75pt" font-family="sans-serif">
            <fo:block font-weight="bold" text-align="center">
              <xsl:text>EXPLANATORY NOTES</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>This contract has a guaranteed maximum cost of insurance (based on 1980 CSO mortality tables) and maximum administrative charges. The actual current charges are lower than these and are reflected in the values. However, these current charges are subject to change.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>This illustration assumes death of the insured at age </xsl:text>
              <xsl:value-of select="illustration/scalar/EndtAge"/>
              <xsl:text>.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/StatePostalAbbrev='NC' or illustration/scalar/StatePostalAbbrev='SC'">
              <fo:block>
                <xsl:text>In the states of North Carolina and South Carolina, Guaranteed Issue Underwriting is referred to as "Limited Underwriting" and Simplified Issue Underwriting is referred to as "Simplified Underwriting".</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <xsl:text>The loan interest rate may be fixed or adjustable as elected by the sponsor.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>This illustration must be preceded or accompanied by the current prospectuses for </xsl:text>
              <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
              <xsl:text> variable life insurance contract and its underlying investment choices. Before purchasing a variable life insurance contract, investors should carefully consider the investment objectives, risks, charges and expenses of the variable life insurance contract and its underlying investment choices. Please read the prospectuses carefully before investing or sending money.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/HasTerm='1'">
              <fo:block>
                <xsl:text>A Term Rider is available for attachment to this policy. The Term Rider provides the option to purchase monthly term insurance on the life of the insured. </xsl:text>
                <xsl:text>The term rider selected face amount supplements the selected face amount of the contract. If the Term Rider is attached, the policy to which it is attached may have a lower annual cutoff premium and, as a result, the lower overall sales loads paid may be lower than a contract having the same total face amount, but with no Term Rider. Also, the lapse protection feature of the contract's</xsl:text>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text> does not apply to the Term Rider's selected face amount.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasWP='1'">
              <fo:block>
                <xsl:text>A Waiver of Monthly Charges rider is available for attachment to this policy for insureds with ages 20-64. The Waiver of Monthly Charges Rider provides that in the event of the disability of the insured that begins before attained age 65 and continues for at least 6 months, </xsl:text>
                <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                <xsl:text> will waive certain monthly charges up to age 65, but not less than two years, while the insured remains totally disabled. An additional charge is associated with this rider, if elected. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/UsePartialMort='1'">
              <fo:block>
                <xsl:text>This illustration reflects the client's mortality assumption of </xsl:text>
                  <xsl:value-of select="illustration/data/newcolumn/column[@name='PartMortTableMult']/duration[1]/@column_value"/>
                <xsl:text> of the </xsl:text>
                <xsl:value-of select="illustration/scalar/PartMortTableName"/>
                <xsl:text> table with all deaths at the end of the year.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:choose>
              <xsl:when test="illustration/scalar/GenderBlended='1'">
                <fo:block>
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
                  <fo:block><fo:leader></fo:leader></fo:block>
                </fo:block>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="illustration/scalar/SmokerBlended='1'">
                  <fo:block>
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
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block>
              <xsl:text>The state of issue is </xsl:text>
              <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
              <xsl:text>.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:choose>
              <xsl:when test="illustration/scalar/IsInforce!='1'">
                <xsl:if test="string-length(illustration/scalar/InsCoPhone) &gt; 14">
                  <fo:block>
                    <xsl:text>Compliance tracking number: </xsl:text>
                    <xsl:value-of select="substring(illustration/scalar/InsCoPhone, 1, 15)"/>
                  </fo:block>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="string-length(illustration/scalar/InsCoPhone) &gt; 16">
                  <fo:block>
                    <xsl:text>Compliance tracking number: </xsl:text>
                    <xsl:value-of select="substring(illustration/scalar/InsCoPhone, 16)"/>
                  </fo:block>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block font-weight="bold" text-align="center">
              <xsl:text>SEPARATE ACCOUNT</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>This illustration shows how the death benefit and account value could vary over an extended period of time, assuming the funds experience </xsl:text>
              <xsl:text>hypothetical gross rates of investment return. Actual results of return may be more or less than those shown and in all likelihood will vary </xsl:text>
              <xsl:text>year to year. Timing of premium payments, investment allocations and withdrawals or loans, if taken, may impact investment results. </xsl:text>
              <xsl:text>Separate Account Charges are deducted from the gross investment rate to determine the net investment rate. These charges include a mortality </xsl:text>
              <xsl:text>and expense charge, and an investment management fee and other fund operating expenses. The total fund operating expenses charged vary </xsl:text>
              <xsl:text>by fund. SEE PROSPECTUS.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:if test="illustration/scalar/Composite!='1'">
                <fo:block>
                  <xsl:text>This illustration is based on total Separate Account fund expenses of </xsl:text>
                  <xsl:value-of select="illustration/data/newcolumn/column[@name='TotalIMF']/duration[1]/@column_value"/>
                  <xsl:text>.</xsl:text>
                </fo:block>
                <fo:block><fo:leader></fo:leader></fo:block>
                <xsl:if test="illustration/scalar/AvgFund='1'">
                  <fo:block>
                    <xsl:text>Investment management fees are based on equal initial allocations among the available funds.</xsl:text>
                  </fo:block>
                </xsl:if>
                <xsl:if test="illustration/scalar/CustomFund='1'">
                  <fo:block>
                    <xsl:text>Investment management fees are based on an inital allocation of 100% of a custom fund selected by the purchaser.</xsl:text>
                  </fo:block>
                </xsl:if>
                <fo:block><fo:leader></fo:leader></fo:block>
              </xsl:if>
            </fo:block>
          </fo:block>
          <xsl:choose>
            <xsl:when test="$has_supplemental_report">
            </xsl:when>
            <xsl:when test="illustration/scalar/Composite!='1'">
            </xsl:when>
            <xsl:otherwise>
              <fo:block id="endofdoc"></fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </fo:flow>
      </fo:page-sequence>

      <!-- Illustration Assumption Detail -->
      <xsl:if test="illustration/scalar/Composite!='1'">
        <!-- Body page -->
        <fo:page-sequence master-reference="illustration-assumption">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <xsl:call-template name="standardheader">
              <xsl:with-param name="reporttitle" select="'Illustration Assumption Detail'"/>
            </xsl:call-template>
            <fo:block><fo:leader></fo:leader></fo:block>
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

          <xsl:call-template name="illustration-assumption-report"/>

        </fo:page-sequence>
      </xsl:if>

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
            <fo:block><fo:leader></fo:leader></fo:block>
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
          <xsl:call-template name="supplemental-report-body" />
        </fo:page-sequence>
      </xsl:if>
    </fo:root>
  </xsl:template>

  <xsl:template name="standardheader">
    <xsl:param name="reporttitle"/>
    <fo:block text-align="center" font-size="9.75pt">
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block>
        The purpose of this illustration is to show how the performance of the underlying investment account could affect the contract account value and death benefit. <fo:inline font-weight="bold">These hypothetical returns do not reflect past performance and are not predictive of future results. Actual results could be less than or greater than the hypothetical results and in all likelihood will vary from year to year.</fo:inline>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block>
        <xsl:value-of select="$reporttitle"/>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
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
        <fo:table-column/>
        <fo:table-body padding-before="2pt">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="illustration/scalar/Composite='1'">
                    <fo:block>
                      <xsl:call-template name="limitstring" >
                        <xsl:with-param name="passString" select="illustration/scalar/CorpName"/>
                        <xsl:with-param name="length" select="60"/>
                      </xsl:call-template>
                    </fo:block>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:choose>
                      <xsl:when test="illustration/scalar/Insured1='  '">
                        <fo:block>
                          <xsl:text>&#xA0;</xsl:text>
                        </fo:block>
                      </xsl:when>
                      <xsl:otherwise>
                        <fo:block>
                          <xsl:call-template name="limitstring" >
                            <xsl:with-param name="passString" select="illustration/scalar/Insured1"/>
                            <xsl:with-param name="length" select="30"/>
                          </xsl:call-template>
                          <xsl:text>, </xsl:text>
                          <xsl:value-of select="illustration/scalar/Gender"/>
                          <xsl:text>&#xA0;</xsl:text>
                          <xsl:value-of select="illustration/scalar/Smoker"/>
                          <xsl:text> rates, Age </xsl:text>
                          <xsl:value-of select="illustration/scalar/Age"/>
                        </fo:block>
                      </xsl:otherwise>
                    </xsl:choose>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <fo:block text-align="left">
                <fo:block>
                  <xsl:text>Assumed Gross Rate: </xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                  <xsl:text> (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                  <xsl:text> Net)*</xsl:text>
                </fo:block>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block>
                    <xsl:text>Composite</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Contract:  </xsl:text>
                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Initial </xsl:text>
                <xsl:if test="illustration/scalar/HasTerm!='0'">
                  <xsl:text> Total</xsl:text>
                </xsl:if>
                <xsl:text> Face Amount: $</xsl:text>
                <xsl:value-of select="illustration/scalar/InitTotalSA"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block text-align="left">
                    <xsl:text>Contract:  </xsl:text>
                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Initial Premium: </xsl:text>
                    <xsl:value-of select="illustration/scalar/InitPrem"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:choose>
                  <xsl:when test="illustration/scalar/HasTerm!='0'">
                    <fo:block text-align="left">
                      <xsl:text>Initial Base Face Amount: $</xsl:text>
                      <xsl:value-of select="illustration/scalar/InitBaseSpecAmt"/>
                    </fo:block>
                  </xsl:when>
                  <xsl:otherwise>
                    <fo:block text-align="left">
                      <xsl:text>&#xA0;</xsl:text>
                    </fo:block>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
          <xsl:choose>
            <xsl:when test="illustration/scalar/Composite='1'">
              <fo:block text-align="left">
                <xsl:text>Initial Premium: </xsl:text>
                <xsl:value-of select="illustration/scalar/InitPrem"/>
              </fo:block>
            </xsl:when>
            <xsl:otherwise>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="illustration/scalar/UWType='Medical'">
                      <fo:block text-align="left">
                        <xsl:text>Fully underwritten</xsl:text>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="illustration/scalar/UWClass"/>
                        <xsl:text>, Initial Death Benefit Option: </xsl:text>
                        <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                      </fo:block>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:choose>
                        <xsl:when test="illustration/scalar/StatePostalAbbrev='TX'">
                          <xsl:choose>
                            <xsl:when test="illustration/scalar/UWType='Guaranteed issue'">
                              <fo:block text-align="left">
                                <xsl:text>Substandard *</xsl:text>
                                <xsl:text>, </xsl:text>
                                <xsl:value-of select="illustration/scalar/UWClass"/>
                                <xsl:text>, Initial Death Benefit Option: </xsl:text>
                                <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                              </fo:block>
                            </xsl:when>
                            <xsl:otherwise>
                              <fo:block text-align="left">
                                <xsl:value-of select="illustration/scalar/UWType"/>
                                <xsl:text>, </xsl:text>
                                <xsl:value-of select="illustration/scalar/UWClass"/>
                                <xsl:text>, Initial Death Benefit Option: </xsl:text>
                                <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                              </fo:block>
                            </xsl:otherwise>
                          </xsl:choose>
                        </xsl:when>
                        <xsl:otherwise>
                          <fo:block text-align="left">
                            <xsl:value-of select="illustration/scalar/UWType"/>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="illustration/scalar/UWClass"/>
                            <xsl:text>, Initial Death Benefit Option: </xsl:text>
                            <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                          </fo:block>
                        </xsl:otherwise>
                      </xsl:choose>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:otherwise>
          </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/HasTerm!='0'">
                  <fo:block text-align="left">
                    <xsl:text>Initial Term Face Amount: $</xsl:text>
                    <xsl:value-of select="illustration/scalar/InitTermSpecAmt"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;</xsl:text>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:if test="illustration/scalar/HasWP='1'">
                  <xsl:text>Waiver of Monthly Charges Rider elected.</xsl:text>
                </xsl:if>
                <xsl:text>&#xA0;</xsl:text>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="illustration/scalar/UWClass='Rated'">
                      <fo:block text-align="left">
                        <xsl:text>Table Rating: </xsl:text>
                        <xsl:value-of select="illustration/scalar/SubstandardTable"/>
                      </fo:block>
                    </xsl:when>
                    <xsl:otherwise>
                      <fo:block text-align="left">
                        <xsl:text>&#xA0;</xsl:text>
                      </fo:block>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <xsl:if test="illustration/scalar/Franchise!=''">
                    <fo:block text-align="left">
                      <xsl:text>Master contract: </xsl:text>
                      <xsl:call-template name="limitstring" >
                        <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                        <xsl:with-param name="length" select="30"/>
                      </xsl:call-template>
                    </fo:block>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="illustration/scalar/Franchise!='' and illustration/scalar/PolicyNumber!=''">
                      <fo:block text-align="left">
                        <xsl:text>Master contract: </xsl:text>
                        <xsl:call-template name="limitstring" >
                          <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                          <xsl:with-param name="length" select="15"/>
                        </xsl:call-template>
                        <xsl:text>&#xA0;&#xA0;&#xA0;Contract number: </xsl:text>
                        <xsl:call-template name="limitstring" >
                          <xsl:with-param name="passString" select="illustration/scalar/PolicyNumber"/>
                          <xsl:with-param name="length" select="15"/>
                        </xsl:call-template>
                      </fo:block>
                    </xsl:when>
                    <xsl:when test="illustration/scalar/Franchise!=''">
                      <fo:block text-align="left">
                        <xsl:text>Master contract: </xsl:text>
                        <xsl:call-template name="limitstring" >
                          <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                          <xsl:with-param name="length" select="30"/>
                        </xsl:call-template>
                      </fo:block>
                    </xsl:when>
                    <xsl:when test="illustration/scalar/PolicyNumber!=''">
                      <fo:block text-align="left">
                        <xsl:text>Contract number: </xsl:text>
                        <xsl:call-template name="limitstring" >
                          <xsl:with-param name="passString" select="illustration/scalar/PolicyNumber"/>
                          <xsl:with-param name="length" select="30"/>
                        </xsl:call-template>
                      </fo:block>
                    </xsl:when>
                  </xsl:choose>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="basic-illustration-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".6pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:choose>
          <xsl:when test="illustration/scalar/Composite='1'">
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
          </xsl:when>
          <xsl:otherwise>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AttainedAge']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
          </xsl:otherwise>
        </xsl:choose>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_GuaranteedZero']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_GuaranteedZero']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">&#xA0;</fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Guaranteed']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Guaranteed']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">&#xA0;</fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_CurrentZero']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_CurrentZero']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">&#xA0;</fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <!-- Display Only Summary Years -->
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="basic-illustration-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="supplemental-illustration-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".6pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:choose>
          <xsl:when test="illustration/scalar/Composite='1'">
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
          </xsl:when>
          <xsl:otherwise>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AttainedAge']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
          </xsl:otherwise>
        </xsl:choose>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyFee_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PremTaxLoad_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='DacTaxLoad_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='NetPmt_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='COICharge_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <!-- Display Only Summary Years -->
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="supplemental-illustration-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="illustration-assumption-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".6pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:choose>
          <xsl:when test="illustration/scalar/Composite='1'">
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='NetWD']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='NewCashLoan']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">&#xA0;</fo:block>
            </fo:table-cell>
          </xsl:when>
          <xsl:otherwise>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AttainedAge']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AnnSAIntRate_Current']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AnnGAIntRate_Current']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='CurrMandE']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='EeMode']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='ErMode']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/scalar/InitAnnLoanDueRate"/>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='MonthlyFlatExtra']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
          </xsl:otherwise>
        </xsl:choose>
      </fo:table-row>
      <!-- Display Only Summary Years -->
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="illustration-assumption-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="basic-illustration-report">
    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-header>
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
                <fo:block text-align="right"></fo:block>
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
                <xsl:text>&#xA0;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Guaranteed"/>
                  <xsl:text>&#xA0;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_CurrentZero"/>
                  <xsl:text>&#xA0;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">
                  <xsl:text>Return (</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                  <xsl:text>&#xA0;net)</xsl:text>
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell>
                <fo:block text-align="right">Policy</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell>
                    <fo:block text-align="right">Premium</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell>
                    <fo:block text-align="right">End of</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Premium</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Year</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Outlay</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Year Age</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Outlay</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block>
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
          </fo:table-header>
          <!-- Create Basic Illustration Values -->
          <xsl:choose>
            <xsl:when test="illustration/scalar/InforceYear!=0">
              <fo:table-body>
                <xsl:call-template name="basic-illustration-values">
                  <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                  <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:when>
            <xsl:otherwise>
              <fo:table-body>
                <xsl:call-template name="basic-illustration-values">
                  <xsl:with-param name="counter" select="1"/>
                  <xsl:with-param name="inforceyear" select="0"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:otherwise>
          </xsl:choose>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="supplemental-illustration-report">
    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-header>
            <fo:table-row>
              <fo:table-cell>
                <fo:block text-align="right">Policy</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell>
                    <fo:block text-align="right">Premium</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell>
                    <fo:block text-align="right">End of</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Premium</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
              <fo:table-cell>
                <fo:block text-align="right">Admin</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Premium</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">DAC</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Net</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Mortality</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Account</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Year</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Outlay</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Year Age</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Outlay</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Charge</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Tax Load</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Tax Load</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Premium</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Charge</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block>
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
          </fo:table-header>
          <!-- Create Supplemental Illustration Values -->
          <xsl:choose>
            <!-- make inforce illustration start in the inforce year -->
            <xsl:when test="illustration/scalar/InforceYear!=0">
              <fo:table-body>
                <xsl:call-template name="supplemental-illustration-values">
                  <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                  <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:when>
            <xsl:otherwise>
              <fo:table-body>
                <xsl:call-template name="supplemental-illustration-values">
                  <xsl:with-param name="counter" select="1"/>
                  <xsl:with-param name="inforceyear" select="0"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:otherwise>
                </xsl:choose>
        </fo:table>
      </fo:block>
    </fo:flow>
  </xsl:template>

  <xsl:template name="illustration-assumption-report">
    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-header>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="7" padding="0pt">
                <fo:block text-align="center">&#xA0;</fo:block>
              </fo:table-cell>
              <xsl:if test="illustration/scalar/Composite!='1'">
                <fo:table-cell>
                  <fo:block text-align="right">Assumed</fo:block>
                </fo:table-cell>
              </xsl:if>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell>
                <fo:block text-align="right">Policy</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell>
                    <fo:block text-align="right">&#xA0;</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">&#xA0;</fo:block>
                  </fo:table-cell>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell>
                    <fo:block text-align="right">End of</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="2" padding="0pt">
                    <fo:block text-align="center">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; Net Crediting Rate</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell>
                    <fo:block text-align="right">Indiv</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Corp</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Loan</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Flat Extra</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Year</fo:block>
              </fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:table-cell number-columns-spanned="3" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt"/>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Withdrawal</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt"/>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Loan</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="4" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt"/>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Year Age</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Sep Acct</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Gen Acct</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">
                      <xsl:text>M&amp;E</xsl:text>
                    </fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Pmt Mode</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Pmt Mode</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Interest</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                    <fo:block text-align="right">Per 1,000</fo:block>
                  </fo:table-cell>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block>
                </fo:block>
              </fo:table-cell>
            </fo:table-row>
          </fo:table-header>
          <!-- Create Illustration Assumption Detail Values -->
          <xsl:choose>
            <xsl:when test="illustration/scalar/InforceYear!=0">
              <fo:table-body>
                <xsl:call-template name="illustration-assumption-values">
                  <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                  <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:when>
            <xsl:otherwise>
              <fo:table-body>
                <xsl:call-template name="illustration-assumption-values">
                  <xsl:with-param name="counter" select="1"/>
                  <xsl:with-param name="inforceyear" select="0"/>
                </xsl:call-template>
              </fo:table-body>
            </xsl:otherwise>
          </xsl:choose>
        </fo:table>
      </fo:block>
      <xsl:choose>
        <xsl:when test="$has_supplemental_report">
        </xsl:when>
        <xsl:otherwise>
          <fo:block id="endofdoc"></fo:block>
        </xsl:otherwise>
      </xsl:choose>
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
    <fo:block text-align="left" font-size="8.0pt" font-family="sans-serif">
      <fo:block text-align="left">
        <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
      </fo:block>
      <fo:block text-align="left">
        <xsl:text>See the Explanatory Notes for important policy information. This illustration is not complete without all pages.</xsl:text>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
    </fo:block>
    <fo:block padding-before="5pt" font-size="8.5pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-column/>
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
                  <xsl:text>&#xA0;</xsl:text>
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
    <fo:block text-align="left" font-size="8.0pt" font-family="sans-serif">
      <fo:block span="all">
        <fo:block><fo:leader></fo:leader></fo:block>
        <fo:block>
          <xsl:text>The Principal Underwriter is </xsl:text>
          <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
          <xsl:text>, a wholly owned subsidiary of </xsl:text>
          <xsl:value-of select="illustration/scalar/InsCoName"/>
          <xsl:text> and is located at </xsl:text>
        </fo:block>
        <fo:block>
          <xsl:value-of select="illustration/scalar/MainUnderwriterAddress"/>
        </fo:block>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="left">
        <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
      </fo:block>
      <fo:block text-align="left">
        <xsl:text>See the Explanatory Notes for important policy information. This illustration is not complete without all pages.</xsl:text>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
    </fo:block>
    <fo:block padding-before="5pt" font-size="8.0pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-column/>
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
                  <xsl:text>&#xA0;</xsl:text>
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

  <xsl:template name="limitstring">
    <xsl:param name="passString"/>
    <xsl:param name="length"/>
    <xsl:choose>
      <xsl:when test="string-length($passString) &gt; $length">
        <xsl:value-of select="substring($passString,1, $length)"/>
        <xsl:text>...</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$passString"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>

