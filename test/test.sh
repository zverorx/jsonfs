#!/bin/bash

# This script is designed for testing the file system. 
# It must be run from the directory in which it is located, as paths are relative. 
# The test displays the names of files and directories in the file system, as well as their contents.

exec_file="../bin/jsonfs"
example_file="example.json"
mount_point="mnt"

if [ ! -f "$exec_file" ] 
then
	echo "Not found $exec_file" >&2
	exit 1
fi

if [ ! -f "$example_file" ]
then
	echo "Not found $example_file" >&2
	exit 1
fi

########## Mounting ##########

mkdir -p "$mount_point"
if ! "$exec_file" "$example_file" "$mount_point" ; then
	rmdir "$mount_point"
	echo "Failed to mount $exec_file" >&2
	exit 1
fi


########## Test start ##########

echo -e "\e[31m**************************************************\e[0m"
echo "$example_file:"
cat "$example_file"
cd "$mount_point"
echo -e "\e[31m**************************************************\e[0m"
echo 
echo -e "\e[32m**************************************************\e[0m"
ls -Rl
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
