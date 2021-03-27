#!/bin/sh
SERVICE='eve-server'

while true
	do
	if ps ax | grep -v grep | grep $SERVICE > /dev/null
	then
		echo "$SERVICE service running, everything is fine"
	else
		echo "$SERVICE is not running"
		#cd /srv/games/eve/cruc/bin/
		./eve-server
	fi
	sleep 30			# This will set the restart interval.
	done
