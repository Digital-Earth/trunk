#!/bin/bash
dbuser=admin
dbpass=Innovation1
backupDir=/backup/mongo
restoreDir=`ls -dt $backupDir/* | head -1`
echo "mongorestore -u $dbuser -p $dbpass $restoreDir"
