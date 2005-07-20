<?xml version="1.0" encoding="UTF-8"?>
<!--
Life insurance illustrations.

Copyright (C) 2004 Gregory W. Chicares.

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
email: <chicares@mindspring.com>
snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA
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
	<xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
	<xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
	<xsl:variable name="allletters"> ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789&#xA0;</xsl:variable>
	<xsl:variable name="noampletters"> ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_</xsl:variable>
	<xsl:template match="/">
		<fo:root>
			<fo:layout-master-set>

				<!-- Define the cover page. -->
				<fo:simple-page-master master-name="cover" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<fo:region-body margin-top="0.25in" margin-bottom="0.25in"/>
				</fo:simple-page-master>

				<!-- Define the narrative summary page. -->
				<fo:simple-page-master master-name="narrative-summary" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="0.5in" margin-bottom="1in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="3in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent="0.40in"/>
				</fo:simple-page-master>

				<!-- Define the narrative summary page. -->
				<fo:simple-page-master master-name="column-headings-and-key-terms" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="0.15in" margin-bottom=".4in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="2in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent="0.40in"/>
				</fo:simple-page-master>

				<!-- Define the Numeric Summary page. -->
				<fo:simple-page-master master-name="numeric-summary" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="3in" margin-bottom=".47in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="3in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent="0.47in"/>
				</fo:simple-page-master>

				<!-- Define the Tabular Detail page. -->
				<fo:simple-page-master master-name="tabular-detail" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="3in" margin-bottom="1.15in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="3in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent="1.15in"/>
				</fo:simple-page-master>

				<!-- Define the Tabular Detail (Report 2) page. -->
				<fo:simple-page-master master-name="tabular-detail-report2" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="3in" margin-bottom="1.25in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="3in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent="1.15in"/>
				</fo:simple-page-master>

				<!-- Define the Supplemental Illustration page. -->
				<xsl:if test="illustration/scalar/SupplementalReport='1'">
					<fo:simple-page-master master-name="supplemental-report" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
						<!-- Central part of page -->
						<fo:region-body column-count="1" margin-top="3in" margin-bottom="1.2in"/>
						<!-- Header -->
						<fo:region-before border-after-style="solid" extent="3in"/>
						<!-- Footer -->
						<fo:region-after border-before-style="solid" extent="1.15in"/>
					</fo:simple-page-master>
				</xsl:if>

				<!-- Define the Numeric Summary Attachment page. -->
				<fo:simple-page-master master-name="numeric-summary-attachment" page-height="11in" page-width="8.5in" margin-top="0.25in" margin-bottom="0.25in" margin-left="0.25in" margin-right="0.25in">
					<!-- Central part of page -->
					<fo:region-body column-count="1" margin-top="3in" margin-bottom="0.47in"/>
					<!-- Header -->
					<fo:region-before border-after-style="solid" extent="3in"/>
					<!-- Footer -->
					<fo:region-after border-before-style="solid" extent=".47in"/>
				</fo:simple-page-master>
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

			<!-- NARRATIVE SUMMARY - begins here -->
			<fo:page-sequence master-reference="narrative-summary">
				<!-- Define the contents of the header. -->
				<fo:static-content flow-name="xsl-region-before">
<!--
					<fo:block text-align="left">
						<fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
					</fo:block>
