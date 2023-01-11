#!/bin/bash
# mongo-connector    mongo-connector to elasticsearch service
# chkconfig: 2345 90 10
# description: mongo-connector to elasticsearch service
# processname: mongo-connector

DAEMON_PATH="/home/Pyxis/mongo-connector"

COMMIT_INTERVAL=60 #auto-commit
DAEMON="/usr/local/bin/mongo-connector"
DAEMONOPTS="-m mongodb://admin:Innovation1@localhost:27018 -t http://search.pyxis.worldview.gallery:9200 --namespace-set pyxis_licenseserver.Resources --auto-commit-interval=$COMMIT_INTERVAL -d elastic2_doc_manager"

NAME=mongo-connector
DESC="mongo-connector to elasticsearch service"
PIDFILE=/var/run/$NAME.pid

case "$1" in
start)
	printf "%-50s" "Starting $NAME..."
	cd $DAEMON_PATH
	PID=`$DAEMON $DAEMONOPTS > /dev/null 2>&1 & echo $!`
	#echo "Saving PID" $PID " to " $PIDFILE
        if [ -z $PID ]; then
            printf "%s\n" "Fail"
        else
            echo $PID > $PIDFILE
            printf "%s\n" "Ok"
        fi
;;
status)
        printf "%-50s" "Checking $NAME..."
        if [ -f $PIDFILE ]; then
            PID=`cat $PIDFILE`
            if [ -z "`ps axf | grep ${PID} | grep -v grep`" ]; then
                printf "%s\n" "Process dead but pidfile exists"
            else
                echo "Running"
            fi
        else
            printf "%s\n" "Service not running"
        fi
;;
stop)
        printf "%-50s" "Stopping $NAME"
            PID=`cat $PIDFILE`
            cd $DAEMON_PATH
        if [ -f $PIDFILE ]; then
            kill $PID
            printf "%s\n" "Ok"
            rm -f $PIDFILE
        else
            printf "%s\n" "pidfile not found"
        fi
;;

restart)
  	$0 stop
  	$0 start
;;

*)
        echo "Usage: $0 {status|start|stop|restart}"
        exit 1
esac
