#!/bin/bash

# 用于定期封存日志的压缩文件名
dd=`date +"%Y%m%d" --date '2 days ago'`

cd /data/myhub/log

# 定期将日志文件 archive
mkdir $dd
find . -maxdepth 1 -type f -name "*${dd}*" -exec mv {} ${dd} \;
tar -jcf ${dd}.tar.bz2 ${dd}

# 定期将旧的 archive 文件删除
rm -rf ${dd}
find . -maxdepth 1 -type f -name "*.tar.bz2" -mtime +7 -exec rm -f {} \;