-->
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
					<fo:block>
						<fo:leader></fo:leader>
					</fo:block>
					<fo:block>
						<fo:leader></fo:leader>
					</fo:block>
					<fo:block text-align="center" font-size="10pt">
						<xsl:text>NARRATIVE SUMMARY</xsl:text>
					</fo:block>
					<fo:block>
						<fo:leader></fo:leader>
					</fo:block>
					<fo:block text-align="left" font-size="9pt" font-family="sans-serif">
						<fo:block>
							<xsl:value-of select="illustration/scalar/PolicyMktgName"/>
							<xsl:text> is a flexible premium adjustable life insurance contract.</xsl:text>
						</fo:block>
						<fo:block>
							<xsl:text>It features accumulating cash values, adjustable benefits, and flexible premiums.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Coverage may be available on a Guaranteed Standard Issue basis. All proposals are based on case characteristics and must be approved by the </xsl:text>
							<xsl:value-of select="illustration/scalar/InsCoShortName"/>
							<xsl:text> Home Office. For details regarding underwriting and coverage limitations refer to your offer letter or contact your </xsl:text>
							<xsl:value-of select="illustration/scalar/InsCoShortName"/>
							<xsl:text> representative.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>This is an illustration only. An illustration is not intended to predict actual performance. Interest rates </xsl:text>
							<xsl:if test="illustration/scalar/Participating='1'">
								<xsl:text>, dividends,</xsl:text>
							</xsl:if>
							<xsl:text> and values set forth in the illustration are not guaranteed.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>You have the right to return the contract. If you decide not to keep the contract, return it within 10 days after you receive it.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text></xsl:text>
						</fo:block>
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
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Values may be used to pay monthly charges. Monthly charges are due during the life of the insured, and depending on actual results, the premium payor may need to continue or resume premium outlays.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Premiums are assumed to be paid on a</xsl:text>
							<xsl:if test="illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value='Annual'">
								<xsl:text>n </xsl:text>
							</xsl:if>
							<xsl:value-of select="translate(illustration/data/newcolumn/column[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
							<xsl:text> basis and received at the beginning of the contract year. </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Values, </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Values, and death benefits are illustrated as of the end of the contract year.  The method we use to allocate overhead expenses is the fully allocated expense method.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
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
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Loaned amounts of the </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value will be credited a rate equal to the loan interest rate less a spread, guaranteed not to exceed 3.00%.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<xsl:if test="illustration/scalar/HasTerm='1'">
							<fo:block>
								<xsl:text>The term rider provides the option to purchase monthly term insurance on the life of the insured. The term rider selected face amount supplements the selected face amount of the contract. If the term rider is attached, the policy to which it is attached may have a lower annual cutoff premium and, as a result, the lower overall sales loads paid may be lower than a contract having the same total face amount, but with no term rider. Also, the lapse protection feature of the contract's </xsl:text>
								<xsl:value-of select="illustration/scalar/NoLapseProvisionName"/>
								<xsl:text> does not apply to the term rider's selected face amount.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/HasWP='1'">
							<fo:block>
								<xsl:text>The Waiver of Monthly Charges Rider provides for waiver of monthly charges in the event of the disability of the insured that begins before attained age 65 and continues for at least 6 months, as described in the rider. An additional charge is associated with this rider. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/HasADD='1'">
							<fo:block>
								<xsl:text>The Accidental Death benefit provides an additional benefit if death is due to accident. An additional charge is associated with this rider. Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
					</fo:block>>

					<!-- Force Second Page -->
					<fo:block break-after="page"/>
					<fo:block text-align="center" font-size="10pt">
						<xsl:text>NARRATIVE SUMMARY (Continued)</xsl:text>
					</fo:block>
					<fo:block text-align="left" font-size="9pt" font-family="sans-serif">
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
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
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/NoLapse='1'">
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
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/NoLapseForever='1'">
							<fo:block>
								<xsl:text>No-Lapse Guarantee: The contract will remain in force after the first premium has been paid, even if there is insufficient </xsl:text>
								<xsl:value-of select="illustration/scalar/AvName"/>
								<xsl:text> Value to cover the monthly charges provided that the insured is not in a substandard rating class and the policy debt does not exceed </xsl:text>
								<xsl:value-of select="illustration/scalar/AvName"/>
								<xsl:text> Value.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
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
							<xsl:text>This illustration may not reflect your actual tax or accounting consequences. Consult professional advisors for interpretation.</xsl:text>
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
						<xsl:if test="illustration/scalar/Composite='1'">
							<fo:block>
								<xsl:text>Please see the attached census, listing the face amounts, underwriting classes and issue ages for individual participants.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<fo:block>
							<xsl:text>In the states of North Carolina and South Carolina, Guaranteed Issue Underwriting is referred to as "Limited Underwriting" and Simplified Issue Underwriting is referred to as "Simplified Underwriting".</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<xsl:if test="illustration/scalar/StatePostalAbbrev='TX'">
							<xsl:if test="illustration/scalar/UWType='Guaranteed issue'">
								<fo:block>
									<xsl:text>* This policy is classified as substandard guaranteed issue per the requirements of the Texas Insurance Department.</xsl:text>
								</fo:block>
								<fo:block>
									<fo:leader></fo:leader>
								</fo:block>
							</xsl:if>
						</xsl:if>
						<xsl:if test="illustration/scalar/Has1035ExchCharge='1'">
							<fo:block>
								<xsl:text>Upon surrender of this policy, where the surrender proceeds are intended to be applied to an insurance policy or certificate issued in conjunction with an intent to qualify the exchange as a tax free exchange under Section 1035 of the Internal Revenue Code, we may assess an Exchange Charge. The Exchange Charge is the greater of the Market Value Adjustment Charge and the Percentage of Premium Charge. In the states of Florida or Indiana, the Exchange charge (referred to as Assignment Charge in Florida) will be the Percentage of Premium Charge only. The Exchange Charge will potentially reduce the surrender proceeds, but will never increase the surrender proceeds. Please refer to your contract for details.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/HasSpouseRider='1'">
							<fo:block>
								<xsl:text>The $</xsl:text>
								<xsl:value-of select="illustration/scalar/SpouseRiderAmount"/>
								<xsl:text> Spouse rider provides term life insurance on the spouse (issue age </xsl:text>
								<xsl:value-of select="illustration/scalar/SpouseIssueAge"/>
								<xsl:text>) for a limited duration, for an extra charge.  Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<xsl:if test="illustration/scalar/HasChildRider='1'">
							<fo:block>
								<xsl:text>The $</xsl:text>
								<xsl:value-of select="illustration/scalar/ChildRiderAmount"/>
								<xsl:text> Child rider provides term life insurance on the insured's children for a limited duration, for an extra charge.  Please refer to your contract for specific provisions and a detailed schedule of charges.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
					</fo:block>
				</fo:flow>
			</fo:page-sequence>
			<!-- Column Headings and Key Terms - begins here -->
			<fo:page-sequence master-reference="column-headings-and-key-terms">
				<!-- Define the contents of the header. -->
				<fo:static-content flow-name="xsl-region-before">
