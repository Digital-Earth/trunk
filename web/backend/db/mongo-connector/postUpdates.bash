#!/bin/bash

ROOTDIR=/home/Pyxis/mongo-connector
PASS=Innovation1
SERVICE=mongo-connector
STAMP=oplog.timestamp
NEW_STAMP=$ROOTDIR/$STAMP
OLD_STAMP=$ROOTDIR/$STAMP.old
TMP_STAMP=$ROOTDIR/$STAMP.tmp

echo $PASS | sudo -S service $SERVICE stop
cp -f $NEW_STAMP $TMP_STAMP
mv -f $OLD_STAMP $NEW_STAMP
mv -f $TMP_STAMP $OLD_STAMP
echo $PASS | sudo -S service $SERVICE start
rm -f $TMP_STAMP
