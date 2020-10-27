#!/bin/zsh

me=`basename "$0"`

filename=${me}_count.txt

echo > ${filename}

(( count = 0 ))

while true; do

    # 记录跑了多少次
    echo $count >> ${filename}

    # 开始抓log
    isi_for_array tail -f -1 /var/log/isi_audit_syslog.log > isi_audit_syslog_merge_${count}.log &
    tail -f -1 nohup.out > nohup_${count}.out &

    # 跑case
    nosetests -vs /usr/local_qa/bin/audit/integrationTests/test_audit_syslog.py:ProtocolEventTests.test_forward_on_non_system_zone --debug=test_forward_on_non_system_zone

    # 结果如何
    if [ "$?" = 0 ]; then
        # 强制跑一次就退出，用来检查抓取的log是否充足，正确
        if [ $1 = "e" ]; then
            isi_for_array pkill tail
            exit
        else
        # 成功，删除log
            rm isi_audit_syslog_merge_${count}.log
            rm nohup_${count}.out
        fi
    else
        # 失败，退出
        isi_for_array pkill tail
        exit
    fi

    isi_for_array pkill tail

    (( count = $count + 1 ))

done
