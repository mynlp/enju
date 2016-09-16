<?xml version="1.0" encoding="euc-jp"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fs="http://www-tsujii.is.s.u-tokyo.ac.jp/lilfes"
                xmlns="http://www.w3.org/1999/xhtml"
                xmlns:html="http://www.w3.org/1999/xhtml"
                version="1.0">

<!--
######################################################################
Copyright (c) 2005, MIYAO Yusuke
You may distribute this file under the terms of the Artistic License.

Name: fs.xsl
Author: MIYAO Yusuke (yusuke@is.s.u-tokyo.ac.jp)
Stylesheet for browsing feature structures and trees
######################################################################
-->

<!--
############################################################
##
##  Conversion of feature structures
##
############################################################
-->

<xsl:template match="fs:fs">
  <xsl:choose>
  <!-- empty (maybe shared node) -->
  <xsl:when test="count(*)=0">
    <xsl:call-template name="shared_id">
      <xsl:with-param name="shared_id"><xsl:value-of select="./@shared_id" /></xsl:with-param>
      <xsl:with-param name="is_empty">1</xsl:with-param>
    </xsl:call-template>
  </xsl:when>
  <!-- atom -->
  <xsl:when test="count(*)=1">
    <xsl:call-template name="shared_id">
      <xsl:with-param name="shared_id"><xsl:value-of select="./@shared_id" /></xsl:with-param>
    </xsl:call-template>
    <xsl:apply-templates />  
  </xsl:when>
  <!-- list -->
  <xsl:when test="count(*)=3 and fs:type[position()=1]/text()='cons' and
                  count(fs:feat[@edge='hd'])=1 and count(fs:feat[@edge='tl'])=1">
    <table class="list">
      <tr>
        <td>
          <xsl:call-template name="shared_id">
            <xsl:with-param name="shared_id"><xsl:value-of select="./@shared_id" /></xsl:with-param>
          </xsl:call-template>
          &lt;
        </td>
        <xsl:apply-templates select="." mode="list" />
        <td>&gt;</td>
      </tr>
    </table>
  </xsl:when>
  <!-- complex feature structures -->
  <xsl:otherwise>
    <xsl:variable name="num_dtrs" select="count(*)" />
    <xsl:variable name="shared_id"><xsl:value-of select="./@shared_id" /></xsl:variable>
    <table class="fs">
      <xsl:for-each select="*">
        <tr>
          <xsl:if test="position()=1">
            <xsl:if test="$shared_id != ''">
              <!-- output shared ID if necessary -->
              <td>
                <xsl:attribute name="rowspan">
                  <xsl:value-of select="$num_dtrs"/>
                </xsl:attribute>
                <xsl:call-template name="shared_id">
                  <xsl:with-param name="shared_id"><xsl:value-of select="$shared_id" /></xsl:with-param>
                </xsl:call-template>
              </td>
            </xsl:if>
            <!-- left parenthesis -->
            <td class="lprn">
              <xsl:attribute name="rowspan">
                <xsl:value-of select="$num_dtrs"/>
              </xsl:attribute>
            </td>
          </xsl:if>
          <!-- contents -->
          <xsl:apply-templates select="." mode="fs" />
          <xsl:if test="position()=1">
            <!-- right parenthesis -->
            <td class="rprn">
              <xsl:attribute name="rowspan">
                <xsl:value-of select="$num_dtrs"/>
              </xsl:attribute>
            </td>
          </xsl:if>
        </tr>
      </xsl:for-each>
    </table>
  </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="shared_id">
  <xsl:param name="shared_id"></xsl:param>
  <xsl:param name="is_empty">0</xsl:param>
  <xsl:if test="$shared_id != ''">
    <xsl:choose>
    <!-- empty node (make a link) -->
    <xsl:when test="$is_empty=1">
      <span class="shared_id">
        <a>
          <xsl:attribute name="href">#shared_id<xsl:value-of select="$shared_id" /></xsl:attribute>
          <xsl:value-of select="$shared_id" />
        </a>
      </span>
    </xsl:when>
    <!-- non-empty node (make a name) -->
    <xsl:otherwise>
      <span class="shared_id">
        <a>
          <xsl:attribute name="name">shared_id<xsl:value-of select="$shared_id" /></xsl:attribute>
          <xsl:value-of select="$shared_id" />
        </a>
      </span>
    </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
</xsl:template>

<xsl:template match="fs:fs" mode="list">
  <!-- head of list -->
  <xsl:for-each select="fs:feat[@edge='hd']">
    <td><xsl:apply-templates select="*" /></td>
  </xsl:for-each>
  <!-- tail of list -->
  <xsl:for-each select="fs:feat[@edge='tl']">
    <xsl:choose>
    <!-- tail is shared: print as ordinary feature structures -->
    <xsl:when test="fs:fs/@shared_id!=''">
      <td>|<xsl:apply-templates select="." /></td>
    </xsl:when>
    <!-- nil: do nothing -->
    <xsl:when test="count(*)=1 and fs:type/text()='nil'" />
    <!-- list elements -->
    <xsl:when test="count(fs:fs/*)=3 and fs:fs/fs:type[position()=1]/text()='cons' and
		    count(fs:fs/fs:feat[@edge='hd'])=1 and
		    count(fs:fs/fs:feat[@edge='tl'])=1">
      <td>,</td>
      <td><xsl:apply-templates select="*" mode="list" /></td>
    </xsl:when>
    <!-- tail is not specified -->
    <xsl:otherwise>
      <td>|</td>
      <td><xsl:apply-templates select="." /></td>
    </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>

