<div class="paginate">
{if $paginate.page > 1}
<a href="{$paginate.link}{$paginate.page-1}">Previous</a>
{/if}
{if $paginate.first_page > 1}
<a href="{$paginate.link}1">1</a>
{/if}

{section name=pager loop=$paginate.last_page start=$paginate.first_page max=$paginate.visible}
<a href="{$paginate.link}{$smarty.section.pager.index}">{$smarty.section.pager.index}</a>
{if $smarty.section.pager.last == false}
,
{/if}
{/section}

{if $paginate.last_page < $paginate.number_of_pages}
<a href="{$paginate.link}{$paginate.number_of_pages}">{$paginate.number_of_pages}</a>
{/if}
{if $paginate.page < $paginate.number_of_pages}
<a href="{$paginate.link}{$paginate.page+1}">next</a>
{/if}
</div>
