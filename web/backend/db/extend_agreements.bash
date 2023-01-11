#!/bin/bash

ROOTDIR=/home/Pyxis/scripts/mongo
USER=pyxis_licensing
PASS=Innovation1
PORT=27017
DB=pyxis_licenseserver
MONGOSCRIPT=$ROOTDIR/extendAgreements.js

EXTENDTO=`date +%Y-%m-%dT00:00:00Z --date="next month"` #extend until next month

mongo --quiet --port $PORT -u $USER -p $PASS $DB --eval "var extendTo=\"$EXTENDTO\"" $MONGOSCRIPT
