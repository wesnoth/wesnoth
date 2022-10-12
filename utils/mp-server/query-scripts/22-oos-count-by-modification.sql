select content.ID as MODIFICATION, count(*) as OOS_COUNT
from wesnothd_game_info game, wesnothd_game_content_info content
where game.INSTANCE_UUID = content.INSTANCE_UUID
	and game.GAME_ID = content.GAME_ID
	and content.TYPE = 'modification'
	and game.OOS = 1
	and YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by content.ID
order by count(*) desc
