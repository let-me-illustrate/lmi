<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

    Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015 Gregory W. Chicares.

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

  <xsl:variable name="ModifiedSinglePremium">
    <xsl:call-template name="set_modified_single_premium"/>
  </xsl:variable>
  <xsl:variable name="ModifiedSinglePremium0">
    <xsl:call-template name="set_modified_single_premium0"/>
  </xsl:variable>
  <xsl:variable name="SinglePremium">
    <xsl:call-template name="set_single_premium"/>
  </xsl:variable>
  <xsl:variable name="GroupExperienceRating">
    <xsl:call-template name="set_group_experience_rating"/>
  </xsl:variable>
  <xsl:variable name="GroupCarveout">
    <xsl:call-template name="set_group_carveout"/>
  </xsl:variable>
  <xsl:variable name="FlexiblePremium">
    <xsl:call-template name="set_flexible_premium"/>
  </xsl:variable>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover">
          <xsl:call-template name="set-page-size"/>
          <fo:region-body margin=".1in .1in 0"/>
        </fo:simple-page-master>

        <!-- Define the narrative summary page. -->
        <fo:simple-page-master master-name="narrative-summary">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin=".5in 0 1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".5in"/>
        </fo:simple-page-master>

        <!-- Define the column headings and key terms page. -->
        <fo:simple-page-master master-name="column-headings-and-key-terms">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin=".15in 0 .45in"/>
          <!-- Header -->
          <fo:region-before extent="2in"/>
          <!-- Footer -->
          <fo:region-after extent="0.45in"/>
        </fo:simple-page-master>

        <!-- Define the Numeric Summary page. -->
        <fo:simple-page-master master-name="numeric-summary">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 .52in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="0.52in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail page. -->
        <fo:simple-page-master master-name="tabular-detail">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 1.3in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Tabular Detail (Report 2) page. -->
        <fo:simple-page-master master-name="tabular-detail-report2">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.2in 0 1.35in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="1.27in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Report page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="3in 0 1.25in"/>
            <!-- Header -->
            <fo:region-before extent="3.0in"/>
            <!-- Footer -->
            <fo:region-after extent="1.27in"/>
          </fo:simple-page-master>
        </xsl:if>

        <!-- Define the Numeric Summary Attachment page. -->
        <fo:simple-page-master master-name="numeric-summary-attachment">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="3.1in 0 .52in"/>
          <!-- Header -->
          <fo:region-before extent="3.1in"/>
          <!-- Footer -->
          <fo:region-after extent="0.52in"/>
        </fo:simple-page-master>

      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->
      <xsl:call-template name="generic-cover"/>

      <!-- NARRATIVE SUMMARY - begins here -->
      <fo:page-sequence master-reference="narrative-summary" initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- Narrative Summary Body  -->
        <fo:flow flow-name="xsl-region-body">
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10pt" padding-top="1em">
            NARRATIVE SUMMARY
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block padding-top="1em">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:value-of select="$scalars/PolicyMktgName"/> is a
                  <!-- Group Experience Rating Logic -->
                  <xsl:if test="$GroupExperienceRating='1'">
                    group
                  </xsl:if>
                  <xsl:if test="$GroupCarveout='1'">
                    group
                  </xsl:if>
                  flexible premium adjustable life insurance contract.
                  <!-- Group Experience Rating Logic -->
                  <xsl:if test="$GroupExperienceRating='1'">
                    It is a no-load policy and is intended for large case sales.
                    It is primarily marketed to financial institutions
                    to fund certain corporate liabilities.
                  </xsl:if>
                  It features accumulating account values, adjustable benefits,
                  and flexible premiums.
                </xsl:when>
                <xsl:when test="$ModifiedSinglePremium='1' or $ModifiedSinglePremium0='1'">
                  <xsl:value-of select="$scalars/PolicyMktgName"/>
                  is a modified single premium adjustable life
                  insurance contract. It features accumulating
                  account values, adjustable benefits, and single premium.
                </xsl:when>
                <xsl:otherwise>
                  <xsl:value-of select="$scalars/PolicyMktgName"/>
                  is a single premium adjustable life insurance contract.
                  It features accumulating account values,
                  adjustable benefits, and single premium.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <xsl:if test="$scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                Coverage may be available on a Guaranteed Standard Issue basis.
                All proposals are based on case characteristics and must
                be approved by the <xsl:value-of select="$scalars/InsCoShortName"/>
                Home Office. For details regarding underwriting
                and coverage limitations refer to your offer letter
                or contact your <xsl:value-of select="$scalars/InsCoShortName"/>
                representative.
              </fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/AvName"/>&nbsp;
              <xsl:value-of select="$scalars/MonthlyChargesPaymentFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              <xsl:choose>
                <!-- Single Premium Logic -->
                <xsl:when test="$SinglePremium!='1'">
                  Premiums are assumed to be paid on
                  a<xsl:if test="$vectors[@name='ErMode']/duration[1]/@column_value='Annual'">n </xsl:if>
                  <xsl:value-of select="translate($vectors[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                  basis and received at the beginning of the contract year.
                </xsl:when>
                <xsl:otherwise>
                  The single premium is assumed to be paid at the beginning
                  of the contract year.
                </xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="$scalars/AvName"/> Values,
              <xsl:value-of select="$scalars/CsvName"/> Values,
              and death benefits are illustrated as of the end
              of the contract year. The method we use to allocate
              overhead expenses is the fully allocated expense method.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1'">
              <fo:block padding-top="1em">
                In order to guarantee coverage to age
                <xsl:value-of select="$scalars/EndtAge"/>,
                a<xsl:if test="$vectors[@name='ErMode']/duration[1]/@column_value='Annual'">n </xsl:if>
                <xsl:value-of select="translate($vectors[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
                premium
                <xsl:choose>
                  <xsl:when test="$scalars/GuarPrem!='0'">
                    of $<xsl:value-of select="$scalars/GuarPrem"/>
                    must be paid.
                  </xsl:when>
                  <xsl:otherwise>
                    is defined.
                  </xsl:otherwise>
                </xsl:choose>
                This amount is based on the guaranteed monthly charges
                and the guaranteed interest crediting rate.
                <xsl:if test="$scalars/DefnLifeIns='GPT'">
                  This premium may be in excess of the maximum premium allowable
                  in order to qualify this policy as life insurance.
                </xsl:if>
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              Loaned amounts of the <xsl:value-of select="$scalars/AvName"/>
              Value will be credited a rate equal to the loan interest rate less
              a spread, guaranteed not to exceed
              <xsl:choose>
                <xsl:when test="$GroupCarveout='1'">
<!-- This is what it should really be, but it needs to be made available
to the xsl files first.
              <xsl:value-of select="$scalars/GuarRegLoanSpread"/>.
