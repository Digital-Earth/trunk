#!/bin/bash

CRASHDUMPURL=http://search.pyxis.worldview.gallery:9200/pyxdiagnostics/crash-dumps
TMPFILE=wves-tmp-crash-dumps.txt

curl -XGET "$CRASHDUMPURL/_count" > $TMPFILE 2> /dev/null
sed 's/[\":,]/ /g' $TMPFILE | awk '{print "Crash dumps (" $3 ")"}' 
echo "=================="

TODAY=`date +%H:%M:%S`
YESTERDAY=`date -d "yesterday $TODAY" '+%Y-%m-%dT%H\:%M\:%SZ'`
curl -XGET "$CRASHDUMPURL/_search?q=Uploaded:>=$YESTERDAY&size=50&pretty" > $TMPFILE 2> /dev/null
sed -n '/_source\" \: /, /}/ p' $TMPFILE | grep ': "' | sed 's/  //g; s/\"Product/{"Product/g; s/\(\"Up.*$\)/\1}/g;' | tr -d '\n' | sed 's/\}/}\n/g'

rm -f $TMPFILE 
