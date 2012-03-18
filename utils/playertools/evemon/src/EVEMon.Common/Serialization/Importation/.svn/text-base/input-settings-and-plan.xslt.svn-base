<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:msxsl="urn:schemas-microsoft-com:xslt" exclude-result-prefixes="msxsl"
>
    <xsl:output method="xml" indent="yes"/>

  <!-- Root -->
  <xsl:template match="logindata2">
    <xsl:element name="OldSettings">
      <xsl:element name="accounts">
        <xsl:for-each select="Accounts/AccountDetails">
          <xsl:call-template name="account"/>
        </xsl:for-each>
      </xsl:element>
      <xsl:element name="characters">
        <xsl:for-each select="CachedCharacterInfo/SerializableCharacterSheet/result">
          <xsl:call-template name="character"/>
        </xsl:for-each>
      </xsl:element>
      <xsl:element name="plans">
        <xsl:for-each select="Plans/PairOfStringPlan">
          <xsl:call-template name="plan"/>
        </xsl:for-each>
      </xsl:element>
    </xsl:element>
  </xsl:template>

  <!-- Account -->
  <xsl:template name="account">
    <xsl:element name="account">
      <xsl:attribute name="id">
        <xsl:value-of select="UserId"/>
      </xsl:attribute>
      <xsl:attribute name="key">
        <xsl:value-of select="ApiKey"/>
      </xsl:attribute>
    </xsl:element>
  </xsl:template>

  <!-- Character -->
  <xsl:template name="character">
    <xsl:element name="character">
      <xsl:attribute name="id">
        <xsl:value-of select="characterID"/>
      </xsl:attribute>
      <xsl:attribute name="name">
        <xsl:value-of select="name"/>
      </xsl:attribute>
    </xsl:element>
  </xsl:template>

  <!-- Plan -->
  <xsl:template name="plan">
    <xsl:element name="plan">
      <xsl:attribute name="character">
        <xsl:value-of select="substring-before(A, '::')"/>
      </xsl:attribute>
      <xsl:attribute name="name">
        <xsl:value-of select="substring-after(A, '::')"/>
      </xsl:attribute>
      <!-- Entries -->
      <xsl:for-each select="B/Entries/entry">
        <xsl:call-template name="PlanEntry"/>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

  <!-- Plan entry -->
  <xsl:template name="PlanEntry" match="Entry">
    <xsl:element name="entry">
      <xsl:attribute name="skill">
        <xsl:value-of select="SkillName"/>
      </xsl:attribute>
      <xsl:attribute name="level">
        <xsl:value-of select="Level"/>
      </xsl:attribute>
      <xsl:if test="Priority">
        <xsl:attribute name="priority">
          <xsl:value-of select="Priority"/>
        </xsl:attribute>
      </xsl:if>
      <xsl:attribute name="type">
        <xsl:value-of select="EntryType"/>
      </xsl:attribute>
      <xsl:element name="notes">
        <xsl:value-of select="Notes"/>
      </xsl:element>
      <xsl:for-each select="PlanGroups/anyType">
        <xsl:element name="group">
          <xsl:value-of select="."/>
        </xsl:element>
      </xsl:for-each>
      <!-- Remapping point if any -->
      <xsl:if test="Remapping">
        <xsl:element name="remapping">
          <xsl:attribute name="status">
            <xsl:value-of select="Remapping/Status"/>
          </xsl:attribute>
          <xsl:attribute name="per">
            <xsl:value-of select="Remapping/BaseAttributes/int[1]"/>
          </xsl:attribute>
          <xsl:attribute name="mem">
            <xsl:value-of select="Remapping/BaseAttributes/int[2]"/>
          </xsl:attribute>
          <xsl:attribute name="wil">
            <xsl:value-of select="Remapping/BaseAttributes/int[3]"/>
          </xsl:attribute>
          <xsl:attribute name="int">
            <xsl:value-of select="Remapping/BaseAttributes/int[4]"/>
          </xsl:attribute>
          <xsl:attribute name="cha">
            <xsl:value-of select="Remapping/BaseAttributes/int[5]"/>
          </xsl:attribute>
        </xsl:element>
      </xsl:if>
    </xsl:element>
  </xsl:template>

  <!-- Plan root (used for plan importation, not by settings) -->
  <xsl:template name="PlanRoot" match="plan">
    <xsl:element name="SerializablePlan">
      <!-- Entries -->
      <xsl:for-each select="Entries/entry">
        <xsl:call-template name="PlanEntry"/>
      </xsl:for-each>
    </xsl:element>
  </xsl:template>

</xsl:stylesheet>
