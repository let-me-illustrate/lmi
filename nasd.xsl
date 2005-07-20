<?xml version="1.0" encoding="UTF-8"?>
<!--
Life insurance illustrations.

Copyright (C) 2004, 2005 Gregory W. Chicares.

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

$Id: nasd.xsl,v 1.2 2005-07-20 10:56:19 chicares Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
    <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
    <xsl:variable name="counter" select="1"/>
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
    <xsl:variable name="allletters"> ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789&#xA0;</xsl:variable>
    <xsl:variable name="noampletters"> ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_</xsl:variable>
    <xsl:template match="/">
        <fo:root>
            <fo:layout-master-set>

                <!-- Define the cover page. -->
                <fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <fo:region-body margin-top="0.25in" margin-bottom="0.25in"/>
                </fo:simple-page-master>

                <!-- Define the explanatory notes page. -->
                <fo:simple-page-master master-name="explanatory-notes" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.30in"/>
                    <!-- Header -->
                    <fo:region-before border-after-style="solid" extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after border-before-style="solid" extent="0.9in"/>
                </fo:simple-page-master>

                <!-- Define the explanatory notes page. -->
                <fo:simple-page-master master-name="explanatory-notes-separate-account" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="2" margin-top="2.25in" margin-bottom="3.0in"/>
                    <!-- Header -->
                    <fo:region-before border-after-style="solid" extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after border-before-style="solid" extent="1.3in"/>
                </fo:simple-page-master>

                <!-- Define the Basic Illustration page. -->
                <fo:simple-page-master master-name="basic-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.30in"/>
                    <!-- Header -->
                    <fo:region-before border-after-style="solid" extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after border-before-style="solid" extent=".9in"/>
                </fo:simple-page-master>

                <!-- Define the Supplimental Illustration page. -->
                <fo:simple-page-master master-name="supplemental-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.60in"/>
                    <!-- Header -->
                    <fo:region-before border-after-style="solid" extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after border-before-style="solid" extent=".9in"/>
                </fo:simple-page-master>

                <!-- Define the Illustration Assuption Detail page. -->
                <fo:simple-page-master master-name="illustration-assumption" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.60in"/>
                    <!-- Header -->
                    <fo:region-before border-after-style="solid" extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after border-before-style="solid" extent=".9in"/>
                </fo:simple-page-master>

                <!-- Define the Supplemental Illustration page. -->
                <xsl:if test="illustration/scalar/SupplementalReport='1'">
                    <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                        <!-- Central part of page -->
                        <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1.9in"/>
                        <!-- Header -->
                        <fo:region-before border-after-style="solid" extent="3in"/>
                        <!-- Footer -->
                        <fo:region-after border-before-style="solid" extent=".95in"/>
                    </fo:simple-page-master>
                </xsl:if>
            </fo:layout-master-set>

            <!-- The data to be diplayed in the pages, cover page first -->
            <!--
No cover page for this style sheet

            <fo:page-sequence master-reference="cover">
                <fo:flow flow-name="xsl-region-body">
                    <fo:block text-align="center" font-size="14.0pt">
                        <xsl:text>IHS Illustration - Proof of Concept</xsl:text>
                    </fo:block>
                </fo:flow>
            </fo:page-sequence>
