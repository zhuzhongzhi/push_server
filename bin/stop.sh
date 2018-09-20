#!/bin/sh

Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
echo "[${Current_Time}] stop push server begin...." >> ${WORK_DIR}/log/stop.log
result=`ps -ef|grep push_server|grep -v grep|grep -v vi|awk '{print $2}'`
if [ ! "$result" ]
then
		Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
  	echo "[${Current_Time}] check push_server quit, do nothing ..." >> ${WORK_DIR}/log/stop.log
else
    Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
	  echo "[${Current_Time}] push_server pid is $result, now kill it." >> ${WORK_DIR}/log/stop.log
	  kill -9 $result
	 
	  confirm=`ps -ef|grep push_server|grep -v grep|grep -v vi|awk '{print $2}'`
	  if [ ! "$confirm" ]
	  then
				Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
  	    echo "[${Current_Time}] push_server[$result] killed." >> ${WORK_DIR}/log/stop.log
    else
  	   Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
  		 echo "[${Current_Time}] push_server still running pid is $confirm, try kill it again." >> ${WORK_DIR}/log/stop.log
  		 kill -9 $confirm
    fi
fi
echo "stop push server end...." >> ${WORK_DIR}/log/stop.log
