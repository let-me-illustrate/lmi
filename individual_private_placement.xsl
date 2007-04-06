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

    $Id: individual_private_placement.xsl,v 1.6.2.13 2007-04-06 11:08:39 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
    <xsl:include href="xsl_fo_common.xsl" />
    <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
    <xsl:variable name="counter" select="1"/>
    <xsl:template match="/">
        <fo:root>
            <fo:layout-master-set>

                <!-- Define the cover page. -->
                <fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <fo:region-body margin-top="0.25in" margin-bottom="0.25in"/>
                </fo:simple-page-master>

                <!-- Define the IRR (Guaranteed Charges) Illustration page. -->
                <fo:simple-page-master master-name="irr-guaranteed-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.10in"/>
                    <!-- Header -->
                    <fo:region-before extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after extent="1.1in"/>
                </fo:simple-page-master>

                <!-- Define the IRR (Current Charges) Illustration page -->
                <fo:simple-page-master master-name="irr-current-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.10in"/>
                    <!-- Header -->
                    <fo:region-before extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after extent="1.1in"/>
                </fo:simple-page-master>

                <!-- Define the Current Values Illustration page -->
                <fo:simple-page-master master-name="current-illustration" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.20in"/>
                    <!-- Header -->
                    <fo:region-before extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after extent="1.1in"/>
                </fo:simple-page-master>

                <!-- Define the footnotes page. -->
                <fo:simple-page-master master-name="footnotes" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.25in" margin-bottom="1in"/>
                    <!-- Header -->
                    <fo:region-before extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after extent=".5in"/>
                </fo:simple-page-master>

                <!-- Define the Supplemental Illustration page. -->
                <xsl:if test="$has_supplemental_report">
                    <fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
                    <!-- Central part of page -->
                    <fo:region-body column-count="1" margin-top="2.60in" margin-bottom="1.5in"/>
                    <!-- Header -->
                    <fo:region-before extent="3in"/>
                    <!-- Footer -->
                    <fo:region-after extent="1.1in"/>
                    </fo:simple-page-master>
                </xsl:if>
            </fo:layout-master-set>

            <!-- The data to be diplayed in the pages, cover page first -->
