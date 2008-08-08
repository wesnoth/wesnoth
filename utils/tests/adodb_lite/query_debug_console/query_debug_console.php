<?php
function display_query_console(&$db)
{
	if ($db->debug_console == true):
?>
<SCRIPT language=javascript>
    _query_debug_console = window.open("","Query Debug","width=750,height=600,resizable,scrollbars=yes");
    _query_debug_console.document.write("<HTML><TITLE>Query Debug Console</TITLE><BODY onload='self.focus();' bgcolor=#ffffff>");
    _query_debug_console.document.write("<table border=0 width=100%>");
    _query_debug_console.document.write("<tr bgcolor=#cccccc><th colspan=2>Query Debug Console</th></tr>");
    _query_debug_console.document.write("<tr bgcolor=#cccccc><td colspan=2><b>Included page queries (load time in seconds):</b></td></tr>");
    <?php
    if (count((array)$db->query_list)): 
        foreach ((array)$db->query_list as $key => $sql):
    ?>
        _query_debug_console.document.write("<tr bgcolor="<?php if ($key % 2): ?>"#eeeeee"<?php else: ?>"#fafafa"<?php endif; ?>">");
        _query_debug_console.document.write("<td width=\"75%\">
        <?php echo addslashes(preg_replace('!\s+!'," ", $db->query_list[$key])); ?>
<hr><font color=\"red\"><b><?php echo addslashes(preg_replace('!\s+!'," ", $db->query_list_errors[$key])); ?>
</b></font></td>");
        _query_debug_console.document.write("<td width=\"25%\"><font color=\"red\"><b><i>(
        <?php echo sprintf("%.5f", $db->query_list_time[$key]); ?>
 seconds)</i></b></font></td></tr>");
    <?php
        endforeach; 
    endif; ?>
    _query_debug_console.document.write("</table>");
    _query_debug_console.document.write("</BODY></HTML>");
    _query_debug_console.document.close();
</SCRIPT>
<?php
	endif;
}
?>