# Wesnoth-multiplayer-sql-dashboard
 A dashboard that visualizes the data from a SQL database that contains Wesnoth's multiplayer server statistics.

### Setup
Please fill in the required SQL database credentials and connection in **credentials.json** before launching the app.

### Third Party Python Packages Used
The framework used to develop the app is called Plotly. Under Plotly, there are several sublibraries. **Plotly Dash** is the library for creating the app layout and runtime, and it runs using **Flask**; **Plotly Express** is the library for creating figures; and **Plotly Graph Object** is a more advanced version of Plotly Express that allows for deeper customization of figures.

The ever popular **Pandas** library is what the app uses to filter and manipulate the data after it has been queried.

**Mariadb** is a Python library that allows you to write SQL queries to a MariaDB database inside a Python app.

### Data Fetching Trigger
For updating dashboard data, the solution implemented is documented [here](https://dash.plotly.com/live-updates) in the "Updates on Page Load" section. The data is fetched when the layout is served on every page load. Additionally, the data is also fetched during callbacks, which are triggered whenever a filter is toggled.

### Logging
Upon first run a log file called `server_dashboard.log` is generated in this directory. It will contain all log messages for all log importance levels.

### Status of the App
Currently, only a single table, `tmp_game_info`, has been visualized as a dashboard.