<!-- Turn off cover page for now
            <fo:page-sequence master-reference="cover" initial-page-number="200">
                <fo:flow flow-name="xsl-region-body">
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <fo:block>
                            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                        </fo:block>
                        <fo:block>
                            <xsl:value-of select="illustration/scalar/PolicyLegalName"/>
                            <xsl:text> Cover Sheet</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>The purpose of the attached illustration is to show how the performance of the underlying separate account divisions could affect the proposed </xsl:text>
                            <xsl:text>contract's cash values and death benefits. The illustration is hypothetical and may not be used to project or predict investment results. The illustration </xsl:text>
                            <xsl:text>"must be accompanied or preceded by a Confidential Private Placement Memorandum offering the Contract.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Placement agent: </xsl:text>
                            <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <xsl:if test="illustration/scalar/ScaleUnit!=''">
                            <fo:block>
                                <xsl:text>Values per </xsl:text>
                                <xsl:value-of select="illustration/scalar/ScaleUnit"/>
                                <xsl:text> US dollars.</xsl:text>
                            </fo:block>
                            <fo:block>
                                <fo:leader></fo:leader>
                            </fo:block>
                        </xsl:if>
                        <fo:block text-align="center">
                            <xsl:text>THIS ILLUSTRATION COVER SHEET IS FOR REGISTERED REPRESENTATIVE USE ONLY. NOT FOR USE WITH CLIENTS.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Date prepared: </xsl:text>
                            <xsl:value-of select="illustration/scalar/PrepYear"/>
                            <xsl:text>-</xsl:text>
                            <xsl:value-of select="illustration/scalar/PrepMonth"/>
                            <xsl:text>-</xsl:text>
                            <xsl:value-of select="illustration/scalar/PrepDay"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Producer: </xsl:text>
                            <xsl:value-of select="illustration/scalar/ProducerName"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Client: </xsl:text>
                            <xsl:value-of select="illustration/scalar/Insured1"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Gender: </xsl:text>
                            <xsl:value-of select="illustration/scalar/Gender"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Issue age: </xsl:text>
                            <xsl:value-of select="illustration/scalar/Age"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>State of jurisdiction: </xsl:text>
                            <xsl:value-of select="illustration/scalar/GetStatePostalAbbrev"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Country: </xsl:text>
                            <xsl:value-of select="illustration/scalar/CountryIso3166Abbrev"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Non-US corridor factor: </xsl:text>
                            <xsl:value-of select="illustration/scalar/CorridorFactorSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Rate class: </xsl:text>
                            <xsl:value-of select="illustration/scalar/UWClass"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>7702 test: </xsl:text>
                            <xsl:value-of select="illustration/scalar/DefnLifeIns"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>MEC: </xsl:text>
                            <xsl:choose>
                                <xsl:when test="illustration/scalar/IsMec='1'">
                                    <xsl:text>Yes</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>No</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Assumed gross rate: </xsl:text>
                            <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>New cash loan: </xsl:text>
                            <xsl:value-of select="illustration/scalar/LoanSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Withdrawal: </xsl:text>
                            <xsl:value-of select="illustration/scalar/NetWDSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Comments:  </xsl:text>
                            <xsl:value-of select="illustration/scalar/Comments"/>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Premiums and death benefits</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Premium: </xsl:text>
                            <xsl:value-of select="illustration/scalar/GrossPmtSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Dumpin: </xsl:text>
                            <xsl:value-of select="illustration/scalar/DumpinScaled"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Internal 1035 exchange: </xsl:text>
                            <xsl:value-of select="illustration/scalar/Internal1035AmountScaled"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>External 1035 exchange: </xsl:text>
                            <xsl:value-of select="illustration/scalar/External1035AmountScaled"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Specified amount: </xsl:text>
                            <xsl:value-of select="illustration/scalar/SpecAmtSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Death benefit option: </xsl:text>
                            <xsl:value-of select="illustration/scalar/DBOptSquence"/>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Current fees and charges</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>State premium tax load: </xsl:text>
                            <xsl:value-of select="illustration/scalar/StatePremTaxLoad"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>DAC tax premium load: </xsl:text>
                            <xsl:value-of select="illustration/scalar/DacTaxPremLoadRate"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Asset-based compensation: </xsl:text>
                            <xsl:value-of select="illustration/scalar/AddonCompOnAssetsSquence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Premium-based compensation: </xsl:text>
                            <xsl:value-of select="illustration/scalar/AddonCompOnPremiumSequence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Add-on custodial fee: </xsl:text>
                            <xsl:value-of select="illustration/scalar/AddonMonthlyFeeSquence"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Country COI multiplier: </xsl:text>
                            <xsl:value-of select="illustration/scalar/CountryCOIMultiplier"/>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Separate account load: </xsl:text>
                            <xsl:value-of select="illustration/scalar/TieredSepAcctLoadBands"/>
                        </fo:block>
                    </fo:block>
                </fo:flow>
            </fo:page-sequence>
-->

            <!-- COMPLIANCE -->
            <!-- IRR Guaranteed Charges) Illustration -->
            <!-- Body page -->
            <fo:page-sequence master-reference="irr-guaranteed-illustration"  initial-page-number="1">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="displaycontractlanguage" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                    <fo:block text-align="center" font-size="9.0pt" font-family="serif">
                        <xsl:text>End of Year Contract Values using Guaranteed Charges </xsl:text>
                        <xsl:call-template name="dollar-units"/>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
                        <xsl:text> </xsl:text>
                    </fo:block>
                    <xsl:call-template name="standardfooter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                </fo:static-content>
                <xsl:call-template name="irr-guaranteed-illustration-report"/>
            </fo:page-sequence>

            <!-- IRR (Current Charges) Illustration page. -->
            <!-- Body page -->
            <fo:page-sequence master-reference="irr-current-illustration">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="displaycontractlanguage" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                    <fo:block text-align="center" font-size="9.0pt" font-family="serif">
                        <xsl:text>End of Year Contract Values using Current Charges </xsl:text>
                        <xsl:call-template name="dollar-units"/>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
                        <xsl:text> </xsl:text>
                    </fo:block>
                    <xsl:call-template name="standardfooter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                </fo:static-content>
                <xsl:call-template name="irr-current-illustration-report"/>
            </fo:page-sequence>

            <!-- Current Values Illustration -->
            <!-- Body page -->
            <fo:page-sequence master-reference="current-illustration">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--

                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="displaycontractlanguage" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                    <fo:block text-align="center" font-size="9.0pt" font-family="serif">
                        <xsl:text>End of Year Contract Values using Current Charges </xsl:text>
                        <xsl:call-template name="dollar-units"/>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
                        <xsl:text> </xsl:text>
                    </fo:block>
                    <xsl:call-template name="standardfooter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="1"/>
                    </xsl:call-template>
                </fo:static-content>
                <xsl:call-template name="current-illustration-report"/>
            </fo:page-sequence>

            <!-- FOOTNOTES - begins here -->
            <fo:page-sequence master-reference="footnotes">
                <!-- Define the contents of the header. -->
                <fo:static-content flow-name="xsl-region-before">
