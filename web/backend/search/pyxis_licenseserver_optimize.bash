#!/bin/bash

HOST=http://localhost:9200
INDEX=pyxis_licenseserver_read

curl -XPOST "$HOST/$INDEX/_optimize?only_expunge_deletes=1"
