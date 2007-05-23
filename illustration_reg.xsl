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

    $Id: illustration_reg.xsl,v 1.9 2007-05-23 13:46:28 rericksberg Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:variable name="counter" select="1"/>
  <xsl:variable name="inforceyear" select="illustration/scalar/InforceYear"/>
  <xsl:variable name="column1name" select="string(illustration/supplementalreport/columns[1]/name)"/>
  <xsl:variable name="column2name" select="string(illustration/supplementalreport/columns[2]/name)"/>
  <xsl:variable name="column3name" select="string(illustration/supplementalreport/columns[3]/name)"/>
  <xsl:variable name="column4name" select="string(illustration/supplementalreport/columns[4]/name)"/>
  <xsl:variable name="column5name" select="string(illustration/supplementalreport/columns[5]/name)"/>
  <xsl:variable name="column6name" select="string(illustration/supplementalreport/columns[6]/name)"/>
  <xsl:variable name="column7name" select="string(illustration/supplementalreport/columns[7]/name)"/>
  <xsl:variable name="column8name" select="string(illustration/supplementalreport/columns[8]/name)"/>
  <xsl:variable name="column9name" select="string(illustration/supplementalreport/columns[9]/name)"/>
  <xsl:variable name="column10name" select="string(illustration/supplementalreport/columns[10]/name)"/>
  <xsl:variable name="column11name" select="string(illustration/supplementalreport/columns[11]/name)"/>
  <xsl:variable name="column12name" select="string(illustration/supplementalreport/columns[12]/name)"/>
  <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
  <xsl:variable name="allletters">ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789&#xA0;</xsl:variable>
  <xsl:variable name="noampletters">ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_</xsl:variable>
  <xsl:variable name="ModifiedSinglePremium">
    <xsl:call-template name="set_modified_single_premium"/>
  </xsl:variable>
  <xsl:variable name="SinglePremium">
    <xsl:call-template name="set_single_premium"/>
  </xsl:variable>
  <xsl:variable name="GroupExperienceRating">
    <xsl:call-template name="set_group_experience_rating"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year-text">
    <xsl:call-template name="get-max-lapse-year"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year" select="number($max-lapse-year-text)"/>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.1in" margin-bottom="0in" margin-left="0.35in" margin-right="0.35in" >
          <fo:region-body margin-top="0.25in" margin-bottom="0.10in"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="0.5in" margin-bottom="1in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent=".45in"/>
        </fo:simple-page-master>

        <!-- Define the column headings and key terms page. -->
        <fo:simple-page-master master-name="column-headings-and-key-terms" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="0.15in" margin-bottom=".45in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="2in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="0.45in"/>
        </fo:simple-page-master>

        <!-- Define the Numeric Summary page. -->
        <fo:simple-page-master master-name="numeric-summary" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.1in" margin-bottom=".52in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="0.52in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail page. -->
        <fo:simple-page-master master-name="tabular-detail" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.1in" margin-bottom="1.20in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail (Report 2) page. -->
        <fo:simple-page-master master-name="tabular-detail-report2" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.2in" margin-bottom="1.25in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="illustration/scalar/SupplementalReport='1'">
          <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
            <!-- Central part of page -->
            <fo:region-body column-count="1" margin-top="3.0in" margin-bottom="1.25in"/>
            <!-- Header -->
            <fo:region-before border-after-style="solid" extent="3.0in"/>
            <!-- Footer -->
            <fo:region-after border-before-style="solid" extent="1.27in"/>
          </fo:simple-page-master>
        </xsl:if>

        <!-- Define the Numeric Summary Attachment page. -->
        <fo:simple-page-master master-name="numeric-summary-attachment" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
          <!-- Central part of page -->
          <fo:region-body column-count="1" margin-top="3.1in" margin-bottom=".52in"/>
          <!-- Header -->
          <fo:region-before border-after-style="solid" extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after border-before-style="solid" extent="0.52in"/>
        </fo:simple-page-master>

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

      <!-- NARRATIVE SUMMARY - begins here -->
      <fo:page-sequence master-reference="narrative-summary"  initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Narrative Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <xsl:call-template name="standardheader"/>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block text-align="center" font-size="10pt">
            <xsl:text>NARRATIVE SUMMARY</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <fo:block>
                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                    <xsl:text> is a</xsl:text>
                    <!-- Group Experience Rating Logic -->
                    <xsl:if test="$GroupExperienceRating='1'">
                      <xsl:text> group</xsl:text>
                    </xsl:if>
                    <xsl:text> flexible premium adjustable life insurance contract. </xsl:text>
                    <!-- Group Experience Rating Logic -->
                    <xsl:if test="$GroupExperienceRating='1'">
                      <xsl:text> It is a no-load policy and is intended for large case sales. It is primarily marketed to financial institutions to fund certain corporate liabilities. </xsl:text>
                    </xsl:if>
                    <xsl:text> It features accumulating account values, adjustable benefits, and flexible premiums.</xsl:text>
                  </fo:block>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="$ModifiedSinglePremium='1'">
                      <fo:block>
                        <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                        <xsl:text> is a modified single premium adjustable life insurance contract. </xsl:text>
                        <xsl:text> It features accumulating account values, adjustable benefits, and single premium.</xsl:text>
                      </fo:block>
                      <fo:block><fo:leader></fo:leader></fo:block>
                    </xsl:when>
                    <xsl:otherwise>
                      <fo:block>
                        <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                        <xsl:text> is a single premium adjustable life insurance contract. </xsl:text>
                        <xsl:text> It features accumulating account values, adjustable benefits, and single premium.</xsl:text>
                      </fo:block>
                      <fo:block><fo:leader></fo:leader></fo:block>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:otherwise>
              </xsl:choose>
            <fo:block>
              <xsl:text>Coverage may be available on a Guaranteed Standard Issue basis. All proposals are based on case characteristics and must be approved by the </xsl:text>
              <xsl:value-of select="illustration/scalar/InsCoShortName"/>
              <xsl:text> Home Office. For details regarding underwriting and coverage limitations refer to your offer letter or contact your </xsl:text>
              <xsl:value-of select="illustration/scalar/InsCoShortName"/>
              <xsl:text> representative.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>This is an illustration only. An illustration is not intended to predict actual performance. Interest rates </xsl:text>
              <xsl:if test="illustration/scalar/Participating='1'">
                <xsl:text>, dividends,</xsl:text>
              </xsl:if>
              <xsl:text> and values set forth in the illustration are not guaranteed.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Group Experience Rating Logic -->
            <xsl:choose>
              <xsl:when test="illustration/scalar/StatePostalAbbrev!='TX'">
                <fo:block>
                  <xsl:text>This illustration assumes that the currently illustrated non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be more or less favorable than shown. The non-guaranteed benefits and values are not guaranteed and are based on assumptions such as interest credited and current monthly charges, which are subject to change by </xsl:text>
                  <xsl:value-of select="illustration/scalar/InsCoName"/>
                  <xsl:text>.</xsl:text>
                </fo:block>
              </xsl:when>
              <xsl:otherwise>
                <fo:block>
                  <xsl:text>This illustration is based on both non-guaranteed and guaranteed assumptions. Non-guaranteed assumptions include interest rates and monthly charges.  This illustration assumes that the currently illustrated non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be more or less favorable than shown. Factors that may affect future policy performance include the company's expectations for future mortality, investments, persistency, profits and expenses.</xsl:text>
                </fo:block>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:value-of select="illustration/scalar/AvName"/>
              <xsl:text> Values may be used to pay monthly charges. Monthly charges are due during the life of the insured, and depending on actual results, the premium payor may need to continue or resume premium outlays.</xsl:text>
              <fo:block><fo:leader></fo:leader></fo:block>
            </fo:block>
            <fo:block>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:text>Premiums are assumed to be paid on a</xsl:text>
                  <xsl:if test="illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value='Annual'">
                    <xsl:text>n </xsl:text>
                  </xsl:if>
                  <xsl:value-of select="translate(illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                  <xsl:text> basis and received at the beginning of the contract year. </xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>The single premium is assumed to be paid at the beginning of the contract year. </xsl:text>
                </xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="illustration/scalar/AvName"/>
              <xsl:text> Values, </xsl:text>
              <xsl:value-of select="illustration/scalar/CsvName"/>
              <xsl:text> Values, and death benefits are illustrated as of the end of the contract year.  The method we use to allocate overhead expenses is the fully allocated expense method.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1'">
              <fo:block>
                <xsl:text>In order to guarantee coverage to age </xsl:text>
                <xsl:value-of select="illustration/scalar/EndtAge"/>
                <xsl:text>, a</xsl:text>
                <xsl:if test="illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value='Annual'">
                  <xsl:text>n</xsl:text>
                </xsl:if>
                <xsl:text> </xsl:text>
                <xsl:value-of select="translate(illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                <xsl:text> premium</xsl:text>
                <xsl:choose>
                  <xsl:when test="illustration/scalar/GuarPrem!='0'">
                    <xsl:text> of $</xsl:text>
                    <xsl:value-of select="illustration/scalar/GuarPrem"/>
                    <xsl:text> must be paid. </xsl:text>
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:text> is defined. </xsl:text>
                  </xsl:otherwise>
                </xsl:choose>
                <xsl:text>This amount is based on the guaranteed monthly charges and the guaranteed interest crediting rate.</xsl:text>
                <xsl:if test="illustration/scalar/DefnLifeIns='GPT'">
                  <xsl:text>  This premium may be in excess of the maximum premium allowable in order to qualify this policy as life insurance.</xsl:text>
                </xsl:if>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <xsl:text>Loaned amounts of the </xsl:text>
              <xsl:value-of select="illustration/scalar/AvName"/>
              <xsl:text> Value will be credited a rate equal to the loan interest rate less a spread, guaranteed not to exceed 3.00%.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/HasTerm='1'">
              <fo:block>
                <xsl:text>The term rider provides the option to purchase monthly term insurance on the life of the insured. The term rider selected face amount supplements the selected face amount of the contract. If the term rider is attached, the policy to which it is attached may have a lower annual cutoff premium and, as a result, the lower overall sales loads paid may be lower than a contract having the same total face amount, but with no term rider. </xsl:text>
                <xsl:if test="illustration/scalar/NoLapse='1'">
                  <xsl:text> Also, the lapse protection feature of the contract's </xsl:text>
                  <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                  <xsl:text> does not apply to the term rider's selected face amount.</xsl:text>
                </xsl:if>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasWP='1'">
              <fo:block>
                <xsl:text>The Waiver of Monthly Charges Rider provides for waiver of monthly charges in the event of the disability of the insured that begins before attained age 65 and continues for at least 6 months, as described in the rider. An additional charge is associated with this rider. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasADD='1'">
              <fo:block>
                <xsl:text>The Accidental Death benefit provides an additional benefit if death is due to accident. An additional charge is associated with this rider. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <xsl:text>The definition of life insurance for this contract is the </xsl:text>
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
          </fo:block>>

          <!-- Force Second Page -->
          <fo:block break-after="page"/>
          <fo:block text-align="center" font-size="10pt">
            <xsl:text>NARRATIVE SUMMARY (Continued)</xsl:text>
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/SalesLoadRefund!='0%'">
              <fo:block>
                <xsl:text>Sales Load Refund: We will refund a portion of the sales load to you, as part of your </xsl:text>
                <xsl:value-of select="illustration/scalar/CsvName"/>
                <xsl:text> Value, if you surrender your contract within the first two policy years. In policy year 1, we will refund </xsl:text>
                <xsl:value-of select="illustration/scalar/SalesLoadRefundRate0"/>
                <xsl:text> of the first contract year sales load collected and in contract year 2, we will refund </xsl:text>
                <xsl:value-of select="illustration/scalar/SalesLoadRefundRate1"/>
                <xsl:text> of the first contract year sales load collected.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/NoLapse='1'">
              <fo:block>
                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                <xsl:text>: The contract will remain in force after the first premium has been paid, even if there is insufficient </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value to cover the monthly charges provided that the insured is not in a substandard rating class and the policy debt does not exceed </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>

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
            <fo:block>
              <xsl:text>The loan interest rate is fixed at </xsl:text>
              <xsl:value-of select="illustration/scalar/InitAnnLoanDueRate"/>
              <xsl:text> per year.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <xsl:text>The state of issue is </xsl:text>
              <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
              <xsl:text>.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/Composite='1'">
              <fo:block>
                <xsl:text>Please see the attached census, listing the face amounts, underwriting classes and issue ages for individual participants.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/StatePostalAbbrev='NC' or illustration/scalar/StatePostalAbbrev='SC'">
              <fo:block>
                <xsl:text>In the states of North Carolina and South Carolina, Guaranteed Issue Underwriting is referred to as "Limited Underwriting" and Simplified Issue Underwriting is referred to as "Simplified Underwriting".</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/StatePostalAbbrev='TX'">
              <xsl:if test="illustration/scalar/UWType='Guaranteed issue'">
                <fo:block>
                  <xsl:text>* This policy is classified as substandard guaranteed issue per the requirements of the Texas Insurance Department.</xsl:text>
                </fo:block>
                <fo:block><fo:leader></fo:leader></fo:block>
              </xsl:if>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block>
                <xsl:text>We may assess a Market Value Adjustment upon a surrender of the certificate when the surrender proceeds are intended to be applied to an insurance policy issued by an insurer unaffilliated with MML Bay State with an intent to qualify the exchange as a tax free exchange under IRC section 1035.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
              <xsl:if test="illustration/scalar/UseExperienceRating!='1'">
                <fo:block>
                  <xsl:text>This illustration does not reflect experience rating.</xsl:text>
                </fo:block>
                <fo:block><fo:leader></fo:leader></fo:block>
              </xsl:if>
              <fo:block>
                <xsl:text>The guaranteed values reflect the maximum charges permitted by the contract, which may include an Experience Rating Risk Charge.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
              <fo:block>
                <xsl:text>No Experience Rating Risk Charge or a distribution of an Experience Rating Reserve Credit is reflected in the current, non-guaranteed values. Actual charges and credits will be based on the actual experience of the group.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/Has1035ExchCharge='1'">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:if test="illustration/scalar/Has1035ExchCharge='1'">
                    <fo:block>
                      <xsl:text>Upon surrender of this policy, where the surrender proceeds are intended to be applied to an insurance policy or certificate issued in conjunction with an intent to qualify the exchange as a tax free exchange under Section 1035 of the Internal Revenue Code, we may assess an Exchange Charge. The Exchange Charge is the greater of the Market Value Adjustment Charge and the Percentage of Premium Charge. In the states of Florida or Indiana, the Exchange charge (referred to as Assignment Charge in Florida) will be the Percentage of Premium Charge only. The Exchange Charge will potentially reduce the surrender proceeds, but will never increase the surrender proceeds. Please refer to your contract for details.</xsl:text>
                    </fo:block>
                    <fo:block><fo:leader></fo:leader></fo:block>
                  </xsl:if>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block>
                    <xsl:text>Upon surrender of this policy, where the surrender proceeds are intended to be applied to an insurance policy or certificate issued in conjunction with an intent to qualify the exchange as a tax free exchange under Section 1035 of the Internal Revenue Code, we may assess an Exchange Charge. The Exchange Charge will potentially reduce the surrender proceeds, but will never increase the surrender proceeds. Please refer to your contract for details.</xsl:text>
                  </fo:block>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasSpouseRider='1'">
              <fo:block>
                <xsl:text>The $</xsl:text>
                <xsl:value-of select="illustration/scalar/SpouseRiderAmount"/>
                <xsl:text> Spouse rider provides term life insurance on the spouse (issue age </xsl:text>
                <xsl:value-of select="illustration/scalar/SpouseIssueAge"/>
                <xsl:text>) for a limited duration, for an extra charge.  Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/HasChildRider='1'">
              <fo:block>
                <xsl:text>The $</xsl:text>
                <xsl:value-of select="illustration/scalar/ChildRiderAmount"/>
                <xsl:text> Child rider provides term life insurance on the insured's children for a limited duration, for an extra charge.  Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block font-weight="bold" text-align="center">
              <xsl:text>IMPORTANT TAX DISCLOSURE</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
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
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>This contract is a Modified Endowment Contract (MEC) under the Internal Revenue Code. </xsl:text>
                  <xsl:text> To the extent of gain in the contract, loans, distributions and withdrawals from a MEC are subject to income tax and may also trigger a penalty tax.</xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1'">
              <xsl:if test="illustration/scalar/IsInforce!='1'">
                <fo:block>
                  <xsl:text>The initial 7-pay premium limit is $</xsl:text>
                  <xsl:value-of select="illustration/scalar/InitSevenPayPrem"/>
                  <xsl:text>. </xsl:text>
                </fo:block>
                <fo:block><fo:leader></fo:leader></fo:block>
              </xsl:if>
            </xsl:if>
            <fo:block font-weight="bold">
              <xsl:text>The information contained in this illustration is not written or intended as tax or legal advice, and may not be relied upon for purposes of avoiding any federal tax penalties. Neither </xsl:text>
                  <xsl:value-of select="illustration/scalar/InsCoShortName"/>
              <xsl:text> nor any of its employees or representatives are authorized to give tax or legal advice.  For more information pertaining to the tax consequences of purchasing or owning this policy, consult with your own independent tax or legal counsel.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
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
                    <xsl:text>Compliance Tracking Number: </xsl:text>
                    <xsl:value-of select="substring(illustration/scalar/InsCoPhone, 16)"/>
                  </fo:block>
                  <fo:block><fo:leader></fo:leader></fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- Column Headings and Key Terms - begins here -->
      <fo:page-sequence master-reference="column-headings-and-key-terms">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="displaypagenumber" select="1"/>
          </xsl:call-template>
        </fo:static-content>

        <!-- Column Headings and Key Terms Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block text-align="center" font-size="10.0pt">
            <xsl:text>Column Headings and Key Terms Used in This Illustration</xsl:text>
          </fo:block>
          <fo:block><fo:leader></fo:leader></fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block font-weight="bold">
              <xsl:value-of select="illustration/scalar/AvName"/>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                   Value: <fo:inline font-weight="normal">The accumulation at interest of the net premiums paid, less any withdrawals, less any monthly charges deducted.</fo:inline>
                </xsl:when>
                <xsl:otherwise>
                   Value: <fo:inline font-weight="normal">The accumulation at interest of the net premiums paid, less any monthly charges deducted.</fo:inline>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">
                <xsl:value-of select="illustration/scalar/CsvName"/>
                 Value:
              </fo:inline>
              <xsl:value-of select="illustration/scalar/AvName"/>
              <xsl:text> Value less policy debt.</xsl:text>
              <xsl:text> The </xsl:text>
              <xsl:value-of select="illustration/scalar/CsvName"/>
              <xsl:text> Value does not reflect an Exchange Charge, which may be assessed under the policy where surrender proceeds are intended to be applied to an insurance policy or certificate issued with an intent to qualify the exchange as a tax free exchange under Section 1035 of the Internal Revenue Code.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Current Values:</fo:inline>  Values assuming current interest crediting rates and current monthly charges. These values are not guaranteed and are based on the assumption that premium is paid as illustrated.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Death Benefit:</fo:inline>  The amount of benefit provided by the Death Benefit Option in effect on the date of death, prior to adjustments for policy debt and monthly charges payable to the date of death.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Death Benefit Option 1:</fo:inline>  Option in which the death benefit is equal to the selected face amount of the contract on the date of death of the insured, or if greater the
              <xsl:value-of select="illustration/scalar/AvName"/>
              <xsl:text> Value </xsl:text>
              <xsl:if test="illustration/scalar/SalesLoadRefund!='0%'">
                <xsl:text>plus the refund of sales loads (if applicable) </xsl:text>
              </xsl:if>
              <xsl:text>on the insured's date of death multiplied by the minimum face amount percentage for the insured's attained age at death (minimum face amount). Please refer to the contract for a detailed schedule of death benefit factors.</xsl:text>
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <fo:block>
                <fo:inline font-weight="bold">Death Benefit Option 2:</fo:inline>  Option in which the death benefit is equal to the selected face amount of the contract plus the
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value on the date of death of the insured, or if greater, the </xsl:text>
                <xsl:value-of select="illustration/scalar/AvName"/>
                <xsl:text> Value </xsl:text>
                <xsl:if test="illustration/scalar/SalesLoadRefund!='0%'">
                  <xsl:text>plus the refund of sales loads (if applicable) </xsl:text>
                </xsl:if>
                <xsl:text>on the insured's date of death multiplied by the death benefit factor for the insured's attained age at death (minimum face amount). Please refer to the contract for a detailed schedule of death benefit factors.</xsl:text>
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block>
                <fo:inline font-weight="bold">Experience Rating Risk Charge:</fo:inline>  Applies only to certain experience rated groups. This charge is based on the cost of insurance charges assessed during the certificate year. It may be assessed against the account value once per certificate anniversary date and upon surrender of the group policy.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <fo:block>
                <fo:inline font-weight="bold">Exchange Charge:</fo:inline>  Where surrender proceeds are intended to be applied to an insurance policy or certificate issued with an intent to qualify the exchange as a tax free exchange under Section 1035 of the Internal Revenue Code, there is a potential reduction in surrender proceeds.  Please see the contract endorsement for a detailed description of the Exchange Charge.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <fo:block>
                <fo:inline font-weight="bold">Flexible Premiums:</fo:inline>  Premiums that may be increased, reduced, or not paid, if the account value is sufficient to cover the monthly charges.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Guaranteed Values:</fo:inline>  Values assuming the guaranteed crediting rate and the guaranteed maximum monthly charges. These values are based on the assumption that premium is paid as illustrated.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <xsl:if test="illustration/scalar/IsInforce!='1'">
              <fo:block>
                <fo:inline font-weight="bold">Initial Illustrated Crediting Rate:</fo:inline>  The current interest rate illustrated for the first policy year.  This rate is not guaranteed and is subject to change by
                <xsl:value-of select="illustration/scalar/InsCoName"/>.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <fo:inline font-weight="bold">MEC:</fo:inline>  Modified Endowment Contract - this classification is given to a contract in violation of TAMRA (Technical and Miscellaneous Revenues Act), which limits the amount of premium that can be paid into a life insurance contract. To the extent of gain in the contract, loans, distributions and withdrawals from a MEC are subject to income tax and may also trigger a tax penalty.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Midpoint Values:</fo:inline>  Values assuming interest rates that are the average of the illustrated current crediting rates and the guaranteed minimum interest rate, and monthly charges that are the average of the current monthly charges and the guaranteed monthly charges. These values are not guaranteed and are based on the assumption that premium is paid as illustrated.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$ModifiedSinglePremium='1'">
              <fo:block>
                <fo:inline font-weight="bold">Modified Single Premium:</fo:inline> After the single premium is paid, additional payment under this policy will only be accepted for repayment of policy debt, payment required to keep the policy from lapsing, or payment required to reinstate the policy.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <fo:block>
              <fo:inline font-weight="bold">Monthly Charges:</fo:inline> The monthly charges for the following month which include: cost of insurance, plus face amount charges (if applicable), plus the administrative charge shown on the contract schedule page.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <fo:block>
              <fo:inline font-weight="bold">Premium Outlay:</fo:inline>  The amount of premium assumed to be paid by the contract owner or other premium payor.
            </fo:block>
            <fo:block><fo:leader></fo:leader></fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1' and $ModifiedSinglePremium!='1'">
              <fo:block>
                <fo:inline font-weight="bold">Single Premium:</fo:inline> After the single premium is paid, additional payment under this policy will only be accepted for repayment of policy debt, payment required to keep the policy from lapsing, or payment required to reinstate the policy.
              </fo:block>
              <fo:block><fo:leader></fo:leader></fo:block>
            </xsl:if>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1'">
              <fo:block>
                <fo:inline font-weight="bold">Ultimate Illustrated Crediting Rate:</fo:inline>
                <xsl:text>  The current interest rate illustrated for policy years 6 and later.  The illustrated crediting rates for policy years 2 through 5 are based on a blend of the Initial and Ultimate Illustrated Crediting Rates.  These rates are not guaranteed and are subject to change by </xsl:text>
                <xsl:value-of select="illustration/scalar/InsCoName"/>
                <xsl:text>.</xsl:text>
              </fo:block>
            </xsl:if>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>
      <xsl:if test="illustration/scalar/IsInforce!='1'">
        <!-- Numeric Summary (only for new business)-->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
              <xsl:text>Numeric Summary</xsl:text>
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
          <xsl:call-template name="numeric-summary-report"/>
        </fo:page-sequence>
      </xsl:if>

      <!-- Tabular Detail -->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
            <xsl:text>Tabular Detail</xsl:text>
          </fo:block>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
            <xsl:text>
              The Non-Guaranteed Values depicted above reflect interest rates described in the Tabular Detail, and current monthly charges. These values
              are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
              illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
              more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
              include the company's expectations for future mortality, investments, persistency, profits and expenses.
            </xsl:text>
          </fo:block>
          <fo:block>
            <xsl:call-template name="standardfooter">
              <xsl:with-param name="displaypagenumber" select="1"/>
            </xsl:call-template>
          </fo:block>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-size="9.0pt" font-family="serif">
            <fo:table table-layout="fixed" width="100%">
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column  column-width="2mm"/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column  column-width="2mm"/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-header>
                <fo:table-row>
                  <fo:table-cell number-columns-spanned="4" padding="0pt"></fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell number-columns-spanned="3" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="center">Guaranteed Values</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell number-columns-spanned="3" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="center">Non-Guaranteed Values</fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <fo:table-row>
                  <fo:table-cell padding="1pt">
                    <fo:block>
                    </fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <fo:table-row>
                  <fo:table-cell>
                    <fo:block text-align="right">Policy</fo:block>
                  </fo:table-cell>
                  <xsl:choose>
                    <!-- make inforce illustration start in the inforce year -->
                    <xsl:when test="illustration/scalar/Composite!='1'">
                      <fo:table-cell>
                        <fo:block text-align="right">End of</fo:block>
                      </fo:table-cell>
                    </xsl:when>
                    <xsl:otherwise>
                      <fo:table-cell>
                        <fo:block text-align="right">&#xA0;</fo:block>
                      </fo:table-cell>
                    </xsl:otherwise>
                  </xsl:choose>
                  <fo:table-cell>
                    <fo:block text-align="right">Premium</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell>
                    <fo:block text-align="right">Account</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Cash Surr</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Death </fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell>
                    <fo:block text-align="right">Account</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Cash Surr</fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right">Death </fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <fo:table-row>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Year</fo:block>
                  </fo:table-cell>
                  <xsl:choose>
                    <!-- make inforce illustration start in the inforce year -->
                    <xsl:when test="illustration/scalar/Composite!='1'">
                      <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                        <fo:block text-align="right">Year Age</fo:block>
                      </fo:table-cell>
                    </xsl:when>
                    <xsl:otherwise>
                      <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                        <fo:block text-align="right">&#xA0;</fo:block>
                      </fo:table-cell>
                    </xsl:otherwise>
                  </xsl:choose>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Outlay</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Value</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Value</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Benefit</fo:block>
                  </fo:table-cell>
                  <fo:table-cell number-columns-spanned="1" padding="0pt"/>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Value</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Value</fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
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
              <!-- Create Tabular Detail Values -->
              <xsl:choose>
                <!-- make inforce illustration start in the inforce year -->
                <xsl:when test="illustration/scalar/InforceYear!=0">
                  <fo:table-body>
                    <xsl:call-template name="tabular-detail-values">
                      <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                      <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                    </xsl:call-template>
                  </fo:table-body>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-body>
                    <xsl:call-template name="tabular-detail-values">
                      <xsl:with-param name="counter" select="1"/>
                      <xsl:with-param name="inforceyear" select="0"/>
                    </xsl:call-template>
                  </fo:table-body>
                </xsl:otherwise>
                    </xsl:choose>
            </fo:table>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>
      <!-- Tabular Detail (Report 2)-->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail-report2">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10.0pt" space-before="5.0pt">
            <xsl:text>Tabular Detail, continued</xsl:text>
          </fo:block>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <fo:static-content flow-name="xsl-region-after">
          <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
            <xsl:text>
              The Non-Guaranteed Values depicted above reflect interest rates described in the Tabular Detail, and current monthly charges. These values
              are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
              illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
              more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
              include the company's expectations for future mortality, investments, persistency, profits and expenses.
            </xsl:text>
          </fo:block>
          <fo:block>
            <xsl:call-template name="standardfooter">
              <xsl:with-param name="displaypagenumber" select="1"/>
            </xsl:call-template>
          </fo:block>
        </fo:static-content>

        <fo:flow flow-name="xsl-region-body">
          <fo:block font-size="9.0pt" font-family="serif">
            <fo:table table-layout="fixed" width="100%">
              <fo:table-column/>
              <xsl:if test="illustration/scalar/Composite!='1'">
                <fo:table-column/>
              </xsl:if>
              <fo:table-column column-width="1.2in"/>
              <xsl:if test="illustration/scalar/Composite!='1'">
                <fo:table-column/>
              </xsl:if>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-column/>
              <fo:table-header>
                <fo:table-row>
                  <fo:table-cell>
                    <fo:block text-align="right">Policy</fo:block>
                  </fo:table-cell>
                  <xsl:if test="illustration/scalar/Composite!='1'">
                  <fo:table-cell>
                      <fo:block text-align="right">End of</fo:block>
                    </fo:table-cell>
                  </xsl:if>
                  <fo:table-cell>
                    <fo:block text-align="right">Illustrated</fo:block>
                  </fo:table-cell>
                  <xsl:if test="illustration/scalar/Composite!='1'">
                  <fo:table-cell>
                      <fo:block text-align="right">Annual Flat Extra</fo:block>
                    </fo:table-cell>
                  </xsl:if>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell>
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <fo:table-row>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Year</fo:block>
                  </fo:table-cell>
                  <xsl:if test="illustration/scalar/Composite!='1'">
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                      <fo:block text-align="right">Year Age</fo:block>
                    </fo:table-cell>
                  </xsl:if>
                  <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right">Crediting Rate</fo:block>
                  </fo:table-cell>
                  <xsl:if test="illustration/scalar/Composite!='1'">
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                      <fo:block text-align="right">per $1,000</fo:block>
                    </fo:table-cell>
                  </xsl:if>
                  <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                  <fo:table-cell border-bottom-style="none" border-bottom-width="1pt" border-bottom-color="blue">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <fo:table-row>
                  <fo:table-cell padding="2pt">
                    <fo:block>
                    </fo:block>
                  </fo:table-cell>
                </fo:table-row>
              </fo:table-header>
              <!-- Create Tabular Detail Values -->
              <xsl:choose>
                <!-- make inforce illustration start in the inforce year -->
                <xsl:when test="illustration/scalar/InforceYear!=0">
                  <fo:table-body>
                    <xsl:call-template name="tabular-detail-report2-values">
                      <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                      <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                    </xsl:call-template>
                  </fo:table-body>
                </xsl:when>
                <xsl:otherwise>
                  <fo:table-body>
                    <xsl:call-template name="tabular-detail-report2-values">
                      <xsl:with-param name="counter" select="1"/>
                      <xsl:with-param name="inforceyear" select="0"/>
                    </xsl:call-template>
                  </fo:table-body>
                </xsl:otherwise>
                    </xsl:choose>
            </fo:table>
          </fo:block>
          <!-- endofdoc block id implemented as the "otherwise" condition in
               an xsl:choose instead of xsl:if !='1' so that the XML item
             'Supplemental Report' need not exist in the XML document for
             page numbering to work properly -->
          <xsl:choose>
            <xsl:when test="illustration/scalar/SupplementalReport='1'">
            </xsl:when>
            <xsl:otherwise>
              <fo:block id="endofdoc"></fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </fo:flow>
      </fo:page-sequence>

      <!-- Supplemental Report -->
      <!-- Body page -->
      <xsl:if test="illustration/scalar/SupplementalReport='1'">
        <fo:page-sequence master-reference="supplemental-report">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="3.0pt">
              <xsl:value-of select="illustration/supplementalreport/title"/>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <fo:static-content flow-name="xsl-region-after">
            <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
              <xsl:text>
                The Non-Guaranteed Values depicted above reflect an interest rate scale described in the Tabular Detail, and current scale monthly charges. These values
                are not guaranteed and depend upon company experience. Column headings indicate whether benefits and values are guaranteed or not guaranteed. This
                illustration assumes that non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur and actual results may be
                more or less favorable than shown. Non-guaranteed elements are subject to change by the insurer. Factors that may affect future policy performance
                include the company's expectations for future mortality, investments, persistency, profits and expenses.
              </xsl:text>
            </fo:block>
            <fo:block>
              <xsl:call-template name="standardfooter">
                <xsl:with-param name="displaypagenumber" select="1"/>
              </xsl:call-template>
            </fo:block>
          </fo:static-content>

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
                <fo:table-column/>
                <fo:table-header>
                  <fo:table-row>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[1]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[2]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[3]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[4]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[5]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[6]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[7]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[8]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[9]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[10]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[11]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                      <fo:block text-align="right">
                        <xsl:value-of select="translate(illustration/supplementalreport/columns[12]/title,$noampletters,$allletters)"/>
                      </fo:block>
                    </fo:table-cell>
                  </fo:table-row>
                  <fo:table-row>
                    <fo:table-cell padding="2pt">
                      <fo:block>
                      </fo:block>
                    </fo:table-cell>
                  </fo:table-row>
                </fo:table-header>
                <!-- Create Supplemental Report Values -->
                <xsl:choose>
                  <!-- make inforce illustration start in the inforce year -->
                  <xsl:when test="illustration/scalar/InforceYear!=0">
                    <fo:table-body>
                      <xsl:call-template name="supplemental-report-values">
                        <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                        <xsl:with-param name="inforceyear" select="5 - illustration/scalar/InforceYear"/>
                      </xsl:call-template>
                    </fo:table-body>
                  </xsl:when>
                  <xsl:otherwise>
                    <fo:table-body>
                      <xsl:call-template name="supplemental-report-values">
                        <xsl:with-param name="counter" select="1"/>
                        <xsl:with-param name="inforceyear" select="0"/>
                      </xsl:call-template>
                    </fo:table-body>
                  </xsl:otherwise>
                      </xsl:choose>
              </fo:table>
            </fo:block>
            <xsl:if test="illustration/scalar/SupplementalReport='1'">
              <fo:block id="endofdoc"></fo:block>
            </xsl:if>

          </fo:flow>
        </fo:page-sequence>
      </xsl:if>

      <xsl:if test="illustration/scalar/IsInforce!='1'">
        <!-- Numeric Summary Attachment - (Only for New Business) -->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary-attachment">

          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt">
              <xsl:text>Numeric Summary</xsl:text>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <fo:static-content flow-name="xsl-region-after">
            <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
              <xsl:text> </xsl:text>
            </fo:block>
            <xsl:call-template name="standardfooter">
              <xsl:with-param name="displaypagenumber" select="0"/>
            </xsl:call-template>
          </fo:static-content>

          <xsl:call-template name="numeric-summary-report"/>

        </fo:page-sequence>
      </xsl:if>
    </fo:root>
  </xsl:template>

  <xsl:template name="standardheader">
    <fo:block text-align="center" font-size="9.0pt">
      <xsl:choose>
        <xsl:when test="illustration/scalar/IsInforce!='1'">
          <fo:block>
            <xsl:text>LIFE INSURANCE BASIC ILLUSTRATION</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <fo:block>
            <xsl:text>LIFE INSURANCE IN FORCE BASIC ILLUSTRATION</xsl:text>
          </fo:block>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block>
        <xsl:value-of select="illustration/scalar/InsCoName"/>
      </fo:block>
      <xsl:if test="illustration/scalar/ProducerName!='0'">
        <fo:block>
          <xsl:text>Presented by: </xsl:text>
          <xsl:value-of select="illustration/scalar/ProducerName"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="illustration/scalar/ProducerStreet!='0'">
        <fo:block>
          <xsl:value-of select="illustration/scalar/ProducerStreet"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="illustration/scalar/ProducerCity!='0'">
        <fo:block>
          <xsl:value-of select="illustration/scalar/ProducerCity"/>
        </fo:block>
      </xsl:if>
      <fo:block><fo:leader></fo:leader></fo:block>
    </fo:block>
    <xsl:variable name="header-width" select="33"/>
    <xsl:variable name="header-field-width">
      <xsl:value-of select="$header-width * 0.44"/>
      <xsl:text>pc</xsl:text>
    </xsl:variable>
    <fo:block padding-before="5pt" font-size="9.0pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="125mm"/>
        <fo:table-column  column-width="2mm"/>
        <fo:table-column/>
        <fo:table-body padding-before="2pt">
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Prepared for:</xsl:text>
              </fo:block>
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
              <!-- Remove date prepared....now exists in footer
              <fo:block text-align="left">
                <xsl:text>Date Prepared: </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepMonth"/>
                <xsl:text> </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepDay"/>
                <xsl:text>, </xsl:text>
                <xsl:value-of select="illustration/scalar/PrepYear"/>
              </fo:block> -->
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Group Name: </xsl:text>
                <xsl:call-template name="limitstring" >
                  <xsl:with-param name="passString" select="illustration/scalar/CorpName"/>
                  <xsl:with-param name="length" select="50"/>
                </xsl:call-template>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
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
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Composite Illustration</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Insured: </xsl:text>
                    <xsl:call-template name="limitstring" >
                      <xsl:with-param name="passString" select="illustration/scalar/Insured1"/>
                      <xsl:with-param name="length" select="50"/>
                    </xsl:call-template>
                  </fo:block>
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
              <xsl:choose>
                <xsl:when test="illustration/scalar/Composite='1'">
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Age: </xsl:text>
                    <xsl:value-of select="illustration/scalar/Age"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Guaranteed Crediting Rate: </xsl:text>
                <xsl:value-of select="illustration/scalar/InitAnnGenAcctInt_Guaranteed"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <fo:block text-align="left">
                <xsl:text>Product: </xsl:text>
                <xsl:value-of select="illustration/scalar/PolicyForm"/>
                <xsl:text> (</xsl:text>
                <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                <xsl:text>)</xsl:text>
              </fo:block>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <xsl:choose>
              <xsl:when test="illustration/scalar/InforceYear!=0">
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Current Illustrated Crediting Rate: </xsl:text>
                    <xsl:call-template name="ultimate_interest_rate">
                      <xsl:with-param name="counter" select="illustration/scalar/InforceYear + 1"/>
                    </xsl:call-template>
                  </fo:block>
                </fo:table-cell>
              </xsl:when>
              <xsl:otherwise>
                <fo:table-cell>
                  <fo:block text-align="left">
                    <xsl:text>Initial Illustrated Crediting Rate: </xsl:text>
                    <xsl:value-of select="illustration/scalar/InitAnnGenAcctInt_Current"/>
                  </fo:block>
                </fo:table-cell>
              </xsl:otherwise>
            </xsl:choose>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$ModifiedSinglePremium='1'">
                  <fo:block text-align="left">
                    <xsl:text>Modified Single Premium Adjustable Life Insurance Policy</xsl:text>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1' or illustration/scalar/InforceYear &gt; 4">
                  <xsl:text>&#xA0;</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Ultimate Illustrated Crediting Rate: </xsl:text>
                      <xsl:value-of select="illustration/data/newcolumn/column[@name='AnnGAIntRate_Current']/duration[6]/@column_value"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <fo:block text-align="left">
                    <xsl:text>Initial Premium: </xsl:text>
                    <xsl:value-of select="illustration/scalar/InitPrem"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Single Premium: </xsl:text>
                    <xsl:value-of select="illustration/scalar/InitPrem"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell number-columns-spanned="1" padding="0pt"/>
            <fo:table-cell>
              <!-- Update with "FriendlyUWType"  - Get From Greg -->
              <!-- "&IF(UWType="Medical","Fully underwritten",
              IF(AND(State="TX",UWType="Guaranteed issue"),"Substandard *",UWType))) -->
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
                        <xsl:text>Underwriting Type: Fully underwritten</xsl:text>
                      </fo:block>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:choose>
                        <xsl:when test="illustration/scalar/StatePostalAbbrev='TX'">
                          <xsl:choose>
                            <xsl:when test="illustration/scalar/UWType='Guaranteed issue'">
                              <fo:block text-align="left">
                                <xsl:text>Underwriting Type: Substandard *</xsl:text>
                              </fo:block>
                            </xsl:when>
                            <xsl:otherwise>
                              <fo:block text-align="left">
                                <xsl:text>Underwriting Type: </xsl:text>
                                <xsl:value-of select="illustration/scalar/UWType"/>
                              </fo:block>
                            </xsl:otherwise>
                          </xsl:choose>
                        </xsl:when>
                        <xsl:otherwise>
                          <fo:block text-align="left">
                            <xsl:text>Underwriting Type: </xsl:text>
                            <xsl:value-of select="illustration/scalar/UWType"/>
                          </fo:block>
                        </xsl:otherwise>
                      </xsl:choose>
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
                  <xsl:choose>
                    <xsl:when test="illustration/scalar/Franchise!=''">
                      <fo:block text-align="left">
                        <xsl:text>Master contract: </xsl:text>
                        <xsl:call-template name="limitstring" >
                          <xsl:with-param name="passString" select="illustration/scalar/Franchise"/>
                          <xsl:with-param name="length" select="30"/>
                        </xsl:call-template>
                      </fo:block>
                    </xsl:when>
                    <xsl:otherwise>
                      <fo:block text-align="left">
                        <xsl:text>&#xA0;</xsl:text>
                      </fo:block>
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="left">
                    <xsl:text>Initial Death Benefit Option: </xsl:text>
                    <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
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
                  <fo:block text-align="left">
                    <xsl:text>Rate Classification: </xsl:text>
                    <xsl:value-of select="illustration/scalar/UWClass"/>
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="illustration/scalar/Smoker"/>
                    <xsl:text>, </xsl:text>
                    <xsl:value-of select="illustration/scalar/Gender"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
          </fo:table-row>
          <xsl:if test="illustration/scalar/Composite!='1'">
            <fo:table-row>
              <fo:table-cell>
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
                  <xsl:otherwise>
                    <fo:block text-align="left">
                      <xsl:text>&#xA0;</xsl:text>
                    </fo:block>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell>
                <xsl:if test="illustration/scalar/UWClass='Rated'">
                  <fo:block text-align="left">
                    <xsl:text>&#xA0;&#xA0;&#xA0;Table Rating: </xsl:text>
                    <xsl:value-of select="illustration/scalar/SubstandardTable"/>
                  </fo:block>
                </xsl:if>
              </fo:table-cell>
            </fo:table-row>
          </xsl:if>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="dollar-units">
    <xsl:choose>
      <xsl:when test="illustration/scalar/ScaleUnit=''">
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in dollars)</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:otherwise>
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in </xsl:text>
          <xsl:value-of select="illustration/scalar/ScaleUnit"/>
          <xsl:text>s of dollars)</xsl:text>
        </fo:block>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="numeric-summary-values">
    <xsl:param name="counter"/>
    <xsl:param name="age70"/>
    <xsl:param name="prioryears"/>
    <fo:table-row>
      <fo:table-cell padding=".2pt">
        <xsl:choose>
          <xsl:when test="$age70!=1">
            <fo:block text-align="right">
              <xsl:choose>
                <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
                  <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="$counter"/>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </xsl:when>
          <xsl:otherwise>
            <fo:block text-align="right">
              <xsl:text>Age 70</xsl:text>
            </fo:block>
          </xsl:otherwise>
        </xsl:choose>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block><xsl:text>&#xA0;</xsl:text></fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Guaranteed']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Guaranteed']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Guaranteed']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block><xsl:text>&#xA0;</xsl:text></fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Midpoint']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Midpoint']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Midpoint']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block><xsl:text>&#xA0;</xsl:text></fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Current']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Current']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
      <fo:table-cell>
        <fo:block text-align="right">
          <xsl:choose>
            <xsl:when test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Current']/duration[$counter]/@column_value"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="0"/>
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
      </fo:table-cell>
    </fo:table-row>
    <!-- Display Only Summary Years -->
    <xsl:if test="$age70!=1">
      <xsl:if test="$prioryears!=1">
        <xsl:choose>
          <xsl:when test="$counter &lt; 30">
            <xsl:choose>
              <xsl:when test="$counter=5">
                <!-- Display lapse years that occur prior to year 10 -->
                <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; 9">
                  <xsl:if test="illustration/scalar/LapseYear_Guaranteed &gt; 4">
                    <xsl:call-template name="numeric-summary-values">
                      <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Guaranteed + 1"/>
                      <xsl:with-param name="age70" select="0"/>
                      <xsl:with-param name="prioryears" select="1"/>
                    </xsl:call-template>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; 9">
                  <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; 4">
                    <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; illustration/scalar/LapseYear_Guaranteed">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="illustration/scalar/LapseYear_Current &lt; 9">
                  <xsl:if test="illustration/scalar/LapseYear_Current &gt; 4">
                    <xsl:if test="illustration/scalar/LapseYear_Current &gt; illustration/scalar/LapseYear_Midpoint">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Current + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 10 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="$counter + 5"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:when>
              <xsl:otherwise>
                <!-- Display lapse years that occur prior to next display year -->
                <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; $counter + 9">
                  <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; illustration/scalar/MaxDuration">
                    <xsl:if test="illustration/scalar/LapseYear_Guaranteed &gt; $counter - 1">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Guaranteed + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; $counter + 9">
                  <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; illustration/scalar/MaxDuration">
                    <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; $counter - 1">
                      <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; illustration/scalar/LapseYear_Guaranteed">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="illustration/scalar/LapseYear_Current &lt; $counter + 9">
                  <xsl:if test="illustration/scalar/LapseYear_Current &lt; illustration/scalar/MaxDuration">
                    <xsl:if test="illustration/scalar/LapseYear_Current &gt; $counter - 1">
                      <xsl:if test="illustration/scalar/LapseYear_Current &gt; illustration/scalar/LapseYear_Midpoint">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Current + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 20 and 30 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="$counter + 10"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:otherwise>
            <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; illustration/scalar/MaxDuration">
              <!-- Display lapse years that occur prior to next display year -->
              <xsl:if test="illustration/scalar/LapseYear_Guaranteed &gt; $counter - 1">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Guaranteed + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; illustration/scalar/MaxDuration">
              <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; $counter - 1">
                <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; illustration/scalar/LapseYear_Guaranteed">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint + 1"/>
                    <xsl:with-param name="age70" select="0"/>
                    <xsl:with-param name="prioryears" select="1"/>
                  </xsl:call-template>
                </xsl:if>
              </xsl:if>
            </xsl:if>
            <xsl:if test="illustration/scalar/LapseYear_Current &lt; illustration/scalar/MaxDuration">
              <xsl:if test="illustration/scalar/LapseYear_Current &gt; $counter - 1">
                <xsl:if test="illustration/scalar/LapseYear_Current &gt; illustration/scalar/LapseYear_Midpoint">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Current + 1"/>
                    <xsl:with-param name="age70" select="0"/>
                    <xsl:with-param name="prioryears" select="1"/>
                  </xsl:call-template>
                </xsl:if>
              </xsl:if>
            </xsl:if>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template name="numeric-summary-report">
    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column  column-width="2mm"/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-column/>
          <fo:table-header>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3" padding="0pt"></fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="3" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">Guaranteed Values</fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="7" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                <fo:block text-align="center">Non-Guaranteed Values</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell number-columns-spanned="3" padding="0pt"></fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="3" padding="0pt"></fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="3" padding-before="3pt" padding="3pt">
                <fo:block text-align="center">Midpoint Values</fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
              <fo:table-cell number-columns-spanned="3" padding-before="3pt" padding="3pt">
                <fo:block text-align="center">Current Values</fo:block>
              </fo:table-cell>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell>
                <fo:block text-align="right">Policy</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Premium</fo:block>
              </fo:table-cell>
              <fo:table-cell padding="0pt"/>
              <fo:table-cell>
                <fo:block text-align="right">Account</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
              <fo:table-cell padding="0pt"/>
              <fo:table-cell>
                <fo:block text-align="right">Account</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Cash Surr</fo:block>
              </fo:table-cell>
              <fo:table-cell>
                <fo:block text-align="right">Death</fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
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
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right"></fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Outlay</fo:block>
              </fo:table-cell>
              <fo:table-cell padding="0pt"/>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
              <fo:table-cell padding="0pt"/>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Value</fo:block>
              </fo:table-cell>
              <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                <fo:block text-align="right">Benefit</fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="1" padding="0pt"/>
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

          <!-- Create Numeric Summary Values -->
          <fo:table-body>
              <!-- Display summary values if policy lapses prior to year 5 -->
            <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; 4">
              <xsl:call-template name="numeric-summary-values">
                <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Guaranteed + 1"/>
                <xsl:with-param name="age70" select="0"/>
                <xsl:with-param name="prioryears" select="1"/>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; 4">
              <xsl:if test="illustration/scalar/LapseYear_Midpoint &gt; illustration/scalar/LapseYear_Guaranteed">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="illustration/scalar/LapseYear_Current &lt; 4">
              <xsl:if test="illustration/scalar/LapseYear_Current &gt; illustration/scalar/LapseYear_Midpoint">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="illustration/scalar/LapseYear_Current + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:call-template name="numeric-summary-values">
              <xsl:with-param name="counter" select="5"/>
              <xsl:with-param name="age70" select="0"/>
            </xsl:call-template>
            <xsl:if test="illustration/scalar/Composite!='1'">
              <xsl:if test="illustration/scalar/Age &lt; 70">
                <fo:table-row>
                  <fo:table-cell padding="8pt">
                    <fo:block text-align="right"></fo:block>
                  </fo:table-cell>
                </fo:table-row>
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="counter" select="70 - /illustration/scalar/Age"/>
                  <xsl:with-param name="age70" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
          </fo:table-body>
        </fo:table>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <xsl:choose>
        <xsl:when test="illustration/scalar/Composite!='1'">
          <xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; /illustration/scalar/MaxDuration">
            <fo:block text-align="left" font-size="9.0pt">
              <xsl:text>Additional premium will be required in year </xsl:text>
              <xsl:value-of select="illustration/scalar/LapseYear_Guaranteed+1"/>
              <xsl:text> or contract will lapse based on guaranteed monthly charges and interest rate.</xsl:text>
            </fo:block>
            <xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; /illustration/scalar/MaxDuration">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>Additional premium will be required in year </xsl:text>
                <xsl:value-of select="illustration/scalar/LapseYear_Midpoint+1"/>
                <xsl:text> or contract will lapse based on midpoint monthly charges and interest rate.</xsl:text>
              </fo:block>
            </xsl:if>
            <xsl:if test="illustration/scalar/LapseYear_Current &lt; /illustration/scalar/MaxDuration">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>Additional premium will be required in year </xsl:text>
                <xsl:value-of select="illustration/scalar/LapseYear_Current+1"/>
                <xsl:text> or contract will lapse based on current monthly charges and interest rate.</xsl:text>
              </fo:block>
            </xsl:if>
          </xsl:if>
        </xsl:when>
        <xsl:otherwise>
          <fo:block text-align="left" font-size="9.0pt">
            <xsl:text>The year of policy lapse on a guaranteed, midpoint and current basis is not depicted in the above table of values for this composite illustration because it is not applicable on a case basis.</xsl:text>
          </fo:block>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block><fo:leader></fo:leader></fo:block>
      <xsl:if test="illustration/scalar/IsMec='1'">
        <fo:block text-align="left" font-size="9.0pt">
          <xsl:text>IMPORTANT TAX DISCLOSURE: This is a Modified Endowment Contract. Please refer to the Narrative Summary for additional information.</xsl:text>
        </fo:block>
      </xsl:if>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="center" font-size="9.0pt">
        <xsl:text>Certification Statements</xsl:text>
      </fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="left" font-size="9.0pt">
        <xsl:text>CONTRACT OWNER / APPLICANT</xsl:text>
      </fo:block>
      <xsl:if test="illustration/scalar/InterestDisclaimer!=''">
        <fo:block text-align="left" font-size="9.0pt">
          <xsl:text>I understand that at the present time higher current interest rates are credited for policies with case premiums in the amount of </xsl:text>
          <xsl:value-of select="illustration/scalar/InterestDisclaimer"/>
        </fo:block>
        <fo:block><fo:leader></fo:leader></fo:block>
      </xsl:if>
      <xsl:choose>
        <xsl:when test="illustration/scalar/StatePostalAbbrev='IL'">
          <fo:block text-align="left" font-size="9.0pt">
            <xsl:text>I have received a copy of this illustration and understand that this illustration assumes that the currently illustrated non-guaranteed elements will continue unchanged for all years shown. This is not likely to occur, and actual results may be more or less favorable than those shown.</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="illustration/scalar/StatePostalAbbrev='TX'">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>A copy of this illustration has been provided to the Applicant/Policyowner. </xsl:text>
              </fo:block>
            </xsl:when>
            <xsl:otherwise>
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>I have received a copy of this illustration, and I understand that any non-guaranteed elements illustrated are subject to change and could be either higher or lower.  Additionally, I have been informed by my agent that these values are not guaranteed.</xsl:text>
              </fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="left" font-size="9.0pt" text-decoration="overline">CONTRACT OWNER OR APPLICANT SIGNATURE &#xA0;&#xA0;&#xA0;
        <fo:inline text-decoration="no-overline">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:inline>DATE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="left" font-size="9.0pt">
        <xsl:text>AGENT / AUTHORIZED REPRESENTATIVE</xsl:text>
      </fo:block>
      <xsl:choose>
        <xsl:when test="illustration/scalar/StatePostalAbbrev='IL'">
          <fo:block text-align="left" font-size="9.0pt">
            <xsl:text>I have informed the applicant or policyowner that this illustration assumes that the currently illustrated non-guaranted elements will continue unchanged for all years shown. This is not likely to occur, and actual results may be more or less favorable than those shown.</xsl:text>
          </fo:block>
        </xsl:when>
        <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="illustration/scalar/StatePostalAbbrev='TX'">
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>A copy of this illustration has been provided to the Applicant/Policyowner.</xsl:text>
              </fo:block>
            </xsl:when>
            <xsl:otherwise>
              <fo:block text-align="left" font-size="9.0pt">
                <xsl:text>I certify that this illustration has been presented to the applicant, and that I have explained that any non-guaranteed elements illustrated are subject to change.  I have made no statements that are inconsistent with the illustration.</xsl:text>
              </fo:block>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block><fo:leader></fo:leader></fo:block>
      <fo:block text-align="left" font-size="9.0pt" text-decoration="overline">AGENT OR AUTHORIZED REPRESENTATIVE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;
        <fo:inline text-decoration="no-overline">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:inline>DATE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:block>
    </fo:flow>
  </xsl:template>

  <!-- TEMPLATE: tabular-detail-values -->
  <!-- Create Tabular Detail Values    -->

  <xsl:template name="tabular-detail-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".2pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:choose>
          <xsl:when test="illustration/scalar/Composite!='1'">
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/data/newcolumn/column[@name='AttainedAge']/duration[$counter]/@column_value"/>
              </fo:block>
            </fo:table-cell>
          </xsl:when>
          <xsl:otherwise>
              <fo:table-cell>
                <fo:block text-align="right">&#xA0;</fo:block>
              </fo:table-cell>
          </xsl:otherwise>
        </xsl:choose>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='GrossPmt']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">&#xA0;</fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">&#xA0;</fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Guaranteed']/duration[$counter]/@column_value"/>
          </fo:block>
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
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="3pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="tabular-detail-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="tabular-detail-report2-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".2pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:if test="illustration/scalar/Composite!='1'">
          <fo:table-cell>
            <fo:block text-align="right">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='AttainedAge']/duration[$counter]/@column_value"/>
            </fo:block>
          </fo:table-cell>
        </xsl:if>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name='AnnGAIntRate_Current']/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <xsl:if test="illustration/scalar/Composite!='1'">
          <fo:table-cell>
            <fo:block text-align="right">
              <xsl:value-of select="illustration/data/newcolumn/column[@name='MonthlyFlatExtra']/duration[$counter]/@column_value"/>
            </fo:block>
          </fo:table-cell>
        </xsl:if>
        <fo:table-cell>
          <fo:block>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="3pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="tabular-detail-report2-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="supplemental-report-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <fo:table-cell padding=".2pt">
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column1name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column2name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column3name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column4name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column5name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column6name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column7name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column8name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column9name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column10name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column11name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
        <fo:table-cell>
          <fo:block text-align="right">
            <xsl:value-of select="illustration/data/newcolumn/column[@name=$column12name]/duration[$counter]/@column_value"/>
          </fo:block>
        </fo:table-cell>
      </fo:table-row>
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="3pt">
            <fo:block text-align="right"></fo:block>
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="supplemental-report-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="removeamps">
    <xsl:param name="title"/>
    <xsl:if test="contains($title, '&amp;')">
      <xsl:call-template name="concatenate">
        <xsl:with-param name="textbefore" select="substring-before($title,'amp;')"/>
        <xsl:with-param name="textafter" select="substring-after($title,'amp;')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="concatenate">
    <xsl:param name="textbefore"/>
    <xsl:param name="textafter"/>
    <xsl:call-template name="removeamps">
      <xsl:with-param name="title" select="concat($textbefore,$textafter)"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="standardfooter">
    <xsl:param name="displaypagenumber"/>
    <fo:block padding-before="5pt" font-size="8.0pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-column/>
        <fo:table-body padding-before="2pt" border-top-style="solid" border-top-width="1pt" border-top-color="blue">
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
                    <xsl:text>Page </xsl:text>
                    <fo:page-number/>
                    <xsl:text> of </xsl:text>
                    <fo:page-number-citation ref-id="endofdoc"/>
                  </fo:block>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block text-align="center">
                    <xsl:text>Attachment</xsl:text>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:table-cell>
            <fo:table-cell>
              <fo:block text-align="right">
                <xsl:value-of select="illustration/scalar/InsCoName"/>
              </fo:block>
            </fo:table-cell>
          </fo:table-row>
          <fo:table-row>
            <fo:table-cell>
                <!-- Version Number -->
              <xsl:if test="illustration/scalar/LmiVersion!=''">
                <fo:block text-align="left">System Version:
                  <xsl:value-of select="illustration/scalar/LmiVersion"/>
                </fo:block>
              </xsl:if>
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

  <xsl:template name="ultimate_interest_rate">
    <xsl:param name="counter"/>
    <xsl:value-of select="illustration/data/newcolumn/column[@name='AnnGAIntRate_Current']/duration[$counter]/@column_value"/>
  </xsl:template>

  <xsl:template name="set_single_premium">
    <xsl:choose>
      <xsl:when test="string-length(illustration/scalar/PolicyMktgName) &gt; 5">
        <xsl:choose>
          <xsl:when test="substring(illustration/scalar/PolicyLegalName, 1, 6) !='Single'">
            <xsl:text>0</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>1</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="set_modified_single_premium">
    <xsl:choose>
      <xsl:when test="string-length(illustration/scalar/PolicyMktgName) &gt; 5">
        <xsl:choose>
          <xsl:when test="substring(illustration/scalar/PolicyLegalName, 1, 6) !='Single'">
            <xsl:text>0</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:choose>
              <xsl:when test="illustration/scalar/StatePostalAbbrev='MA'">
                <xsl:text>1</xsl:text>
              </xsl:when>
              <xsl:otherwise>
                <xsl:text>0</xsl:text>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="set_group_experience_rating">
    <xsl:choose>
      <xsl:when test="illustration/scalar/PolicyLegalName='Group Flexible Premium Adjustable Life Insurance Policy'">
        <xsl:text>1</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>0</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!--
  This may appear to be a clumsy way to get the max value but there is no clean way
  to do this with the available XSL functionality and the values being distinct
  entities. The way it is done in Calculation Summary and all examples to be found
  have the values grouped in a common node within the XML. Simplified example:
  <values>
    <value>55</value>
    <value>45</value>
    <value>50</value>
    <value>40</value>
  </values>
  NOTE: illustration_reg cases do not currently have LapseYear_CurrentZero and
  LapseYear_GuaranteedZero in their XML. If they are ever added, this should be
  amended as in the other XSL-FO files.
  -->

  <xsl:template name="get-max-lapse-year">
    <xsl:call-template name="max-comparison">
      <xsl:with-param name="value1" select="illustration/scalar/LapseYear_Current"/>
      <xsl:with-param name="value2" select="illustration/scalar/LapseYear_Guaranteed"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="max-comparison">
    <xsl:param name="value1"/>
    <xsl:param name="value2"/>
    <xsl:choose>
      <xsl:when test="$value1 &lt; $value2">
        <xsl:value-of select="$value2"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$value1"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>