-->
                  1.25%
                </xsl:when>
                <xsl:otherwise>
                  3.00%.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <xsl:if test="$scalars/HasTerm='1'">
              <fo:block padding-top="1em">
                The term rider provides the option to purchase monthly
                term insurance on the life of the insured. The term rider
                selected face amount supplements the selected face amount
                of the contract. If the term rider is attached, the policy
                to which it is attached may have a lower annual cutoff premium
                and, as a result, the lower overall sales loads paid may be
                lower than a contract having the same total face amount,
                but with no term rider.
                <xsl:if test="$scalars/NoLapse='1'">
                  Also, the lapse protection feature of the contract's
                  <xsl:value-of select="$scalars/NoLapseProvisionName"/>
                  does not apply to the term rider's selected face amount.
                </xsl:if>
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasWP='1'">
              <fo:block padding-top="1em">
                The Waiver of Monthly Charges Rider provides for waiver
                of monthly charges in the event of the disability
                of the insured that begins before attained age 65
                and continues for at least 6 months, as described in the rider.
                An additional charge is associated with this rider. Please refer
                to your contract for specific provisions and a detailed schedule
                of charges.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasADD='1'">
              <fo:block padding-top="1em">
                The Accidental Death benefit provides an additional benefit
                if death is due to accident. An additional charge is associated
                with this rider. Please refer to your contract
                for specific provisions and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              The definition of life insurance for this contract is the
              <xsl:choose>
                <xsl:when test="$scalars/DefnLifeIns='GPT'">
                  guideline premium test. The guideline single premium
                  is $<xsl:value-of select="$scalars/InitGSP"/>
                  and the guideline level premium
                  is $<xsl:value-of select="$scalars/InitGLP"/>
                </xsl:when>
                <xsl:otherwise>
                  cash value accumulation test.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              This is an illustration only. An illustration is not intended
              to predict actual performance. Interest rates
              <xsl:if test="$scalars/Participating='1'">, dividends,</xsl:if>
              and values set forth in the illustration are not guaranteed.
            </fo:block>
            <fo:block padding-top="1em">
<!-- Retain this logic in case the inforce footnotes are ever needed again.
              <xsl:choose>
                <xsl:when test="$scalars/IsInforce='1'">
                  <xsl:if test="$SinglePremium='1'">
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote0"/>
                  </xsl:if>
                  <xsl:if test="$FlexiblePremium='1'">
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote0"/>
                  </xsl:if>
                  <xsl:if test="$SinglePremium!='1'">
                    <xsl:if test="$FlexiblePremium!='1'">
                      <fo:block>
                        <xsl:choose>
                          <xsl:when test="$scalars/StatePostalAbbrev!='TX'">
                            This illustration assumes that the currently illustrated
                            non-guaranteed elements will continue unchanged
                            for all years shown. This is not likely to occur
                            and actual results may be more or less favorable than shown.
                            The non-guaranteed benefits and values are not guaranteed
                            and are based on assumptions such as interest credited
                            and current monthly charges, which are subject to change by
                            <xsl:value-of select="$scalars/InsCoName"/>.
                          </xsl:when>
                          <xsl:otherwise>
                            This illustration is based on both non-guaranteed
                            and guaranteed assumptions. Non-guaranteed assumptions
                            include interest rates and monthly charges.
                            This illustration assumes that the currently illustrated
                            non-guaranteed elements will continue unchanged
                            for all years shown. This is not likely to occur
                            and actual results may be more or less favorable than shown.
                            Factors that may affect future policy performance include
                            the company's expectations for future mortality, investments,
                            persistency, profits and expenses.
                          </xsl:otherwise>
                        </xsl:choose>
                      </fo:block>
                    </xsl:if>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
-->
                  <fo:block>
                    <xsl:choose>
                      <xsl:when test="$scalars/StatePostalAbbrev!='TX'">
                        This illustration assumes that the currently illustrated
                        non-guaranteed elements will continue unchanged
                        for all years shown. This is not likely to occur
                        and actual results may be more or less favorable than shown.
                        The non-guaranteed benefits and values are not guaranteed
                        and are based on assumptions such as interest credited
                        and current monthly charges, which are subject to change by
                        <xsl:value-of select="$scalars/InsCoName"/>.
                      </xsl:when>
                      <xsl:otherwise>
                        This illustration is based on both non-guaranteed
                        and guaranteed assumptions. Non-guaranteed assumptions
                        include interest rates and monthly charges.
                        This illustration assumes that the currently illustrated
                        non-guaranteed elements will continue unchanged
                        for all years shown. This is not likely to occur
                        and actual results may be more or less favorable than shown.
                        Factors that may affect future policy performance include
                        the company's expectations for future mortality, investments,
                        persistency, profits and expenses.
                      </xsl:otherwise>
                    </xsl:choose>
                  </fo:block>
<!-- Retain this logic in case the inforce footnotes are ever needed again.
                </xsl:otherwise>
              </xsl:choose>
-->
            </fo:block>
<!-- Retain this logic in case the inforce footnotes are ever needed again.
            <xsl:if test="$scalars/IsInforce='1'">
              <fo:block font-weight="bold" font-style="italic" text-align="center" padding-top="10em">
                <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote1"/>
              </fo:block>
              <fo:block font-weight="bold" font-style="italic" padding-top="1em">
                <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote2"/>
              </fo:block>
            </xsl:if>
