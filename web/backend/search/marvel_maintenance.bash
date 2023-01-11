#!/bin/bash

SINCEDAYS=7
HOST=http://localhost:9200

DATE=`date --date="$SINCEDAYS days ago" +%Y.%m.%d`
INDEX=.marvel-$DATE

curl -XDELETE "$HOST/$INDEX"
