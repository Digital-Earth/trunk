#!/bin/bash 
find /home/pyxis/webapps/htdocs/data/logging/logFiles/ -type f -mtime +30 -exec rm {} \;
