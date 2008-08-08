<table class="build_{$build_result}" border="1">
<tr>
<th>The latest unit test build</th>
<th>{$build_time|date_format:"%H:%M %b %e, %Y"}</th>
<th>r{$build_svn_rev}</th>
<th>{$build_result}</th>
<th>{$result_passed}/{$result_passed+$result_failed}</th>
</tr>
</table>
<br/>
<br/>
<table class="test_error" border="1">
<tr>
<th>Type</th>
<th>First rev</th>
<th>Last rev</th>
<th>File</th>
<th>Line</th>
<th style="width:300">Message</th>
</tr>
{foreach from=$errors item=err}
<tr>
<td>{$err.error_type}</td>
<td>r{$err.start_version}</td>
<td>r{$err.end_version}</td>
<td>{$err.file}</td>
<td>{$err.line}</td>
<td>{$err.error_msg}</td>
</tr>
{/foreach}
</table>
