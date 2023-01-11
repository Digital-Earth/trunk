#!/bin/bash

HOST=http://localhost:9200
INDEX=pyxis_licenseserver_v
CONNECTOR_INDEX=mongodb_meta
WRITE_ALIAS=pyxis_licenseserver
READ_ALIAS=pyxis_licenseserver_read

curl -XDELETE "$HOST/$INDEX*"
curl -XDELETE "$HOST/$CONNECTOR_INDEX"
