#!/bin/bash

while true; do
	PID=`ps aux | grep /usr/sbin/isi_audit_d | grep -v grep | awk '{print $2}'`
	if [[ "" !=  "$PID" ]]; then
		echo "killing $PID"
		kill $PID
	fi
	sleep 5
	$(/usr/sbin/isi_audit_d -d)
	sleep 5
done
