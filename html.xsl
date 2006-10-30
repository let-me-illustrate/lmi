<?xml version="1.0"?>
<!--
    Xsl template used to render illustration data into html.

    Copyright (C) 2002, 2003, 2004, 2005, 2006 Gregory W. Chicares.

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

    $Id: html.xsl,v 1.1.2.3 2006-10-30 12:46:13 etarassov Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<xsl:stylesheet version="1.0"
    xmlns:lmi="http://www.letmeillustrate.com"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xsi:schemaLocation="http://www.letmeillustrate.com schema.xsd">

<xsl:output method="html" encoding="iso-8859-1" indent="yes" />

<xsl:include href="common.xsl" />

<!--
    The list of columns to be shown.
    The supplemental report columns will be appended to this list.

    If a 'column' content is not empty then it will be used as a column title.
    Example:
    <column name="AcctVal" basis="run_guar_basis"
        >Guaranteed Account Value</column>

    will generate a column with values from AcctVal:run_guar_basis column with
    'Guaranteed Account Value' as its title instead of the default
    'Guar Account Value' title from 'format.xml'.

    Note: the variable '$basic_columns_xml' contains the unparsed XML as text.
    Use '$basic_columns' to get the parsed list of nodes.
-->
<xsl:variable name="basic_columns_xml">
    <column name="Outlay" />
    <column name="AcctVal"     basis="run_guar_basis" />
    <column name="CSVNet"      basis="run_guar_basis" />
    <column name="EOYDeathBft" basis="run_guar_basis" />
    <column name="AcctVal"     basis="run_curr_basis" />
    <column name="CSVNet"      basis="run_curr_basis" />
    <column name="EOYDeathBft" basis="run_curr_basis" />
</xsl:variable>
<xsl:variable name="basic_columns" select="document('')/xsl:stylesheet/xsl:variable[@name='basic_columns_xml']/column" />

<!-- Basic columns and columns from supplemental report -->
<xsl:variable name="all_columns" select="$basic_columns | $supplemental_columns" />

<!--
    Replace spaces by line breaks (<br /> in html) in a title.
    TODO ?? Is it really needed?
-->
<xsl:template name="replace_space_by_line_breaks">
    <xsl:param name="title" />
    <xsl:value-of select="$title" />
<!--
    <xsl:variable name="br"><br /></xsl:variable>
    <xsl:choose>
        <xsl:when test="contains($title, ' ')">
            <xsl:call-template name="replace_space_by_line_breaks">
                <xsl:with-param name="title" select="substring-before($title, ' ')" />
            </xsl:call-template>
            <br />
            <xsl:call-template name="replace_space_by_line_breaks">
                <xsl:with-param name="title" select="substring-after($title, ' ')" />
            </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$title" />
        </xsl:otherwise>
    </xsl:choose>
-->
</xsl:template>

<!-- Main template. -->
<xsl:template match="/illustration">

