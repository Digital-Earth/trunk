#!/bin/bash
ROOTDIR=/home/Pyxis/scripts/analytics
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
DAY=`date +%D | sed 's%/%-%g'`
RESOURCES=$ROOTDIR/resources-$DAY.csv

STARTTIME=`date +%s%N`
mongo --quiet --port $PORT -u $USER -p $PASS $DB $ROOTDIR/resourceReport.js > $RESOURCES
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

echo "Generating this report took ${ELAPSED}s." 
