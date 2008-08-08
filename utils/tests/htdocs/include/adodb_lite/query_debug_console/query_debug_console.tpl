{if $debug_console == true }
<SCRIPT language=javascript>
	_query_debug_console = window.open("","Query Debug","width=750,height=600,resizable,scrollbars=yes");
	_query_debug_console.document.write("<HTML><TITLE>Query Debug Console</TITLE><BODY onload='self.focus();' bgcolor=#ffffff>");
	_query_debug_console.document.write("<table border=0 width=100%>");
	_query_debug_console.document.write("<tr bgcolor=#cccccc><th colspan=2>Query Debug Console</th></tr>");
	_query_debug_console.document.write("<tr bgcolor=#cccccc><td colspan=2><b>Included page queries (load time in seconds):</b></td></tr>");
	{foreach key=key value=sql from=$query_list}
		_query_debug_console.document.write("<tr bgcolor={if $key % 2}#eeeeee{else}#fafafa{/if}>");
		_query_debug_console.document.write('<td width="75%">{$query_list[$key]|strip|addslashes }<hr><font color=\"red\"><b>{$query_list_errors[$key]|strip|addslashes}</b></font></td>');
		_query_debug_console.document.write("<td width=\"25%\"><font color=\"red\"><b><i>({$query_list_time[$key]|string_format:"%.5f"} seconds)</i></b></font></td></tr>");
	{/foreach}
	_query_debug_console.document.write("</table>");
	_query_debug_console.document.write("</BODY></HTML>");
	_query_debug_console.document.close();
</SCRIPT>
{/if}