<!--
					<fo:block text-align="left">
						<fo:external-graphic width="121.1pt" height="24.8pt" src="company_logo.png"/>
					</fo:block>
-->
				</fo:static-content>
				<!-- Define the contents of the footer. -->
				<fo:static-content flow-name="xsl-region-after">
					<xsl:call-template name="standardfooter">
						<xsl:with-param name="displaypagenumber" select="1"/>
					</xsl:call-template>
				</fo:static-content>
				<!-- Column Headings and Key Terms Body  -->
				<fo:flow flow-name="xsl-region-body">
					<fo:block>
						<fo:leader></fo:leader>
					</fo:block>
					<fo:block text-align="center" font-size="10.0pt">
						<xsl:text>Column Headings and Key Terms Used in This Illustration</xsl:text>
					</fo:block>
					<fo:block>
						<fo:leader></fo:leader>
					</fo:block>
					<fo:block text-align="left" font-size="9pt" font-family="sans-serif">
						<fo:block>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value: The accumulation at interest of the net premiums paid, less any withdrawals, less any monthly charges deducted.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value: </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value less policy debt.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Current Scale </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value: The </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value of the contract assuming current interest rates and current monthly charges.  This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Current Scale Death Benefit:  The death benefit of the contract, assuming current interest rates and current monthly charges.  This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Current Scale </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value:  The </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> of the contract, assuming current interest rates and current monthly charges.  This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Death Benefit Option 1:  Option in which the death benefit is equal to the selected face amount of the contract on the date of death of the insured, or if greater the </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value </xsl:text>
							<xsl:if test="illustration/scalar/SalesLoadRefund!='0%'">
								<xsl:text>plus the refund of sales loads (if applicable) </xsl:text>
							</xsl:if>
							<xsl:text>on the insured's date of death multiplied by the minimum face amount percentage for the insured's attained age at death (minimum face amount). Please refer to the contract for a detailed schedule of death benefit factors.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Death Benefit Option 2:  Option in which the death benefit is equal to the selected face amount of the contract plus the </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value on the date of death of the insured, or if greater, the </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value </xsl:text>
							<xsl:if test="illustration/scalar/SalesLoadRefund='1'">
								<xsl:text>plus the refund of sales loads (if applicable) </xsl:text>
							</xsl:if>
							<xsl:text>on the insured's date of death multiplied by the death benefit factor for the insured's attained age at death (minimum face amount). Please refer to the contract for a detailed schedule of death benefit factors.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<xsl:if test="illustration/scalar/SalesLoadRefund='1'">
							<fo:block>
								<xsl:text>Death Benefit Option 3: Option in which the death benefit is equal to the greatest of (1) the selected face amount of the contract on the date of the death of the insured; (2) the selected face amount of the contract on the date of the death of the insured plus the sum of all premiums paid less withdrawals, or; (3) the </xsl:text>
								<xsl:value-of select="illustration/scalar/AvName"/>
								<xsl:text> Value plus the refund of sales loads (if applicable) on the date of death of the insured multiplied by the death benefit factor for the insured's attained age at death (minimum face amount). Please refer to the contract for a detailed schedule of death benefit factors.</xsl:text>
							</fo:block>
							<fo:block>
								<fo:leader></fo:leader>
							</fo:block>
						</xsl:if>
						<fo:block>
							<xsl:text>Flexible Premiums:  Premiums that may be increased, reduced, or not paid, if the </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value is sufficient to cover the monthly charges.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Guaranteed </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value: The </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value of the contract assuming the guaranteed minimum interest rate, the guaranteed maximum monthly charges.  This value is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Guaranteed </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value:  The </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value of the contract assuming the guaranteed minimum interest rate, the guaranteed maximum monthly charges. This value is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Guaranteed Death Benefit:  The amount of proceeds payable upon the death of the insured assuming the guaranteed minimum interest rate and the guaranteed maximum monthly charges. This value is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>MEC:  Modified Endowment Contract - this classification is given to a contract in violation of TAMRA (Technical and Miscellaneous Revenues Act), which limits the amount of premium that can be paid into a life insurance contract. A MEC may be subject to adverse tax treatment upon loan or surrender of </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Midpoint </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value: The </xsl:text>
							<xsl:value-of select="illustration/scalar/AvName"/>
							<xsl:text> Value of the contract assuming monthly charges that are the average of the guaranteed monthly charges and the current monthly charges and an interest rate that is the average of the illustrated crediting rate (or current scale interest rate) and the guaranteed minimum interest rate. This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Midpoint </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value:  The </xsl:text>
							<xsl:value-of select="illustration/scalar/CsvName"/>
							<xsl:text> Value of the contract, assuming monthly charges that are the average of the guaranteed monthly charges and the current monthly charges and an interest rate that is the average of the illustrated crediting rate (or current scale interest rate) and the guaranteed minimum interest rate. This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Midpoint Death Benefit:  The amount of proceeds payable upon the death of the insured, assuming monthly charges that are the average of the guaranteed monthly charges and the current monthly charges and an interest rate that is the average of the illustrated crediting rate (or current scale interest rate) and the guaranteed minimum interest rate. This value is not guaranteed and is based on the assumption that premiums are paid as illustrated.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Monthly Charges:  The monthly charges for the following month which include: cost of insurance,</xsl:text>
							<xsl:if test="illustration/scalar/OffersRiders='1'">
								<xsl:text> plus the cost of any additional benefits provided by rider (if elected),</xsl:text>
							</xsl:if>
							<xsl:text> plus face amount charges (if applicable), plus the administrative charge shown on the contract schedule page.</xsl:text>
						</fo:block>
						<fo:block>
							<fo:leader></fo:leader>
						</fo:block>
						<fo:block>
							<xsl:text>Premium Outlays:  The amount of premium assumed to be paid by the contract owner or other premium payor.</xsl:text>
						</fo:block>
					</fo:block>
				</fo:flow>
			</fo:page-sequence>
			<!-- COMPLIANCE -->
			<!-- Numeric Summary -->
			<!-- Body page -->
			<fo:page-sequence master-reference="numeric-summary">
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

			<!-- Tabular Detail -->
			<!-- Body page -->
			<fo:page-sequence master-reference="tabular-detail">
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
						<xsl:text>Tabular Detail</xsl:text>
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
							<xsl:if test="illustration/scalar/Composite!='1'">
								<fo:table-column/>
							</xsl:if>
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
									<xsl:choose>
										<xsl:when test="illustration/scalar/Composite!='1'">
											<fo:table-cell number-columns-spanned="4" padding="0pt"></fo:table-cell>
										</xsl:when>
										<xsl:otherwise>
											<fo:table-cell number-columns-spanned="3" padding="0pt"></fo:table-cell>
										</xsl:otherwise>
									</xsl:choose>
									<fo:table-cell number-columns-spanned="3" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
										<fo:block text-align="center">Guaranteed Values</fo:block>
									</fo:table-cell>
									<fo:table-cell number-columns-spanned="3" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
										<fo:block text-align="center">Non-Guaranteed Values</fo:block>
									</fo:table-cell>
								</fo:table-row>
								<fo:table-row>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Policy &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Year</fo:block>
									</fo:table-cell>
									<xsl:if test="illustration/scalar/Composite!='1'">
										<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
											<fo:block text-align="right">EOY
												<fo:block></fo:block>Age</fo:block>
										</fo:table-cell>
									</xsl:if>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Premium &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Outlay</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right"></fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Account &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Value</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Cash Surr &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Value</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Death &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Benefit</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Account &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Value</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Cash Surr &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Value</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Death &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Benefit</fo:block>
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
							<fo:table-body>
								<xsl:call-template name="tabular-detail-values">
									<xsl:with-param name="counter" select="1"/>
								</xsl:call-template>
							</fo:table-body>
						</fo:table>
					</fo:block>
				</fo:flow>
			</fo:page-sequence>
			<!-- Tabular Detail (Report 2)-->
			<!-- Body page -->
			<fo:page-sequence master-reference="tabular-detail-report2">
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
						<xsl:text>Tabular Detail, continued</xsl:text>
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
							<xsl:if test="illustration/scalar/Composite!='1'">
								<fo:table-column/>
							</xsl:if>
							<fo:table-column column-width="1.2in"/>
							<fo:table-column/>
							<fo:table-column/>
							<fo:table-column/>
							<fo:table-column/>
							<xsl:if test="illustration/scalar/Composite!='1'">
								<fo:table-column/>
							</xsl:if>
							<fo:table-header>
								<fo:table-row>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Policy
											<fo:block></fo:block>Year</fo:block>
									</fo:table-cell>
									<xsl:if test="illustration/scalar/Composite!='1'">
										<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
											<fo:block text-align="right">EOY
												<fo:block></fo:block>Age</fo:block>
										</fo:table-cell>
									</xsl:if>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right">Illustrated &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Crediting Rate</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right"></fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block space-before="3.5mm" text-align="right">Withdrawal</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block space-before="3.5mm" text-align="right">Loan</fo:block>
									</fo:table-cell>
									<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
										<fo:block text-align="right"></fo:block>
									</fo:table-cell>
									<xsl:if test="illustration/scalar/Composite!='1'">
										<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
											<fo:block text-align="right">Annual Flat Extra per $1,000</fo:block>
										</fo:table-cell>
									</xsl:if>
								</fo:table-row>
								<fo:table-row>
									<fo:table-cell padding="2pt">
										<fo:block>
										</fo:block>
									</fo:table-cell>
								</fo:table-row>
							</fo:table-header>
							<!-- Create Tabular Detail Values -->
							<fo:table-body>
								<xsl:call-template name="tabular-detail-report2-values">
									<xsl:with-param name="counter" select="1"/>
								</xsl:call-template>
							</fo:table-body>
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

			<!-- Numeric Summary Attachment-->
			<!-- Body page -->
			<fo:page-sequence master-reference="numeric-summary-attachment">
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
		</fo:root>
	</xsl:template>
	<xsl:template name="standardheader">
		<fo:block text-align="center" font-size="9.0pt">
			<fo:block>
				<xsl:text>LIFE INSURANCE BASIC ILLUSTRATION</xsl:text>
			</fo:block>
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
						<xsl:text>Prepared for:</xsl:text>
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
									<xsl:text>Date Prepared: </xsl:text>
									<xsl:value-of select="illustration/scalar/PrepMonth"/>
									<xsl:text> </xsl:text>
									<xsl:value-of select="illustration/scalar/PrepDay"/>
									<xsl:text>, </xsl:text>
									<xsl:value-of select="illustration/scalar/PrepYear"/>
								</fo:block>
							</fo:list-item-body>
						</fo:list-item>
					</fo:list-block>
				</fo:list-item-body>
			</fo:list-item>
			<fo:list-item>
				<fo:list-item-label end-indent="label-end()">
					<fo:block text-align="left">
						<xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Group Name: </xsl:text>
						<xsl:value-of select="illustration/scalar/CorpName"/>
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
										<xsl:text> Total</xsl:text>
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
								<xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Insured: </xsl:text>
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
								<xsl:choose>
									<xsl:when test="illustration/scalar/HasTerm!='0'">
										<fo:block text-align="left" font-size="9.0pt">
											<xsl:text>Initial Base Face Amount: </xsl:text>
											<xsl:value-of select="illustration/scalar/InitBaseSpecAmt"/>
										</fo:block>
									</xsl:when>
									<xsl:otherwise>
										<fo:block text-align="left" color="white">
											<xsl:text>.</xsl:text>
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
							<fo:block text-align="left" font-size="9.0pt" color="white">
								<xsl:text>.</xsl:text>
							</fo:block>
						</xsl:when>
						<xsl:otherwise>
							<fo:block text-align="left" font-size="9.0pt">
								<xsl:text>&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;Age: </xsl:text>
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
									<xsl:when test="illustration/scalar/HasTerm!='0'">
										<fo:block text-align="left" font-size="9.0pt">
											<xsl:text>Initial Term Face Amount: </xsl:text>
											<xsl:value-of select="illustration/scalar/InitTermSpecAmt"/>
										</fo:block>
									</xsl:when>
									<xsl:otherwise>
										<fo:block text-align="left" color="white">
											<xsl:text>.</xsl:text>
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
						<xsl:text>Product: </xsl:text>
						<xsl:value-of select="illustration/scalar/PolicyForm"/>
						<xsl:text> (</xsl:text>
						<xsl:value-of select="illustration/scalar/PolicyMktgName"/>
						<xsl:text>)</xsl:text>
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
									<xsl:text>Initial Illustrated Crediting Rate: </xsl:text>
									<xsl:value-of select="illustration/scalar/InitAnnGenAcctInt_Current"/>
								</fo:block>
							</fo:list-item-body>
						</fo:list-item>
					</fo:list-block>
				</fo:list-item-body>
			</fo:list-item>
			<fo:list-item>
				<fo:list-item-label end-indent="label-end()">
					<fo:block text-align="left">
						<xsl:value-of select="illustration/scalar/PolicyLegalName"/>
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
									<xsl:text>Guaranteed Crediting Rate: </xsl:text>
									<xsl:value-of select="illustration/scalar/InitAnnGenAcctInt_Guaranteed"/>
								</fo:block>
							</fo:list-item-body>
						</fo:list-item>
					</fo:list-block>
				</fo:list-item-body>
			</fo:list-item>
			<fo:list-item>
				<fo:list-item-label end-indent="label-end()">
					<fo:block text-align="left">
						<xsl:text>Initial Premium </xsl:text>
						<xsl:value-of select="illustration/scalar/InitPrem"/>
					</fo:block>
				</fo:list-item-label>
				<fo:list-item-body start-indent="body-start()">
					<fo:list-block provisional-label-separation="0pt">
						<fo:list-item>
							<fo:list-item-label end-indent="label-end()">
								<fo:block text-align="center"></fo:block>
							</fo:list-item-label>
							<!-- Update with "FriendlyUWType"  - Get From Greg -->
							<!-- "&IF(UWType="Medical","Fully underwritten",
							IF(AND(State="TX",UWType="Guaranteed issue"),"Substandard *",UWType))) -->
							<fo:list-item-body start-indent="body-start()">
								<xsl:choose>
									<xsl:when test="illustration/scalar/Composite='1'">
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
							</fo:list-item-body>
						</fo:list-item>
					</fo:list-block>
				</fo:list-item-body>
			</fo:list-item>
			<fo:list-item>
				<fo:list-item-label end-indent="label-end()">
					<xsl:choose>
						<xsl:when test="illustration/scalar/Composite='1'">
							<fo:block text-align="left" color="white">
								<xsl:text>.</xsl:text>
							</fo:block>
						</xsl:when>
						<xsl:otherwise>
							<fo:block text-align="left">
								<xsl:text>Initial Death Benefit Option: </xsl:text>
								<xsl:value-of select="illustration/scalar/DBOptInitInteger+1"/>
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
									<xsl:when test="illustration/scalar/Composite='1'">
										<fo:block text-align="left" color="white">
											<xsl:text>.</xsl:text>
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
	<!-- Create Numeric Summary Values -->
	<xsl:template name="numeric-summary-values">
		<xsl:param name="counter"/>
		<xsl:param name="age70"/>
		<xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
			<fo:table-row>
				<fo:table-cell padding=".6pt">
					<xsl:choose>
						<xsl:when test="$age70!=1">
							<fo:block text-align="right">
								<xsl:value-of select="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value"/>
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
						<xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
					</fo:block>
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
					<fo:block text-align="right">
						<xsl:value-of select="illustration/data/newcolumn/column[@name='AcctVal_Midpoint']/duration[$counter]/@column_value"/>
					</fo:block>
				</fo:table-cell>
				<fo:table-cell>
					<fo:block text-align="right">
						<xsl:value-of select="illustration/data/newcolumn/column[@name='CSVNet_Midpoint']/duration[$counter]/@column_value"/>
					</fo:block>
				</fo:table-cell>
				<fo:table-cell>
					<fo:block text-align="right">
						<xsl:value-of select="illustration/data/newcolumn/column[@name='EOYDeathBft_Midpoint']/duration[$counter]/@column_value"/>
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
			<xsl:if test="$age70!=1">
				<xsl:choose>
					<xsl:when test="$counter &lt; 30">
						<xsl:choose>
							<xsl:when test="$counter=5">
								<xsl:choose>
									<xsl:when test="illustration/scalar/LapseYear_Midpoint &lt; $counter + 5">
										<xsl:call-template name="numeric-summary-values">
											<xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint"/>
											<xsl:with-param name="age70" select="0"/>
										</xsl:call-template>
									</xsl:when>
									<xsl:otherwise>
										<xsl:call-template name="numeric-summary-values">
											<xsl:with-param name="counter" select="$counter + 5"/>
											<xsl:with-param name="age70" select="0"/>
										</xsl:call-template>
									</xsl:otherwise>
								</xsl:choose>
							</xsl:when>
							<xsl:otherwise>
								<xsl:call-template name="numeric-summary-values">
									<xsl:with-param name="counter" select="$counter + 10"/>
									<xsl:with-param name="age70" select="0"/>
								</xsl:call-template>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:if test="illustration/scalar/LapseYear_Guaranteed &lt; illustration/scalar/MaxDuration">
							<xsl:choose>
								<xsl:when test="illustration/scalar/LapseYear_Guaranteed &gt; $counter">
									<xsl:call-template name="numeric-summary-values">
										<xsl:with-param name="counter" select="illustration/scalar/LapseYear_Guaranteed + 1"/>
										<xsl:with-param name="age70" select="0"/>
									</xsl:call-template>
								</xsl:when>
								<xsl:otherwise>
									<xsl:if test="illustration/scalar/LapseYear_Midpoint &lt; illustration/scalar/MaxDuration">
										<xsl:choose>
											<xsl:when test="illustration/scalar/LapseYear_Midpoint &gt; $counter">
												<xsl:call-template name="numeric-summary-values">
													<xsl:with-param name="counter" select="illustration/scalar/LapseYear_Midpoint + 1"/>
													<xsl:with-param name="age70" select="0"/>
												</xsl:call-template>
											</xsl:when>
											<xsl:otherwise>
												<xsl:if test="illustration/scalar/LapseYear_Current &lt; illustration/scalar/MaxDuration">
													<xsl:choose>
														<xsl:when test="illustration/scalar/LapseYear_Current &gt; $counter">
															<xsl:call-template name="numeric-summary-values">
																<xsl:with-param name="counter" select="illustration/scalar/LapseYear_Current + 1"/>
																<xsl:with-param name="age70" select="0"/>
															</xsl:call-template>
														</xsl:when>
														<xsl:otherwise>
														</xsl:otherwise>
													</xsl:choose>
												</xsl:if>
											</xsl:otherwise>
										</xsl:choose>
									</xsl:if>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:if>
		</xsl:if>
	</xsl:template>

	<!-- Create Tabular Detail Report -->
	<xsl:template name="numeric-summary-report">
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
					<fo:table-column/>
					<fo:table-header>
						<fo:table-row>
							<fo:table-cell number-columns-spanned="6" padding="0pt"></fo:table-cell>
							<fo:table-cell number-columns-spanned="6" padding="0pt" border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue">
								<fo:block text-align="center">Non-Guaranteed Values</fo:block>
							</fo:table-cell>
						</fo:table-row>
						<fo:table-row>
							<fo:table-cell number-columns-spanned="3" padding="0pt"></fo:table-cell>
							<fo:table-cell number-columns-spanned="3" padding-before="3pt" padding="3pt">
								<fo:block text-align="center">Guaranteed Values</fo:block>
							</fo:table-cell>
							<fo:table-cell number-columns-spanned="3" padding-before="3pt" padding="3pt">
								<fo:block text-align="center">Midpoint Values</fo:block>
							</fo:table-cell>
							<fo:table-cell number-columns-spanned="3" padding-before="3pt" padding="3pt">
								<fo:block text-align="center">Current Values</fo:block>
							</fo:table-cell>
						</fo:table-row>
						<fo:table-row>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Policy
									<fo:block></fo:block>Year</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right"></fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Premium Outlay</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Account Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Cash Surr Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Death Benefit</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Account Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Cash Surr Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Death Benefit</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Account Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Cash Surr Value</fo:block>
							</fo:table-cell>
							<fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="0pt">
								<fo:block text-align="right">Death Benefit</fo:block>
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
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
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
			<xsl:if test="illustration/scalar/IsMec='1'">
				<fo:block text-align="left" font-size="9.0pt">
					<xsl:text>IMPORTANT TAX DISCLOSURE: This is a Modified Endowment Contract. Please refer to the Explanatory Notes for additional information.</xsl:text>
				</fo:block>
			</xsl:if>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block text-align="center" font-size="9.0pt">
				<xsl:text>Certification Statements</xsl:text>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block text-align="left" font-size="9.0pt">
				<xsl:text>CONTRACT OWNER / APPLICANT</xsl:text>
			</fo:block>
			<xsl:if test="illustration/scalar/InterestDisclaimer!=''">
				<fo:block text-align="left" font-size="9.0pt">
					<xsl:text>I understand that at the present time higher current interest rates are credited for policies with case premiums in the amount of </xsl:text>
					<xsl:value-of select="illustration/scalar/InterestDisclaimer"/>
				</fo:block>
				<fo:block>
					<fo:leader></fo:leader>
				</fo:block>
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
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block text-align="left" font-size="9.0pt" text-decoration="overline">CONTRACT OWNER OR APPLICANT SIGNATURE &#xA0;&#xA0;&#xA0;
				<fo:inline text-decoration="no-overline">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:inline>DATE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
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
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block>
				<fo:leader></fo:leader>
			</fo:block>
			<fo:block text-align="left" font-size="9.0pt" text-decoration="overline">AGENT OR AUTHORIZED REPRESENTATIVE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;
				<fo:inline text-decoration="no-overline">&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:inline>DATE &#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;&#xA0;</fo:block>
		</fo:flow>
	</xsl:template>

	<!-- Create Tabular Detail Values -->
	<xsl:template name="tabular-detail-values">
		<xsl:param name="counter"/>
			<xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!=0"> 
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
						<xsl:value-of select="illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value"/>
					</fo:block>
				</fo:table-cell>
				<fo:table-cell>
					<fo:block>
					</fo:block>
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
			<xsl:if test="$counter mod 5=0">
				<fo:table-row>
					<fo:table-cell padding="4pt">
						<fo:block text-align="right"></fo:block>
					</fo:table-cell>
				</fo:table-row>
			</xsl:if>
			<xsl:call-template name="tabular-detail-values">
				<xsl:with-param name="counter" select="$counter + 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<!-- Create Tabular Detail (report 2) Values -->
	<xsl:template name="tabular-detail-report2-values">
		<xsl:param name="counter"/>
		<xsl:if test="illustration/data/newcolumn/column[@name='PolicyYear']/duration[$counter]/@column_value!='0'">
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
				<fo:table-cell>
					<fo:block>
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
					<fo:block>
					</fo:block>
				</fo:table-cell>
				<xsl:if test="illustration/scalar/Composite!='1'">
					<fo:table-cell>
						<fo:block text-align="right">
							<xsl:value-of select="illustration/data/newcolumn/column[@name='MonthlyFlatExtra']/duration[$counter]/@column_value"/>
						</fo:block>
					</fo:table-cell>
				</xsl:if>
			</fo:table-row>
			<!-- Blank Row Every 5th Year -->
			<xsl:if test="$counter mod 5=0">
				<fo:table-row>
					<fo:table-cell padding="4pt">
						<fo:block text-align="right"></fo:block>
					</fo:table-cell>
				</fo:table-row>
			</xsl:if>
			<xsl:call-template name="tabular-detail-report2-values">
				<xsl:with-param name="counter" select="$counter + 1"/>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	<!-- Create Tabular Detail Values -->
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

	<xsl:template name="removeamps">
		<xsl:param name="title"/>
		<xsl:if test="contains($title, '&amp;')" >
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


	<!-- Standard Footer -->
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
							<xsl:if test="illustration/scalar/VersionNumber!=''">
								<fo:block text-align="left">Version Number:
									<xsl:value-of select="illustration/scalar/VersionNumber"/>
								</fo:block>
							</xsl:if>
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
				</fo:table-body>
			</fo:table>
		</fo:block>
	</xsl:template>
</xsl:stylesheet>