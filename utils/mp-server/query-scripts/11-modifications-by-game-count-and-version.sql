select IFNULL(modif.MODIFICATION_NAME,'No Modifications') as MODIFICATION_NAME, IFNULL(modif.SOURCE_ADDON,'') as SOURCE_ADDON, IFNULL(modif.VERSION,'') as VERSION, count(*) as MODIFICATION_COUNT
from wesnothd_game_info game
left join wesnothd_game_modification_info modif
   on game.INSTANCE_UUID = modif.INSTANCE_UUID
  and game.GAME_ID = modif.GAME_ID
where YEAR(game.START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(game.START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and game.END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, game.START_TIME, game.END_TIME) > 5
group by modif.MODIFICATION_NAME, modif.SOURCE_ADDON, modif.VERSION
order by count(*) desc
