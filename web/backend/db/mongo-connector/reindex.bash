#!/bin/bash

ROOTDIR=/home/Pyxis/mongo-connector
PASS=Innovation1
SERVICE=mongo-connector
STAMP=oplog.timestamp
NEW_STAMP=$ROOTDIR/$STAMP
OLD_STAMP=$ROOTDIR/$STAMP.old

echo $PASS | sudo -S service $SERVICE stop
mv -f $NEW_STAMP $OLD_STAMP
echo $PASS | sudo -S service $SERVICE start
