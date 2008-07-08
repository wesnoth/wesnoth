require 'googlechart/chart'

def chart_bar(hash = {})

    # generate final image
    GoogleChartLib::Chart.bar hash
end