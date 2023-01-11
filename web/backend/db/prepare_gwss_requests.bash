#!/bin/bash
dbUser=pyxis_licensing
dbPass=Innovation1
mongo pyxis_licenseserver -u $dbUser -p $dbPass /home/Pyxis/scripts/mongo/gwssController.js