<!--
                    <fo:block text-align="left">
                        <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                    </fo:block>
-->
                    <xsl:call-template name="standardheader">
                        <xsl:with-param name="displaycontractlanguage" select="0"/>
                        <xsl:with-param name="displaydisclaimer" select="0"/>
                    </xsl:call-template>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                    <fo:block text-align="center" font-size="10.0pt" font-family="serif">
                        <xsl:text>Footnotes</xsl:text>
                    </fo:block>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </fo:static-content>

                <!-- Define the contents of the footer. -->
                <fo:static-content flow-name="xsl-region-after">
                    <xsl:call-template name="standardfooter">
                        <xsl:with-param name="displaypagenumber" select="1"/>
                        <xsl:with-param name="displaydisclaimer" select="0"/>
                    </xsl:call-template>
                </fo:static-content>

                <!-- FOOTNOTES Body  -->
                <fo:flow flow-name="xsl-region-body">
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <fo:block>
                            <xsl:text>This Contract is only available to persons who are deemed accredited investors or qualified purchasers under applicable Federal securities laws. The minimum initial case premium is $1,000,000. In the case of a dedicated separate account or division, the minimum initial case premium is $5,000,000.</xsl:text>
                            <xsl:text>You must be able to bear the risk of loss of your entire investment in the Contract. You will be required to represent to </xsl:text>
                            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                            <xsl:text> that you satisfy these Contract requirements.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>The information provided is an illustration only and is not intended to predict actual performance.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Interest rates and values set forth in the illustration are not guaranteed. This illustration assumes that the currently illustrated elements will continue unchanged for all years shown. This is not likely to occur and actual results may be more or less favorable than shown. Benefits and values are not guaranteed and are based on assumptions such as investment income and current monthly charges. Current charges are subject to change.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>These illustrations assume a gross rate of return of 0% and </xsl:text>
                            <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                            <xsl:text>, respectively, on the account value in the separate account. For purposes of this </xsl:text>
                            <xsl:text>illustration, the assumed "gross rate of return" takes into account all investment management, custody and other expenses charged by the investment </xsl:text>
                            <xsl:text>manager or underlying fund. Actual rates of return will be different because they will be based on the actual performance of the separate account </xsl:text>
                            <xsl:text>divisions. However, the actual returns of the separate account divisions will also be net of investment management, custody and other expenses of </xsl:text>
                            <xsl:text>the investment manager or the fund. For some separate accounts or divisions, there may be additional investment management and/or separate </xsl:text>
                            <xsl:text>account administrative fees that may be charged, in which case those fees will be included in the Asset Charges illustrated above. If additional fees </xsl:text>
                            <xsl:text>are determined subsequent to issuance of this illustration, such fees will be added to the Asset Charges.  You should request a new illustration to review </xsl:text>
                            <xsl:text>the impact of such fees on contract performance. No tax charge is currently applied to the investment returns of the separate account. A charge could </xsl:text>
                            <xsl:text>be made in the future.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Asset charges in the illustration include the contract’s mortality and expense (asset charges), separate account administrative charges (including in </xsl:text>
                            <xsl:text>certain cases investment management expenses), and, where applicable, asset-based compensation and/or amortized premium loads.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Account values may be used to pay monthly charges. Monthly charges are due during the life of the insured, and depending on actual results, the </xsl:text>
                            <xsl:text>premium payer may need to continue or resume premium outlays. If account values are allocated to certain separate account divisions, on each </xsl:text>
                            <xsl:text>Contract Anniversary Date there must be at least 18 months of current monthly charges maintained in the money market division to pay contract charges.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>The premium outlay column includes premium payments, reduced by withdrawals and loan disbursements, if any.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Premiums are assumed to be paid on a</xsl:text>
                            <xsl:if test="illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value='Annual'">
                                <xsl:text>n </xsl:text>
                            </xsl:if>
                            <xsl:value-of select="translate(illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value,$UPPER_CASE_LETTERS,$LOWER_CASE_LETTERS)"/>
                            <xsl:text> basis and received at the beginning of the contract year. Age, account values, cash surrender values and death benefits are illustrated as of the end of the contract year.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>PLEASE READ THE FOLLOWING IMPORTANT MAXIMUM NET AMOUNT AT RISK DISCLOSURE</xsl:text>
                        </fo:block>
                        <fo:block>
                            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                            <xsl:text>  has the right to promptly refund any amount of premium paid if the premium payment will cause the net amount at risk to exceed the maximum net amount at risk under the Contract. </xsl:text>
                            <xsl:value-of select="illustration/scalar/InsCoShortName"/>
                            <xsl:text> also has the right to automatically withdraw from the divisions of the separate account, and distribute to the Contract holder, excess amounts that cause the net amount at risk to exceed the maximum net amount at risk under the Contract.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>If extensive loans or withdrawals are made, additional premium payments may be necessary to avoid lapse of the Contract. Periods of poor performance </xsl:text>
                            <xsl:text>in the underlying investment portfolios may also contribute to the potential need for additional premium payments.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Certain investment divisions have restrictions on the ability to access account values allocated to those divisions. If account values are allocated to such </xsl:text>
                            <xsl:text>divisions, the Owner will not be able to transfer account values or effect a loan, withdrawal or surrender of the Contract until specified dates. In addition, </xsl:text>
                            <xsl:text>payment of a portion of the death benefit may be deferred until the account values in such division can be liquidated. Refer to the private placement memorandum for details.</xsl:text>
                        </fo:block>
                </fo:block>

                    <!-- Forced New Page -->
                    <fo:block break-after="page"/>
                    <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
                        <fo:block>
                            <xsl:text>In the states of Alaska or South Dakota, there may be a surrender charge in connection with premium tax liability that would generally be limited to 6 years. </xsl:text>
                            <xsl:text>This surrender charge is not reflected in the illustrated values. Nevertheless, surrender charges under the Contract are limited by applicable non-forfeiture laws and regulations.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>PLEASE READ THE FOLLOWING IMPORTANT TAX DISCLOSURE</xsl:text>
                        </fo:block>
                        <fo:block>
                            <xsl:text>The definition of life insurance elected for this Contract is the </xsl:text>
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
                            <xsl:text>The state of issue is </xsl:text>
                            <xsl:value-of select="illustration/scalar/StatePostalAbbrev"/>
                            <xsl:text>.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Please refer to the Contract for a complete explanation of benefits, rights and obligations. In the event of a conflict between the illustration and the Contract, the terms of the Contract will control.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Internal Rate of Return ("IRR") is an interest rate at which the Premium Outlay illustrated would have to be invested outside the Contract to generate the Cash Surrender Value or Death Benefit. The IRR is illustrative only and does not reflect past or future results.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Placement Agent:  </xsl:text>
                            <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
                            <xsl:text>, </xsl:text>
                            <xsl:value-of select="illustration/scalar/MainUnderwriterAddress"/>
                            <xsl:text>. </xsl:text>
                            <xsl:value-of select="illustration/scalar/MainUnderwriter"/>
                            <xsl:text> is a wholly owned subsidiary of </xsl:text>
                            <xsl:value-of select="illustration/scalar/InsCoName"/>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>This illustration reflects a fixed policy loan interest rate of </xsl:text>
                            <xsl:value-of select="illustration/scalar/InitAnnLoanDueRate"/>
                            <xsl:text>.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>In general, policy loan interest is not deductible. If the policy is owned by a business, deductibility is extremely limited. Please see your tax counsel for advice.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>Taking a policy loan could have adverse tax consequences. If your aggregate policy loans substantially exceed your cost basis, you may incur a significant </xsl:text>
                            <xsl:text>income tax liability if the policy terminates before the insured's death. You may have to make substantial payments to cover policy charges and policy loan </xsl:text>
                            <xsl:text>interest to prevent termination of the policy and to avoid the potential income tax liability. Some of the indications that such a situation may arise include: </xsl:text>
                            <xsl:text>(1) high outstanding debt relative to the policy's account value; (2) low account value relative to a high death benefit; and (3) lower than expected interest </xsl:text>
                            <xsl:text>credited to your account value. You should also understand that generally your insurance risk charge rates increase annually as the age of the insured </xsl:text>
                            <xsl:text>increases, and that the risk charge increases as the amount of policy risk increases. You can reduce the likelihood that you either will incur a significant </xsl:text>
                            <xsl:text>income tax liability should your policy terminate before the death of the insured, or that you will need to make substantial payments to keep your policy </xsl:text>
                            <xsl:text>from terminating by monitoring and reviewing all aspects of your policy on a regular basis with your tax advisor, your financial representative, and/or any </xsl:text>
                            <xsl:text>other financial advisor you might have.</xsl:text>
                        </fo:block>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block>
                            <xsl:text>This illustration must be preceded or accompanied by the current confidential private placement memorandum for </xsl:text>
                            <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                            <xsl:text> variable life insurance contract and the current prospectuses and private placement memorandums for its underlying investment choices.  Before purchasing </xsl:text>
                            <xsl:text>a variable life insurance contract, investors should carefully consider the investment objectives, risks, charges and expenses of the variable life insurance </xsl:text>
                            <xsl:text>contract and its underlying investment choices. Please read the prospectuses and private placement memorandums carefully before investing or sending money.</xsl:text>
                        </fo:block>
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

            <!-- Supplemental Illustration -->
            <!-- Body page -->
            <xsl:if test="$has_supplemental_report">
                <fo:page-sequence master-reference="supplemental-report">
                    <!-- Define the contents of the header. -->
                    <fo:static-content flow-name="xsl-region-before">
