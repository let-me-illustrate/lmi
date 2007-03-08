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

    $Id: xsl_fo_common.xsl,v 1.1.2.12 2007-03-08 17:23:14 etarassov Exp $
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
    <!--
        Frequently used value. If true then the data is for a composite case.
    -->
    <xsl:variable name="is_composite" select="boolean(/illustration/scalar/Composite='1')"/>
    <xsl:variable name="supplemental_report" select="/illustration/supplementalreport"/>
    <xsl:variable name="has_supplemental_report" select="boolean(/illustration/scalar/SupplementalReport='1')"/>

<!--
    TODO ?? These constants are used for decoding the numbers from text.
    Get rid of it once the arithmetics is moved to C++ code.
-->
    <xsl:variable name="NUMBERS_WITHOUT_COMMA">0123456789</xsl:variable>
    <xsl:variable name="NUMBERS_WITH_COMMA">0123456789,</xsl:variable>
    <xsl:variable name="NUMBERS_FORMATTING_STRING">###,###,###</xsl:variable>

    <xsl:variable name="LOWER_CASE_LETTERS">abcdefghijklmnopqrstuvwxyz</xsl:variable>
    <xsl:variable name="UPPER_CASE_LETTERS">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

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

    <!-- Generic template that generates data table column definitions -->
    <xsl:template name="data-table-columns">
        <xsl:param name="columns"/>
        <xsl:for-each select="$columns">
            <xsl:choose>
                <xsl:when test="@name or (name and (name/text()!='[none]')) or @scalar">
                    <fo:table-column column-width="proportional-column-width(1)"/>
                </xsl:when>
                <xsl:otherwise>
                    <fo:table-column column-width="proportional-column-width(.2)"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

    <!-- Generic template that generates table header row (just above the data cells) -->
    <xsl:template name="data-table-headers">
        <xsl:param name="columns"/>
        <fo:table-row text-align="right">
            <xsl:for-each select="$columns">
                <fo:table-cell border-bottom-style="solid" border-bottom-width="1pt" border-bottom-color="blue" padding-top="4pt" padding-bottom="2pt" display-align="after">
                    <xsl:call-template name="normalize_underscored_name">
                        <xsl:with-param name="text" select="concat(./title, @title, ./text())"/>
                    </xsl:call-template>
                </fo:table-cell>
            </xsl:for-each>
        </fo:table-row>
        <!-- TODO ?? Force header margin using some other trick... -->
        <fo:table-row>
            <fo:table-cell padding="2pt">
                <fo:block/>
            </fo:table-cell>
        </fo:table-row>
    </xsl:template>

    <!-- Generic template that generates the actual data in the table -->
    <xsl:template name="data-table-data">
        <xsl:param name="columns"/>
        <xsl:param name="counter" select="1"/>
        <xsl:param name="control_column_name" select="'PolicyYear'"/>
        <xsl:if test="/illustration/data/newcolumn/column[@name=$control_column_name]/duration[$counter]/@column_value!='0'">
            <fo:table-row text-align="right">
                <xsl:for-each select="$columns">
                    <fo:table-cell padding-top="1.2pt">
                        <!-- Add some space if it the first row and some space after each 5th year -->
                        <xsl:if test="$counter mod 5=0">
                            <xsl:attribute name="padding-bottom">8pt</xsl:attribute>
                        </xsl:if>
                        <fo:block>
                            <xsl:choose>
                                <xsl:when test="@name or ./name">
                                    <xsl:variable name="column_name" select="concat(@name, ./name)"/>
<!--
    TODO ?? We have to take care of some special cases, like a row not being present
    in xml data, but which should be calculated from some other data.
    It will much better to move such a row calculations into C++ code.
-->
                                    <xsl:choose>
                                        <xsl:when test="$column_name='Special_PremiumLoad'">
                                            <xsl:value-of select="format-number(translate($illustration/data/newcolumn/column[@name='Outlay']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)-translate($illustration/data/newcolumn/column[@name='NetPmt_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='NetWD']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='Loan']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA),$NUMBERS_FORMATTING_STRING)"/>
                                        </xsl:when>
                                        <xsl:when test="$column_name='Special_AdminCharges'">
                                            <xsl:value-of select="format-number(translate($illustration/data/newcolumn/column[@name='AnnPolFee_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='SpecAmtLoad_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='MlyPolFee_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA),$NUMBERS_FORMATTING_STRING)"/>
                                        </xsl:when>
                                        <xsl:when test="$column_name='Special_AssetCharges'">
                                            <xsl:value-of select="format-number(translate($illustration/data/newcolumn/column[@name='GrossIntCredited_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)-translate($illustration/data/newcolumn/column[@name='NetIntCredited_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='AcctValLoadAMD_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA)+translate($illustration/data/newcolumn/column[@name='AcctValLoadBOM_Current']/duration[$counter]/@column_value,$NUMBERS_WITH_COMMA,$NUMBERS_WITHOUT_COMMA),$NUMBERS_FORMATTING_STRING)"/>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="$illustration/data/newcolumn/column[@name=$column_name]/duration[$counter]/@column_value"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:when>
                                <xsl:when test="@scalar">
                                    <xsl:variable name="scalar_name" select="@scalar"/>
<!--
    TODO ?? Temporary workaround. Retrieve the scalar value by its element name too.
    Exactly as for columns of data.
    Note that 'InitAnnLoanDueRate' is hardcoded!
-->
                                    <xsl:value-of select="$illustration/scalar/InitAnnLoanDueRate"/>
                                </xsl:when>
                            </xsl:choose>
                        </fo:block>
                    </fo:table-cell>
                </xsl:for-each>
            </fo:table-row>
            <xsl:call-template name="data-table-data">
                <xsl:with-param name="columns" select="$columns"/>
                <xsl:with-param name="control_column_name" select="$control_column_name"/>
                <xsl:with-param name="counter" select="$counter + 1"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <!-- Create Supplemental Report -->
    <xsl:template name="supplemental-report-body">
        <xsl:variable name="columns" select="$supplemental_report/columns"/>
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
            <xsl:if test="$has_supplemental_report">
                <fo:block id="endofdoc"/>
            </xsl:if>
        </fo:flow>
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
