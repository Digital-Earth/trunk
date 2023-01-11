#!/bin/bash

# ==[ contains ] ==============================================================
# Function to test a string is in a given array
#
# contains "${Array[@]}" "string"
function contains() {
    local n=$#
    local value=${!n}
    for ((i=1; i < $n; i++)) {
        if [ "${!i}" == "${value}" ]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

# ==[ printSlack ]=============================================================
# Function to send output from the commandline to Slack.
#
# @parameter string $LEVEL   INFO/ERROR/WARNING message. Changes emoji
# @parameter string $MESSAGE Message to send to slack. 
printSlack()
{
  SLACK_URL=${SLACK_URL-'https://hooks.slack.com/services/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'};
  SLACK_CHANNEL=${SLACK_CHANNEL-'#alerts'};
  SLACK_BOTNAME=${SLACK_BOTNAME-$(hostname -s)};
  SLACK_BOTEMOJI=${SLACK_BOTEMOJI-':computer:'}

  SEVERITY=${1-'INFO'};
  ICON=':slack:';

  case "$SEVERITY" in
    INFO)
      ICON=':page_with_curl:';
      shift;
      ;;
    WARN|WARNING)
      ICON=':warning:';
      shift;
      ;;
    ERROR|ERR)
      ICON=':bangbang:';
      shift;
      ;;
    *)
      ICON=':slack:';
      ;;
  esac

  MESSAGE=$@;

  PAYLOAD="payload={\"channel\": \"${SLACK_CHANNEL}\", \"username\": \"${SLACK_BOTNAME}\", \"text\": \"${ICON} ${MESSAGE}\", \"icon_emoji\": \"${SLACK_BOTEMOJI}\"}";
#  CURL_RESULT=$(curl -s -S -X POST --data-urlencode "$PAYLOAD" ${SLACK_URL});
echo $PAYLOAD
  if [ -z "$CURL_RESULT" ]; then
    return 0;
  else
    return 1;
  fi

}

ROOTDIR=/home/Pyxis/scripts/analytics
SERVER=wvdbs01r01vm02
USER=pyxis_licensing
PASS=Innovation1
DB=pyxis_licenseserver
PORT=27018
ALERTS=$ROOTDIR/alerts.dat
OLDEXT=old
MOREINFO="https://goo.gl/uijqor" # "https://api.pyxis.pyxisglobe.com/api/v1/Gwss?$$select=Name"

STARTTIME=`date +%s%N`
mv $ALERTS $ALERTS.$OLDEXT
mongo --quiet --port $PORT -u $USER -p $PASS $DB $ROOTDIR/alerts.js >> $ALERTS
echo -en "\n" >> $ALERTS
ENDTIME=`date +%s%N`
ELAPSED=`echo "scale=2; ($ENDTIME - $STARTTIME)/1000000000" | bc -l`

servers=( `cat $ALERTS` )
oldServers=( `cat $ALERTS.$OLDEXT` )
icon='INFO'
message=

for server in ${servers[@]}; do
	if [[ $(contains "${oldServers[@]}" "$server") != "y" ]]; then
		message+="${server/-/ } is up, "
	fi
done

for server in ${oldServers[@]}; do
        if [[ $(contains "${servers[@]}" "$server") != "y" ]]; then
                message+="${server/-/ } is down, "
		icon='WARN'
        fi
done

if [[ ${#message} -gt 0 ]]; then
	message+=" (${#servers[@]} servers currently up: $MOREINFO)"
	printSlack "$icon" "$message"
fi
