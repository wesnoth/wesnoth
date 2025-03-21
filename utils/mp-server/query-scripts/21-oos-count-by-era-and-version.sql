select content.ID as ERA, content.ADDON_ID as ADDON, content.ADDON_VERSION, count(*) as OOS_COUNT
from wesnothd_game_info game, wesnothd_game_content_info content
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
	and game.OOS = 1
	and game.INSTANCE_UUID = content.INSTANCE_UUID
	and game.GAME_ID = content.GAME_ID
	and content.TYPE = 'era'
group by content.ID, content.ADDON_ID, content.ADDON_VERSION
order by count(*) desc
