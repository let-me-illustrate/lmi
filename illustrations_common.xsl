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

    $Id: illustrations_common.xsl,v 1.1.2.3 2007-05-23 21:07:18 etarassov Exp $
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:fo="http://www.w3.org/1999/XSL/Format">
  <xsl:variable name="max-lapse-year-text">
    <xsl:call-template name="get-max-lapse-year"/>
  </xsl:variable>
  <xsl:variable name="max-lapse-year" select="number($max-lapse-year-text)"/>

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
          <xsl:with-param name="value1" select="/illustration/scalar/LapseYear_Current"/>
          <xsl:with-param name="value2" select="/illustration/scalar/LapseYear_Guaranteed"/>
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="value2">
        <xsl:call-template name="max-comparison">
          <xsl:with-param name="value1" select="/illustration/scalar/LapseYear_CurrentZero"/>
          <xsl:with-param name="value2" select="/illustration/scalar/LapseYear_GuaranteedZero"/>
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
      <xsl:when test="/illustration/scalar/ScaleUnit=''">
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in dollars)</xsl:text>
        </fo:block>
      </xsl:when>
      <xsl:otherwise>
        <fo:block text-align="center" font-size="9pt">
          <xsl:text>(Values shown are in </xsl:text>
          <xsl:value-of select="/illustration/scalar/ScaleUnit"/>
          <xsl:text>s of dollars)</xsl:text>
        </fo:block>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

</xsl:stylesheet>
