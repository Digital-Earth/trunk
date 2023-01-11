#!/bin/bash
dbuser=admin
dbpass=Innovation1
backupDir=/backup/mongo
backupName=pyxis_licenseserver
port=27017
timeStamp=`date "+%m-%d-%y_%H-%M-%S"`
stampedBackupName=${backupName}_$timeStamp
# include --oplog for a true point in time snapshot (include --oplogreplay in mongorestore if included)
mongodump --port $port -u $dbuser -p $dbpass -o $backupDir/$stampedBackupName
tar -zcf $backupDir/$stampedBackupName.tgz $backupDir/$stampedBackupName
remoteUser=pyxis
remotePass=cb29d532
remoteUrl=web344.webfaction.com
remoteDir=/home/$remoteUser/backup
sshpass -p$remotePass scp $backupDir/$stampedBackupName.tgz $remoteUser@$remoteUrl:$remoteDir/
rm $backupDir/$stampedBackupName.tgz
