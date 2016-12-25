#!/bin/bash
appli=`netstat -anpe |grep $1 |grep LISTEN |cut -d "/" -f 2`
kill -9 `pidof $appli`
