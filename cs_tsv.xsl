<?xml version="1.0"?>
<!--
    Xsl template used to render illustration data taken from data.xml into csv.

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

    $Id$

    Uses format.xml - column titles, number-formatting and other information.
-->
<!DOCTYPE xsl:stylesheet [
<!ENTITY tab "&#x9;">
<!ENTITY nl "&#xA;">
]>
<xsl:stylesheet xmlns:lmi="http://savannah.nongnu.org/projects/lmi" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xsi:schemaLocation="http://savannah.nongnu.org/projects/lmi schema.xsd">

<xsl:include href="tsv_common.xsl"/>

<xsl:template match="/illustration">
    <xsl:text>&nl;</xsl:text>
    <xsl:text>&nl;</xsl:text>
    <xsl:text>FOR BROKER-DEALER USE ONLY. NOT TO BE SHARED WITH CLIENTS.&nl;</xsl:text>
    <xsl:text>&nl;</xsl:text>
    <xsl:text>Calculation summary for&nl;</xsl:text>
    <xsl:choose>
        <xsl:when test="double_scalar[@name='Composite']='1'">
            <xsl:text>composite&nl;</xsl:text>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="string_scalar[@name='Insured1']"/><xsl:text>&nl;</xsl:text>

            <xsl:value-of select="string_scalar[@name='Gender']"/><xsl:text>&tab;</xsl:text>
            <xsl:value-of select="string_scalar[@name='Smoker']"/><xsl:text>&tab;</xsl:text>
            <xsl:text>age </xsl:text>
            <xsl:value-of select="double_scalar[@name='Age']"/><xsl:text>&nl;</xsl:text>

            <xsl:if test="double_scalar[@name='IsSubjectToIllustrationReg']='1'">
                <xsl:value-of select="double_scalar[@name='GuarPrem']"/>
                <xsl:text> guaranteed premium&nl;</xsl:text>
            </xsl:if>
            <xsl:text>&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitGLP']"/>
            <xsl:text>&tab;initial guideline level premium&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitGSP']"/>
            <xsl:text>&tab;initial guideline single premium&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitSevenPayPrem']"/>
            <xsl:text>&tab;initial seven-pay premium&nl;</xsl:text>

            <xsl:choose>
                <xsl:when test="double_scalar[@name='IsMec']='1'">
                    <xsl:text>MEC</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>Non-MEC</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:text>&nl;&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitTgtPrem']"/>
            <xsl:text>&tab;initial target premium&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitBaseSpecAmt']"/>
            <xsl:text>&tab;initial base specified amount&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitTermSpecAmt']"/>
            <xsl:text>&tab;initial term specified amount&nl;</xsl:text>

            <xsl:value-of select="double_scalar[@name='InitTotalSA']"/>
            <xsl:text>&tab;initial total specified amount&nl;</xsl:text>

            <xsl:value-of select="string_scalar[@name='StatePostalAbbrev']"/>
            <xsl:text>&tab;state of jurisdiction&nl;</xsl:text>
        </xsl:otherwise>
    </xsl:choose>

    <xsl:text>&nl;</xsl:text>

    <xsl:call-template name="data_table">
        <xsl:with-param name="pos" select="1"/>
        <xsl:with-param name="columns" select="$calculation_summary_columns"/>
        <xsl:with-param name="headers" select="$empty_nodeset"/>
        <xsl:with-param name="vectors" select="$empty_nodeset"/>
    </xsl:call-template>
</xsl:template>

<!--
    Templates to be called from 'do_data_table' for every row in a table.
    The purpose is to generate some static columns in the table.
-->
<xsl:template name="do_data_table_pre_headers">
    <xsl:text>Policy Year&tab;</xsl:text>
</xsl:template>
<xsl:template name="do_data_table_pre_data">
    <xsl:param name="position"/>
    <xsl:value-of select="$policy_year/duration[$position]"/>
    <xsl:text>&tab;</xsl:text>
</xsl:template>

</xsl:stylesheet>
