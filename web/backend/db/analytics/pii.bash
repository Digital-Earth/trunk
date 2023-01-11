#!/bin/bash
ROOTDIR=/home/Pyxis/scripts/analytics
GDRIVE=/home/Pyxis/bin/gdrive
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
PARENTID=0B32J1Y6LianmfkNpaV9aWDhCRmptVU5fTFZDdXZacldxN2pJRklrcTVPR3pBNFV4Z3Zqd1k # PYXIS Shared Documents > True Fans > Growth Hacking > Google Analytics

PII=$ROOTDIR/PII.csv
MONGOSCRIPT=$ROOTDIR/getPIICSV.js

cd $ROOTDIR
STARTTIME=`date +%s%N`
mongo --quiet --port $PORT -u $USER -p $PASS $DB $MONGOSCRIPT > $PII
OLDPIIIDS=(`$GDRIVE list -q "'$PARENTID' in parents" | grep PII.csv | awk '{print $1}'`)
if [[ ${#OLDPIIIDS[@]} -gt 0 ]]; then
	for i in `seq 0 $((${#OLDPIIIDS[@]} - 1))`; do
		$GDRIVE delete --id ${OLDPIIIDS[$i]} > /dev/null
	done
fi
$GDRIVE upload --parent $PARENTID --file $PII > /dev/null
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

rm $PII
