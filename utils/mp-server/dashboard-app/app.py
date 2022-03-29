import sys
import logging
import math
import json

from dash import Dash, dcc, html
from dash.dependencies import Input, Output
import plotly.express as px
import plotly.graph_objects as go
import pandas as pd
import mariadb

logging.basicConfig(
    filename="server_dashboard.log", 
    level=logging.INFO, 
    format="[%(asctime)s] -%(name)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

def query_database():
    # Connect to MariaDB Platform
    with open('credentials.json') as file:
        credentials = json.load(file)

    try:
        conn = mariadb.connect(
            user=credentials['user'],
            password=credentials['password'],
            host=credentials['host'],
            port=credentials['port'],
            database=credentials['database']
        )
    except mariadb.Error as e:
        logger.error(f"Error connecting to MariaDB Platform: {e}")
        sys.exit(1)

    # Get Cursor
    cur = conn.cursor()

    try:
        cur.execute(
            """
            SELECT 
                INSTANCE_VERSION,
                OOS,
                RELOAD,
                OBSERVERS,
                PASSWORD,
                PUBLIC,
                START_TIME,
                END_TIME,
                TIMEDIFF(END_TIME, START_TIME)
            FROM tmp_game_info
            """
        )
    except mariadb.Error as e:
        logger.error(f"Error: {e}")

    df = (
        pd.DataFrame(cur)
        .applymap(lambda x: x[0] if type(x) is bytes else x)
    )
    df.columns = [field[0] for field in cur.description]
    df['TIMEDIFF_MINUTES'] = df["TIMEDIFF(END_TIME, START_TIME)"].apply(lambda x: x.total_seconds()/60)
    df['log_TIMEDIFF_MINUTES'] = df['TIMEDIFF_MINUTES'].apply(lambda x: math.log(x))

    try:
        cur.execute("SELECT MIN(START_TIME), MAX(START_TIME) FROM tmp_game_info;")
    except mariadb.Error as e:
        logger.error(f"Error: {e}")
    min_date, max_date = cur.fetchone()

    conn.close()
    return df, min_date, max_date

def generate_figures(filtered_df):
    exclude_list = ['TIMEDIFF(END_TIME, START_TIME)', 'START_TIME', 'END_TIME', 'TIMEDIFF_MINUTES', 'log_TIMEDIFF_MINUTES']

    def make_donut(field_name):
        view = (
            pd.DataFrame(filtered_df[field_name].value_counts())
            .assign(color=lambda x: x.index.map({0: px.colors.qualitative.Plotly[0], 1:px.colors.qualitative.Plotly[1]}))
        )
        fig = (
            go.Figure(data=[go.Pie(labels=view.index, values=view[field_name], hole=.8, title=field_name)])
            .update_layout(
                margin=dict(l=5, r=5, t=0, b=0),
                showlegend=False,
                height=250,
                paper_bgcolor='rgba(0,0,0,0)',
            )
            .update_traces(marker=dict(colors=view.color))
        )
        return fig

    figures = [make_donut(column) for column in filtered_df.columns if column not in exclude_list]
    fig = px.histogram(
        filtered_df,
        x="TIMEDIFF_MINUTES",
        title="Game Durations",
        labels={'TIMEDIFF_MINUTES':'duration(minutes)'},
        histnorm='percent'
    )
    fig.update_layout(paper_bgcolor='rgba(0,0,0,0)')
    figures.append(fig)
    return figures

def prepare_layout():
    df, min_date, max_date = query_database()
    layout = html.Div(children=[
        html.H1(children='Wesnoth Multiplayer Server Dashboard', style={
            'text-align': 'center', 
            # 'font-family': '"Times New Roman", Montaga,Junicode, serif'
            }),
        html.H3(children="Filter by Date"),
        html.Div(dcc.DatePickerRange(
            id='date-picker',
            min_date_allowed=min_date,
            max_date_allowed=max_date,
            initial_visible_month=max_date,
            start_date=min_date,
            end_date=max_date
            ), style={'float': 'inline-start'}),
        html.H3(children="Filter by Values"),
        html.Div(children=[
            html.Div(children=[
                html.Label('INSTANCE_VERSION'),
                dcc.Checklist(df.INSTANCE_VERSION.unique(), df.INSTANCE_VERSION.unique(), id='checklist0')
            ]),
            html.Div(children=[
                html.Label('OOS'),
                dcc.Checklist(df.OOS.unique(), df.OOS.unique(), id='checklist1')
            ]),
            html.Div(children=[
                html.Label('RELOAD'),
                dcc.Checklist(df.RELOAD.unique(), df.RELOAD.unique(), id='checklist2')
            ]),
            html.Div(children=[
                html.Label('OBSERVERS'),
                dcc.Checklist(df.OBSERVERS.unique(), df.OBSERVERS.unique(), id='checklist3')
            ]),
            html.Div(children=[
                html.Label('PASSWORD'),
                dcc.Checklist(df.PASSWORD.unique(), df.PASSWORD.unique(), id='checklist4')
            ]),
            html.Div(children=[
                html.Label('PUBLIC'),
                dcc.Checklist(df.PUBLIC.unique(), df.PUBLIC.unique(), id='checklist5')
            ])
        ], style={'columnCount': 6}),
        html.H3(children="Filter by Game Duration(minutes)"),
        dcc.RangeSlider(
            min=0,
            max=df['log_TIMEDIFF_MINUTES'].max(),
            # max=math.log(130), # leaving this in but commented so that it can be demonstrated that with log transforming, the slider looks good with any range of values
            step=None,
            marks={
                0: '0',
                math.log(5): '5',
                math.log(10): '10',
                math.log(15): '15',
                math.log(20): '20',
                math.log(25): '25',
                math.log(30): '30',
                math.log(40): '40',
                math.log(50): '50',
                math.log(60): '60',
                math.log(70): '70',
                math.log(80): '80',
                math.log(90): '90',
                math.log(100): '100',
                df['log_TIMEDIFF_MINUTES'].max(): str(df['TIMEDIFF_MINUTES'].max())
                # math.log(130): '130',  # leaving this in but commented so that it can be demonstrated that with log transforming, the slider looks good with any range of values
            },
            value=[0, df['log_TIMEDIFF_MINUTES'].max()],
            id='slider0'
        ),
        html.Em('Tip: Mouse over the figures to reveal tooltips showing labels and additional information. The equivalent of mouseover on mobile is a long tap.'),
        html.Div(children=[
            dcc.Graph(id='graph0'),
            dcc.Graph(id='graph1'),
            dcc.Graph(id='graph2'),
            dcc.Graph(id='graph3'),
            dcc.Graph(id='graph4'),
            dcc.Graph(id='graph5'),
        ], style={'columnCount': 6, 'clear': 'both'}),
        html.Em('Tip: Note the additional context menu that appears at the top right upon mouseover. You can click and drag to zoom in on the figure. Reset Axes in the context menu returns you to the original zoom level.'),
        html.Div(dcc.Graph(id='graph6'))
    ], style={'backgroundColor': '#f3ead8'})
    return layout


app = Dash(__name__)
app.title = "Wesnoth Server Dashboard"
app.layout = prepare_layout


@app.callback(
    output=[
        Output('graph0', 'figure'),
        Output('graph1', 'figure'),
        Output('graph2', 'figure'),
        Output('graph3', 'figure'),
        Output('graph4', 'figure'),
        Output('graph5', 'figure'),
        Output('graph6', 'figure')
    ],
    inputs=[
        Input('date-picker', 'start_date'),
        Input('date-picker', 'end_date'),
        Input('checklist0', 'value'),
        Input('checklist1', 'value'),
        Input('checklist2', 'value'),
        Input('checklist3', 'value'),
        Input('checklist4', 'value'),
        Input('checklist5', 'value'),
        Input('slider0', 'value')
    ]
)
def update_figures(start_date, end_date, checklist0, checklist1, checklist2, checklist3, checklist4, checklist5, slider0):
    for each in [checklist0, checklist1, checklist2, checklist3, checklist4, checklist5]:
        if each is None:
            return []
    df, min_date, max_date = query_database()
    filtered_df = df[df['START_TIME'].between(start_date, end_date)]
    filtered_df = filtered_df[filtered_df['INSTANCE_VERSION'].isin(checklist0)]
    filtered_df = filtered_df[filtered_df['OOS'].isin(checklist1)]
    filtered_df = filtered_df[filtered_df['RELOAD'].isin(checklist2)]
    filtered_df = filtered_df[filtered_df['OBSERVERS'].isin(checklist3)]
    filtered_df = filtered_df[filtered_df['PASSWORD'].isin(checklist4)]
    filtered_df = filtered_df[filtered_df['PUBLIC'].isin(checklist5)]
    filtered_df = filtered_df[filtered_df['log_TIMEDIFF_MINUTES'].between(*slider0)]
    figures = generate_figures(filtered_df)
    return figures


if __name__ == '__main__':
    app.run_server(debug=True)

