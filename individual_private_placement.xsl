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

    $Id: individual_private_placement.xsl,v 1.58 2007-07-04 00:26:39 etarassov Exp $
-->
<!DOCTYPE stylesheet [
<!ENTITY nbsp "&#xA0;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
  <xsl:import href="fo_common.xsl"/>
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>
  <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
  <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
  <xsl:variable name="numberswoc">0123456789</xsl:variable>
  <xsl:variable name="numberswc">0123456789,</xsl:variable>

  <xsl:template match="/">
    <fo:root>
      <fo:layout-master-set>

        <!-- Define the cover page. -->
        <fo:simple-page-master master-name="cover" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <fo:region-body margin=".25in 0"/>
        </fo:simple-page-master>

        <!-- Define the IRR (Guaranteed Charges) Illustration page. -->
        <fo:simple-page-master master-name="irr-guaranteed-illustration" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the IRR (Current Charges) Illustration page -->
        <fo:simple-page-master master-name="irr-current-illustration" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.1in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the Current Values Illustration page -->
        <fo:simple-page-master master-name="current-illustration" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.6in 0 1.2in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent="1.1in"/>
        </fo:simple-page-master>

        <!-- Define the footnotes page. -->
        <fo:simple-page-master master-name="footnotes" margin=".25in">
          <xsl:call-template name="set-page-size"/>
          <!-- Central part of page -->
          <fo:region-body margin="2.25in 0 .6in"/>
          <!-- Header -->
          <fo:region-before extent="3in"/>
          <!-- Footer -->
          <fo:region-after extent=".6in"/>
        </fo:simple-page-master>

        <!-- Define the Supplemental Illustration page. -->
        <xsl:if test="$has_supplemental_report">
          <fo:simple-page-master master-name="supplemental-report" margin=".25in">
            <xsl:call-template name="set-page-size"/>
            <!-- Central part of page -->
            <fo:region-body margin="2.6in 0 1.5in"/>
            <!-- Header -->
            <fo:region-before extent="3in"/>
            <!-- Footer -->
            <fo:region-after extent="1.1in"/>
          </fo:simple-page-master>
        </xsl:if>
      </fo:layout-master-set>

      <!-- The data to be diplayed in the pages, cover page first -->
      <!-- Disabled for now
        <fo:page-sequence master-reference="cover" initial-page-number="200">
          <fo:flow flow-name="xsl-region-body">
            <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
              <fo:block>
                <xsl:value-of select="$scalars/PolicyMktgName"/>
              </fo:block>
              <fo:block>
                <xsl:value-of select="$scalars/PolicyLegalName"/> Cover Sheet
              </fo:block>
              <fo:block padding-top="1em">
                The purpose of the attached illustration is to show
                how the performance of the underlying separate account divisions
                could affect the proposed contract's cash values
                and death benefits. The illustration is hypothetical
                and may not be used to project or predict investment results.
                The illustration must be accompanied or preceded
                by a Confidential Private Placement Memorandum
                offering the Contract.
              </fo:block>
              <fo:block padding-top="1em">
                Placement agent:
                <xsl:value-of select="$scalars/MainUnderwriter"/>
              </fo:block>
              <xsl:if test="$scalars/ScaleUnit!=''">
                <fo:block padding-top="1em">
                  Values per
                  <xsl:value-of select="$scalars/ScaleUnit"/> US dollars.
                </fo:block>
              </xsl:if>
              <fo:block text-align="center" padding-top="1em">
                THIS ILLUSTRATION COVER SHEET IS
                FOR REGISTERED REPRESENTATIVE USE ONLY.
                NOT FOR USE WITH CLIENTS.
              </fo:block>
              <fo:block padding-top="1em">
                Date prepared:
                <!- Comment: The original date format was 'Y-M-D' ->
                <xsl:call-template name="date-prepared"/>
              </fo:block>
              <fo:block>
                Producer: <xsl:value-of select="$scalars/ProducerName"/>
              </fo:block>
              <fo:block>
                Client: <xsl:value-of select="$scalars/Insured1"/>
              </fo:block>
              <fo:block>
                Gender: <xsl:value-of select="$scalars/Gender"/>
              </fo:block>
              <fo:block>
                Issue age: <xsl:value-of select="$scalars/Age"/>
              </fo:block>
              <fo:block>
                State of jurisdiction:
                <xsl:value-of select="$scalars/GetStatePostalAbbrev"/>
              </fo:block>
              <fo:block>
                Country: <xsl:value-of select="$scalars/CountryIso3166Abbrev"/>
              </fo:block>
              <fo:block>
                Non-US corridor factor:
                <xsl:value-of select="$scalars/CorridorFactorSequence"/>
              </fo:block>
              <fo:block>
                Rate class: <xsl:value-of select="$scalars/UWClass"/>
              </fo:block>
              <fo:block>
                7702 test: <xsl:value-of select="$scalars/DefnLifeIns"/>
              </fo:block>
              <fo:block>
                MEC:
                <xsl:choose>
                  <xsl:when test="$scalars/IsMec='1'">
                    Yes
                  </xsl:when>
                  <xsl:otherwise>
                    No
                  </xsl:otherwise>
                </xsl:choose>
              </fo:block>
              <fo:block>
                Assumed gross rate:
                <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>
              </fo:block>
              <fo:block>
                New cash loan:
                <xsl:value-of select="$scalars/LoanSequence"/>
              </fo:block>
              <fo:block>
                Withdrawal:
                <xsl:value-of select="$scalars/NetWDSequence"/>
              </fo:block>
              <fo:block>
                Comments: <xsl:value-of select="$scalars/Comments"/>
              </fo:block>
              <fo:block padding-top="1em">
                &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                Premiums and death benefits
              </fo:block>
              <fo:block padding-top="1em">
                Premium: <xsl:value-of select="$scalars/GrossPmtSequence"/>
              </fo:block>
              <fo:block>
                Dumpin: <xsl:value-of select="$scalars/DumpinScaled"/>
              </fo:block>
              <fo:block>
                Internal 1035 exchange:
                <xsl:value-of select="$scalars/Internal1035AmountScaled"/>
              </fo:block>
              <fo:block>
                External 1035 exchange:
                <xsl:value-of select="$scalars/External1035AmountScaled"/>
              </fo:block>
              <fo:block>
                Specified amount:
                <xsl:value-of select="$scalars/SpecAmtSequence"/>
              </fo:block>
              <fo:block>
                Death benefit option:
                <xsl:value-of select="$scalars/DBOptSquence"/>
              </fo:block>
              <fo:block padding-top="1em">
                &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                Current fees and charges
              </fo:block>
              <fo:block padding-top="1em">
                State premium tax load:
                <xsl:value-of select="$scalars/StatePremTaxLoad"/>
              </fo:block>
              <fo:block>
                DAC tax premium load:
                <xsl:value-of select="$scalars/DacTaxPremLoadRate"/>
              </fo:block>
              <fo:block>
                Asset-based compensation:
                <xsl:value-of select="$scalars/AddonCompOnAssetsSquence"/>
              </fo:block>
              <fo:block>
                Premium-based compensation:
                <xsl:value-of select="$scalars/AddonCompOnPremiumSequence"/>
              </fo:block>
              <fo:block>
                Add-on custodial fee:
                <xsl:value-of select="$scalars/AddonMonthlyFeeSquence"/>
              </fo:block>
              <fo:block>
                Country COI multiplier:
                <xsl:value-of select="$scalars/CountryCOIMultiplier"/>
              </fo:block>
              <fo:block>
                Separate account load:
                <xsl:value-of select="$scalars/TieredSepAcctLoadBands"/>
              </fo:block>
            </fo:block>
          </fo:flow>
        </fo:page-sequence>
      -->

      <!-- IRR (Guaranteed Charges) Illustration -->
      <!-- Body page -->
      <fo:page-sequence master-reference="irr-guaranteed-illustration" initial-page-number="1">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standardheader">
            <xsl:with-param name="displaycontractlanguage" select="1"/>
            <xsl:with-param name="displaydisclaimer" select="1"/>
          </xsl:call-template>
          <fo:block text-align="center" font-size="9.0pt" font-family="serif" padding-top="1em">
            End of Year Contract Values using Guaranteed Charges
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
          <fo:block text-align="center" font-size="9.0pt" font-family="serif" padding-top="1em">
            End of Year Contract Values using Current Charges
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
          <fo:block text-align="center" font-size="9.0pt" font-family="serif" padding-top="1em">
            End of Year Contract Values using Current Charges
            <xsl:call-template name="dollar-units"/>
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter"/>

        <xsl:call-template name="current-illustration-report"/>

      </fo:page-sequence>

      <!-- FOOTNOTES - begins here -->
      <fo:page-sequence master-reference="footnotes">

        <!-- Define the contents of the header. -->
        <fo:static-content flow-name="xsl-region-before">
          <xsl:call-template name="standardheader">
            <xsl:with-param name="displaycontractlanguage" select="0"/>
            <xsl:with-param name="displaydisclaimer" select="0"/>
          </xsl:call-template>
          <fo:block text-align="center" font-size="10.0pt" font-family="serif" padding-top="2em" padding-bottom="1em">
            Footnotes
          </fo:block>
        </fo:static-content>

        <!-- Define the contents of the footer. -->
        <xsl:call-template name="standardfooter">
          <xsl:with-param name="omit-disclaimer" select="1"/>
        </xsl:call-template>

        <!-- FOOTNOTES Body  -->
        <fo:flow flow-name="xsl-region-body">
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block>
              This Contract is only available to persons who are deemed
              accredited investors or qualified purchasers under applicable
              Federal securities laws. The minimum initial case premium
              is $1,000,000. In the case of a dedicated separate account
              or division, the minimum initial case premium is $5,000,000.
              You must be able to bear the risk of loss of your entire
              investment in the Contract. You will be required to represent
              to <xsl:value-of select="$scalars/InsCoShortName"/> that
              you satisfy these Contract requirements.
            </fo:block>
            <fo:block padding-top="1em">
              The information provided is an illustration only
              and is not intended to predict actual performance.
            </fo:block>
            <fo:block padding-top="1em">
              Interest rates and values set forth in the illustration
              are not guaranteed. This illustration assumes that the currently
              illustrated elements will continue unchanged for all years shown.
              This is not likely to occur and actual results may be more or less
              favorable than shown. Benefits and values are not guaranteed
              and are based on assumptions such as investment income and current
              monthly charges. Current charges are subject to change.
            </fo:block>
            <fo:block padding-top="1em">
              These illustrations assume a gross rate of return of 0% and
              <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>,
              respectively, on the account value in the separate account.
              For purposes of this illustration, the assumed
              "gross rate of return" takes into account all investment
              management, custody and other expenses charged by the investment
              manager or underlying fund. Actual rates of return
              will be different because they will be based on the actual
              performance of the separate account divisions. However, the actual
              returns of the separate account divisions will also be net
              of investment management, custody and other expenses
              of the investment manager or the fund. For some separate accounts
              or divisions, there may be additional investment management
              and/or separate account administrative fees that may be charged,
              in which case those fees will be included in the Asset Charges
              illustrated above. If additional fees are determined subsequent
              to issuance of this illustration, such fees will be added
              to the Asset Charges. You should request a new illustration
              to review the impact of such fees on contract performance.
              No tax charge is currently applied to the investment returns
              of the separate account. A charge could be made in the future.
            </fo:block>
            <fo:block padding-top="1em">
              Asset charges in the illustration include the contract's
              mortality and expense (asset charges), separate account
              administrative charges (including in certain cases investment
              management expenses), and, where applicable,
              asset-based compensation and/or amortized premium loads.
            </fo:block>
            <fo:block padding-top="1em">
              Account values may be used to pay monthly charges. Monthly charges
              are due during the life of the insured, and depending
              on actual results, the premium payer may need to continue
              or resume premium outlays. If account values are allocated
              to certain separate account divisions, on each
              Contract Anniversary Date there must be at least 18 months
              of current monthly charges maintained in the money
              market division to pay contract charges.
            </fo:block>
            <fo:block padding-top="1em">
              The premium outlay column includes premium payments,
              reduced by withdrawals and loan disbursements, if any.
            </fo:block>
            <fo:block padding-top="1em">
              Premiums are assumed to be paid on
              <xsl:choose>
                <xsl:when test="$vectors[@name='ErMode']/duration[1]/@column_value='Annual'">
                  an
                </xsl:when>
                <xsl:otherwise>
                  a
                </xsl:otherwise>
              </xsl:choose>
              <xsl:value-of select="translate($vectors[@name='ErMode']/duration[1]/@column_value,$ucletters,$lcletters)"/>
              basis and received at the beginning of the contract year.
              Age, account values, cash surrender values and death benefits
              are illustrated as of the end of the contract year.
            </fo:block>
            <fo:block padding-top="1em">
              PLEASE READ THE FOLLOWING IMPORTANT MAXIMUM NET AMOUNT
              AT RISK DISCLOSURE
            </fo:block>
            <fo:block>
              <xsl:value-of select="$scalars/InsCoShortName"/> has the right
              to promptly refund any amount of premium paid if
              the premium payment will cause the net amount at risk
              to exceed the maximum net amount at risk under the Contract.
              <xsl:value-of select="$scalars/InsCoShortName"/> also
              has the right to automatically withdraw from the divisions
              of the separate account, and distribute to the Contract holder,
              excess amounts that cause the net amount at risk to exceed
              the maximum net amount at risk under the Contract.
            </fo:block>
            <fo:block padding-top="1em">
              If extensive loans or withdrawals are made, additional
              premium payments may be necessary to avoid lapse of the Contract.
              Periods of poor performance in the underlying
              investment portfolios may also contribute to the potential need
              for additional premium payments.
            </fo:block>
            <fo:block padding-top="1em">
              Certain investment divisions have restrictions on the ability
              to access account values allocated to those divisions.
              If account values are allocated to such divisions, the Owner
              will not be able to transfer account values or effect a loan,
              withdrawal or surrender of the Contract until specified dates.
              In addition, payment of a portion of the death benefit
              may be deferred until the account values in such division
              can be liquidated. Refer to the private placement memorandum
              for details.
            </fo:block>
          </fo:block>

          <!-- Forced New Page -->
          <fo:block break-after="page"/>
          <fo:block text-align="left" font-size="9pt" font-family="sans-serif">
            <fo:block>
              In the states of Alaska or South Dakota, there may be a surrender
              charge in connection with premium tax liability that would
              generally be limited to 6 years. This surrender charge
              is not reflected in the illustrated values. Nevertheless,
              surrender charges under the Contract are limited by applicable
              non-forfeiture laws and regulations.
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              PLEASE READ THE FOLLOWING IMPORTANT TAX DISCLOSURE
            </fo:block>
            <fo:block>
              The definition of life insurance elected for this Contract is
              <xsl:choose>
                <xsl:when test="$scalars/DefnLifeIns='GPT'">
                  the guideline premium test. The guideline single premium
                  is $<xsl:value-of select="$scalars/InitGSP"/>
                  and the guideline level premium
                  is $<xsl:value-of select="$scalars/InitGLP"/>
                </xsl:when>
                <xsl:otherwise>
                  the cash value accumulation test.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              The initial 7-pay premium limit
              is $<xsl:value-of select="$scalars/InitSevenPayPrem"/>.
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
                  becomes a Modified Endowment Contract (MEC)
                  in policy year <xsl:value-of select="$scalars/MecYear+1"/>
                </xsl:when>
                <xsl:otherwise>
                  is not a Modified Endowment Contract (MEC). Subsequent changes
                  in the contract, including but not limited to increases
                  and decreases in premiums or benefits, may cause the contract
                  to be retested and may result in the contract becoming a MEC.
                </xsl:otherwise>
              </xsl:choose>
            </fo:block>
            <fo:block padding-top="1em">
              If a contract is a MEC, any distributions are taxed to the extent
              of any gain in the contract, and an additional 10% penalty tax
              will apply to the taxable portion of the distribution.
              The 10% penalty tax applies if the contract owner is an individual
              under age 59 1/2 and does not meet any applicable exception,
              or if the contract is owned by a corporation or other entity.
            </fo:block>
            <fo:block font-weight="bold" padding-top="1em">
              This illustration is not written or intended as tax
              or legal advice and may not be relied on for purposes
              of avoiding any federal tax penalties.
              For more information pertaining to the tax consequences
              of purchasing or owning this policy, consult with your own
              independent tax or legal counsel.
            </fo:block>
            <fo:block padding-top="1em">
              The state of issue
              is <xsl:value-of select="$scalars/StatePostalAbbrev"/>.
            </fo:block>
            <xsl:choose>
              <xsl:when test="$scalars/IsInforce!='1'">
                <xsl:if test="string-length($scalars/InsCoPhone) &gt; 14">
                  <fo:block padding-top="1em">
                    Compliance tracking number:
                    <xsl:value-of select="substring($scalars/InsCoPhone, 1, 15)"/>
                  </fo:block>
                </xsl:if>
              </xsl:when>
              <xsl:otherwise>
                <xsl:if test="string-length($scalars/InsCoPhone) &gt; 16">
                  <fo:block padding-top="1em">
                    Compliance Tracking Number:
                    <xsl:value-of select="substring($scalars/InsCoPhone, 16)"/>
                  </fo:block>
                </xsl:if>
              </xsl:otherwise>
            </xsl:choose>
            <fo:block padding-top="1em">
              Please refer to the Contract for a complete explanation
              of benefits, rights and obligations. In the event
              of a conflict between the illustration and the Contract,
              the terms of the Contract will control.
            </fo:block>
            <fo:block padding-top="1em">
              Internal Rate of Return ("IRR") is an interest rate
              at which the Premium Outlay illustrated would have to be invested
              outside the Contract to generate the Cash Surrender Value
              or Death Benefit. The IRR is illustrative only
              and does not reflect past or future results.
            </fo:block>
            <fo:block padding-top="1em">
              Placement Agent: <xsl:value-of select="$scalars/MainUnderwriter"/>,
              <xsl:value-of select="$scalars/MainUnderwriterAddress"/>.
              <xsl:value-of select="$scalars/MainUnderwriter"/> is a wholly owned
              subsidiary of <xsl:value-of select="$scalars/InsCoName"/>.
            </fo:block>
            <fo:block padding-top="1em">
              This illustration reflects a fixed policy loan interest rate
              of <xsl:value-of select="$scalars/InitAnnLoanDueRate"/>.
            </fo:block>
            <fo:block padding-top="1em">
              In general, policy loan interest is not deductible. If the policy
              is owned by a business, deductibility is extremely limited.
              Please see your tax counsel for advice.
            </fo:block>
            <fo:block padding-top="1em">
              Taking a policy loan could have adverse tax consequences.
              If your aggregate policy loans substantially exceed
              your cost basis, you may incur a significant income tax liability
              if the policy terminates before the insured's death. You may
              have to make substantial payments to cover policy charges
              and policy loan interest to prevent termination of the policy
              and to avoid the potential income tax liability.
              Some of the indications that such a situation may arise include:
              (1) high outstanding debt relative to the policy's account value;
              (2) low account value relative to a high death benefit;
              and (3) lower than expected interest credited
              to your account value. You should also understand that generally
              your insurance risk charge rates increase annually as the age
              of the insured increases, and that the risk charge increases
              as the amount of policy risk increases. You can reduce
              the likelihood that you either will incur a significant income
              tax liability should your policy terminate before the death
              of the insured, or that you will need to make substantial payments
              to keep your policy from terminating by monitoring
              and reviewing all aspects of your policy on a regular basis
              with your tax advisor, your financial representative,
              and/or any other financial advisor you might have.
            </fo:block>
            <fo:block padding-top="1em">
              This illustration must be preceded or accompanied
              by the current confidential private placement memorandum
              for <xsl:value-of select="$scalars/PolicyMktgName"/> variable
              life insurance contract and the current prospectuses
              and private placement memorandums for its underlying investment
              choices. Before purchasing a variable life insurance contract,
              investors should carefully consider the investment objectives,
              risks, charges and expenses of the variable life insurance
              contract and its underlying investment choices. Please read
              the prospectuses and private placement memorandums carefully
              before investing or sending money.
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
            <fo:block text-align="center" font-size="9.0pt" font-family="serif" padding-top="1em">
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
    <xsl:param name="displaycontractlanguage"/>
    <xsl:param name="displaydisclaimer"/>
    <fo:block text-align="center" font-size="9.0pt">
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
          The purpose of the Illustration is to show how the performance
          of the underlying separate account divisions could affect
          the Contract's cash values and death benefits. This Illustration
          is hypothetical and may not be used to project
          or predict investment results.
        </fo:block>
        <fo:block padding-top=".5em" padding-bottom="1em">
          This Illustration must be accompanied or preceded
          by a Confidential Private Placement Memorandum
          offering the Contract.
        </fo:block>
      </xsl:if>
    </fo:block>

    <fo:table table-layout="fixed" width="100%">
      <fo:table-column column-width="125mm"/>
      <fo:table-column column-width="proportional-column-width(1)"/>
      <fo:table-body>
        <fo:table-row>
          <fo:table-cell padding-right="2mm">
            <fo:block text-align="left" font-size="9.0pt">
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
                  Gender: <xsl:value-of select="$scalars/Gender"/>
                </fo:block>
              </xsl:if>
              <xsl:if test="not($is_composite)">
                <fo:block>
                  Age: <xsl:value-of select="$scalars/Age"/>
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
            <fo:block text-align="left" font-size="9.0pt">
              <fo:block>
                Contract: <xsl:value-of select="$scalars/PolicyMktgName"/>
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

    <!--
    This monstrosity truncates 'Franchise' and 'PolicyNumber' to:
    * 30 characters if both are present;
    * 15 characters if only one is present.
    -->
    <xsl:variable name="has_franchise" select="number($scalars/Franchise!='')"/>
    <xsl:variable name="has_polnumber" select="number($scalars/PolicyNumber!='' and not($is_composite))"/>
    <xsl:variable name="contracts" select="$has_franchise + $has_polnumber"/>
    <xsl:if test="$contracts">
      <xsl:variable name="number_length" select="floor(30 div $contracts)"/>
      <fo:block text-align="left" font-size="9.0pt" font-family="sans-serif">
        <xsl:if test="$has_franchise">
          Master contract:
          <xsl:call-template name="limitstring">
            <xsl:with-param name="passString" select="$scalars/Franchise"/>
            <xsl:with-param name="length" select="$number_length"/>
          </xsl:call-template>
        </xsl:if>
        <xsl:if test="$has_polnumber">
          Contract number:
          <xsl:call-template name="limitstring">
            <xsl:with-param name="passString" select="$scalars/PolicyNumber"/>
            <xsl:with-param name="length" select="$number_length"/>
          </xsl:call-template>
        </xsl:if>
      </fo:block>
    </xsl:if>
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
      <fo:block font-size="9.0pt" font-family="serif">
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
                  Gross / Net Rate
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Guaranteed"/>
                  Gross / Net Rate
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
      <fo:block font-size="9.0pt" font-family="serif">
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
                  Gross / Net Rate
                </fo:block>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="3">
                <fo:block/>
              </fo:table-cell>
              <fo:table-cell number-columns-spanned="2">
                <xsl:call-template name="header-cell-with-border"/>
                <fo:block text-align="center">
                  <xsl:value-of select="$scalars/InitAnnSepAcctGrossInt_Current"/>
                  Gross / Net Rate
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
      <column special="Asset_Charges">Asset _Charges</column>
      <column name="GrossIntCredited_Current">Investment _Income</column>
      <column name="AcctVal_Current">Account _Value</column>
      <column name="CSVNet_Current">Cash _Surr Value</column>
      <column name="EOYDeathBft_Current">Death_ Benefit</column>
    </xsl:variable>
    <xsl:variable name="columns_raw" select="document('')//xsl:variable[@name='current_illustration_columns_raw']/column"/>
    <xsl:variable name="columns" select="$columns_raw[not(@composite)] | $columns_raw[boolean(@composite='1')=$is_composite]"/>

    <!-- The main contents of the body page -->
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:call-template name="generate-table-columns">
            <xsl:with-param name="columns" select="$columns"/>
          </xsl:call-template>

          <fo:table-header>
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
      <xsl:when test="$special='Asset_Charges'">
        <xsl:variable name="GrossIntCredited_Current" select="$vectors[@name='GrossIntCredited_Current']/duration[$counter]/@column_value"/>
        <xsl:variable name="NetIntCredited_Current" select="$vectors[@name='NetIntCredited_Current']/duration[$counter]/@column_value"/>
        <xsl:variable name="SepAcctLoad_Current" select="$vectors[@name='SepAcctLoad_Current']/duration[$counter]/@column_value"/>
        <xsl:value-of select="format-number(translate($GrossIntCredited_Current,$numberswc,$numberswoc) - translate($NetIntCredited_Current,$numberswc,$numberswoc) + translate($SepAcctLoad_Current,$numberswc,$numberswoc),'###,###,###')"/>
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
        <xsl:if test="not($omit-disclaimer)">
          This Illustration is not a contract, or an offer or solicitation
          to enter into a contract. Illustrated values are based on
          the investment earnings assumptions shown above and are
          not guaranteed. Values based on current charges reflect applicable
          fees and charges which are subject to change. There are no guaranteed
          values under this contract. The impact of tax requirements
          is not reflected in these values. Consult your tax advisor.
        </xsl:if>
      </xsl:with-param>
      <xsl:with-param name="left-block">
        <fo:block><xsl:value-of select="$scalars/InsCoName"/></fo:block>
        <fo:block><xsl:value-of select="$scalars/InsCoAddr"/></fo:block>
      </xsl:with-param>
      <xsl:with-param name="right-block">
        <xsl:if test="$scalars/LmiVersion!=''">
          <fo:block>
            System Version:
            <xsl:value-of select="$scalars/LmiVersion"/>
          </fo:block>
        </xsl:if>
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
