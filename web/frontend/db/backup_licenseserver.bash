#!/bin/bash
dbuser=admin
dbpass=Innovation1
backupDir=/backup/mongo
backupName=pyxis_licenseserver
timeStamp=`date "+%m-%d-%y_%H-%M-%S"`
stampedBackupName=${backupName}_$timeStamp
mongodump -u $dbuser -p $dbpass -o $backupDir/$stampedBackupName
