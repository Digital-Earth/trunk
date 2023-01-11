#!/bin/bash

HOST=http://localhost:9200
INDEX=pyxis_licenseserver_v3
WRITE_ALIAS=pyxis_licenseserver
READ_ALIAS=pyxis_licenseserver_read
MAPPING=mapping/Resources_v3.json

curl -XPUT "$HOST/$INDEX" -d "`cat $MAPPING`"
curl -XPUT "$HOST/$INDEX/_alias/$WRITE_ALIAS"
curl -XPUT "$HOST/$INDEX/_alias/$READ_ALIAS"
