#/bin/bash

./mk-castle.sh -b castle-bg -c cutout-bg.png -o castle -t castle-walls.tmpl castle.png
./mk-castle.sh -b castle-fg -c cutout-fg.png -o castle -t castle-walls.tmpl castle.png

./mk-castle.sh -b keep-bg -c cutout-bg.png -o castle -t keep1.tmpl keep1.png
./mk-castle.sh -b keep-fg -c cutout-fg.png -o castle -t keep1.tmpl keep1.png

./mk-castle.sh -b keep-bg -c cutout-bg.png -o castle -t keep2.tmpl keep2.png
./mk-castle.sh -b keep-fg -c cutout-fg.png -o castle -t keep2.tmpl keep2.png

./mk-castle.sh -b encampment-bg -c cutout-bg.png -o castle -t castle-walls.tmpl encampment.png
./mk-castle.sh -b encampment-fg -c cutout-fg.png -o castle -t castle-walls.tmpl encampment.png


