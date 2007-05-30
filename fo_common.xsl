<?xml version="1.0" encoding="UTF-8"?>
<!--
    Common part of various illustrations.

    Copyright (C) 2007 Gregory W. Chicares.

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

    $Id: fo_common.xsl,v 1.11 2007-05-30 16:07:46 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <!--
  In some xsl:for-each loop we could iterate over a node set from some
  other document (not the current document being transformed, for example
  we could iterate over a list of columns from the separate format file
  to generate a list of available columns).

  In such a case inside the loop the root node '/' binds to the root node
  of that external document and we cannot use '/illustration' no longer
  to acces our current xml data.
  Use the global variable '$illustration' to access the data.
  -->
  <xsl:variable name="illustration" select="/illustration"/>
  <xsl:variable name="max-lapse-year-text">
    <xsl:call-template name="get-max-lapse-year"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year" select="number($max-lapse-year-text)"/>
  <xsl:variable name="supplemental_report" select="$illustration/supplementalreport"/>
  <xsl:variable name="has_supplemental_report" select="boolean($illustration/scalar/SupplementalReport='1')"/>

  <!--
  Frequently used value. For a composite case the variable is_composite is true.
  -->
  <xsl:variable name="is_composite" select="boolean($illustration/scalar/Composite='1')"/>

  <!--
  The two strings below define how the special symbols in are escaped
  when the title text is written into the report.
  Currently only '_' (the underscore character) is translated - into a hard-space.
  For example a text '____Right_Aligned' would be translated into
  '&#xA0;&#xA0;&#xA0;&#xA0;Right&#xA0;Aligned' which will result in the text
  being padded to the right even if the text is aligned to the left.
  -->
  <xsl:variable name="SPECIAL_LETTERS">_</xsl:variable>
  <xsl:variable name="SPECIAL_LETTERS_ESCAPED">&#xA0;</xsl:variable>

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
  LapseYear_GuaranteedZero in their XML. We rely on the fact that a inexistent
  node evaluates to zero in a comparison expression.
  -->

  <xsl:template name="get-max-lapse-year">
    <xsl:call-template name="max-comparison">
      <xsl:with-param name="value1">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$illustration/scalar/LapseYear_Current"/>
          <xsl:with-param name="value2" select="$illustration/scalar/LapseYear_Guaranteed"/>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="value2">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="$illustration/scalar/LapseYear_CurrentZero"/>
          <xsl:with-param name="value2" select="$illustration/scalar/LapseYear_GuaranteedZero"/>
        </xsl:call-template>
      </xsl:with-param>
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

  <!--
    Print Dollar Units
    This template is common to all the illustration xsl templates.
    Originally it has appeared in files with minor differencies such as:
      * explicitly specified text alignment, font size or not specified;
      * paranteses added around the text or omitted;
      * an extra space added just before the text.
  -->
  <xsl:template name="dollar-units">
    <xsl:choose>
      <xsl:when test="$illustration/scalar/ScaleUnit=''">
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in dollars)</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:otherwise>
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in </xsl:text>
          <xsl:value-of select="$illustration/scalar/ScaleUnit"/>
          <xsl:text>s of dollars)</xsl:text>
        </fo:block>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="supplemental-report-body">
    <fo:flow flow-name="xsl-region-body">
      <fo:block font-size="9.0pt" font-family="serif">
        <fo:table table-layout="fixed" width="100%">
          <xsl:for-each select="$supplemental_report/columns">
            <fo:table-column/>
          </xsl:for-each>
          <fo:table-header>
            <fo:table-row>
              <xsl:for-each select="$supplemental_report/columns">
                <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt">
                  <fo:block text-align="right">
                    <xsl:value-of select="translate(./title,$SPECIAL_LETTERS,$SPECIAL_LETTERS_ESCAPED)"/>
                  </fo:block>
                </fo:table-cell>
              </xsl:for-each>
            </fo:table-row>
            <fo:table-row>
              <fo:table-cell padding="2pt">
                <fo:block/>
              </fo:table-cell>
            </fo:table-row>
          </fo:table-header>
          <!-- Create Supplemental Report Values -->
          <!-- make inforce illustration start in the inforce year -->
          <fo:table-body>
            <xsl:call-template name="supplemental-report-values">
              <xsl:with-param name="counter" select="$illustration/scalar/InforceYear + 1"/>
              <xsl:with-param name="inforceyear" select="0 - $illustration/scalar/InforceYear"/>
            </xsl:call-template>
          </fo:table-body>
        </fo:table>
      </fo:block>
      <xsl:if test="$has_supplemental_report">
        <fo:block id="endofdoc"/>
      </xsl:if>
    </fo:flow>
  </xsl:template>

  <!-- Create Supplemental Report Values -->
  <xsl:template name="supplemental-report-values">
    <xsl:param name="counter"/>
    <xsl:param name="inforceyear"/>
    <xsl:if test="$counter &lt;= $max-lapse-year">
      <fo:table-row>
        <xsl:for-each select="$supplemental_report/columns">
          <xsl:variable name="column_name" select="string(./name)"/>
          <fo:table-cell padding=".2pt">
            <fo:block text-align="right">
              <xsl:value-of select="$illustration/data/newcolumn/column[@name=$column_name]/duration[$counter]/@column_value"/>
            </fo:block>
          </fo:table-cell>
        </xsl:for-each>
      </fo:table-row>
      <!-- Blank Row Every 5th Year -->
      <xsl:if test="($counter + $inforceyear) mod 5=0">
        <fo:table-row>
          <fo:table-cell padding="4pt">
            <fo:block text-align="right" />
          </fo:table-cell>
        </fo:table-row>
      </xsl:if>
      <xsl:call-template name="supplemental-report-values">
        <xsl:with-param name="counter" select="$counter + 1"/>
        <xsl:with-param name="inforceyear" select="$inforceyear"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>