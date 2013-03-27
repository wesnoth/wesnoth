<!--
   Copyright (C) 2009 by Gregory Shikhman <cornmander@cornmander.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
-->

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns:py="http://purl.org/kid/ns#">
<head>
<title>Wesnoth Stats - Win/Loss/Quit Breakdown</title>
</head>
<body>
<h2>Current filters:</h2>
<table border="1">
<tr><td>Campaigns</td></tr>
<tr py:for="camp in camps"><td>${camp}</td></tr>
</table>
<table border="1">
<tr><td>Scenarios</td></tr>
<tr py:for="scen in scens"><td>${scen}</td></tr>
</table>
<table border="1">
<tr><td>Difficulties</td></tr>
<tr py:for="diff in diffs"><td>${diff}</td></tr>
</table>
<table border="1">
<tr><td>Versions</td></tr>
<tr py:for="ver in vers"><td>${ver}</td></tr>
</table>
<p>${total} results for current filters.</p>
<img border="0" src="http://chart.apis.google.com/chart?cht=p3&amp;chd=t:${chd}&amp;chs=500x150&amp;chl=${chn}" />
<br />
<form method="get">
<select name="campaigns" multiple="multiple" size="5">
<option py:for="camp in clist">${camp[0]}</option>
</select>
<select name="scens" multiple="multiple" size="5">
<option py:for="scen in slist">${scen[0]}</option>
</select>
<select name="versions" multiple="multiple" size="5">
<option py:for="vers in vlist">${vers[0]}</option>
</select>
<select name="diffs" multiple="multiple" size="5">
<option py:for="diff in dlist">${diff[0]}</option>
</select>
<input type="submit" value="Submit" />
</form>
</body>
</html>

