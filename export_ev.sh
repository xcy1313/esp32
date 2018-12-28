#!/bin/bash

if [ x$1 != x ]
then
	export TEMPLATE=$1
	echo -e "[environment variable]:TEMPLATE=$TEMPLATE"
else
	echo "Please input one param value, 0 or 1, 1 for enable vesync template !"
fi
