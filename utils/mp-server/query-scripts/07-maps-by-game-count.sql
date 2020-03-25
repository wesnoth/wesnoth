select MAP_NAME, count(*) as MAP_COUNT
from wesnothd_game_info
where YEAR(START_TIME) = YEAR(CURRENT_DATE - INTERVAL 1 MONTH)
  and MONTH(START_TIME) = MONTH(CURRENT_DATE - INTERVAL 1 MONTH)
  and END_TIME is not NULL
  and TIMESTAMPDIFF(MINUTE, START_TIME, END_TIME) > 5
group by MAP_NAME
order by count(*) desc
