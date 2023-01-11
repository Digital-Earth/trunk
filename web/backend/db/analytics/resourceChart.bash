#!/bin/bash
ROOTDIR=/home/Pyxis/scripts/analytics
SERVER=wvdbs01r01vm02
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
DAY=`date +%D | sed 's%/%-%g'`

PLOTS=plots

BASHSCRIPT=$ROOTDIR/resourcePlot.bash
MONGOSCRIPT=$ROOTDIR/resourceChart.js
CHART=resourceDocSizeChart
NUMBINS=50

DATA=$ROOTDIR/$PLOTS/$CHART.csv
RESOURCEIMAGE=$ROOTDIR/$PLOTS/$CHART.png
PLOTTITLE="Resource Doc Sizes"
PLOTDIMS="1024,768"
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var chartType='DocSize', bins=$NUMBINS" $MONGOSCRIPT > $DATA
bash $BASHSCRIPT $DATA $NUMBINS $RESOURCEIMAGE "$PLOTTITLE" $PLOTDIMS
