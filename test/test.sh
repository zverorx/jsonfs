#!/bin/bash

exec_file="../jsonfs"
example_file="example.json"
mount_point="mnt"

if [ ! -f "$exec_file" ] 
then
	echo "Not found $exec_file"
	exit 1
fi

if [ ! -f "$example_file" ]
then
	echo Not found $example_file
	exit 1
fi

mkdir -p "$mount_point"
"$exec_file" "$example_file" "$mount_point"

cd "$mount_point"
ls -Rl

echo 
echo "**************************************************"
echo

for file in $(find)
do
	if [ -f "$file" ] ; then
		echo -n "$file: "
		cat "$file"
		echo
	fi
done

sleep 1

cd .. 
fusermount3 -u "$mount_point"

exit 0