-->

            <!-- COMPLIANCE -->
            <!-- Basic Illustration -->
            <!-- Body page -->
            <fo:page-sequence master-reference="basic-illustration">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="reporttitle" select="'Basic Illustration'"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
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
                <xsl:call-template name="basic-illustration-report"/>
            </fo:page-sequence>

            <!-- Supplemental Illustration -->
            <!-- Body page -->
            <fo:page-sequence master-reference="supplemental-illustration">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="reporttitle" select="'Supplemental Illustration'"/>
                    </xsl:call-template>
                    <fo:block text-align="center" font-size="8pt" font-family="sans-serif">
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


            <!-- EXPLANATORY NOTES - begins here -->
            <fo:page-sequence master-reference="explanatory-notes">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="reporttitle" select="'Explanatory Notes'"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <xsl:call-template name="standardfooter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                    </xsl:call-template>
                </fo:static-content>

                <!-- Explanatory Notes Summary Body  -->
                <fo:flow flow-name="xsl-region-body">
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <fo:block>
                            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                            <xsl:text> is a </xsl:text>
                            <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
                            <xsl:text> issued by </xsl:text>
                            <xsl:value-of select="illustration/scalar/InsCoName"/>
                            <xsl:text>. The policy provides lifetime insurance protection for as long as it remains in force. The policy is available at issue with at least two death benefit options: Option 1 (death benefit equal to the greater of (a) the selected face amount at death or (b) the minimum face amount at death); and Option 2 (death benefit equal to the greater of (a) the selected face amount plus account value at death or (b) the minimum face amount at death). If available under the policy, Option 3 is a death benefit option equal to the greatest of (a) the selected face amount at death plus the sum of premiums paid less withdrawals; or (b) selected face amount at death; or (c) minimum face amount at death.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Account values may be used to pay contract charges. Contract charges are due during the life of the insured; depending on actual results, the premium payer may need to continue or resume premium outlays.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
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
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:value-of select="illustration/scalar/InsCoName"/>
                            <xsl:text> has the right to promptly refund any amount of premium paid if it will increase the net amount at risk. </xsl:text>
                            <xsl:text>(referred to in the contract as the Amount of Insurance that Requires a Charge).</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Payments are assumed to be paid as described in the 'Illustration Assumption Detail'. Account values, cash surrender values, and death benefits are illustrated as of the end of the year.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
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
                                    <xsl:text>becomes a Modified Endowment Contract (MEC)</xsl:text>
                                    <xsl:text>in policy year </xsl:text>
                                    <xsl:value-of select="illustration/scalar/MecYear+1"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>is not a Modified Endowment Contract (MEC)</xsl:text>
                                    <xsl:text>.  Subsequent changes in the contract, including but not limited to increases and decreases in premiums or benefits, may cause the contract to be retested and may result in the contract becoming a MEC.</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </fo:block>
                        <fo:block>
                            <xsl:text>If a contract is a MEC, any loans or withdrawals are taxed to the extent of any gain in the contract, and an additional 10% penalty tax will apply to the taxable portion of the loan or withdrawal. The 10% penalty tax applies if the contract owner is an individual under age 59 1/2 and does not meet any applicable exception, or if the contract is owned by a corporation or other entity.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>This contract has a guaranteed maximum cost of insurance (based on 1980 CSO mortality tables) and maximum administrative charges. The actual current charges are lower than these and are reflected in the values. However, these current charges are subject to change.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>This illustration assumes death of the insured at age </xsl:text>
                            <xsl:value-of select="illustration/scalar/EndtAge"/>
                            <xsl:text>.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>In the states of North Carolina and South Carolina, Guaranteed Issue Underwriting is referred to as "Limited Underwriting" and Simplified Issue Underwriting is referred to as "Simplified Underwriting".</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>The loan interest rate may be fixed or adjustable as elected by the sponsor.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>This illustration must be preceded or accompanied by the current prospectuses for </xsl:text>
                            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                            <xsl:text> variable life insurance contract and its underlying investment choices. Before purchasing a variable life insurance contract, investors should carefully consider the investment objectives, risks, charges and expenses of the variable life insurance contract and its underlying investment choices. Please read the prospectuses carefully before investing or sending money.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                    </fo:block>


                    <!-- Forced New Page -->
                    <fo:block break-after="page"/>
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <xsl:if test="illustration/scalar/HasTerm='1'">
                            <fo:block>
                                <xsl:text>A Term Rider is available for attachment to this policy. The Term Rider provides the option to purchase monthly term insurance on the life of the insured. </xsl:text>
                                <xsl:text>The term rider selected face amount supplements the selected face amount of the contract. If the Term Rider is attached, the policy to which it is attached may have a lower annual cutoff premium and, as a result, the lower overall sales loads paid may be lower than a contract having the same total face amount, but with no Term Rider. Also, the lapse protection feature of the contract's</xsl:text>
                                <xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
                                <xsl:text> does not apply to the Term Rider's selected face amount.</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
                        </xsl:if>
                        <xsl:if test="illustration/scalar/HasWP='1'">
                            <fo:block>
                                <xsl:text>A Waiver of Monthly Charges rider is available for attachment to this policy for insureds with ages 20-64. The Waiver of Monthly Charges Rider provides that in the event of the disability of the insured that begins before attained age 65 and continues for at least 6 months, </xsl:text>
                                <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                                <xsl:text> will waive certain monthly charges up to age 65, but not less than two years, while the insured remains totally disabled. An additional charge is associated with this rider, if elected. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
                        </xsl:if>
                        <fo:block>
                            <xsl:text>This illustration may not reflect your actual tax and accounting consequences. Consult professional advisors for tax advice.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <xsl:text>No tax charge is made to the Separate Account. However, such a charge may be made in the future.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <xsl:if test="illustration/scalar/UsePartialMort='1'">
                            <fo:block>
                                <xsl:text>This illustration reflects the client's mortality assumption of </xsl:text>
                                <xsl:value-of select="illustration/scalar/PartialMortPct"/>
                                <xsl:text> of the </xsl:text>
                                <xsl:value-of select="illustration/scalar/PartMortTableName"/>
                                <xsl:text> table with all deaths at the end of the year.</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
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
                                    <fo:block>
                                        <fo:leader></fo:leader>
                                    </fo:block>
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
                                    <fo:block>
                                        <fo:leader></fo:leader>
                                    </fo:block>
                                </xsl:if>
                            </xsl:otherwise>
                        </xsl:choose>
                        <fo:block>
                            <xsl:text>The state of issue is </xsl:text>
                            <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
                            <xsl:text>.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                    </fo:block>
                </fo:flow>
            </fo:page-sequence>

            <!-- EXPLANATORY NOTES - begins here -->
            <fo:page-sequence master-reference="explanatory-notes-separate-account">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="reporttitle" select="'Explanatory Notes'"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <xsl:call-template name="footer-with-underwriter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                    </xsl:call-template>
                </fo:static-content>

                <!-- Explanatory Notes Summary Body  -->
                <fo:flow flow-name="xsl-region-body">
                    <fo:block span="all" text-align="left" font-size="9pt" font-family="sans-serif">
                            <fo:block>
                                <xsl:text>SEPARATE ACCOUNT</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
                            <fo:block>
                                <xsl:text>This illustration shows how the death benefit and account value could vary over an extended period of time, assuming the funds experience </xsl:text>
                                <xsl:text>hypothetical gross rates of investment return. Actual results of return may be more or less than those shown and in all likelihood will vary </xsl:text>
                                <xsl:text>year to year. Timing of premium payments, investment allocations and withdrawals or loans, if taken, may impact investment results. </xsl:text>
                                <xsl:text>Separate Account Charges are deducted from the gross investment rate to determine the net investment rate. These charges include a mortality </xsl:text>
                                <xsl:text>and expense charge, and an investment management fee and other fund operating expenses. The total fund operating expenses charged vary </xsl:text>
                                <xsl:text>by fund. SEE PROSPECTUS.</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
                            <xsl:if test="illustration/scalar/Composite!='1'">
                                <fo:block>
                                    <xsl:text>This illustration is based on total Separate Account fund expenses of </xsl:text>
                                    <xsl:value-of select="illustration/data/newcolumn/column[@name='TotalIMF']/duration[1]/@column_value"/>
                                    <xsl:text>.</xsl:text>
                                </fo:block>
                                <fo:block>
                                    <xsl:text>Investment management fees are based on </xsl:text>
                                    <xsl:choose>
                                        <xsl:when test="illustration/scalar/AvgFund='1'">
                                            <xsl:text>equal </xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:text>the following </xsl:text>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                    <xsl:text>initial allocations among the available funds</xsl:text>
                                    <xsl:choose>
                                        <xsl:when test="illustration/scalar/GenAcctAllocation='0%'">
                                            <xsl:text>:</xsl:text>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:text> with the remaining </xsl:text>
                                            <xsl:value-of select="illustration/scalar/GenAcctAllocation"/>
                                            <xsl:text> in the general account:</xsl:text>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </fo:block>
                                <fo:block>
                                    <fo:leader></fo:leader>
                                </fo:block>
                            </xsl:if>
                    </fo:block>
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <xsl:choose>
                            <xsl:when test="illustration/scalar/CustomFund='1'">
                                <fo:block span="all">
                                    <xsl:text>100% A custom fund selected by the purchaser</xsl:text>
                                </fo:block>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:call-template name="list-fund-allocation">
                                    <xsl:with-param name="counter" select="1"/>
                                </xsl:call-template>
                            </xsl:otherwise>
                        </xsl:choose>
                    </fo:block>
                </fo:flow>
            </fo:page-sequence>

            <!-- Illustration Assumption Detail -->
            <!-- Body page -->
            <fo:page-sequence master-reference="illustration-assumption">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="reporttitle" select="'Illustration Assumption Detail'"/>
                    </xsl:call-template>
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

            <!-- Supplemental Illustration -->
            <!-- Body page -->
            <xsl:if test="illustration/scalar/SupplementalReport='1'">
                <fo:page-sequence master-reference="supplemental-report">
                    <!-- Define the contents of the header. -->
                    <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                        <xsl:call-template name="standardheader"/>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block text-align="center" font-size="10.0pt">
                            <xsl:value-of select="illustration/supplementalreport/title"/>
                        </fo:block>
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
                                <fo:table-body>
                                    <xsl:call-template name="supplemental-report-values">
                                        <xsl:with-param name="counter" select="1"/>
                                    </xsl:call-template>
                                </fo:table-body>
                            </fo:table>
                        </fo:block>
                        <xsl:if test="illustration/scalar/SupplementalReport='1'">
                            <fo:block id="endofdoc"></fo:block>
                        </xsl:if>
                    </fo:flow>
                </fo:page-sequence>
            </xsl:if>
        </fo:root>
    </xsl:template>
    <xsl:template name="standardheader">
        <xsl:param name="reporttitle"/>
        <fo:block text-align="center" font-size="9.0pt">
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
            <fo:block>
                <xsl:text>The purpose of this illustration is to show how the performance of the underlying separate account divisions could affect the contract</xsl:text>
            </fo:block>
            <fo:block>
                <xsl:text>account value and death benefit. This illustration is hypothetical and may not be used to project or predict investment results.</xsl:text>
            </fo:block>
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
            <fo:block>
                <xsl:value-of select="$reporttitle"/>
            </fo:block>
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
        </fo:block>
        <xsl:variable name="header-width" select="33"/>
        <xsl:variable name="header-field-width">
            <xsl:value-of select="$header-width * 0.44"/>
            <xsl:text>pc</xsl:text>
        </xsl:variable>
        <fo:list-block font-size="9pt" provisional-label-separation="-100pt">
            <xsl:attribute name="provisional-distance-between-starts">
                <xsl:value-of select="$header-field-width"/>
            </xsl:attribute>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <fo:block text-align="left">
                        <xsl:choose>
                            <xsl:when test="illustration/scalar/Composite='1'">
                                <fo:block>
                                    <xsl:text>Composite</xsl:text>
                                </fo:block>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:choose>
                                    <xsl:when test="illustration/scalar/InsuredName='  '">
                                        <fo:block>
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <fo:block>
                                            <xsl:value-of select="illustration/scalar/InsuredName"/>
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
                </fo:list-item-label>
                <fo:list-item-body start-indent="body-start()">
                    <fo:list-block provisional-label-separation="0pt">
                        <fo:list-item>
                            <fo:list-item-label end-indent="label-end()">
                                <fo:block text-align="center"></fo:block>
                            </fo:list-item-label>
                            <fo:list-item-body start-indent="body-start()">
                                <fo:block text-align="left">
                                    <xsl:choose>
                                        <xsl:when test="illustration/scalar/Composite='1'">
                                            <fo:block>
                                                <xsl:text>&#xA0;</xsl:text>
                                            </fo:block>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <fo:block>
                                                <xsl:text>Assumed Gross Rate: </xsl:text>
                                                <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                                                <xsl:text> (</xsl:text>
                                                <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                                                <xsl:text> Net)*</xsl:text>
                                            </fo:block>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </fo:block>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <fo:block text-align="left">
                        <xsl:text>Contract:  </xsl:text>
                        <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                    </fo:block>
                </fo:list-item-label>
                <fo:list-item-body start-indent="body-start()">
                    <fo:list-block provisional-label-separation="0pt">
                        <fo:list-item>
                            <fo:list-item-label end-indent="label-end()">
                                <fo:block text-align="center"></fo:block>
                            </fo:list-item-label>
                            <fo:list-item-body start-indent="body-start()">
                                <fo:block text-align="left">
                                    <xsl:text>Initial </xsl:text>
                                    <xsl:if test="illustration/scalar/HasTerm!='0'">
                                        <xsl:text>, Total</xsl:text>
                                    </xsl:if>
                                    <xsl:text> Face Amount: $</xsl:text>
                                    <xsl:value-of select="illustration/scalar/InitTotalSA"/>
                                </fo:block>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <xsl:choose>
                        <xsl:when test="illustration/scalar/Composite='1'">
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Composite Illustration</xsl:text>
                            </fo:block>
                        </xsl:when>
                        <xsl:otherwise>
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>Initial Premium </xsl:text>
                                <xsl:value-of select="illustration/scalar/InitPrem"/>
                            </fo:block>
                        </xsl:otherwise>
                    </xsl:choose>
                </fo:list-item-label>
                <fo:list-item-body start-indent="body-start()">
                    <fo:list-block provisional-label-separation="0pt">
                        <fo:list-item>
                            <fo:list-item-label end-indent="label-end()">
                                <fo:block text-align="center"></fo:block>
                            </fo:list-item-label>
                            <fo:list-item-body start-indent="body-start()">
                                <xsl:choose>
                                    <xsl:when test="illustration/scalar/HasTerm!='0'">
                                        <fo:block text-align="left" font-size="9.0pt">
                                            <xsl:text>Initial Base Face Amount: </xsl:text>
                                            <xsl:value-of select="illustration/scalar/InitBaseSA"/>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <fo:block text-align="left">
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <xsl:choose>
                        <xsl:when test="illustration/scalar/Composite='1'">
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>&#xA0;</xsl:text>
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
                                                <xsl:when test="illustration/scalar/State='TX'">
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
                </fo:list-item-label>
                <fo:list-item-body start-indent="body-start()">
                    <fo:list-block provisional-label-separation="0pt">
                        <fo:list-item>
                            <fo:list-item-label end-indent="label-end()">
                                <fo:block text-align="center"></fo:block>
                            </fo:list-item-label>
                            <fo:list-item-body start-indent="body-start()">
                                <xsl:choose>
                                    <xsl:when test="illustration/scalar/HasTerm!='0'">
                                        <fo:block text-align="left" font-size="9.0pt">
                                            <xsl:text>Initial Term Face Amount: </xsl:text>
                                            <xsl:value-of select="illustration/scalar/InitTermSA"/>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <fo:block text-align="left">
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <fo:block text-align="left">
                        <xsl:if test="illustration/scalar/HasWP='1'">
                            <xsl:text>Waiver of Monthly Charges Rider elected.</xsl:text>
                        </xsl:if>
                        <xsl:text>&#xA0;</xsl:text>
                    </fo:block>
                </fo:list-item-label>
                <fo:list-item-body start-indent="body-start()">
                    <fo:list-block provisional-label-separation="0pt">
                        <fo:list-item>
                            <fo:list-item-label end-indent="label-end()">
                                <fo:block text-align="center"></fo:block>
                            </fo:list-item-label>
                            <fo:list-item-body start-indent="body-start()">
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
                                                    <xsl:value-of select="illustration/scalar/SubstdTable"/>
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
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
        </fo:list-block>
    </xsl:template>
    <!-- Print Dollar Units -->
    <xsl:template name="dollar-units">
        <xsl:choose>
            <xsl:when test="illustration/scalar/ScaleUnit=''">
                <fo:block text-align="center" font-size="8pt">
                    <xsl:text>Values shown are in dollars</xsl:text>
                </fo:block>
            </xsl:when>
            <xsl:otherwise>
                <fo:block text-align="center" font-size="8pt">
                    <xsl:text>Values shown are in </xsl:text>
                    <xsl:value-of select="illustration/scalar/ScaleUnit"/>
                    <xsl:text>s of dollars</xsl:text>
                </fo:block>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    <!-- Create Basic Illustration Values -->
    <xsl:template name="basic-illustration-values">
        <xsl:param name="counter"/>
        <xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
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
            <xsl:if test="$counter mod 5=0">
                <fo:table-row>
                    <fo:table-cell padding="4pt">
                        <fo:block text-align="right"></fo:block>
                    </fo:table-cell>
                </fo:table-row>
            </xsl:if>
            <xsl:call-template name="basic-illustration-values">
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- Create Supplemental Illustration Values -->
    <xsl:template name="supplemental-illustration-values">
        <xsl:param name="counter"/>
        <xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
                            </fo:block>
                        </fo:table-cell>
                    </xsl:otherwise>
                </xsl:choose>

                <fo:table-cell>
                    <fo:block text-align="right">
                        <xsl:value-of select="illustration/data/newcolumn/column[@name='PolFee_Current']/duration[$counter]/@column_value"/>
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
            <xsl:if test="$counter mod 5=0">
                <fo:table-row>
                    <fo:table-cell padding="4pt">
                        <fo:block text-align="right"></fo:block>
                    </fo:table-cell>
                </fo:table-row>
            </xsl:if>
            <xsl:call-template name="supplemental-illustration-values">
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- Create Illustration Assumption Detail Values -->
    <xsl:template name="illustration-assumption-values">
        <xsl:param name="counter"/>
        <xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Loan']/duration[$counter]/@column_value"/>
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
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='NetWD']/duration[$counter]/@column_value"/>
                            </fo:block>
                        </fo:table-cell>
                        <fo:table-cell>
                            <fo:block text-align="right">
                                <xsl:value-of select="illustration/data/newcolumn/column[@name='Loan']/duration[$counter]/@column_value"/>
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
            <xsl:if test="$counter mod 5=0">
                <fo:table-row>
                    <fo:table-cell padding="4pt">
                        <fo:block text-align="right"></fo:block>
                    </fo:table-cell>
                </fo:table-row>
            </xsl:if>
            <xsl:call-template name="illustration-assumption-values">
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- Create Basic Illustration Report -->
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
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-header>
                        <fo:table-row>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="4" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                                <fo:block text-align="center">Using guaranteed charges</fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="4" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
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
                                <fo:block text-align="right">Gross / Net</fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">Gross / Net</fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">Gross / Net</fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">Gross / Net</fo:block>
                            </fo:table-cell>
                        </fo:table-row>
                        <fo:table-row>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_GuaranteedZero"/>
                                    <xsl:text> / </xsl:text>
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_GuaranteedZero"/>
                                </fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Guaranteed"/>
                                    <xsl:text> / </xsl:text>
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Guaranteed"/>
                                </fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_CurrentZero"/>
                                    <xsl:text> / </xsl:text>
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_CurrentZero"/>
                                </fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="2" padding="0pt">
                                <fo:block text-align="right">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                                    <xsl:text> / </xsl:text>
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctNetInt_Current"/>
                                </fo:block>
                            </fo:table-cell>
                        </fo:table-row>
                        <fo:table-row>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Policy
                                    <fo:block></fo:block>Year</fo:block>
                            </fo:table-cell>
                            <xsl:choose>
                                <xsl:when test="illustration/scalar/Composite='1'">
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">Premium Outlay</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right"></fo:block>
                                    </fo:table-cell>
                                </xsl:when>
                                <xsl:otherwise>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">End of &#xA0;&#xA0;&#xA0;&#xA0;Year Age</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">Premium Outlay</fo:block>
                                    </fo:table-cell>
                                </xsl:otherwise>
                            </xsl:choose>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Cash Surr Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Death &#xA0;&#xA0;Benefit</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Cash Surr Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Death &#xA0;&#xA0;Benefit</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Cash Surr Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Death &#xA0;&#xA0;Benefit</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Cash Surr Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Death &#xA0;&#xA0;Benefit</fo:block>
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
                    <fo:table-body>
                        <xsl:call-template name="basic-illustration-values">
                            <xsl:with-param name="counter" select="1"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
        </fo:flow>
    </xsl:template>

    <!-- Create Supplemental Illustration Report -->
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
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Policy
                                    <fo:block></fo:block>Year</fo:block>
                            </fo:table-cell>
                            <xsl:choose>
                                <xsl:when test="illustration/scalar/Composite='1'">
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">Premium Outlay</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right"></fo:block>
                                    </fo:table-cell>
                                </xsl:when>
                                <xsl:otherwise>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">End of &#xA0;&#xA0;&#xA0;&#xA0;Year Age</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">Premium Outlay</fo:block>
                                    </fo:table-cell>
                                </xsl:otherwise>
                            </xsl:choose>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Admin &#xA0;&#xA0;&#xA0;&#xA0;Charge</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Premium &#xA0;&#xA0;&#xA0;Tax Load</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">&#xA0; &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; DAC Tax Load</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Net &#xA0;&#xA0;Premium</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">&#xA0; &#xA0;&#xA0; Mortality Charge</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Account &#xA0;&#xA0;Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Cash Surr Value</fo:block>
                            </fo:table-cell>
                            <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                <fo:block text-align="right">Death &#xA0;&#xA0;Benefit</fo:block>
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
                    <fo:table-body>
                        <xsl:call-template name="supplemental-illustration-values">
                            <xsl:with-param name="counter" select="1"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
        </fo:flow>
    </xsl:template>

    <!-- Create Supplemental Illustration Report -->
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
                    <fo:table-column/>
                    <fo:table-column/>
                    <fo:table-header>
                        <fo:table-row>
                            <fo:table-cell number-columns-spanned="9" padding="0pt">
                                <fo:block text-align="center">&#xA0;</fo:block>
                            </fo:table-cell>
                            <fo:table-cell>
                                <fo:block text-align="right">Assumed</fo:block>
                            </fo:table-cell>
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
                                        <fo:block text-align="center">&#xA0; &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0; Net Crediting Rate</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell number-columns-spanned="3" padding="0pt"/>
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
                                        <fo:block text-align="right">Withdrawal</fo:block>
                                    </fo:table-cell>
                                    <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
                                        <fo:block text-align="right">Loan</fo:block>
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
                    <fo:table-body>
                        <xsl:call-template name="illustration-assumption-values">
                            <xsl:with-param name="counter" select="1"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
            <xsl:choose>
                <xsl:when test="illustration/scalar/SupplementalReport='1'">
                </xsl:when>
                <xsl:otherwise>
                    <fo:block id="endofdoc"></fo:block>
                </xsl:otherwise>
            </xsl:choose>
        </fo:flow>
    </xsl:template>

    <!-- Create Supplemental Report Values -->
    <xsl:template name="supplemental-report-values">
        <xsl:param name="counter"/>
        <xsl:if test="illustration/data/newcolumn/column[1]/duration[$counter]/@column_value!='0'">
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
            <xsl:if test="$counter mod 5=0">
                <fo:table-row>
                    <fo:table-cell padding="4pt">
                        <fo:block text-align="right"></fo:block>
                    </fo:table-cell>
                </fo:table-row>
            </xsl:if>
            <xsl:call-template name="supplemental-report-values">
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
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

    <!-- Standard Footer -->
    <xsl:template name="standardfooter">
        <xsl:param name="displaypagenumber"/>
        <fo:block text-align="left" font-size="8.0pt" font-family="sans-serif">
            <fo:block text-align="left">
                <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
            </fo:block>
            <fo:block text-align="left">
                <xsl:text>See the Explanatory Notes for important policy information. This illustration is not complete without all pages.</xsl:text>
            </fo:block>
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
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
                            <xsl:text>&#xA0;</xsl:text>
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
    <!-- Standard Footer -->
    <xsl:template name="footer-with-underwriter">
        <xsl:param name="displaypagenumber"/>
        <fo:block text-align="left" font-size="8.0pt" font-family="sans-serif">
            <fo:block span="all">
                <fo:block>
                    <fo:leader></fo:leader>
                </fo:block>
                <fo:block>
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
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
            <fo:block text-align="left">
                <xsl:text>* This illustration is based on the assumed Gross Rate shown. The Net Rate is provided for information purposes only.</xsl:text>
            </fo:block>
            <fo:block text-align="left">
                <xsl:text>See the Explanatory Notes for important policy information. This illustration is not complete without all pages.</xsl:text>
            </fo:block>
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
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
                            <xsl:text>&#xA0;</xsl:text>
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

