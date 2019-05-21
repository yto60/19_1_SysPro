#!/bin/sh
for file in `ls t/test* | sort`; do
    echo "*******************************"
    sh test-cmp.sh "${file##*/}"
done
echo "*******************************"
