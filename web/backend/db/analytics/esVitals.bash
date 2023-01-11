#!/bin/bash

URL=http://search.pyxis.worldview.gallery:9200
TMPFILE=wves`date +%F`.txt
curl -XGET "$URL/_cat/indices/pyxis_licenseserver*" > $TMPFILE 2> /dev/null
awk '{print "Elasticsearch index " $3 " status: " $2 ", health: " $1 ", size: " $8", docs: " $6 ", deleted: " $7}' $TMPFILE
rm -f $TMPFILE
