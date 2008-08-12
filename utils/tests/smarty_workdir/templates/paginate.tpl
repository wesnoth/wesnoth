{strip}
<div class="paginate">
{if $paginate.visible > 1}
	{if $paginate.page > 1}
		<a href="{$paginate.link}{$paginate.page-1}">
	{else}
		<span class="active">
	{/if}
		Previous
	{if $paginate.page > 1}
		</a>
	{else}
		</span>
	{/if}
&nbsp;
	{if $paginate.first_page > 1}
		<a href="{$paginate.link}1">1</a>
		{if $paginate.first_page > 2}
			...
		{else}
			,&nbsp;
		{/if}
	{/if}

	{section name=pager loop=$paginate.last_page start=$paginate.first_page max=$paginate.visible}
		<a href="{$paginate.link}{$smarty.section.pager.index}"
		{if $paginate.page == $smarty.section.pager.index}
			class="active"
		{/if}
		>{$smarty.section.pager.index}</a>
		{if $smarty.section.pager.last == false}
			,&nbsp;
		{/if}
	{/section}

	{if $paginate.last_page <= $paginate.number_of_pages}
		{if $paginate.last_page <= $paginate.number_of_pages - 1}
			...
		{else}
			,&nbsp;
		{/if}
		<a href="{$paginate.link}{$paginate.number_of_pages}">{$paginate.number_of_pages}</a>
	{/if}
	&nbsp;
	{if $paginate.page < $paginate.number_of_pages}
		<a href="{$paginate.link}{$paginate.page+1}">
	{else}
		<span class="active">
	{/if}
	next
	{if $paginate.page < $paginate.number_of_pages}
		</a>
	{else}
		</span>
	{/if}
{/if}
</div>
{/strip}
