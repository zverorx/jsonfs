#!/bin/bash

# This script is designed for testing jsonfs. 
# The test displays file and directory names and their attributes and contents.

set -e

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$PWD/$1"
mount_point="$test_dir/mnt"

red_is_enabled=false
green_is_enabled=false
blue_is_enabled=false

options="$2"
if [ -z "$options" ] ; then
	red_is_enabled=true
	green_is_enabled=true
	blue_is_enabled=true
fi

if echo "$options" | egrep -q r ; then red_is_enabled=true ; fi
if echo "$options" | egrep -q g ; then green_is_enabled=true ; fi
if echo "$options" | egrep -q b ; then blue_is_enabled=true ; fi

if [ ! -f "$exec_file" ] ; then
	echo "Not found $exec_file" >&2
	exit 1
fi

if [ ! -f "$json_file" ] ; then
	echo "Error: JSON file not found" >&2
	exit 1
fi

########## Mounting ##########
mkdir -p "$mount_point"
if ! "$exec_file" "$json_file" "$mount_point" ; then
	rmdir "$mount_point"
	echo "Failed to mount $exec_file" >&2
	exit 1
	else cd "$mount_point"
fi

trap 'cd $test_dir ; sync ; fusermount3 -u $mount_point ; rmdir $mount_point' ERR EXIT

########## Test start ##########
if [ "$red_is_enabled" == true ] ; then
	echo -e "\e[31m**************************************************\e[0m" # red
	echo "$json_file:"
	cat "$json_file"
	echo -e "\e[31m**************************************************\e[0m" # red
	echo 
fi

if [ "$green_is_enabled" == true ] ; then
	echo -e "\e[32m**************************************************\e[0m" # green
	ls -Rla
	echo -e "\e[32m**************************************************\e[0m" # green
	echo
fi

if [ "$blue_is_enabled" == true ] ; then
	echo -e "\e[34m**************************************************\e[0m" # blue

	for file in $(find)
	do
		if [ -f "$file" ] ; then
			echo -n "$file: "
			cat "$file"
			echo
		fi
	done
	echo -e "\e[34m**************************************************\e[0m" #blue
fi
########## Test end ##########

exit 0
