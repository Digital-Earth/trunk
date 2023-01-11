#!/bin/bash
ROOTDIR=/home/Pyxis/scripts/analytics
SERVER=wvdbs01r01vm02
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
DAY=`date +%D | sed 's%/%-%g'`
ANALYTICS=$ROOTDIR/analytics-$DAY.txt

#separate emails by commas
TO="lrakai@pyxisinnovation.com,shatzi@pyxisinnovation.com,nhamekasi@pyxisinnovation.com,rtaylor@pyxisinnovation.com,yklymko@pyxisinnovation.com,myoung@pyxisinnovation.com,clay@utl.io,dean@utl.io"
#TO="lrakai@pyxisinnovation.com"
SUBJECT="[LS-Monitoring] wvdbs01r01vm02 Daily Status"
HTML=$ROOTDIR/$DAY.html
HTMLHEAD="<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\">
<head>
<title>$SERVER Day Ending $DAY</title>
</head><body><h2>PYXIS Analytics for $SERVER</h2>"
HTMLTAIL="<h5>You are receiving this because you are a member of [LS-Monitoring]. <a href=\"mailto:lrakai@pyxisinnovation.com?subject=[LS-Monitoring]Unsubscribe\">Unsubscribe</a> if you would like to stop receiving these emails.</h5></body></html>"

STARTTIME=`date +%s%N`
mongo --quiet --port $PORT -u admin -p Innovation1 admin $ROOTDIR/adminAnalytics.js > $ANALYTICS
mongo --quiet --port $PORT -u $USER -p $PASS $DB $ROOTDIR/analytics.js >> $ANALYTICS
echo -en "\n" >> $ANALYTICS
bash $ROOTDIR/esVitals.bash >> $ANALYTICS
echo -en "\n" >> $ANALYTICS
bash $ROOTDIR/esCrashDumps.bash >> $ANALYTICS
sed -i 's/$/<br \/>/g' $ANALYTICS
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

echo $HTMLHEAD > $HTML
echo "Generating this report took ${ELAPSED}s.\n\n" >> $HTML
cat $ANALYTICS >> $HTML
echo $HTMLTAIL >> $HTML
sed -i 's/\\n/<br \/>/g' $HTML

PLOTS=plots

BASHSCRIPT=$ROOTDIR/resourcePlot.bash
MONGOSCRIPT=$ROOTDIR/resourceChart.js
PLOTDIMS="1024,768"
CHART=resourceChart
NUMDAYS=8

DATA=$ROOTDIR/$PLOTS/$CHART.csv
RESOURCEIMAGE=$ROOTDIR/$PLOTS/$CHART.png
PLOTTITLE="Resources Created"
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var chartType='Resource', bins=$NUMDAYS" $MONGOSCRIPT > $DATA
bash $BASHSCRIPT $DATA $NUMDAYS $RESOURCEIMAGE "$PLOTTITLE" $PLOTDIMS

CHART=resourceUpdateChart

DATA=$ROOTDIR/$PLOTS/$CHART.csv
RESOURCEUPDATEIMAGE=$ROOTDIR/$PLOTS/$CHART.png
PLOTTITLE="Resource Updates"
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var chartType='Updates', bins=$NUMDAYS" $MONGOSCRIPT > $DATA
bash $BASHSCRIPT $DATA $NUMDAYS $RESOURCEUPDATEIMAGE "$PLOTTITLE" $PLOTDIMS

CHART=resourceDocSizeChart
NUMBINS=50

DATA=$ROOTDIR/$PLOTS/$CHART.csv
RESOURCESIZEIMAGE=$ROOTDIR/$PLOTS/$CHART.png
PLOTTITLE="Resource Doc Sizes"
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var chartType='DocSize', bins=$NUMBINS" $MONGOSCRIPT > $DATA
bash $BASHSCRIPT $DATA $NUMBINS $RESOURCESIZEIMAGE "$PLOTTITLE" $PLOTDIMS

CHART=resourceArchiveDocSizeChart
NUMBINS=50

DATA=$ROOTDIR/$PLOTS/$CHART.csv
RESOURCEARCHIVESIZEIMAGE=$ROOTDIR/$PLOTS/$CHART.png
PLOTTITLE="Resource Archive Doc Sizes"
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var chartType='DocSizeArchive', bins=$NUMBINS" $MONGOSCRIPT > $DATA
bash $BASHSCRIPT $DATA $NUMBINS $RESOURCEARCHIVESIZEIMAGE "$PLOTTITLE" $PLOTDIMS

mutt -e "set content_type=text/html" -s "$SUBJECT" -a $RESOURCEIMAGE -a $RESOURCEUPDATEIMAGE -a $RESOURCESIZEIMAGE -a $RESOURCEARCHIVESIZEIMAGE -- $TO < $HTML
rm $HTML $ANALYTICS