-->
          </fo:block>

          <!-- Force Second Page -->
          <fo:block break-after="page"/>
          <fo:block text-align="center" font-size="10pt">
            NARRATIVE SUMMARY (Continued)
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <xsl:if test="$scalars/SalesLoadRefund!='0%'">
              <fo:block padding-top="2em">
                Sales Load Refund: We will refund a portion of the sales load
                to you, as part of your <xsl:value-of select="$scalars/CsvName"/>
                Value, if you surrender your contract within the first two
                policy years. In policy year 1, we will refund
                <xsl:value-of select="$scalars/SalesLoadRefundRate0"/>
                of the first contract year sales load collected
                and in contract year 2, we will refund
                <xsl:value-of select="$scalars/SalesLoadRefundRate1"/>
                of the first contract year sales load collected.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/NoLapse='1'">
              <fo:block padding-top="1em">
                <xsl:value-of select="$scalars/NoLapseProvisionName"/>:
                The contract will remain in force after the first premium
                has been paid, even if there is insufficient
                <xsl:value-of select="$scalars/AvName"/> Value
                to cover the monthly charges provided that the insured
                is not in a substandard rating class and the policy debt
                does not exceed <xsl:value-of select="$scalars/AvName"/> Value.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/GuarMortalityFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
              This illustration assumes death of the insured
              at age <xsl:value-of select="$scalars/EndtAge"/>.
            </fo:block>
            <fo:block padding-top="1em">
              The loan interest rate is fixed
              at <xsl:value-of select="$scalars/InitAnnLoanDueRate"/> per year.
            </fo:block>
            <fo:block padding-top="1em">
              The state of issue
              is <xsl:value-of select="$scalars/StatePostalAbbrev"/>.
            </fo:block>
            <fo:block padding-top="1em">
              This illustration assumes an initial Selected Face Amount of
              $<xsl:value-of select="$scalars/InitBaseSpecAmt"/>. Selected Face
              Amount reductions assumed in this illustration (if any) are shown
              in the Tabular Detail.
            </fo:block>
            <xsl:if test="$scalars/IsInforce='1'">
              <fo:block padding-top="1em">
                This illustration assumes a beginning account value of
                $<xsl:value-of select="$scalars/InforceUnloanedAV"/> as
                of the date of this illustration.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/IsInforce='1'">
              <fo:block padding-top="1em">
                This illustration assumes a beginning cost basis of
                $<xsl:value-of select="$scalars/InforceTaxBasis"/> as
                of the date of this illustration; the actual cost basis
                may be higher or lower. Consult the Home Office for cost
                basis information.
              </fo:block>
            </xsl:if>
            <xsl:if test="$is_composite">
              <fo:block padding-top="1em">
                Please see the attached census, listing the face amounts,
                underwriting classes and issue ages for individual participants.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/StatePostalAbbrev='NC' or $scalars/StatePostalAbbrev='SC'">
              <fo:block padding-top="1em">
                In the states of North Carolina and South Carolina,
                Guaranteed Issue Underwriting is referred
                to as "Limited Underwriting" and Simplified
                Issue Underwriting is referred to as "Simplified Underwriting".
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/StatePostalAbbrev='MD'">
              <fo:block padding-top="1em">
                In the state of Maryland, Guaranteed Issue Underwriting
                is referred to as "Nonstandard Limited Underwriting"
                and Simplified Issue Underwriting is referred to as
                "Nonstandard Simplified Underwriting".
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/StatePostalAbbrev='TX'">
              <xsl:if test="$scalars/UWType='Guaranteed issue'">
                <fo:block padding-top="1em">
                  * This policy is classified as substandard guaranteed issue
                  per the requirements of the Texas Insurance Department.
                </fo:block>
              </xsl:if>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block padding-top="1em">
                We may assess a Market Value Adjustment upon a surrender
                of the certificate when the surrender proceeds are intended
                to be applied to an insurance policy issued by an insurer
                unaffilliated with
                <xsl:value-of select="$scalars/InsCoShortName"/>
                with an intent to qualify the exchange as a tax free exchange
                under IRC section 1035.
              </fo:block>
              <xsl:if test="$scalars/UseExperienceRating!='1'">
                <fo:block padding-top="1em">
                  This illustration does not reflect experience rating.
                </fo:block>
              </xsl:if>
              <fo:block padding-top="1em">
                The guaranteed values reflect the maximum charges permitted
                by the contract, which may include an Experience Rating
                Risk Charge.
              </fo:block>
              <fo:block padding-top="1em">
                No Experience Rating Risk Charge or a distribution
                of an Experience Rating Reserve Credit is reflected
                in the current, non-guaranteed values. Actual charges
                and credits will be based on the actual experience of the group.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/Has1035ExchCharge='1'">
              <!-- Single Premium Logic -->
              <xsl:choose>
                <xsl:when test="$SinglePremium!='1'">
                  <xsl:if test="$scalars/Has1035ExchCharge='1'">
                    <fo:block padding-top="1em">
                      Upon surrender of this policy, where the surrender
                      proceeds are intended to be applied to an insurance policy
                      or certificate issued in conjunction with an intent
                      to qualify the exchange as a tax free exchange
                      under Section 1035 of the Internal Revenue Code,
                      we may assess an Exchange Charge. The Exchange Charge
                      is the greater of the Market Value Adjustment Charge
                      and the Percentage of Premium Charge. In the states
                      of Florida or Indiana, the Exchange charge
                      (referred to as Assignment Charge in Florida)
                      will be the Percentage of Premium Charge only.
                      The Exchange Charge will potentially reduce
                      the surrender proceeds, but will never increase
                      the surrender proceeds. Please refer to your policy
                      for details.
                    </fo:block>
                  </xsl:if>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block padding-top="1em">
                    Upon surrender of this policy, where the surrender proceeds
                    are intended to be applied to an insurance policy
                    or certificate issued in conjunction with an intent
                    to qualify the exchange as a tax free exchange
                    under Section 1035 of the Internal Revenue Code (1035
                    Exchange), we may assess an Exchange Charge. The Exchange
                    Charge will potentially reduce the surrender proceeds,
                    but will never increase the surrender proceeds.
                    Please refer to your policy for details.
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:if>
            <xsl:if test="$scalars/HasSpouseRider='1'">
              <fo:block padding-top="1em">
                The $<xsl:value-of select="$scalars/SpouseRiderAmount"/> Spouse
                rider provides term life insurance on the spouse
                (issue age <xsl:value-of select="$scalars/SpouseIssueAge"/>)
                for a limited duration, for an extra charge.
                Please refer to your contract for specific provisions
                and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/HasChildRider='1'">
              <fo:block padding-top="1em">
                The $<xsl:value-of select="$scalars/ChildRiderAmount"/> Child
                rider provides term life insurance on the insured's children
                for a limited duration, for an extra charge. Please refer
                to your contract for specific provisions
                and a detailed schedule of charges.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <xsl:value-of select="$scalars/SurrenderFootnote"/>
            </fo:block>
            <fo:block padding-top="1em">
             <xsl:value-of select="$scalars/FundRateFootnote"/>
            </fo:block>
            <fo:block>
              &nbsp;&nbsp;<xsl:value-of select="$scalars/FundRateFootnote0"/>
            </fo:block>
            <fo:block>
              &nbsp;&nbsp;<xsl:value-of select="$scalars/FundRateFootnote1"/>
            </fo:block>

            <fo:block font-weight="bold" text-align="center" padding-top="1em">
              IMPORTANT TAX DISCLOSURE
            </fo:block>
            <fo:block padding-top="1em">
              As illustrated, this contract
              <xsl:choose>
                <xsl:when test="$scalars/IsMec='1'">
                  becomes
                </xsl:when>
                <xsl:otherwise>
                  would not become
                </xsl:otherwise>
              </xsl:choose>
              a Modified Endowment Contract (MEC)
              under the Internal Revenue Code<!-- prevent trailing space -->
              <xsl:if test="$scalars/IsMec='1'">
                in year <xsl:value-of select="$scalars/MecYear+1"/>
              </xsl:if>.
              To the extent of gain in the contract, loans, distributions
              and withdrawals from a MEC are subject to income tax
              and may also trigger a penalty tax.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium!='1' and $scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                The initial 7-pay premium limit
                is $<xsl:value-of select="$scalars/InitSevenPayPrem"/>.
              </fo:block>
            </xsl:if>
            <fo:block font-weight="bold" padding-top="1em">
              The information contained in this illustration is not written
              or intended as tax or legal advice, and may not be relied upon
              for purposes of avoiding any federal tax penalties.
              Neither <xsl:value-of select="$scalars/InsCoShortName"/> nor any
              of its employees or representatives are authorized to give tax
              or legal advice. For more information pertaining
              to the tax consequences of purchasing or owning this policy,
              consult with your own independent tax or legal counsel.
            </fo:block>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <!-- Column Headings and Key Terms - begins here -->
      <fo:page-sequence master-reference="column-headings-and-key-terms">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <!-- Column Headings and Key Terms Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="center" font-size="10pt" padding-top="1em">
            Column Headings and Key Terms Used in This Illustration
          </fo:block>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif" padding-top="1em">
            <fo:block>
              <fo:inline font-weight="bold">
                <xsl:value-of select="$scalars/AvName"/> Value:
              </fo:inline>
              The accumulation at interest of the net premiums paid,
              <xsl:if test="$SinglePremium!='1'">
                less any withdrawals,
              </xsl:if>
              less any monthly charges deducted.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                <xsl:value-of select="$scalars/CsvName"/> Value:
              </fo:inline>
              <xsl:value-of select="$scalars/AvName"/> Value less policy debt.
              <xsl:if test="$scalars/Has1035ExchCharge='1'">
                <xsl:value-of select="$scalars/CashSurrValueFootnote"/>
              </xsl:if>
            </fo:block>
            <xsl:if test="$scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Current Illustrated Crediting Rate:
                </fo:inline>
                  <xsl:value-of select="$scalars/CreditingRateFootnote"/>
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Current Values:
              </fo:inline>
              Values assuming current interest crediting rates
              and current monthly charges. These values are not guaranteed
              and are based on the assumption that premium is paid
              as illustrated.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Death Benefit:
              </fo:inline>
              The amount of benefit provided by the Death Benefit Option
              in effect on the date of death, prior to adjustments
              for policy debt and monthly charges payable to the date of death.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Death Benefit Option 1:
              </fo:inline>
              Option in which the death benefit is equal to the selected
              face amount of the contract on the date of death of the insured,
              or if greater the <xsl:value-of select="$scalars/AvName"/> Value
              <xsl:if test="$scalars/SalesLoadRefund!='0%'">
                plus the refund of sales loads (if applicable)
              </xsl:if>
              on the insured's date of death multiplied by the minimum face
              amount percentage for the insured's attained age at death
              (minimum face amount). Please refer to the contract
              for a detailed schedule of death benefit factors.
            </fo:block>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Death Benefit Option 2:
                </fo:inline>
                Option in which the death benefit is equal to the selected
                face amount of the contract
                plus the <xsl:value-of select="$scalars/AvName"/> Value
                on the date of death of the insured, or if greater,
                the <xsl:value-of select="$scalars/AvName"/> Value
                <xsl:if test="$scalars/SalesLoadRefund!='0%'">
                  plus the refund of sales loads (if applicable)
                </xsl:if>
                on the insured's date of death multiplied
                by the death benefit factor for the insured's attained age
                at death (minimum face amount). Please refer to the contract
                for a detailed schedule of death benefit factors.
              </fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Experience Rating Risk Charge:
                </fo:inline>
                Applies only to certain experience rated groups.
                This charge is based on the cost of insurance charges
                assessed during the certificate year. It may be assessed against
                the account value once per certificate anniversary date
                and upon surrender of the group policy.
              </fo:block>
            </xsl:if>
            <!-- Group Experience Rating Logic -->
            <xsl:if test="$GroupExperienceRating!='1'">
              <xsl:if test="$scalars/Has1035ExchCharge='1'">
                <fo:block padding-top="1em">
                  <fo:inline font-weight="bold">
                    Exchange Charge:
                  </fo:inline>
                    <xsl:value-of select="$scalars/ExchangeChargeFootnote1"/>
                </fo:block>
              </xsl:if>
            </xsl:if>

            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='0'">
              <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Flexible Premiums:
              </fo:inline>
              Premiums that may be increased, reduced, or not paid,
              if the account value is sufficient to cover the monthly charges.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Guaranteed Values:
              </fo:inline>
              Values assuming the guaranteed crediting rate
              and the guaranteed maximum monthly charges. These values
              are based on the assumption that premium is paid as illustrated.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                MEC:
              </fo:inline>
              Modified Endowment Contract - this classification is given
              to a contract in violation of TAMRA
              (Technical and Miscellaneous Revenue Act), which limits
              the amount of premium that can be paid into a life
              insurance contract. To the extent of gain in the contract, loans,
              distributions and withdrawals from a MEC are subject
              to income tax and may also trigger a tax penalty.
            </fo:block>
            <xsl:if test="$scalars/IsInforce!='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Midpoint Values:
                </fo:inline>
                Values assuming interest rates that are the average
                of the illustrated current crediting rates
                and the guaranteed minimum interest rate, and monthly charges
                that are the average of the current monthly charges
                and the guaranteed monthly charges.
                These values are not guaranteed and are based on the assumption
                that premium is paid as illustrated.
              </fo:block>
            </xsl:if>
            <!-- Single Premium Logic -->
            <xsl:if test="$ModifiedSinglePremium='1' or $ModifiedSinglePremium0='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Modified Single Premium:
                </fo:inline>
                After the single premium is paid, additional payment
                under this policy will only be accepted for repayment
                of policy debt, payment required to keep the policy
                from lapsing, or payment required to reinstate the policy.
              </fo:block>
            </xsl:if>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Monthly Charges:
              </fo:inline>
              The monthly charges for the following month which include:
              cost of insurance, plus face amount charges (if applicable),
              plus the administrative charge shown
              on the contract schedule page.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Premium Outlay:
              </fo:inline>
              The amount of premium assumed to be paid by the contract owner
              or other premium payor.
            </fo:block>
            <fo:block padding-top="1em">
              <fo:inline font-weight="bold">
                Selected Face Amount:
              </fo:inline>
              The Selected Face Amount may be decreased upon written application
              satisfactory to <xsl:value-of select="$scalars/InsCoName"/>. A
              requested decrease is allowed only once per Policy Year, and the
              resulting Selected Face Amount after decrease must meet the
              minimum defined in your policy.
            </fo:block>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1' and $ModifiedSinglePremium0='0' and $ModifiedSinglePremium='0'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Single Premium:
                </fo:inline>
                After the single premium is paid, additional payment
                under this policy will only be accepted for repayment
                of policy debt, payment required to keep the policy
                from lapsing, or payment required to reinstate the policy.
              </fo:block>
            </xsl:if>
            <!-- Single Premium Logic -->
            <xsl:if test="$SinglePremium='1'">
              <fo:block padding-top="1em">
                <fo:inline font-weight="bold">
                  Ultimate Illustrated Crediting Rate:
                </fo:inline>
                  <xsl:value-of select="$scalars/UltCreditingRateFootnote"/>
              </fo:block>
            </xsl:if>
          </fo:block>
        </fo:flow>
      </fo:page-sequence>

      <xsl:if test="$scalars/IsInforce!='1'">
        <!-- Numeric Summary (only for new business)-->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary">
          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="5pt">
              Numeric Summary
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter"/>

          <xsl:call-template name="numeric-summary-report"/>
        </fo:page-sequence>
      </xsl:if>

      <!-- Tabular Detail -->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10pt" space-before="5.0pt">
            Tabular Detail
          </fo:block>
          <xsl:call-template name="dollar-units"/>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="disclaimer">
            <fo:block padding-top="1em">
              <xsl:choose>
                <xsl:when test="$scalars/IsInforce!='1'">
                  <xsl:value-of select="$scalars/NonGuaranteedFootnote"/>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block>
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote0"/>
                  </fo:block>
                  <fo:block font-weight="bold">
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote3"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </xsl:with-param>
        </xsl:call-template>

        <fo:flow flow-name="xsl-region-body">
          <xsl:variable name="tabular-detail-report-columns">
            <column name="PolicyYear">                               | Policy _Year    </column>
            <column composite="0" name="AttainedAge">                | End of _Year Age</column>
            <column name="GrossPmt">                                 | Premium _Outlay </column>
            <column name="AcctVal_Guaranteed">      Guaranteed Values| Account _Value  </column>
            <column name="CSVNet_Guaranteed">       Guaranteed Values| Cash Surr _Value</column>
            <column name="EOYDeathBft_Guaranteed">  Guaranteed Values| Death _Benefit  </column>
            <column/>
            <column name="AcctVal_Current">     Non-Guaranteed Values| Account _Value  </column>
            <column name="CSVNet_Current">      Non-Guaranteed Values| Cash Surr _Value</column>
            <column name="EOYDeathBft_Current"> Non-Guaranteed Values| Death _Benefit  </column>
          </xsl:variable>
          <xsl:variable name="tabular-detail-report-columns-raw" select="document('')//xsl:variable[@name='tabular-detail-report-columns']/column"/>
          <xsl:variable name="columns" select="$tabular-detail-report-columns-raw[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

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
      </fo:page-sequence>

      <!-- Tabular Detail (Report 2)-->
      <!-- Body page -->
      <fo:page-sequence master-reference="tabular-detail-report2">
        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <fo:block text-align="left">
            <xsl:call-template name="company-logo"/>
          </fo:block>
          <xsl:call-template name="standardheader"/>
          <fo:block text-align="center" font-size="10pt" space-before="5.0pt">
            Tabular Detail, continued
          </fo:block>
