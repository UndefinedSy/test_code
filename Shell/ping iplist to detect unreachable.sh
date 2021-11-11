#!/bin/bash

IP_LIST="1.2.3.4"
while true; do
    for IP in $IP_LIST; do
        FAIL_COUNT=0
        for ((i=1;i<=3;i++)); do
            if timeout 0.5 ping -W 0.5 -c 1 -i 0.5 $IP >/dev/null; then
                break
            else
                echo "failure++"
                let FAIL_COUNT++
            fi
        done
        if [ $FAIL_COUNT -eq 3 ]; then
            time=$(date "+%Y-%m-%d %H:%M:%S")
            echo -e "failed to ping ${IP} on ${time}.\n"
            echo -e "failed to ping ${IP} on ${time}.\n" >> ping_result
        fi
    done
    sleep 5
done


#!/bin/bash
while true; do
    echo -e "sar -u 1 1"
    echo -e "top -b -n 1 | grep myhub"
done