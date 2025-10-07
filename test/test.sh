#!/bin/bash

exec_file="../jsonfs"
example_file="example.json"
mount_point="mnt"

if [ ! -f "$exec_file" ] 
then
	echo "Not found $exec_file"
	exit
fi

if [ ! -f "$example_file" ]
then
	echo Not found $example_file
	exit
fi

mkdir -p "$mount_point"
"$exec_file" "$example_file" "$mount_point"

cd "$mount_point"
ls -Rl 

sleep 1

cd .. 
fusermount3 -u "$mount_point"

exit
