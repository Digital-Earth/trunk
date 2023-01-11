#!/bin/bash
backupDir=/backup/mongo
backupPrefix=pyxis_licenseserver_
olderThanDays=30
find $backupDir/* -type d -mtime +$olderThanDays -exec rm -rf {} \;