<!-- These units don't make sense for the ratios that are actually displayed.
          <xsl:call-template name="dollar-units"/>
-->
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="disclaimer">
            <fo:block padding-top="1em">
              <xsl:choose>
                <xsl:when test="$scalars/IsInforce!='1'">
                  <xsl:value-of select="$scalars/NonGuaranteedFootnote"/>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block>
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote0"/>
                  </fo:block>
                  <fo:block font-weight="bold">
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote3"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
          </xsl:with-param>
        </xsl:call-template>

        <fo:flow flow-name="xsl-region-body">
          <xsl:variable name="tabular-detail-report2-columns">
            <column name="PolicyYear">Policy _Year</column>
            <column composite="0" name="AttainedAge">End of _Year Age</column>
            <column name="AnnGAIntRate_Current">Illustrated _Crediting Rate</column>
            <column name="SpecAmt">Selected_Face Amount</column>
          </xsl:variable>
          <xsl:variable name="tabular-detail-report2-columns-raw" select="document('')//xsl:variable[@name='tabular-detail-report2-columns']/column"/>
          <xsl:variable name="columns" select="$tabular-detail-report2-columns-raw[not(@composite) or boolean(boolean(@composite='1')=$is_composite)]"/>

          <fo:block font-size="9pt" font-family="serif">
            <fo:table table-layout="fixed" width="{8*count($columns)}em">
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
          <!-- endofdoc block id implemented as the "otherwise" condition in
               an xsl:choose instead of xsl:if !='1' so that the XML item
             'Supplemental Report' need not exist in the XML document for
             page numbering to work properly -->
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
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt" space-before="3.0pt">
              <xsl:value-of select="illustration/supplementalreport/title"/>
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="disclaimer">
            <fo:block padding-top="1em">
              <xsl:choose>
                <xsl:when test="$scalars/IsInforce!='1'">
                  <xsl:value-of select="$scalars/NonGuaranteedFootnote"/>
                </xsl:when>
                <xsl:otherwise>
                  <fo:block>
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote0"/>
                  </fo:block>
                  <fo:block font-weight="bold">
                    <xsl:value-of select="$scalars/InforceNonGuaranteedFootnote3"/>
                  </fo:block>
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            </xsl:with-param>
          </xsl:call-template>

          <!-- Supplemental report body -->
          <xsl:call-template name="supplemental-report-body"/>
        </fo:page-sequence>
      </xsl:if>

      <xsl:if test="$scalars/IsInforce!='1'">
        <!-- Numeric Summary Attachment - (Only for New Business) -->
        <!-- Body page -->
        <fo:page-sequence master-reference="numeric-summary-attachment">
          <!-- Define the contents of the header. -->
          <fo:static-content flow-name="xsl-region-before">
            <fo:block text-align="left">
              <xsl:call-template name="company-logo"/>
            </fo:block>
            <xsl:call-template name="standardheader"/>
            <fo:block text-align="center" font-size="10.0pt">
              Numeric Summary
            </fo:block>
            <xsl:call-template name="dollar-units"/>
          </fo:static-content>

          <!-- Define the contents of the footer. -->
          <xsl:call-template name="standardfooter">
            <xsl:with-param name="omit-pagenumber" select="1"/>
          </xsl:call-template>

          <xsl:call-template name="numeric-summary-report"/>
        </fo:page-sequence>
      </xsl:if>
    </fo:root>
  </xsl:template>

  <xsl:template name="standardheader">
    <fo:block text-align="center" font-size="9.0pt" padding-bottom="1em">
      <fo:block>
        <xsl:choose>
          <xsl:when test="$scalars/IsInforce='1'">
            LIFE INSURANCE IN FORCE BASIC ILLUSTRATION
          </xsl:when>
          <xsl:otherwise>
            LIFE INSURANCE BASIC ILLUSTRATION
          </xsl:otherwise>
        </xsl:choose>
      </fo:block>
      <fo:block>
        <xsl:value-of select="$scalars/InsCoName"/>
      </fo:block>
      <xsl:if test="$scalars/ProducerName!='0'">
        <fo:block>
          Presented by: <xsl:value-of select="$scalars/ProducerName"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="$scalars/ProducerStreet!='0'">
        <fo:block>
          <xsl:value-of select="$scalars/ProducerStreet"/>
        </fo:block>
      </xsl:if>
      <xsl:if test="$scalars/ProducerCity!='0'">
        <fo:block>
          <xsl:value-of select="$scalars/ProducerCity"/>
        </fo:block>
      </xsl:if>
    </fo:block>

    <fo:block padding-before="5pt" font-size="9pt" font-family="sans-serif">
      <fo:table table-layout="fixed" width="100%">
        <fo:table-column column-width="125mm"/>
        <fo:table-column column-width="proportional-column-width(1)"/>
        <fo:table-body>
          <fo:table-row>
            <fo:table-cell text-align="left">
              <fo:block>
                Prepared for:
              </fo:block>
              <fo:block>
                &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Group Name:
                <xsl:call-template name="limitstring">
                  <xsl:with-param name="passString" select="$scalars/CorpName"/>
                  <xsl:with-param name="length" select="50"/>
                </xsl:call-template>
              </fo:block>
              <fo:block>
                <xsl:choose>
                  <xsl:when test="$is_composite">
                    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Composite Illustration
                  </xsl:when>
                  <xsl:otherwise>
                    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Insured:
                    <xsl:call-template name="limitstring">
                      <xsl:with-param name="passString" select="$scalars/Insured1"/>
                      <xsl:with-param name="length" select="50"/>
                    </xsl:call-template>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Age:
                  <xsl:value-of select="$scalars/Age"/>
                </fo:block>
              </xsl:if>
              <fo:block>
                Product: <xsl:value-of select="$scalars/PolicyForm"/>
                &nbsp;<xsl:value-of select="$scalars/PolicyMktgName"/>
              </fo:block>
              <fo:block>
                <!-- Single Premium Logic -->
                <xsl:choose>
                  <xsl:when test="$ModifiedSinglePremium='1'">
                    Modified Single Premium Adjustable Life Insurance Policy
                  </xsl:when>
                  <xsl:otherwise>
                    <xsl:value-of select="$scalars/PolicyLegalName"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
            <xsl:if test="$scalars/IsInforce!='1'">
              <fo:block>
                <!-- Single Premium Logic -->
                 <xsl:choose>
                  <xsl:when test="$SinglePremium!='1'">
                    Initial Premium:
                  </xsl:when>
                  <xsl:otherwise>
                    Single Premium:
                  </xsl:otherwise>
                </xsl:choose>
                &nbsp;$<xsl:value-of select="$scalars/InitPrem"/>
              </fo:block>
            </xsl:if>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Initial Death Benefit Option:
                  <xsl:value-of select="$scalars/DBOptInitInteger+1"/>
                </fo:block>
              </xsl:if>
              <xsl:call-template name="print-mastercontractnumber-and-contractnumber"/>
            </fo:table-cell>
            <fo:table-cell text-align="left">
              <fo:block>
                Initial <xsl:if test="$scalars/HasTerm!='0'">Total</xsl:if>
                Selected Face Amount: $<xsl:value-of select="$scalars/InitTotalSA"/>
              </fo:block>
              <!-- Remove date prepared....now exists in footer
              <fo:block>
                Date Prepared: <xsl:call-template name="date-prepared"/>
              </fo:block> -->
              <xsl:if test="$scalars/HasTerm!='0'">
                <fo:block>
                  Initial Base Face Amount:
                  $<xsl:value-of select="$scalars/InitBaseSpecAmt"/>
                </fo:block>
                <fo:block>
                  Initial Term Face Amount:
                  $<xsl:value-of select="$scalars/InitTermSpecAmt"/>
                </fo:block>
              </xsl:if>
              <fo:block>
                Guaranteed Crediting Rate:
                <xsl:value-of select="$scalars/InitAnnGenAcctInt_Guaranteed"/>
              </fo:block>
              <fo:block>
                <xsl:choose>
                  <xsl:when test="$scalars/InforceYear!=0">
                    Current Illustrated Crediting Rate:
                    <xsl:call-template name="ultimate_interest_rate">
                      <xsl:with-param name="counter" select="$scalars/InforceYear + 1"/>
                    </xsl:call-template>
                  </xsl:when>
                  <xsl:otherwise>
                    Current Illustrated Crediting Rate:
                    <xsl:value-of select="$scalars/InitAnnGenAcctInt_Current"/>
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <xsl:if test="$SinglePremium='1' and $scalars/InforceYear &lt;= 4">
                <!-- Single Premium Logic -->
                <fo:block>
                  Ultimate Illustrated Crediting Rate:
                <xsl:choose>
                  <xsl:when test="$ModifiedSinglePremium0='1'">
                    <xsl:value-of select="$vectors[@name='AnnGAIntRate_Current']/duration[11]/@column_value"/>
                  </xsl:when>
                  <xsl:otherwise>
                     <xsl:value-of select="$vectors[@name='AnnGAIntRate_Current']/duration[6]/@column_value"/>
                  </xsl:otherwise>
                </xsl:choose>
                </fo:block>
              </xsl:if>
              <!-- Update with "FriendlyUWType" - Get From Greg -->
              <!-- "&IF(UWType="Medical","Fully underwritten",
              IF(AND(State="TX",UWType="Guaranteed issue"),"Substandard *",UWType))) -->
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Underwriting Type:
                  <xsl:choose>
                    <xsl:when test="$scalars/UWType='Medical'">
                      Fully underwritten
                    </xsl:when>
                    <xsl:when test="($scalars/StatePostalAbbrev='TX') and ($scalars/UWType='Guaranteed issue')">
                      Substandard *
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$scalars/UWType"/>
                    </xsl:otherwise>
                  </xsl:choose>
                </fo:block>
                <fo:block>
                  Rate Classification:
                  <xsl:value-of select="$scalars/UWClass"/>,
                  <xsl:value-of select="$scalars/Smoker"/>,
                  <xsl:value-of select="$scalars/Gender"/>
                </fo:block>
                <xsl:if test="$scalars/UWClass='Rated'">
                  <fo:block>
                    &nbsp;&nbsp;&nbsp;Table Rating:
                    <xsl:value-of select="$scalars/SubstandardTable"/>
                  </fo:block>
                </xsl:if>
              </xsl:if>
            </fo:table-cell>
          </fo:table-row>
        </fo:table-body>
      </fo:table>
    </fo:block>
  </xsl:template>

  <xsl:template name="numeric-summary-values">
    <xsl:param name="columns"/>
    <xsl:param name="counter"/>
    <xsl:param name="age70"/>
    <xsl:param name="prioryears"/>
    <xsl:variable name="PolicyYear_is_not_zero" select="$vectors[@name='PolicyYear']/duration[$counter]/@column_value!='0'"/>
    <fo:table-row>
      <xsl:for-each select="$columns">
        <fo:table-cell padding-top=".2pt" padding-bottom=".2pt">
          <xsl:if test="position()=1">
            <xsl:attribute name="padding-right">6pt</xsl:attribute>
          </xsl:if>
          <fo:block text-align="right">
            <xsl:choose>
              <xsl:when test="not(@name)">
              </xsl:when>
              <xsl:when test="(position() = 1) and ($age70 = 1)">
                Age 70
              </xsl:when>
              <xsl:when test="$PolicyYear_is_not_zero">
                <xsl:variable name="name" select="./@name"/>
                <xsl:value-of select="$vectors[@name=$name]/duration[$counter]/@column_value"/>
              </xsl:when>
              <xsl:when test="position() = 1">
                <xsl:value-of select="$counter"/>
              </xsl:when>
              <xsl:otherwise>
                0
              </xsl:otherwise>
            </xsl:choose>
          </fo:block>
        </fo:table-cell>
      </xsl:for-each>
    </fo:table-row>
    <!-- Display Only Summary Years -->
    <xsl:if test="$age70!=1">
      <xsl:if test="$prioryears!=1">
        <xsl:choose>
          <xsl:when test="$counter &lt; 30">
            <xsl:choose>
              <xsl:when test="$counter=5">
                <!-- Display lapse years that occur prior to year 10 -->
                <xsl:if test="$scalars/LapseYear_Guaranteed &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Guaranteed &gt; 4">
                    <xsl:call-template name="numeric-summary-values">
                      <xsl:with-param name="columns" select="$columns"/>
                      <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                      <xsl:with-param name="age70" select="0"/>
                      <xsl:with-param name="prioryears" select="1"/>
                    </xsl:call-template>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Midpoint &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Midpoint &gt; 4">
                    <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Current &lt; 9">
                  <xsl:if test="$scalars/LapseYear_Current &gt; 4">
                    <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 10 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$counter + 5"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:when>
              <xsl:otherwise>
                <!-- Display lapse years that occur prior to next display year -->
                <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Guaranteed &gt; $counter - 1">
                      <xsl:call-template name="numeric-summary-values">
                        <xsl:with-param name="columns" select="$columns"/>
                        <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                        <xsl:with-param name="age70" select="0"/>
                        <xsl:with-param name="prioryears" select="1"/>
                      </xsl:call-template>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Midpoint &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Midpoint &gt; $counter - 1">
                      <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="columns" select="$columns"/>
                          <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <xsl:if test="$scalars/LapseYear_Current &lt; $counter + 9">
                  <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
                    <xsl:if test="$scalars/LapseYear_Current &gt; $counter - 1">
                      <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                        <xsl:call-template name="numeric-summary-values">
                          <xsl:with-param name="columns" select="$columns"/>
                          <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                          <xsl:with-param name="age70" select="0"/>
                          <xsl:with-param name="prioryears" select="1"/>
                        </xsl:call-template>
                      </xsl:if>
                    </xsl:if>
                  </xsl:if>
                </xsl:if>
                <!-- Create year 20 and 30 values -->
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$counter + 10"/>
                  <xsl:with-param name="age70" select="0"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:otherwise>
            <xsl:if test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
              <!-- Display lapse years that occur prior to next display year -->
              <xsl:if test="$scalars/LapseYear_Guaranteed &gt; $counter - 1">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
              <xsl:if test="$scalars/LapseYear_Midpoint &gt; $counter - 1">
                <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="columns" select="$columns"/>
                    <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                    <xsl:with-param name="age70" select="0"/>
                    <xsl:with-param name="prioryears" select="1"/>
                  </xsl:call-template>
                </xsl:if>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
              <xsl:if test="$scalars/LapseYear_Current &gt; $counter - 1">
                <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                  <xsl:call-template name="numeric-summary-values">
                    <xsl:with-param name="columns" select="$columns"/>
                    <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
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
    <xsl:variable name="numeric-summary-report-columns">
      <column name="PolicyYear">                               |               |Policy _Year    </column>
      <column name="GrossPmt">                                 |               |Premium _Outlay </column>
      <column name="AcctVal_Guaranteed">    Guaranteed Values  |               |Account _Value  </column>
      <column name="CSVNet_Guaranteed">     Guaranteed Values  |               |Cash Surr _Value</column>
      <column name="EOYDeathBft_Guaranteed">Guaranteed Values  |               |Death _Benefit  </column>
      <column/>
      <column name="AcctVal_Midpoint">    Non-Guaranteed Values|Midpoint Values|Account _Value  </column>
      <column name="CSVNet_Midpoint">     Non-Guaranteed Values|Midpoint Values|Cash Surr _Value</column>
      <column name="EOYDeathBft_Midpoint">Non-Guaranteed Values|Midpoint Values|Death _Benefit  </column>
      <column>                            Non-Guaranteed Values|</column>
      <column name="AcctVal_Current">     Non-Guaranteed Values| Current Values|Account _Value  </column>
      <column name="CSVNet_Current">      Non-Guaranteed Values| Current Values|Cash Surr _Value</column>
      <column name="EOYDeathBft_Current"> Non-Guaranteed Values| Current Values|Death _Benefit  </column>
    </xsl:variable>
    <xsl:variable name="columns" select="document('')//xsl:variable[@name='numeric-summary-report-columns']/column"/>

    <!-- The main contents of the body page -->
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

          <!-- Create Numeric Summary Values -->
          <fo:table-body>
              <!-- Display summary values if policy lapses prior to year 5 -->
            <xsl:if test="$scalars/LapseYear_Guaranteed &lt; 4">
              <xsl:call-template name="numeric-summary-values">
                <xsl:with-param name="columns" select="$columns"/>
                <xsl:with-param name="counter" select="$scalars/LapseYear_Guaranteed + 1"/>
                <xsl:with-param name="age70" select="0"/>
                <xsl:with-param name="prioryears" select="1"/>
              </xsl:call-template>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Midpoint &lt; 4">
              <xsl:if test="$scalars/LapseYear_Midpoint &gt; $scalars/LapseYear_Guaranteed">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Midpoint + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Current &lt; 4">
              <xsl:if test="$scalars/LapseYear_Current &gt; $scalars/LapseYear_Midpoint">
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="$scalars/LapseYear_Current + 1"/>
                  <xsl:with-param name="age70" select="0"/>
                  <xsl:with-param name="prioryears" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
            <xsl:call-template name="numeric-summary-values">
              <xsl:with-param name="columns" select="$columns"/>
              <xsl:with-param name="counter" select="5"/>
              <xsl:with-param name="age70" select="0"/>
            </xsl:call-template>
            <xsl:if test="not($is_composite)">
              <xsl:if test="$scalars/Age &lt; 70">
                <fo:table-row>
                  <fo:table-cell padding="8pt">
                    <fo:block/>
                  </fo:table-cell>
                </fo:table-row>
                <xsl:call-template name="numeric-summary-values">
                  <xsl:with-param name="columns" select="$columns"/>
                  <xsl:with-param name="counter" select="70 - $scalars/Age"/>
                  <xsl:with-param name="age70" select="1"/>
                </xsl:call-template>
              </xsl:if>
            </xsl:if>
          </fo:table-body>
        </fo:table>
      </fo:block>

      <fo:block text-align="left" font-size="9pt">
        <xsl:choose>
          <xsl:when test="$is_composite">
            <fo:block padding-top="2em">
              The year of policy lapse on a guaranteed, midpoint
              and current basis is not depicted in the above table of values
              for this composite illustration because it is not applicable
              on a case basis.
            </fo:block>
          </xsl:when>
          <xsl:when test="$scalars/LapseYear_Guaranteed &lt; $scalars/MaxDuration">
            <fo:block padding-top="2em">
              Additional premium will be required
              in year <xsl:value-of select="$scalars/LapseYear_Guaranteed+1"/>
              or contract will lapse based on guaranteed monthly charges
              and interest rate.
            </fo:block>
            <xsl:if test="$scalars/LapseYear_Midpoint &lt; $scalars/MaxDuration">
              <fo:block>
                Additional premium will be required
                in year <xsl:value-of select="$scalars/LapseYear_Midpoint+1"/>
                or contract will lapse based on midpoint monthly charges
                and interest rate.
              </fo:block>
            </xsl:if>
            <xsl:if test="$scalars/LapseYear_Current &lt; $scalars/MaxDuration">
              <fo:block>
                Additional premium will be required
                in year <xsl:value-of select="$scalars/LapseYear_Current+1"/>
                or contract will lapse based on current monthly charges
                and interest rate.
              </fo:block>
            </xsl:if>
          </xsl:when>
        </xsl:choose>
        <xsl:if test="$scalars/IsMec='1'">
          <fo:block padding-top="1em">
            IMPORTANT TAX DISCLOSURE: This is a Modified Endowment Contract.
            Please refer to the Narrative Summary for additional information.
          </fo:block>
        </xsl:if>
        <fo:block padding-top="2em" text-align="center">
          Certification Statements
        </fo:block>
        <fo:block padding-top="1em">
          CONTRACT OWNER / APPLICANT
        </fo:block>
        <xsl:if test="$scalars/InterestDisclaimer!=''">
          <fo:block padding-top="1em">
            I understand that at the present time higher current interest rates
            are credited for policies with case premiums in the amount
            of <xsl:value-of select="$scalars/InterestDisclaimer"/>
          </fo:block>
        </xsl:if>
        <fo:block padding-top="1em">
          <xsl:choose>
            <xsl:when test="$scalars/StatePostalAbbrev='IL'">
              I have received a copy of this illustration and understand
              that this illustration assumes that the currently illustrated
              non-guaranteed elements will continue unchanged
              for all years shown. This is not likely to occur,
              and actual results may be more or less favorable than those shown.
            </xsl:when>
            <xsl:when test="$scalars/StatePostalAbbrev='TX'">
              A copy of this illustration has been provided
              to the Applicant/Policyowner.
            </xsl:when>
            <xsl:otherwise>
              I have received a copy of this illustration, and I understand
              that any non-guaranteed elements illustrated are subject
              to change and could be either higher or lower. Additionally,
              I have been informed by my agent that these values
              are not guaranteed.
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
        <fo:block text-decoration="overline" padding-top="3em">
          CONTRACT OWNER OR APPLICANT SIGNATURE &nbsp;&nbsp;&nbsp;
          <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>
          DATE &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        </fo:block>
        <fo:block padding-top="2em">
          AGENT / AUTHORIZED REPRESENTATIVE
        </fo:block>
        <fo:block padding-top="1em">
          <xsl:choose>
            <xsl:when test="$scalars/StatePostalAbbrev='IL'">
              I have informed the applicant or policyowner
              that this illustration assumes that the currently illustrated
              non-guaranted elements will continue unchanged
              for all years shown. This is not likely to occur,
              and actual results may be more or less favorable than those shown.
            </xsl:when>
            <xsl:when test="$scalars/StatePostalAbbrev='TX'">
              A copy of this illustration has been provided
              to the Applicant/Policyowner.
            </xsl:when>
            <xsl:otherwise>
              I certify that this illustration has been presented
              to the applicant, and that I have explained
              that any non-guaranteed elements illustrated
              are subject to change. I have made no statements
              that are inconsistent with the illustration.
            </xsl:otherwise>
          </xsl:choose>
        </fo:block>
        <fo:block text-decoration="overline" padding-top="3em">
          AGENT OR AUTHORIZED REPRESENTATIVE
          &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
          <fo:inline text-decoration="no-overline">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</fo:inline>DATE
          &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        </fo:block>
      </fo:block>
    </fo:flow>
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
    <xsl:param name="omit-pagenumber" select="boolean(0)"/>
    <xsl:param name="disclaimer" select="string('')"/>
    <xsl:call-template name="generic-footer">
      <xsl:with-param name="top-block" select="$disclaimer"/>
      <xsl:with-param name="left-block">
        <fo:block>
          Date Prepared:
          <xsl:call-template name="date-prepared"/>
        </fo:block>
        <!-- Version Number -->
        <xsl:if test="$scalars/LmiVersion!=''">
          <fo:block>
            System Version:
            <xsl:value-of select="$scalars/LmiVersion"/>
          </fo:block>
        </xsl:if>
      </xsl:with-param>
      <xsl:with-param name="center-block">
        <xsl:choose>
          <xsl:when test="$omit-pagenumber">
            Attachment
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="page-of"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="right-block">
        <!--
        APACHE !! The '/text()' suffix in the XPath expression is a workaround
        for Apache-FOP 0.20.5 bug that somehow treats <xsl:value-of/> as
        <xsl:copy-of/> if it is encountered inside a <xsl:with-param/> as
        a single child. Therefore we explicitly convert the expression into
        a string.
        -->
        <xsl:value-of select="$scalars/InsCoName/text()"/>
        <xsl:if test="$compliance_tracking_number">
          <fo:block>
            <xsl:value-of select="$compliance_tracking_number"/>
          </fo:block>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="ultimate_interest_rate">
    <xsl:param name="counter"/>
    <xsl:value-of select="$vectors[@name='AnnGAIntRate_Current']/duration[$counter]/@column_value"/>
  </xsl:template>

  <xsl:template name="set_single_premium">
    <xsl:variable name="A" select="string-length($scalars/PolicyMktgName) &gt; 5"/>
    <xsl:variable name="B" select="substring($scalars/PolicyLegalName, 1, 6) = 'Single' or substring($scalars/PolicyLegalName, 1, 8) = 'Modified'"/>
    <xsl:value-of select="number($A and $B)"/>
  </xsl:template>

  <xsl:template name="set_modified_single_premium">
    <xsl:variable name="A" select="string-length($scalars/PolicyMktgName) &gt; 5"/>
    <xsl:variable name="B" select="substring($scalars/PolicyLegalName, 1, 6) = 'Single'"/>
    <xsl:variable name="C" select="$scalars/StatePostalAbbrev = 'MA'"/>
    <xsl:value-of select="number($A and $B and $C)"/>
  </xsl:template>

  <xsl:template name="set_modified_single_premium0">
    <xsl:variable name="A" select="string-length($scalars/PolicyMktgName) &gt; 5"/>
    <xsl:variable name="B" select="substring($scalars/PolicyLegalName, 1, 8) = 'Modified'"/>
    <xsl:value-of select="number($A and $B)"/>
  </xsl:template>

  <xsl:template name="set_group_experience_rating">
    <xsl:value-of select="number($scalars/PolicyLegalName='Group Flexible Premium Adjustable Life Insurance Policy')"/>
  </xsl:template>

  <xsl:template name="set_group_carveout">
    <xsl:value-of select="number($scalars/PolicyLegalName='Group Flexible Premium Adjustable Life Insurance Certificate')"/>
  </xsl:template>

  <xsl:template name="set_flexible_premium">
    <xsl:value-of select="number($scalars/PolicyLegalName='Flexible Premium Adjustable Life Insurance')"/>
  </xsl:template>

</xsl:stylesheet>
