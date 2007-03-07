<?xml version="1.0" encoding="UTF-8"?>
<!--
    Life insurance illustrations.

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

    $Id: xsl_fo_common.xsl,v 1.1.2.5 2007-03-07 15:30:10 etarassov Exp $
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format" version="1.0">
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
    <xsl:variable name="supplemental_report" select="/illustration/supplementalreport"/>
    <xsl:variable name="has_supplemental_report" select="string(/illustration/scalar/SupplementalReport)"/>

    <xsl:template name="normalize_underscored_name">
        <xsl:param name="text"/>
        <xsl:choose>
            <xsl:when test="starts-with($text, '_') or starts-with($text, ' ')">
                <!-- chop off the leading junk character -->
                <xsl:call-template name="normalize_underscored_name">
                    <xsl:with-param name="text" select="substring($text, 2)"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:when test="contains($text, '_')">
                <!-- cut the text into two parts and process it separately -->
                <xsl:call-template name="normalize_underscored_name">
                    <xsl:with-param name="text" select="substring-before($text, '_')"/>
                </xsl:call-template>
                <xsl:call-template name="normalize_underscored_name">
                    <xsl:with-param name="text" select="substring-after($text, '_')"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <fo:block>
                    <xsl:value-of select="$text"/>
                </fo:block>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- Create Supplemental Report Body -->
    <!-- Calls supplemental-report-values template to generate data part -->
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
                                <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding="2pt" display-align="after" text-align="right">
                                    <xsl:call-template name="normalize_underscored_name">
                                        <xsl:with-param name="text" select="./title"/>
                                    </xsl:call-template>
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
                    <fo:table-body>
                        <xsl:call-template name="supplemental-report-values">
                            <xsl:with-param name="counter" select="1"/>
                        </xsl:call-template>
                    </fo:table-body>
                </fo:table>
            </fo:block>
            <xsl:if test="$has_supplemental_report='1'">
                <fo:block id="endofdoc"/>
            </xsl:if>
        </fo:flow>
    </xsl:template>

    <!-- Create Supplemental Report Values -->
    <xsl:template name="supplemental-report-values">
        <xsl:param name="counter"/>
        <xsl:if test="illustration/data/newcolumn/column[1]/duration[$counter]/@column_value!='0'">
            <fo:table-row>
                <xsl:for-each select="$supplemental_report/columns">
                    <xsl:variable name="column_name" select="string(./name)"/>
                    <fo:table-cell padding=".2pt">
                        <fo:block text-align="right">
                            <xsl:value-of select="/illustration/data/newcolumn/column[@name=$column_name]/duration[$counter]/@column_value"/>
                        </fo:block>
                    </fo:table-cell>
                </xsl:for-each>
            </fo:table-row>
            <!-- Blank Row Every 5th Year -->
            <xsl:if test="$counter mod 5=0">
                <fo:table-row>
                    <fo:table-cell padding="4pt">
                        <fo:block text-align="right"/>
                    </fo:table-cell>
                </fo:table-row>
            </xsl:if>
            <xsl:call-template name="supplemental-report-values">
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- Print Dollar Units -->
    <xsl:template name="dollar-units">
        <xsl:choose>
            <xsl:when test="/illustration/scalar/ScaleUnit=''">
                <xsl:text>(Values shown are in dollars)</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>(Values shown are in </xsl:text>
                <xsl:value-of select="/illustration/scalar/ScaleUnit"/>
                <xsl:text>s of dollars)</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
