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
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA

    http://savannah.nongnu.org/projects/lmi
    email: <chicares@cox.net>
    snail: Chicares, 186 Belle Woods Drive, Glastonbury CT 06033, USA

    $Id: html.xsl,v 1.1.2.18 2006-11-10 17:57:02 rericksberg Exp $

    Uses format.xml - column titles, number-formatting and other information.
-->
<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xsi:schemaLocation="http://savannah.nongnu.org/projects/lmi schema.xsd">

<xsl:output method="html" encoding="iso-8859-1" indent="yes"/>

<xsl:include href="common.xsl"/>

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
<!-- Disable column list in here in favour of 'configurable_settings.xml' -->
<!--
    <column name="Outlay"/>
    <column name="AcctVal" basis="run_guar_basis"/>
    <column name="CSVNet" basis="run_guar_basis"/>
    <column name="EOYDeathBft" basis="run_guar_basis"/>
    <column name="AcctVal" basis="run_curr_basis"/>
    <column name="CSVNet" basis="run_curr_basis"/>
    <column name="EOYDeathBft" basis="run_curr_basis"/>
-->
</xsl:variable>
<xsl:variable name="basic_columns" select="document('')/xsl:stylesheet/xsl:variable[@name='basic_columns_xml']/column"/>

<!-- Basic columns and columns from supplemental report -->
<xsl:variable name="all_columns" select="$basic_columns | $calculation_summary_columns"/>

<!-- Main template. -->
<xsl:template match="/illustration">

<html>
    <head>
        <title>Let me illustrate...</title>
    </head>
    <body>
        Calculation summary for:
        <xsl:choose>
            <xsl:when test="double_scalar[@name='Composite']='1'">
                composite
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="string_scalar[@name='Insured1']"/><br/>
                <table border="0" cellpadding="0" cellspacing="0">
                <tr>
                <td valign="top" align="left">
                <table valign="top" cellpadding="0" cellspacing="0">
                <!-- Here goes all the scalars for the LEFT bucket -->
                    <tr><td colspan="2" align="left" nowrap="1"><xsl:value-of select="string_scalar[@name='Gender']"/>, <xsl:value-of select="string_scalar[@name='Smoker']"/>, age <xsl:value-of select="double_scalar[@name='Age']"/></td></tr>
                    <tr><td colspan="2" align="left" nowrap="1"><xsl:value-of select="string_scalar[@name='StatePostalAbbrev']"/> state of jurisdiction</td></tr>
                    <tr><td colspan="2" align="left" nowrap="1">
                    <xsl:choose>
                        <xsl:when test="double_scalar[@name='IsMec']='1'">
                            MEC
                        </xsl:when>
                        <xsl:otherwise>
                            Non-MEC
                        </xsl:otherwise>
                    </xsl:choose>
                    </td></tr>
                    <tr><td><br/></td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']"/></td>
                        <td align="left" nowrap="1">initial base specified amount</td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitTermSpecAmt']"/></td>
                        <td align="left" nowrap="1">initial term specified amount</td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitTotalSA']"/></td>
                        <td align="left" nowrap="1">initial total specified amount</td></tr>
                <!-- End: Here goes all the scalars for the LEFT bucket -->
                </table>
                </td>
                <td width="5%"><br/></td>
                <td valign="top" align="left">
                <table valign="top" border="0" cellpadding="0" cellspacing="0">
                <!-- Here goes all the scalars for the RIGHT bucket -->
                    <tr><td align="right" nowrap="1"><br/></td>
                        <td align="left" nowrap="1"><br/></td></tr>
                    <tr><td align="right" nowrap="1"><br/></td>
                        <td align="left" nowrap="1"><br/></td></tr>
                    <xsl:if test="double_scalar[@name='IsSubjectToIllustrationReg']='1'">
                        <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='GuarPrem']"/></td>
                            <td align="left" nowrap="1">guaranteed premium</td></tr>
                    </xsl:if>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitGLP']"/></td>
                        <td align="left" nowrap="1">initial guideline level premium</td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitGSP']"/></td>
                        <td align="left" nowrap="1">initial guideline single premium</td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitSevenPayPrem']"/></td>
                        <td align="left" nowrap="1">initial seven-pay premium</td></tr>
                    <tr><td align="right" nowrap="1"><xsl:value-of select="double_scalar[@name='InitTgtPrem']"/></td>
                        <td align="left" nowrap="1">initial target premium</td></tr>
                <!-- End: Here goes all the scalars for the RIGHT bucket -->
                </table>
                </td>
                </tr>
                </table>
            </xsl:otherwise>
        </xsl:choose>

        <hr/>

        <table border="0" cellpadding="2" cellspacing="0" width="100%">

        <!-- Call 'data_table' template defined in 'common.xsl'. -->
        <xsl:call-template name="data_table">
            <xsl:with-param name="pos" select="1"/>
            <xsl:with-param name="columns" select="$all_columns"/>
            <xsl:with-param name="headers" select="$empty_nodeset"/>
            <xsl:with-param name="vectors" select="$empty_nodeset"/>
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
    <xsl:param name="headers"/>
    <xsl:param name="vectors"/>

    <xsl:variable name="width" select="format-number(100. div (count($headers) + 1), '###.##')"/>
    <tr align="right">
        <th width="{$width}%" valign="top">Age</th>
    <xsl:for-each select="$headers">
        <th width="{$width}%" valign="top">
        <xsl:choose>
            <!-- a spacer -->
            <xsl:when test="not(@name)">
            <!-- leave the cell empty for a spacer column -->
            </xsl:when>
            <!-- a normal column -->
            <xsl:otherwise>
                <xsl:call-template name="title">
                    <xsl:with-param name="name" select="@name"/>
                    <xsl:with-param name="basis" select="@basis"/>
                    <xsl:with-param name="column" select="."/>
                </xsl:call-template>
            </xsl:otherwise>
        </xsl:choose>
        </th>
    </xsl:for-each>
    </tr>

    <xsl:variable name="age" select="number(double_scalar[@name='Age'])"/>
    <!--
        We know that all the columns have the same length. Let's pick one for iteration.
    -->
    <xsl:variable name="cols_total" select="count($vectors)"/>
    <xsl:variable name="rows_total" select="count($vectors[1]/duration)"/>
    <xsl:for-each select="$vectors[1]/duration">
        <xsl:variable name="position" select="position()"/>
        <tr align="right">
            <td nowrap="1">
                <xsl:value-of select="$age + $position - 1"/>
            </td>
            <xsl:for-each select="$headers">
                <xsl:variable name="name" select="@name"/>
                <xsl:variable name="basis" select="@basis"/>
                <td nowrap="1">
                    <xsl:choose>
                        <xsl:when test="not($name)">
                        <!-- leave the cell empty for a spacer column -->
                        </xsl:when>
                        <xsl:when test="not($basis)">
                            <xsl:value-of select="$vectors[@name=$name]/duration[$position]/text()"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:value-of select="$vectors[@name=$name][@basis=$basis]/duration[$position]/text()"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </td>
            </xsl:for-each>
        </tr>
        <xsl:if test="$position mod 5 = 0 and not($position = $rows_total)">
            <tr>
                <td colspan="{$cols_total}"><br/></td>
            </tr>
        </xsl:if>
    </xsl:for-each>
</xsl:template>

</xsl:stylesheet>
