<?xml version="1.0" encoding="UTF-8"?>
<!--
    Convert ledger XML in the new format to the old format.

    Copyright (C) 2007, 2008 Gregory W. Chicares.

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

    $Id: xml2to1.xsl,v 1.3 2008-01-01 18:30:00 chicares Exp $
-->
<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:import href="fo_common.xsl"/>

  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <xsl:template match="/illustration">
    <illustration>
      <scalar>
        <xsl:for-each select="string_scalar|double_scalar">
          <xsl:variable name="name-raw">
            <xsl:call-template name="get-column-old-name">
              <xsl:with-param name="column" select="."/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="name" select="normalize-space($name-raw)"/>

          <xsl:element name="{$name}"><xsl:value-of select="text()"/></xsl:element>
        </xsl:for-each>
      </scalar>
      <data>
        <xsl:for-each select="string_vector|double_vector">
          <xsl:variable name="name-raw">
            <xsl:call-template name="get-column-old-name">
              <xsl:with-param name="column" select="."/>
            </xsl:call-template>
          </xsl:variable>
          <xsl:variable name="name" select="normalize-space($name-raw)"/>

          <newcolumn>
            <column name="{$name}">
              <xsl:for-each select="duration">
                <duration number="{position() - 1}" column_value="{./text()}"/>
              </xsl:for-each>
            </column>
          </newcolumn>
        </xsl:for-each>
      </data>
      <xsl:for-each select="supplementalreport">
        <supplementalreport>
          <xsl:if test="title">
            <xsl:copy-of select="title"/>
          </xsl:if>
          <xsl:for-each select="spacer|column">
            <columns>
              <xsl:choose>
                <xsl:when test="name()='spacer'">
                  <name><xsl:value-of select="$NO_SUPPLEMENTAL_COLUMN"/></name>
                  <title/>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:variable name="name">
                    <xsl:call-template name="get-column-old-name">
                      <xsl:with-param name="column" select="."/>
                    </xsl:call-template>
                  </xsl:variable>
                  <xsl:variable name="title">
                    <xsl:call-template name="get-column-old-title">
                      <xsl:with-param name="column" select="."/>
                    </xsl:call-template>
                  </xsl:variable>
                  <name><xsl:value-of select="normalize-space($name)"/></name>
                  <title><xsl:value-of select="normalize-space($title)"/></title>
                </xsl:otherwise>
              </xsl:choose>
            </columns>
          </xsl:for-each>
        </supplementalreport>
      </xsl:for-each>
    </illustration>
  </xsl:template>

  <xsl:template name="get-column-old-name">
    <xsl:param name="column"/>
    <xsl:variable name="name" select="$column/@name"/>
    <xsl:variable name="basis" select="$column/@basis"/>
    <xsl:choose>
      <xsl:when test="@basis='run_curr_basis'">
        <xsl:value-of select="concat($name, '_Current')"/>
      </xsl:when>
      <xsl:when test="@basis='run_guar_basis'">
        <xsl:value-of select="concat($name, '_Guaranteed')"/>
      </xsl:when>
      <xsl:when test="@basis='run_mdpt_basis'">
        <xsl:value-of select="concat($name, '_Midpoint')"/>
      </xsl:when>

      <xsl:when test="@basis='run_curr_basis_sa_zero'">
        <xsl:value-of select="concat($name, '_CurrentZero')"/>
      </xsl:when>
      <xsl:when test="@basis='run_guar_basis_sa_zero'">
        <xsl:value-of select="concat($name, '_GuaranteedZero')"/>
      </xsl:when>
      <xsl:when test="@basis='run_mdpt_basis_sa_zero'">
        <xsl:value-of select="concat($name, '_MidpointZero')"/>
      </xsl:when>

      <xsl:otherwise>
        <xsl:value-of select="$name"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:variable name="ledger_columns" select="document('ledger_formats.xml')/lmi:columns/lmi:column"/>
  <xsl:template name="get-column-old-title">
    <xsl:param name="column"/>
    <xsl:variable name="name" select="$column/@name"/>
    <xsl:variable name="basis" select="$column/@basis"/>
    <xsl:variable name="ledger_column" select="$ledger_columns[@name=$name]"/>
    <xsl:choose>
      <xsl:when test="not(@basis)">
        <xsl:value-of select="$ledger_column/lmi:title"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$ledger_column/lmi:title[@basis=$basis]"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
