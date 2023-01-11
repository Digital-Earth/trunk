#!/bin/bash
backupDir=/home/pyxis/backup
backupPrefix=pyxis_licenseserver_
olderThanDays=30
find $backupDir -name "$backupPrefix*" -type f -mtime +$olderThanDays -exec rm {} \;