<xsl:template match="fs:feat" mode="fs">
  <td>
  <span class="edge_fs"><xsl:value-of select="@edge" /></span>
  </td>
  <td>
    <xsl:apply-templates />
  </td>
</xsl:template>

<xsl:template match="*" mode="fs">
  <td colspan="2">
    <xsl:apply-templates select="." />
  </td>
</xsl:template>

<xsl:template match="fs:type">
  <span class="type">
    <a onclick="showhide(this)">
    <xsl:apply-templates />
    </a>
  </span>
</xsl:template>

<xsl:template match="fs:decimal">
  <span class="decimal">
    <xsl:apply-templates />
  </span>
</xsl:template>

<xsl:template match="fs:float">
  <span class="float">
    <xsl:apply-templates />
  </span>
</xsl:template>

<xsl:template match="fs:doublequoted">
  <span class="doublequoted">
    <xsl:apply-templates />
  </span>
</xsl:template>

<xsl:template match="fs:singlequoted">
  <span class="singlequoted">
    <xsl:apply-templates />
  </span>
</xsl:template>

<xsl:template match="html:*">
  <xsl:copy>
    <xsl:copy-of select="@*"/> 
    <xsl:apply-templates />
  </xsl:copy>
</xsl:template>

<xsl:template match="html:head">
  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates />
<script type="text/javascript">
  function showhide (obj) {
  var tr = obj.parentNode.parentNode.parentNode.nextSibling;
  var td = obj.parentNode.parentNode;
  if (td.getAttribute("colspan") == 2) {
    while (tr != null) {
      if (tr.style.display == "none") {
        tr.style.display = "";
      } else {
        tr.style.display = "none";
      }
      tr = tr.nextSibling;
    }
  }
}
</script>

  </xsl:copy>
</xsl:template>

<!--
############################################################
##
##  Conversion of tree structures
##
############################################################
-->

<!-- Root node of a tree -->
<xsl:template match="fs:tree">
  <table class="tree">
    <xsl:call-template name="draw_tree">
      <xsl:with-param name="line1">space</xsl:with-param>
      <xsl:with-param name="line2">space</xsl:with-param>
    </xsl:call-template>
  </table>
</xsl:template>

<!-- draw tree node -->
<xsl:template name="draw_tree_node">
  <xsl:param name="num_dtrs">0</xsl:param>
  <xsl:param name="line1">space</xsl:param>
  <xsl:param name="line2">space</xsl:param>
  <xsl:param name="edge_label"></xsl:param>
  <!-- number of columns -->
  <xsl:variable name="num_cols">
    <xsl:choose>
      <xsl:when test="$num_dtrs = 0">2</xsl:when>
      <xsl:otherwise><xsl:value-of select="$num_dtrs" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <!-- write the tree node -->
  <tr>
    <td class="tree_space">
      <xsl:attribute name="colspan"><xsl:value-of select="$num_cols * 2" /></xsl:attribute>
      <table class="tree_node" style="width: 100%">
        <tr>
          <td class="tree_space" style="width: 50%">
            <xsl:attribute name="class"><xsl:value-of select="$line1" /></xsl:attribute>
          </td>
          <td class="tree_space" style="width: 50%">
            <xsl:attribute name="class"><xsl:value-of select="$line2" /></xsl:attribute>
            <span class="edge_tree"><xsl:value-of select="$edge_label" /></span>
          </td>
        </tr>
        <tr>
          <td colspan="2">
            <xsl:attribute name="class">
              <xsl:choose>
                <xsl:when test="$num_dtrs = 0">tree_term</xsl:when>
                <xsl:otherwise>tree_node</xsl:otherwise>
              </xsl:choose>
            </xsl:attribute>
            <div align="center">
              <xsl:apply-templates select="./text()|*[not(self::fs:dtr)]" />
            </div>
          </td>
        </tr>
        <xsl:if test="$num_dtrs > 1">
          <tr>
            <td style="width: 50%" class="space" />
            <td style="width: 50%" class="right" />
          </tr>
        </xsl:if>
      </table>
    </td>
  </tr>
</xsl:template>