<!--
                        <fo:block text-align="left">
                            <fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
                        </fo:block>
-->
                        <xsl:call-template name="standardheader">
                            <xsl:with-param name="displaycontractlanguage" select="1"/>
                            <xsl:with-param name="displaydisclaimer" select="1"/>
                        </xsl:call-template>
                        <fo:block>
                            <fo:leader></fo:leader>
                        </fo:block>
                        <fo:block text-align="center" font-size="9.0pt" font-family="serif">
                            <xsl:value-of select="illustration/supplementalreport/title"/>
                            <xsl:text> </xsl:text>
                            <xsl:call-template name="dollar-units"/>
                        </fo:block>
                    </fo:static-content>
                    <!-- Define the contents of the footer. -->
                    <fo:static-content flow-name="xsl-region-after">
                        <fo:block font-size="8.0pt" font-family="sans-serif" padding-after="2.0pt" space-before="4.0pt" text-align="left">
                            <xsl:text> </xsl:text>
                        </fo:block>
                        <xsl:call-template name="standardfooter">
                            <xsl:with-param name="displaypagenumber" select="1"/>
                            <xsl:with-param name="displaydisclaimer" select="1"/>
                        </xsl:call-template>
                    </fo:static-content>
                    <!-- Supplemental report body -->
                    <xsl:call-template name="supplemental-report-body" />
                </fo:page-sequence>
            </xsl:if>
        </fo:root>
    </xsl:template>
    <xsl:template name="standardheader">
        <xsl:param name="displaycontractlanguage"/>
        <xsl:param name="displaydisclaimer"/>
        <fo:block text-align="center" font-size="9.0pt">
            <fo:block>
                <fo:leader></fo:leader>
            </fo:block>
            <xsl:choose>
                <xsl:when test="$displaycontractlanguage=1">
                    <fo:block>
                        <xsl:text>Illustration for Flexible Premium Variable Adjustable Life Insurance Contract.</xsl:text>
                    </fo:block>
                </xsl:when>
                <xsl:otherwise>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:choose>
                <xsl:when test="$displaydisclaimer=1">
                    <fo:block>
                        <xsl:text>The purpose of the attached illustration is to show how the performance of the underlying separate account divisions could affect the proposed contract's cash values and death benefits. The illustration is hypothetical and may not be used to project or predict investment results. The illustration must be accompanied or preceded by a Confidential Private Placement Memorandum offering the Contract.</xsl:text>
                    </fo:block>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </xsl:when>
                <xsl:otherwise>
                    <fo:block>
                        <fo:leader></fo:leader>
                    </fo:block>
                </xsl:otherwise>
            </xsl:choose>
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
                        <xsl:text>Date Prepared: </xsl:text>
                        <xsl:value-of select="illustration/scalar/PrepMonth"/>
                        <xsl:text> </xsl:text>
                        <xsl:value-of select="illustration/scalar/PrepDay"/>
                        <xsl:text>, </xsl:text>
                        <xsl:value-of select="illustration/scalar/PrepYear"/>
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
                                    <xsl:text>Contract:  </xsl:text>
                                    <xsl:value-of select="illustration/scalar/PolicyMktgName"/>
                                </fo:block>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <xsl:choose>
                        <xsl:when test="$is_composite">
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>Composite of individuals</xsl:text>
                            </fo:block>
                        </xsl:when>
                        <xsl:otherwise>
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>Prepared for: </xsl:text>
                                <xsl:value-of select="illustration/scalar/Insured1"/>
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
                                <fo:block text-align="left">
                                    <xsl:text>&#xA0; </xsl:text>
                                </fo:block>
                            </fo:list-item-body>
                        </fo:list-item>
                    </fo:list-block>
                </fo:list-item-body>
            </fo:list-item>
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <xsl:choose>
                        <xsl:when test="$is_composite">
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>&#xA0; </xsl:text>
                            </fo:block>
                        </xsl:when>
                        <xsl:otherwise>
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>Gender: </xsl:text>
                                <xsl:value-of select="illustration/scalar/Gender"/>
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
                                    <xsl:when test="$is_composite">
                                        <fo:block text-align="left" color="white">
                                            <xsl:text>.</xsl:text>
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
                                                <fo:block text-align="left">
                                                    <xsl:text>Underwriting Type: </xsl:text>
                                                    <xsl:value-of select="illustration/scalar/UWType"/>
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
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <xsl:choose>
                        <xsl:when test="$is_composite">
                            <fo:block text-align="left" font-size="9.0pt">
                                <xsl:text>&#xA0;</xsl:text>
                            </fo:block>
                        </xsl:when>
                        <xsl:otherwise>
                            <fo:block text-align="left">
                                <xsl:text>Age: </xsl:text>
                                <xsl:value-of select="illustration/scalar/Age"/>
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
                                    <xsl:when test="$is_composite">
                                        <fo:block text-align="left" font-size="9.0pt">
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <fo:block text-align="left">
                                            <xsl:text>Rate Classification: </xsl:text>
                                            <xsl:value-of select="illustration/scalar/Gender"/>
                                            <xsl:text>, </xsl:text>
                                            <xsl:value-of select="illustration/scalar/Smoker"/>
                                            <xsl:text>, </xsl:text>
                                            <xsl:value-of select="illustration/scalar/UWClass"/>
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
                        <xsl:text>Selected Face Amount: $</xsl:text>
                        <xsl:value-of select="illustration/scalar/InitTotalSA"/>
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
                                    <xsl:when test="$is_composite">
                                        <fo:block text-align="left">
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:choose>
                                            <xsl:when test="illustration/scalar/UWClass='Rated'">
                                                <fo:block text-align="left">
                                                    <xsl:text>&#xA0;&#xA0;&#xA0;Table Rating: </xsl:text>
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
            <fo:list-item>
                <fo:list-item-label end-indent="label-end()">
                    <fo:block text-align="left">
                        <xsl:text>Initial Death Benefit Option: </xsl:text>
                        <xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
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
                                    <xsl:when test="$is_composite">
                                        <fo:block text-align="left">
                                            <xsl:text>&#xA0;</xsl:text>
                                        </fo:block>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:choose>
                                            <xsl:when test="illustration/scalar/UWClass='Rated'">
                                                <fo:block text-align="left">
                                                    <xsl:text>&#xA0;&#xA0;&#xA0;Initial Annual Flat Extra: </xsl:text>
                                                    <xsl:value-of select="illustration/data/newcolumn/column[@name='MonthlyFlatExtra']/duration[1]/@column_value"/>
                                                    <xsl:text> per 1,000</xsl:text>
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

    <!-- Create Define the IRR (Guaranteed Charges) Report -->
    <xsl:template name="irr-guaranteed-illustration-report">
        <xsl:variable name="irr_guaranteed_illustration_columns_raw">
            <column name="PolicyYear">Year</column>
            <column composite="1"/>
            <column composite="0" name="AttainedAge">Age</column>
            <column name="Outlay">Premium Outlay</column>
            <column name="CSVNet_GuaranteedZero">Cash _ Surr Value</column>
            <column name="EOYDeathBft_GuaranteedZero">Death _ Benefit</column>
            <column name="IrrCsv_GuaranteedZero">IRR on _ Surr Value</column>
            <column name="IrrDb_GuaranteedZero">IRR on _ Death Bft</column>
            <column />
            <column name="CSVNet_Guaranteed">Cash _ Surr Value</column>
            <column name="EOYDeathBft_Guaranteed">Death _ Benefit</column>
            <column name="IrrCsv_Guaranteed">IRR on _ Surr Value</column>
            <column name="IrrDb_Guaranteed">IRR on _ Death Bft</column>
        </xsl:variable>
        <xsl:variable name="columns_raw" select="document('')/xsl:stylesheet/xsl:template[@name='irr-guaranteed-illustration-report']/xsl:variable[@name='irr_guaranteed_illustration_columns_raw']/column"/>
        <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>
        <!-- The main contents of the body page -->
        <fo:flow flow-name="xsl-region-body">
            <fo:block font-size="9.0pt" font-family="serif">
                <fo:table table-layout="fixed" width="100%">
                    <xsl:call-template name="data-table-columns">
                        <xsl:with-param name="columns" select="$columns"/>
                    </xsl:call-template>
                    <fo:table-header>
                        <fo:table-row>
                            <fo:table-cell />
                        </fo:table-row>
                        <fo:table-row>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                                <fo:block text-align="center">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_GuaranteedZero"/>
                                    <xsl:text> Gross / Net Rate</xsl:text>
                                </fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                                <fo:block text-align="center">
                                <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Guaranteed"/>
                                    <xsl:text> Gross / Net Rate</xsl:text>
                                </fo:block>
                            </fo:table-cell>
                        </fo:table-row>
                        <fo:table-row>
                            <fo:table-cell padding="2pt">
                                <fo:block text-align="right"></fo:block>
                            </fo:table-cell>
                        </fo:table-row>
                        <xsl:call-template name="data-table-headers">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-header>
                    <fo:table-body>
                        <xsl:call-template name="data-table-data">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
        </fo:flow>
    </xsl:template>

    <!-- Create IRR (Current Charges) Report -->
    <xsl:template name="irr-current-illustration-report">
        <xsl:variable name="irr_current_illustration_columns_raw">
            <column name="PolicyYear">Year</column>
            <column composite="1"/>
            <column composite="0" name="AttainedAge">Age</column>
            <column name="Outlay">Premium Outlay</column>
            <column name="CSVNet_CurrentZero">Cash Surr Value</column>
            <column name="EOYDeathBft_CurrentZero">Death _ Benefit</column>
            <column name="IrrCsv_CurrentZero">IRR on Surr Value</column>
            <column name="IrrDb_CurrentZero">IRR on Death Bft</column>
            <column/>
            <column name="CSVNet_Current">Cash Surr Value</column>
            <column name="EOYDeathBft_Current">Death _ Benefit</column>
            <column name="IrrCsv_Current">IRR on Surr Value</column>
            <column name="IrrDb_Current">IRR on Death Bft</column>
        </xsl:variable>
        <xsl:variable name="columns_raw" select="document('')/xsl:stylesheet/xsl:template[@name='irr-current-illustration-report']/xsl:variable[@name='irr_current_illustration_columns_raw']/column"/>
        <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>
        <!-- The main contents of the body page -->
        <fo:flow flow-name="xsl-region-body">
            <fo:block font-size="9.0pt" font-family="serif">
                <fo:table table-layout="fixed" width="100%">
                    <xsl:call-template name="data-table-columns">
                        <xsl:with-param name="columns" select="$columns"/>
                    </xsl:call-template>
                    <fo:table-header>
                        <fo:table-row>
                            <fo:table-cell />
                        </fo:table-row>
                        <fo:table-row>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                                <fo:block text-align="center">
                                    <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_CurrentZero"/>
                                    <xsl:text> Gross / Net Rate</xsl:text>
                                </fo:block>
                            </fo:table-cell>
                            <fo:table-cell number-columns-spanned="3" padding="0pt"/>
                            <fo:table-cell number-columns-spanned="2" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
                                <fo:block text-align="center">
                                <xsl:value-of select="illustration/scalar/InitAnnSepAcctGrossInt_Current"/>
                                    <xsl:text> Gross / Net Rate</xsl:text>
                                </fo:block>
                            </fo:table-cell>
                        </fo:table-row>
                        <xsl:call-template name="data-table-headers">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-header>
                    <fo:table-body>
                        <xsl:call-template name="data-table-data">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
        </fo:flow>
    </xsl:template>

    <!-- Current Values Report -->
    <xsl:template name="current-illustration-report">
        <xsl:variable name="current_illustration_columns_raw">
            <column name="PolicyYear">Year</column>
            <column composite="1"/>
            <column composite="0" name="AttainedAge">Age</column>
            <column name="Outlay">Premium Outlay</column>
            <column name="Special_PremiumLoad">Premium Loads</column>
            <column name="Special_AdminCharges">Admin Charges</column>
            <column name="COICharge_Current">Mortality Charges</column>
            <column name="Special_AssetCharges">Asset Charges</column>
            <column name="GrossIntCredited_Current">Investment Income</column>
            <column name="AcctVal_Current">Account Value</column>
            <column name="CSVNet_Current">Cash _ Surr Value</column>
            <column name="EOYDeathBft_Current">Death _ Benefit</column>
        </xsl:variable>
        <xsl:variable name="columns_raw" select="document('')/xsl:stylesheet/xsl:template[@name='current-illustration-report']/xsl:variable[@name='current_illustration_columns_raw']/column"/>
        <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>
        <!-- The main contents of the body page -->
        <fo:flow flow-name="xsl-region-body">
            <fo:block font-size="9.0pt" font-family="serif">
                <fo:table table-layout="fixed" width="100%">
                    <xsl:call-template name="data-table-columns">
                        <xsl:with-param name="columns" select="$columns"/>
                    </xsl:call-template>
                    <fo:table-header>
                        <xsl:call-template name="data-table-headers">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-header>
                    <fo:table-body>
                        <xsl:call-template name="data-table-data">
                            <xsl:with-param name="columns" select="$columns"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
        </fo:flow>
    </xsl:template>

    <!-- Standard Footer -->
    <xsl:template name="standardfooter">
        <xsl:param name="displaypagenumber"/>
        <xsl:param name="displaydisclaimer"/>
        <fo:block text-align="left" font-size="8.0pt" font-family="sans-serif">
            <xsl:if test="$displaydisclaimer=1">
                <fo:block text-align="left">
                    <xsl:text>This Illustration is not a contract, or an offer or solicitation to enter into a contract. Illustrated values are based on the investment earnings assumptions shown above and are not guaranteed. Values based on current charges reflect applicable fees and charges which are subject to change. There are no guaranteed values under this contract. The impact of tax requirements is not reflected in these values. Consult your tax advisor.</xsl:text>
                </fo:block>
                <fo:block>
                    <fo:leader></fo:leader>
                </fo:block>
            </xsl:if>
        </fo:block>
        <fo:block padding-before="5pt" font-size="8.0pt" font-family="sans-serif">
            <fo:table table-layout="fixed" width="100%" padding-before="2pt" border-top-style="solid" border-top-width="1pt" border-top-color="blue">
                <fo:table-column column-width="proportional-column-width(1)"/>
                <fo:table-column column-width="proportional-column-width(1)"/>
                <fo:table-column column-width="proportional-column-width(1)"/>
                <fo:table-body>
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
                                <xsl:text>&#xA0;</xsl:text>
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

