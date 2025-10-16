#!/bin/bash

# This script is designed for testing jsonfs. 
# The test displays file and directory names and their attributes and contents.

set -e

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$1"
mount_point="$test_dir/mnt"

if [ -z "$json_file" ]
then
	json_file="$test_dir/ex_obj.json"
fi

if [ ! -f "$exec_file" ] 
then
	echo "Not found $exec_file" >&2
	exit 1
fi

if [ ! -f "$json_file" ]
then
	echo "Not found $json_file" >&2
	exit 1
fi

########## Mounting ##########
mkdir -p "$mount_point"
if ! "$exec_file" "$json_file" "$mount_point" ; then
	rmdir "$mount_point"
	echo "Failed to mount $exec_file" >&2
	exit 1
fi


########## Test start ##########

echo -e "\e[31m**************************************************\e[0m"
echo "$json_file:"
cat "$json_file"
echo -e "\e[31m**************************************************\e[0m"
cd "$mount_point"
echo 
echo -e "\e[32m**************************************************\e[0m"
ls -Rla
echo -e "\e[32m**************************************************\e[0m"
echo
echo -e "\e[34m**************************************************\e[0m"

for file in $(find)
do
	if [ -f "$file" ] ; then
		echo -n "$file: "
		cat "$file"
		echo
	fi
done
echo -e "\e[34m**************************************************\e[0m"

########## Test end ##########


########## Unmounting ##########
cd ..
fusermount3 -u "$mount_point"
rmdir "$mount_point"

exit 0