<!-- draw daughter structure -->
<xsl:template name="draw_dtr">
  <xsl:param name="line1">space</xsl:param>
  <xsl:param name="line2">space</xsl:param>
  <xsl:variable name="edge_label" select="@edge" />
  <xsl:variable name="num_trees" select="count(fs:tree)" />
  <xsl:choose>
  <!-- no tree node -->
  <xsl:when test="$num_trees = 0">
    <xsl:call-template name="draw_tree_node">
      <xsl:with-param name="num_dtrs">0</xsl:with-param>
      <xsl:with-param name="line1"><xsl:value-of select="$line1" /></xsl:with-param>
      <xsl:with-param name="line2"><xsl:value-of select="$line2" /></xsl:with-param>
      <xsl:with-param name="edge_label"><xsl:value-of select="$edge_label" /></xsl:with-param>
    </xsl:call-template>
  </xsl:when>
  <!-- tree node -->
  <xsl:otherwise>
    <xsl:for-each select="fs:tree[1]">
      <xsl:call-template name="draw_tree">
        <xsl:with-param name="line1" select="$line1" />
        <xsl:with-param name="line2" select="$line2" />
        <xsl:with-param name="edge_label" select="$edge_label" />
      </xsl:call-template>
    </xsl:for-each>
  </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- draw tree -->
<xsl:template name="draw_tree">
  <xsl:param name="line1">space</xsl:param>
  <xsl:param name="line2">space</xsl:param>
  <xsl:param name="edge_label"></xsl:param>
  <xsl:variable name="num_dtrs" select="count(fs:dtr)" />
  <xsl:choose>

  <!-- no daughter nodes -->
  <xsl:when test="$num_dtrs = 0">
    <xsl:call-template name="draw_tree_node">
      <xsl:with-param name="num_dtrs">0</xsl:with-param>
      <xsl:with-param name="line1"><xsl:value-of select="$line1" /></xsl:with-param>
      <xsl:with-param name="line2"><xsl:value-of select="$line2" /></xsl:with-param>
      <xsl:with-param name="edge_label"><xsl:value-of select="$edge_label" /></xsl:with-param>
    </xsl:call-template>
  </xsl:when>

  <!-- unary tree -->
  <xsl:when test="$num_dtrs = 1">
    <xsl:call-template name="draw_tree_node">
      <xsl:with-param name="num_dtrs">
        <xsl:call-template name="count_daughters">
          <xsl:with-param name="target" select="." />
        </xsl:call-template>
      </xsl:with-param>
      <xsl:with-param name="line1"><xsl:value-of select="$line1" /></xsl:with-param>
      <xsl:with-param name="line2"><xsl:value-of select="$line2" /></xsl:with-param>
      <xsl:with-param name="edge_label"><xsl:value-of select="$edge_label" /></xsl:with-param>
    </xsl:call-template>
    <xsl:for-each select="fs:dtr">
      <xsl:call-template name="draw_dtr">
        <xsl:with-param name="line1">space</xsl:with-param>
        <xsl:with-param name="line2">right</xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
  </xsl:when>

  <!-- n-ary (n>1) tree -->
  <xsl:otherwise>
    <xsl:call-template name="draw_tree_node">
      <xsl:with-param name="num_dtrs"><xsl:value-of select="$num_dtrs" /></xsl:with-param>
      <xsl:with-param name="line1"><xsl:value-of select="$line1" /></xsl:with-param>
      <xsl:with-param name="line2"><xsl:value-of select="$line2" /></xsl:with-param>
      <xsl:with-param name="edge_label"><xsl:value-of select="$edge_label" /></xsl:with-param>
    </xsl:call-template>
    <tr>
      <xsl:for-each select="fs:dtr">
        <xsl:variable name="dtr_id">
          <xsl:number level="single" count="fs:dtr" format="1" />
        </xsl:variable>
        <td class="dtr" colspan="2">
          <table class="tree">
            <xsl:choose>
            <xsl:when test="$dtr_id = 1">
              <xsl:call-template name="draw_dtr">
                <xsl:with-param name="line1">space</xsl:with-param>
                <xsl:with-param name="line2">left</xsl:with-param>
              </xsl:call-template>
            </xsl:when>
            <xsl:when test="$dtr_id = $num_dtrs">
              <xsl:call-template name="draw_dtr">
                <xsl:with-param name="line1">top</xsl:with-param>
                <xsl:with-param name="line2">right</xsl:with-param>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
              <xsl:call-template name="draw_dtr">
                <xsl:with-param name="line1">top</xsl:with-param>
                <xsl:with-param name="line2">left</xsl:with-param>
              </xsl:call-template>
            </xsl:otherwise>
            </xsl:choose>
          </table>
        </td>
      </xsl:for-each>
    </tr>
  </xsl:otherwise>

  </xsl:choose>
</xsl:template>

<!-- count the number of non-unary daughters -->
<xsl:template name="count_daughters">
  <xsl:param name="target">.</xsl:param>
  <xsl:variable name="num" select="count($target/fs:dtr)" />
  <xsl:choose>
  <xsl:when test="$num = 0">
    1
  </xsl:when>
  <xsl:when test="$num = 1">
    <xsl:call-template name="count_daughters">
      <xsl:with-param name="target" select="$target/fs:dtr/fs:tree" />
    </xsl:call-template>
  </xsl:when>
  <xsl:otherwise>
    <xsl:value-of select="$num" />
  </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