<html>
    <head>
        <title>Let me illustrate...</title>
    </head>
    <body>
        Calculation summary for<br />
        <xsl:choose>
            <xsl:when test="double_scalar[@name='Composite']='1'">
                composite<br />
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="string_scalar[@name='Insured1']" /><br />

                <xsl:value-of select="string_scalar[@name='Gender']" />,
                <xsl:value-of select="string_scalar[@name='Smoker']" />,
                age <xsl:value-of select="double_scalar[@name='Age']" />
                <br />

                <xsl:if test="double_scalar[@name='IsSubjectToIllustrationReg']='1'">
                    <xsl:value-of select="double_scalar[@name='GuarPrem']" />
                    guaranteed premium
                    <br />
                </xsl:if>
                <br />

                <xsl:value-of select="double_scalar[@name='InitGLP']" />
                initial guideline level premium
                <br />

                <xsl:value-of select="double_scalar[@name='InitGSP']" />
                initial guideline single premium
                <br />

                <xsl:value-of select="double_scalar[@name='InitSevenPayPrem']" />
                initial seven-pay premium
                <br />

                <xsl:choose>
                    <xsl:when test="double_scalar[@name='IsMec']='1'">
                        MEC
                    </xsl:when>
                    <xsl:otherwise>
                        Non-MEC
                    </xsl:otherwise>
                </xsl:choose>
                <br /><br />

                <xsl:value-of select="double_scalar[@name='InitTgtPrem']" />
                initial target premium
                <br />

                <xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']" />
                initial base specified amount
                <br />

                <xsl:value-of select="double_scalar[@name='InitTermSpecAmt']" />
                initial term specified amount
                <br />

                <xsl:value-of select="double_scalar[@name='InitTotalSA']" />
                initial total specified amount
                <br />

                <xsl:value-of select="string_scalar[@name='StatePostalAbbrev']" />
                state of jurisdiction
                <br />
            </xsl:otherwise>
        </xsl:choose>

        <hr />

        <table border="1" cellpadding="4" cellspacing="0" width="100%">

        <!-- Call 'data_table' template defined in 'common.xsl'. -->
        <xsl:call-template name="data_table">
            <xsl:with-param name="pos" select="1" />
            <xsl:with-param name="columns" select="$all_columns" />
            <xsl:with-param name="headers" select="$empty_nodeset" />
            <xsl:with-param name="vectors" select="$empty_nodeset" />
        </xsl:call-template>

        </table>
    </body>
</html>

</xsl:template>

<!--
    A user-defined template that will be called from 'data_table.
    - $headers - nodeset of columns to generate column titles.
    - $vectors - nodeset of column data vectors.

    Note: the order of nodes in $headers and $vectors is not the same.
    An extra-care should be taken to maintain the same order in table
    headers and rows.
-->
<xsl:template name="do_data_table">
    <xsl:param name="headers" />
    <xsl:param name="vectors" />

    <xsl:variable name="width" select="format-number(100. div (count($headers) + 1), '###.##')" />
    <tr align="right">
        <th width="{$width}%">Age</th>
    <xsl:for-each select="$headers">
        <th width="{$width}%">
        <xsl:choose>
            <!-- a spacer -->
            <xsl:when test="not(@name)">
            <!-- leave the cell empty for a spacer column -->
            </xsl:when>
            <!-- a normal column -->
            <xsl:otherwise>
                <!-- xsl:value-of select="@name" / -->
                <xsl:variable name="tmp">
                    <xsl:call-template name="title">
                        <xsl:with-param name="name" select="@name" />
                        <xsl:with-param name="basis" select="@basis" />
                        <xsl:with-param name="column" select="." />
                    </xsl:call-template>
                </xsl:variable>
                <xsl:call-template name="replace_space_by_line_breaks">
                    <xsl:with-param name="title" select="normalize-space($tmp)" />
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
        </th>
    </xsl:for-each>
    </tr>

    <xsl:variable name="age" select="number(double_scalar[@name='Age'])" />
    <!--
        We know that all the columns have the same length. Let's pick one for iteration.
    -->
    <xsl:for-each select="$vectors[1]/duration">
        <xsl:variable name="position" select="position()" />
        <tr align="right">
            <td>
                <xsl:value-of select="$age + $position - 1" />
            </td>
            <xsl:for-each select="$headers">
                <xsl:variable name="name" select="./@name" />
                <xsl:variable name="basis" select="./@basis" />
                <td>
                    <xsl:choose>
                        <xsl:when test="not($name)">
                        <!-- leave the cell empty for a spacer column -->
                        </xsl:when>
                        <xsl:when test="not($basis)">
                            <xsl:value-of select="$vectors[@name=$name]/duration[$position]/text()" />
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$vectors[@name=$name][@basis=$basis]/duration[$position]/text()" />
                        </xsl:otherwise>
                    </xsl:choose>
                </td>
            </xsl:for-each>
        </tr>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
