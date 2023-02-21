select IFNULL(content.ID,'No Modifications') as MODIFICATION, IFNULL(content.ADDON_ID,'') as ADDON, IFNULL(content.ADDON_VERSION,'') as VERSION, count(*) as MODIFICATION_COUNT
from wesnothd_game_info game
left join wesnothd_game_content_info content
   on game.INSTANCE_UUID = content.INSTANCE_UUID
  and game.GAME_ID = content.GAME_ID
  and content.TYPE = 'modification'
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by content.ID, content.ADDON_ID, content.ADDON_VERSION
order by count(*) desc
