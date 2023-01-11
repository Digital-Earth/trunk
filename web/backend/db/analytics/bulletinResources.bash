#!/bin/bash
ROOTDIR=/home/Pyxis/scripts/analytics
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
GALLERIES=$ROOTDIR/bulletin-galleries.csv
GEOSOURCES=$ROOTDIR/bulletin-geosources.csv
MAPS=$ROOTDIR/bulletin-maps.csv
ACCOUNTS=$ROOTDIR/bulletin-accounts.csv
BULLETINTXT=$ROOTDIR/bulletin.txt
BULLETINCSV=$ROOTDIR/bulletin.csv
HTML=$ROOTDIR/bulletin.html
DAY=`date +%D | sed 's%/%-%g'`
DAYS=7

#separate emails by commas
TO="lrakai@pyxisinnovation.com,gclarke@pyxisinnovation.com"
#TO="lrakai@pyxisinnovation.com"
SUBJECT="[LS-Bulletin] Weekly Bulletin"
HTMLHEAD="<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">
<html xmlns=\"http://www.w3.org/1999/xhtml\">
<head>
<title>Day Ending $DAY</title>
</head><body><h2>LS Weekly Bulletin</h2>"
HTMLTAIL="<h5>You are receiving this because you are a member of [LS-Bulletin]. <a href=\"mailto:lrakai@pyxisinnovation.com?subject=[LS-Bulletin]Unsubscribe\">Unsubscribe</a> if you would like to stop receiving these emails.</h5></body></html>"

MONGOSCRIPT=$ROOTDIR/bulletinStats.js
STARTTIME=`date +%s%N`
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var format='txt', days=$DAYS" $MONGOSCRIPT > $BULLETINTXT
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var format='csv', days=$DAYS" $MONGOSCRIPT > $BULLETINCSV

MONGOSCRIPT=$ROOTDIR/bulletinResources.js
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var type='Gallery', days=$DAYS" $MONGOSCRIPT > $GALLERIES
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var type='GeoSource', days=$DAYS" $MONGOSCRIPT > $GEOSOURCES
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var type='Map', days=$DAYS" $MONGOSCRIPT > $MAPS
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

MONGOSCRIPT=$ROOTDIR/bulletinAccounts.js
mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var days=$DAYS" $MONGOSCRIPT > $ACCOUNTS
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

sed -i 's/$/<br \/>/g' $BULLETINTXT

echo $HTMLHEAD > $HTML
echo "Generating this report took ${ELAPSED}s.\n\n" >> $HTML
cat $BULLETINTXT >> $HTML
echo $HTMLTAIL >> $HTML
sed -i 's/\\n/<br \/>/g' $HTML

mutt -e "set content_type=text/html" -s "$SUBJECT" -a $BULLETINCSV -a $GALLERIES -a $GEOSOURCES -a $MAPS -a $ACCOUNTS -- $TO < $HTML

rm $BULLETINTXT $BULLETINCSV $GALLERIES $GEOSOURCES $MAPS $ACCOUNTS $HTML
