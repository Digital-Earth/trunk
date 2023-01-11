#!/bin/bash

SINCEDAYS=60
HOST=http://localhost:9200

DATE=`date --date="$SINCEDAYS days ago" +%Y.%m.%d`
INDEX=tweets-$DATE

curl -XPOST "$HOST/$INDEX/_flush"
curl -XPOST "$HOST/$INDEX/_close"
