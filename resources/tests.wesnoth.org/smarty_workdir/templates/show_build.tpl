<h3>Build report</h3>
{include file='build_list.tpl' hide_build_link=true}
<br/>
{if count($errors) eq 0}
<h3>All unit test passed! Congratulations!</h3>
{else}
<h3>Current list of errors in unit tests</h3>
<table class="test_error" border="1">
<tr>
<th>Type</th>
<th>First broken</th>
<th>File</th>
<th>Line</th>
<th>Message</th>
</tr>
{foreach from=$errors item=err}
<tr>
<td class="{$err.error_type}">{$err.error_type}</td>
<td class="{$err.error_type}">r{$err.start_version} - r{$err.end_version}</td>
<td class="{$err.error_type}">{$err.file|autohide:25:false:true}</td>
<td class="{$err.error_type}">{$err.line}</td>
<td class="{$err.error_type}">{$err.error_msg|autohide:40:true}</td>
</tr>
{/foreach}
{/if}
</table>
