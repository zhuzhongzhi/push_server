#!/bin/sh
WORK_BIN_DIR=${WORK_DIR}/bin
Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
echo "[${Current_Time}] monitor start...." >> ${WORK_DIR}/log/stop.log
while [ 1 ]
do
    result=`ps -ef|grep push_server|grep -v grep|grep $USER|grep -v vi|awk '{print $2}'`
    if [ ! "$result" ]
    then
    		confirm=`ps -ef|grep push_server|grep -v grep|grep $USER|grep -v vi|awk '{print $2}'`
    		if [ ! "$confirm" ]
    		then
    			  Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
        	  echo "[${Current_Time}] check push_server quit, now restart push_server ..." >> ${WORK_DIR}/log/stop.log
        	  ${WORK_BIN_DIR}/push_server&
        else
        	  Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
        		echo "[${Current_Time}] push_server pid is "$result >> ${WORK_DIR}/log/stop.log
        fi
    else
    	 Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
       echo "[${Current_Time}] push_server pid is "$result >> ${WORK_DIR}/log/stop.log
    fi
    sleep 5
done
Current_Time=`date +"%Y-%m-%d %H:%M:%S.%N"`
echo "[${Current_Time}] monitor end...." >> ${WORK_DIR}/log/stop.log
