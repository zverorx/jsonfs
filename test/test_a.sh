#!/bin/bash

# This script is designed for testing jsonfs. 
# Outputs attributes of files and directories from the mount point.
set -e

test_dir="$(cd $(dirname $BASH_SOURCE[0]) && pwd)"
exec_file="$test_dir/../bin/jsonfs"
json_file="$PWD/$1"
mount_point="$test_dir/mnt"

if [ ! -f "$exec_file" ] ; then
	echo "Error: not found $exec_file" >&2
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
	echo "Error: mount failure" >&2
	exit 1
	else cd "$mount_point"
fi

trap 'cd $test_dir ;                \
     sync ;                         \
     fusermount3 -u $mount_point ;  \
     rmdir $mount_point' ERR EXIT   

########## TEST ##########

echo "=== JSON file ==="
cat "$json_file"
echo
echo "=== Attributes ==="
ls -Rla

exit 0
