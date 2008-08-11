<table class="build" border="1">
<tr>
<th>Time</th>
<th>Revision</th>
<th>Error message</th>
<th>Passed/Total</th>
</tr>
{foreach from=$builds item=build}
<tr>
<td class="time">{$build.time|date_format:"%H:%M %b %e, %Y"}</td>
<td class="revision {$build.style}">r{$build.svn_rev}</td>
<td class="message {$build.style}">{$build.result|autohide:20:true}</td>
<td class="testresult {$build.result_style}">{$build.result_passed}/{$build.result_passed+$build.result_failed}</td>
</tr>
{/foreach}
</table>

