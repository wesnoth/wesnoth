{strip}
<table class="build_history">
<tr><td>
{if $next_id}
<div class="paginate left">
<a href="show_build.php?build={$next_id}">Next</a>
</div>
{/if}
{if $previous_id}
<div class="paginate right">
<a href="show_build.php?build={$previous_id}">Previous</a>
</div>
{/if}
{if $show_header}
<h3>Build report</h3>
{/if}
<table class="build" border="1">
<tr>
<th>Time</th>
<th>Revision</th>
<th>Error message</th>
<th>Passed/Total</th>
</tr>
{foreach from=$builds item=build}
<tr>
<td class="time">
{if $hide_build_link != true}
	<a href="show_build.php?build={$build.id}">
{/if}
{$build.time|date_format:"%H:%M %b %e, %Y"}
{if $hide_build_link != true}
	</a>
{/if}
</td>
<td class="revision {$build.style}">r{$build.svn_rev}</td>
<td class="message {$build.style}">{$build.result|autohide:20:true}</td>
<td class="testresult {$build.result_style}">{$build.result_passed}/{$build.result_passed+$build.result_failed}</td>
</tr>
{/foreach}
</table>
</td></tr></table>
{/strip}
