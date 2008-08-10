<h3>The status from the latest build</h3>
<table class="build" border="1">
<tr>
<th>Time</th>
<th>Revision</th>
<th>Error message</th>
<th>Passed/Total</th>
</tr><tr>
<td class="time">{$build.time|date_format:"%H:%M %b %e, %Y"}</td>
<td class="revision {$build.style}">r{$build.svn_rev}</td>
<td class="message {$build.style}">{$build.result|autohide:20:true}</td>
<td class="testresult {$build.result_style}">{$build.result_passed}/{$build.result_passed+$build.result_failed}</td>
</tr>
</table>
<br/>
<h3>Current list of errors in unit tests</h3>
<table class="test_error" border="1">
<tr>
<th>Type</th>
<th>Broken in version</th>
<th>File</th>
<th>Line</th>
<th>Message</th>
</tr>
{foreach from=$build.errors item=err}
<tr>
<td class="{$err.error_type}">{$err.error_type}</td>
<td class="{$err.error_type}">r{$err.start_version} - r{$err.end_version}</td>
<td class="{$err.error_type}">{$err.file|autohide:25:false:true}</td>
<td class="{$err.error_type}">{$err.line}</td>
<td class="{$err.error_type}">{$err.error_msg|autohide:40:true}</td>
</tr>
{/foreach}
</table>
